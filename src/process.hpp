#pragma once

#define WIN32_LEAN_AND_MEAN
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x600
#include <windows.h>

#include <string>
#include <vector>

namespace pe {

class Process {
public:
  Process();
  Process(const std::string &process_name, const std::string &window_name);
  ~Process();

  /* management */
  bool OpenFromProcessName(const std::string &process_name);
  bool OpenFromWindowName(const std::string &window_name);
  bool OpenFromProcessID(const DWORD &process_id);

  DWORD_PTR GetModuleBaseAddress(const std::string &module_name);

  bool Close();

  /* basic read/write process memory wrappers */
  template <typename T> T ProtectedRead(DWORD_PTR src_address);

  /* num_to_read is number of T to read, not bytes */
  template <typename T>
  std::vector<T> ProtectedRead(DWORD_PTR src_address, size_t num_to_read);

  template <typename T>
  bool ProtectedRead(DWORD_PTR src_address, T *dest, size_t num_to_read);

  template <typename T> T Read(DWORD_PTR src_address);

  /* num_to_read is number of T to read, not bytes */
  template <typename T>
  std::vector<T> Read(DWORD_PTR src_address, size_t num_to_read);

  template <typename T>
  bool Read(DWORD_PTR src_address, T *dest, size_t num_to_read);

  template <typename T>
  bool ProtectedWrite(DWORD_PTR dest_address, const T &value);

  template <typename T>
  bool ProtectedWrite(DWORD_PTR dest_address, const std::vector<T> &data);

  template <typename T>
  bool ProtectedWrite(DWORD_PTR dest_address, T *data, size_t num_to_write);

  template <typename T> bool Write(DWORD_PTR dest_address, const T &value);

  template <typename T>
  bool Write(DWORD_PTR dest_address, const std::vector<T> &data);

  template <typename T>
  bool Write(DWORD_PTR dest_address, T *data, size_t num_to_write);

  /* injection */
  bool InjectDll(const std::string &dll_path);

  /* getters */
  const std::string &GetWindowName() const { return window_name_; }

  const DWORD &GetProcessID() const { return process_id_; }

  const std::string &GetProcessName() const { return process_name_; }

  const HWND &GetWindowHandle() const { return window_handle_; }

private:
  bool HandleError();
  bool WindowNameFromProcessID();

  std::string process_name_;
  std::string window_name_;

  DWORD process_id_;
  HANDLE process_handle_;
  HWND window_handle_;

