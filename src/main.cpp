#include "game_types.hpp"
#include "process.hpp"
#include <format>
#include <iostream>
#include <map>
#include <vector>

std::vector<bo3::objFileInfo_t> get_obj_file_info(pe::Process &proc);

std::vector<bo3::ScrVar_t> get_variable_list(pe::Process &proc);

void find_stack_variables(pe::Process &proc,
                          const std::vector<bo3::ScrVar_t> &variable_list,
                          const std::vector<bo3::objFileInfo_t> &obj_file_info);

uint64_t
find_file_from_pos(pe::Process &proc,
                   const std::vector<bo3::objFileInfo_t> &obj_file_info,
                   DWORD_PTR pos);

uint32_t find_line(pe::Process &proc, const bo3::objFileInfo_t &obj_file_info,
                   DWORD_PTR pos);

void print_files(pe::Process &proc,
                 const std::vector<bo3::objFileInfo_t> &obj_file_info);

void print_vars(const std::vector<bo3::ScrVar_t> &variable_list);

int main() {

  pe::Process proc("boiii.exe", "");
  auto obj_file_info = get_obj_file_info(proc);
  // print_files(proc, obj_file_info);

  auto variable_list = get_variable_list(proc);
  // print_vars(variable_list);

  find_stack_variables(proc, variable_list, obj_file_info);

  return 0;
}

std::vector<bo3::objFileInfo_t> get_obj_file_info(pe::Process &proc) {
  DWORD_PTR base = proc.GetModuleBaseAddress("BlackOps3.exe");
  DWORD_PTR ptr_obj_file_info_count = base + bo3::ptrObjFileInfoCount;
  DWORD_PTR ptr_obj_file_info = base + bo3::ptrObjFileInfo;

  int obj_file_info_count = proc.Read<int>(ptr_obj_file_info_count);

  return proc.Read<bo3::objFileInfo_t>(ptr_obj_file_info, obj_file_info_count);
}

std::vector<bo3::ScrVar_t> get_variable_list(pe::Process &proc) {
  DWORD_PTR base = proc.GetModuleBaseAddress("BlackOps3.exe");
  DWORD_PTR ptr_scr_var_glob = base + bo3::ptrScrVarGlob;
  auto scr_var_glob = proc.Read<bo3::ScrVarGlob_t>(ptr_scr_var_glob, 2);
  DWORD_PTR ptr_variable_list = (DWORD_PTR)scr_var_glob[0].scriptVariables;
  std::cout << ptr_variable_list << std::endl;

  return proc.Read<bo3::ScrVar_t>(ptr_variable_list, 130000);
}

void find_stack_variables(
    pe::Process &proc, const std::vector<bo3::ScrVar_t> &variable_list,
    const std::vector<bo3::objFileInfo_t> &obj_file_info) {
  std::map<uint64_t, uint64_t> thread_usage_count;
  std::map<uint64_t, std::vector<uint32_t>> line_info;

  for (const auto &var : variable_list) {
    if (var.value.type != bo3::VAR_STACK) {
      continue;
    }

    if (var.value.u.stackValue == nullptr) {
      continue;
    }

    auto current_stack_buffer = proc.Read<bo3::ScrVarStackBuffer_t>(
        (DWORD_PTR)var.value.u.stackValue, 1);

    auto code_pos_values = std::vector<DWORD_PTR>(0);

    if (current_stack_buffer[0].size) {
      auto buffer = proc.Read<bo3::InternalStackBufferVar_t>(
          (DWORD_PTR)var.value.u.stackValue +
              offsetof(bo3::ScrVarStackBuffer_t, buf),
          current_stack_buffer[0].size / 9);

      for (const auto &stack_var : buffer) {
        if (stack_var.type == bo3::VAR_CODEPOS) {
          code_pos_values.push_back((DWORD_PTR)(stack_var.value.codePosValue));
        }
      }
    }

    code_pos_values.push_back((DWORD_PTR)(current_stack_buffer[0].pos));

    for (const auto code_pos : code_pos_values) {
      uint64_t file_index = find_file_from_pos(proc, obj_file_info, code_pos);
      if (file_index == 0xffffffffffffffff) {
        continue;
      }
      thread_usage_count[file_index]++;

      if (obj_file_info[file_index].debugInfo.lineStartAddrCount != 0) {
        auto line_number = find_line(proc, obj_file_info[file_index], code_pos);
        if (line_number == 0xffffffff) {
          continue;
        }

        line_info[file_index].push_back(line_number);
      };
    }
  }

  for (const auto &[file_index, usage_count] : thread_usage_count) {
    std::vector filename_vector = proc.Read<char>(
        (DWORD_PTR)obj_file_info[file_index].debugInfo.filename, 50);
    std::cout << std::format(
        "{} has {} threads\n",
        std::string(
            filename_vector.begin(),
            std::find(filename_vector.begin(), filename_vector.end(), '\0')),
        usage_count);

    if (line_info.contains(file_index)) {
      for (const auto &line_number : line_info[file_index]) {
        std::cout << std::format("\t@ line {}\n", line_number);
      }
    }
  }
}

uint64_t
find_file_from_pos(pe::Process &proc,
                   const std::vector<bo3::objFileInfo_t> &obj_file_info,
                   DWORD_PTR pos) {

  for (size_t i = 0; i < obj_file_info.size(); i++) {
    const auto &obj_file = obj_file_info[i];
    DWORD_PTR gsc_obj_base = (DWORD_PTR)obj_file.activeVersion;

    bo3::GSC_OBJ gsc_obj;
    if (!proc.Read<bo3::GSC_OBJ>(gsc_obj_base, &gsc_obj, 1)) {
      continue;
    }

    if (pos >= gsc_obj_base + gsc_obj.cseg_offset &&
        pos < gsc_obj_base + gsc_obj.cseg_offset + gsc_obj.cseg_size) {
      return i;
    }
  }

  return 0xffffffffffffffff;
}

uint32_t find_line(pe::Process &proc, const bo3::objFileInfo_t &obj_file_info,
                   DWORD_PTR pos) {
  auto line_start_address_list =
      proc.Read<uint64_t>((DWORD_PTR)obj_file_info.debugInfo.lineStartAddr,
                          obj_file_info.debugInfo.lineStartAddrCount);

  for (size_t i = 0; i < line_start_address_list.size(); i++) {
    if (pos < line_start_address_list[i]) {
      return (i > 0) ? i - 1 : 0;
    }
  }

  return 0xffffffff;
}

void print_files(pe::Process &proc,
                 const std::vector<bo3::objFileInfo_t> &obj_file_info) {
  for (const auto &info : obj_file_info) {
    // Read filename
    std::vector filename_vector =
        proc.Read<char>((DWORD_PTR)info.debugInfo.filename, 50);

    std::cout << std::format(
        "{}:{}\n", std::string(filename_vector.begin(), filename_vector.end()),
        info.debugInfo.lineStartAddrCount);
  }
}

void print_vars(const std::vector<bo3::ScrVar_t> &variable_list) {
  for (const auto &var : variable_list) {
    std::cout << var.o.size << std::endl;
  }
}
