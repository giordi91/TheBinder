#include "binder/log/log.h"
#include "binder/vm/memory.h"
#include "binder/vm/object.h"
#include "binder/vm/value.h"

namespace binder ::vm {

sObj *allocations = nullptr;

#define ALLOCATE_OBJ(type, objectType)                                         \
  (type *)allocateObject(sizeof(type), objectType);

sObj *allocateObject(size_t size, OBJ_TYPE type) {
  sObj *object = (sObj *)reallocate(nullptr, 0, size);
  if (allocations == nullptr) {
    allocations = object;
    object->next = nullptr;
  } else {
    object->next = allocations;
    allocations = object;
  }

  object->type = type;
  return object;
}

void freeObject(sObj *object) {
  switch (object->type) {
  case OBJ_TYPE::OBJ_STRING: {
    sObjString *str = (sObjString *)object;
    //string is interned, meaning won't be freed here but from the vm
    FREE(sObjString, object);
    break;
  }
  }
}

void freeAllocations() {
  if (allocations == nullptr) {

    return;
  }
  sObj *obj = allocations;
  while (obj != nullptr) {
    sObj *next = obj->next;
    freeObject(obj);
    obj = next;
  }
  allocations = nullptr;
}

sObjString *allocateString(const char *chars, int length) {
  sObjString *string = ALLOCATE_OBJ(sObjString, OBJ_TYPE::OBJ_STRING);
  string->length = length;
  //TODO here we move the cast away mostly because we use the re-alloc workflow,
  //need to clean this up
  string->chars = (char*)chars;
  return string;
}

sObjString *takeString(char *chars, int length) {
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
