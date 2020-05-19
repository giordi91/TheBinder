#pragma once
#include "stdlib.h"
#include "string.h"

namespace binder {

namespace log {

class Log;
}

namespace vm {

struct Value;

enum class OBJ_TYPE { OBJ_STRING };

struct sObj {
  OBJ_TYPE type;
  sObj* next;
};

struct sObjString {
  sObj obj;
  int length;
  char *chars;
};

extern sObj* allocations;

void freeAllocations();
sObjString *takeString(char* chars, int length);
sObjString *copyString(const char *chars, int length);
void printObject(Value *value, log::Log* logger);

} // namespace vm
} // namespace binder