  DWORD win_error_;
  bool open_handle_;
};

/* template function defs */
template <typename T> T Process::ProtectedRead(DWORD_PTR src_address) {
  T buf;
  void *src = reinterpret_cast<void *>(src_address);

  DWORD old_protect;
  VirtualProtectEx(process_handle_, src, sizeof(T), PAGE_EXECUTE_READWRITE,
                   &old_protect);
  if (!ReadProcessMemory(process_handle_, src, &buf, sizeof(T), nullptr)) {
    VirtualProtectEx(process_handle_, src, sizeof(T), old_protect, nullptr);
    return this->HandleError();
  }

  VirtualProtectEx(process_handle_, src, sizeof(T), old_protect, nullptr);
  return buf;
}

template <typename T>
std::vector<T> Process::ProtectedRead(DWORD_PTR src_address,
                                      size_t num_to_read) {
  std::vector<T> buf(num_to_read);
  void *src = reinterpret_cast<void *>(src_address);

  DWORD old_protect;
  VirtualProtectEx(process_handle_, src, sizeof(T) * buf.capacity(),
                   PAGE_EXECUTE_READ, &old_protect);

  if (!ReadProcessMemory(process_handle_, src, buf.data(),
                         buf.capacity() * sizeof(T), nullptr)) {
    VirtualProtectEx(process_handle_, src, sizeof(T), old_protect, nullptr);
    this->HandleError();
    return std::vector<T>(0);
  }

  VirtualProtectEx(process_handle_, src, sizeof(T) * buf.capacity(),
                   old_protect, nullptr);
  return buf;
}

template <typename T>
bool Process::ProtectedRead(DWORD_PTR src_address, T *dest,
                            size_t num_to_read) {
  void *src = reinterpret_cast<void *>(src_address);

  DWORD old_protect;
  VirtualProtectEx(process_handle_, src, num_to_read * sizeof(T),
                   PAGE_EXECUTE_READWRITE, &old_protect);

  if (!ReadProcessMemory(process_handle_, src, dest, num_to_read * sizeof(T),
                         nullptr)) {
    VirtualProtectEx(process_handle_, src, num_to_read * sizeof(T), old_protect,
                     nullptr);
    return this->HandleError();
  }

  VirtualProtectEx(process_handle_, src, num_to_read * sizeof(T), old_protect,
                   nullptr);
  return true;
}

template <typename T> T Process::Read(DWORD_PTR src_address) {
  T buf;
  void *src = reinterpret_cast<void *>(src_address);

  if (!ReadProcessMemory(process_handle_, src, &buf, sizeof(T), nullptr))
    return this->HandleError();

  return buf;
}

template <typename T>
std::vector<T> Process::Read(DWORD_PTR src_address, size_t num_to_read) {
  std::vector<T> buf(num_to_read);
  void *src = reinterpret_cast<void *>(src_address);

  if (!ReadProcessMemory(process_handle_, src, buf.data(),
                         buf.capacity() * sizeof(T), nullptr)) {
    this->HandleError();
    return std::vector<T>(0);
  }

  return buf;
}

template <typename T>
bool Process::Read(DWORD_PTR src_address, T *dest, size_t num_to_read) {
  void *src = reinterpret_cast<void *>(src_address);

  if (!ReadProcessMemory(process_handle_, src, dest, num_to_read * sizeof(T),
                         nullptr))
    return this->HandleError();

  return true;
}

template <typename T>
bool Process::ProtectedWrite(DWORD_PTR dest_address, const T &value) {
  void *dest = reinterpret_cast<void *>(dest_address);

  DWORD old_protect;
  VirtualProtectEx(process_handle_, dest, sizeof(T), PAGE_EXECUTE_READWRITE,
                   &old_protect);

  if (!WriteProcessMemory(process_handle_, dest, &value, sizeof(T), nullptr)) {
    VirtualProtectEx(process_handle_, dest, sizeof(T), old_protect, nullptr);
    return this->HandleError();
  }
  VirtualProtectEx(process_handle_, dest, sizeof(T), old_protect, nullptr);
  return true;
}

template <typename T>
bool Process::ProtectedWrite(DWORD_PTR dest_address,
                             const std::vector<T> &data) {
  void *dest = reinterpret_cast<void *>(dest_address);

  DWORD old_protect;
  VirtualProtectEx(process_handle_, dest, data.size() * sizeof(T),
                   PAGE_EXECUTE_READWRITE, &old_protect);

  if (!WriteProcessMemory(process_handle_, dest, data.data(),
                          sizeof(T) * data.size(), nullptr)) {
    VirtualProtectEx(process_handle_, dest, data.size() * sizeof(T),
                     old_protect, nullptr);
    return this->HandleError();
  }

  VirtualProtectEx(process_handle_, dest, data.size() * sizeof(T), old_protect,
                   nullptr);
  return true;
}

template <typename T>
bool Process::ProtectedWrite(DWORD_PTR dest_address, T *data,
                             size_t num_to_write) {
  void *dest = reinterpret_cast<void *>(dest_address);

  DWORD old_protect;
  VirtualProtectEx(process_handle_, dest, num_to_write * sizeof(T),
                   PAGE_EXECUTE_READWRITE, &old_protect);

  if (!WriteProcessMemory(process_handle_, dest, data, sizeof(T) * num_to_write,
                          nullptr)) {
    VirtualProtectEx(process_handle_, dest, num_to_write * sizeof(T),
                     old_protect, nullptr);
    return this->HandleError();
  }

  VirtualProtectEx(process_handle_, dest, num_to_write * sizeof(T), old_protect,
                   nullptr);
  return true;
}

template <typename T>
bool Process::Write(DWORD_PTR dest_address, const T &value) {
  void *dest = reinterpret_cast<void *>(dest_address);

  if (!WriteProcessMemory(process_handle_, dest, &value, sizeof(T), nullptr))
    return this->HandleError();

  return true;
}

template <typename T>
bool Process::Write(DWORD_PTR dest_address, const std::vector<T> &data) {
  void *dest = reinterpret_cast<void *>(dest_address);

  if (!WriteProcessMemory(process_handle_, dest, data.data(),
                          sizeof(T) * data.size(), nullptr))
    return this->HandleError();

  return true;
}

template <typename T>
bool Process::Write(DWORD_PTR dest_address, T *data, size_t num_to_write) {
  void *dest = reinterpret_cast<void *>(dest_address);

  if (!WriteProcessMemory(process_handle_, dest, data, sizeof(T) * num_to_write,
                          nullptr))
    return this->HandleError();

  return true;
}

}; // namespace pe
