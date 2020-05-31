#include "binder/log/log.h"
#include "binder/vm/memory.h"
#include "binder/vm/object.h"
#include "binder/vm/value.h"

namespace binder ::vm {

sObj *ALLOCATIONS = nullptr;

#define ALLOCATE_OBJ(type, objectType)                                         \
  (type *)allocateObject(sizeof(type), objectType);

sObj *allocateObject(const size_t size, const OBJ_TYPE type) {
  sObj *object = static_cast<sObj*>(reallocate(nullptr, 0, size));
  if (ALLOCATIONS == nullptr) {
    ALLOCATIONS = object;
    object->next = nullptr;
  } else {
    object->next = ALLOCATIONS;
    ALLOCATIONS = object;
  }

  object->type = type;
  return object;
}

void freeObject(sObj *object) {
  switch (object->type) {
  case OBJ_TYPE::OBJ_STRING: {
    //string is interned, meaning won't be freed here but from the vm
    FREE(sObjString, object);
    break;
  }
  }
}

void freeAllocations() {
  if (ALLOCATIONS == nullptr) {

    return;
  }
  sObj *obj = ALLOCATIONS;
  while (obj != nullptr) {
    sObj *next = obj->next;
    freeObject(obj);
    obj = next;
  }
  ALLOCATIONS = nullptr;
}

sObjString *allocateString(const char *chars, int length) {
  sObjString *string = ALLOCATE_OBJ(sObjString, OBJ_TYPE::OBJ_STRING);
  string->length = length;
  //TODO here we move the cast away mostly because we use the re-alloc workflow,
  //need to clean this up
  string->chars = const_cast<char*>(chars);
  return string;
}

sObjString *takeString(char *chars, const int length) {
  // here we take ownership of the chars, they have been already copied to
  // memory we can own
  return allocateString(chars, length);
}

sObjString *copyString(const char *chars, int length) {
  char *heapChars = ALLOCATE(char, length + 1);
  // we don't know exactly where this string comes from and
  // if is nullterminated, so we set it manually;
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';
  return allocateString(heapChars, length);
}

void printObject(Value *value, log::Log *logger) {
  switch (getObjType(*value)) {
  case OBJ_TYPE::OBJ_STRING:
    log::LOG(logger, "%s", valueAsCString(*value));
    break;
  }
}

} // namespace binder::vm
