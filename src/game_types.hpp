#include <cstdint>

namespace bo3 {

struct GSC_OBJ {
  char magic[8];
  uint32_t source_crc;
  uint32_t include_offset;
  uint32_t animtree_offset;
  uint32_t cseg_offset;
  uint32_t stringtablefixup_offset;
  uint32_t field6_0x1c;
  uint32_t field7_0x20;
  uint32_t field8_0x24;
  uint32_t field9_0x28;
  uint32_t field10_0x2c;
  uint32_t cseg_size;
  uint32_t field12_0x34;
  uint16_t field13_0x38;
  uint16_t field14_0x3a;
  uint16_t field15_0x3c;
  uint16_t field16_0x3e;
  uint16_t field17_0x40;
  uint16_t field18_0x42;
  uint8_t field19_0x44;
  uint8_t field20_0x45;
  uint8_t field21_0x46;
  uint8_t field22_0x47;
};

struct debugFileInfo_t {
  char *filename;         // 0x0
  void *startAddr;        // 0x8
  void *endAddr;          // 0x10
  void **lineStartAddr;   // 0x18
  int lineStartAddrCount; // 0x20
  int __field5_0x24;      // 0x24
  char *source;           // 0x28
  int sourceLen;          // 0x30
  int __field8_0x34;      // 0x34
  void *gdb;              // 0x38
};

static_assert(sizeof(debugFileInfo_t) == 0x40);

struct objFileInfo_t {
  void *activeVersion;
  void *baselineVersion;
  debugFileInfo_t debugInfo;
};

static_assert(sizeof(objFileInfo_t) == 0x50);

union EntRefUnion {
  uint64_t val;
};

typedef uint32_t ScrString_t;

typedef uint64_t ScrVarNameIndex_t;

typedef uint32_t ScrVarIndex_t;

enum ScrVarType_t {
  VAR_UNDEFINED = 0,
  VAR_POINTER = 1,
  VAR_STRING = 2,
  VAR_ISTRING = 3,
  VAR_VECTOR = 4,
  VAR_HASH = 5,
  VAR_FLOAT = 6,
  VAR_INTEGER = 7,
  VAR_UINT64 = 8,
  VAR_UINTPTR = 9,
  VAR_ENTITYOFFSET = 10,
  VAR_CODEPOS = 11,
  VAR_PRECODEPOS = 12,
  VAR_APIFUNCTION = 13,
  VAR_FUNCTION = 14,
  VAR_STACK = 15,
  VAR_ANIMATION = 16,
  VAR_THREAD = 17,
  VAR_NOTIFYTHREAD = 18,
  VAR_TIMETHREAD = 19,
  VAR_CHILDTHREAD = 20,
  VAR_CLASS = 21,
  VAR_STRUCT = 22,
  VAR_REMOVEDENTITY = 23,
  VAR_ENTITY = 24,
  VAR_ARRAY = 25,
  VAR_REMOVEDTHREAD = 26,
  VAR_FREE = 27,
  VAR_THREADLIST = 28,
  VAR_ENTLIST = 29,
  VAR_COUNT = 30
} __attribute__((packed));
static_assert(sizeof(ScrVarType_t) == 0x1);

struct ScrVarStackBuffer_t {
  void *pos;
  uint8_t __f2_padding[0x4];
  uint16_t size;
  uint16_t bufLen;
  ScrVarIndex_t threadId;
  uint8_t buf[1];
};

struct ScrVarChildPair_t {
  ScrVarIndex_t firstChild;
  ScrVarIndex_t lastChild;
};

union ScrVarValueUnion_t {
  int64_t intValue;
  int32_t hashValue;
  uint64_t uintptrValue;
  float floatValue;
  ScrString_t stringValue;
  float *vectorValue;
  void *codePosValue;
  ScrVarIndex_t pointerValue;
  ScrVarStackBuffer_t *stackValue;
  ScrVarChildPair_t childPair;
};
static_assert(sizeof(ScrVarValueUnion_t) == 0x8);

union ScrVarObjectInfo_t {
  uint64_t object_o;
  uint32_t size;
  EntRefUnion entRefUnion;
  ScrVarIndex_t nextEntId;
  ScrVarIndex_t self;
  ScrVarIndex_t free;
};

struct ScrVarEntityInfo_t {
  uint16_t classNum;
  uint16_t clientNum;
};

union ScrVarObjectW_t {
  uint32_t object_w;
  ScrVarEntityInfo_t varEntityInfo;
  ScrVarIndex_t stackId;
};

struct ScrVarValue_t {
  ScrVarValueUnion_t u;
  ScrVarType_t type;
  uint8_t field2_0x9;
  uint8_t field3_0xa;
  uint8_t field4_0xb;
  uint32_t pad;
} __attribute__((aligned(8)));
static_assert(sizeof(ScrVarValue_t) == 0x10);

struct ScrVarRuntimeInfo_t {
  uint32_t nameType : 3;
  uint32_t flags : 5;
  uint32_t refCount : 24;
};
static_assert(sizeof(ScrVarRuntimeInfo_t) == 0x4);

struct ScrVar_t {
  ScrVarValue_t value;
  ScrVarRuntimeInfo_t info;
  uint8_t field2_0x14;
  uint8_t field3_0x15;
  uint8_t field4_0x16;
  uint8_t field5_0x17;
  ScrVarObjectInfo_t o;
  ScrVarObjectW_t field7_0x20;
  uint8_t field8_0x24;
  uint8_t field9_0x25;
  uint8_t field10_0x26;
  uint8_t field11_0x27;
  ScrVarNameIndex_t nameIndex;
  ScrVarIndex_t nextSibling;
  ScrVarIndex_t prevSibling;
  ScrVarIndex_t parentId;
  ScrVarIndex_t nameSearchHashList;
};
static_assert(sizeof(ScrVar_t) == 0x40);

struct ScrVarGlob_t {
  ScrVarIndex_t *scriptNameSearchHashList;
  uint8_t __padding[0x78];
  ScrVar_t *scriptVariables;
  uint8_t __padding2[0x78];
};
static_assert(sizeof(ScrVarGlob_t) == 0x100);

struct InternalStackBufferVar_t {
  ScrVarType_t type;
  ScrVarValueUnion_t value;
} __attribute__((packed));

static_assert(sizeof(InternalStackBufferVar_t) == 0x9);

/* ADDRESS */

static constexpr uint64_t ptrObjFileInfoCount = 0x50efb60;
static constexpr uint64_t ptrObjFileInfo = 0x50dc2e0;
static constexpr uint64_t ptrScrVarGlob = 0x51a3500;

}; // namespace bo3
