#include "binder/log/log.h"
#include "binder/vm/object.h"
#include "binder/vm/value.h"

namespace binder ::vm {

//void *reallocate(void *previous, size_t oldSize, size_t newSize) {
void *reallocate(void *previous, size_t , size_t newSize) {
  if (newSize == 0) {
    free(previous);
    return NULL;
  }

  return realloc(previous, newSize);
}

sObj *allocateObject(size_t size, OBJ_TYPE type) {
  sObj *object = (sObj *)reallocate(nullptr, 0, size);
  object->type = type;
  return object;
}

#define ALLOCATE(type, count)                                                  \
  (type *)reallocate(nullptr, 0, sizeof(type) * (count))
#define ALLOCATE_OBJ(type, objectType)                                         \
  (type *)allocateObject(sizeof(type), objectType)

sObjString *allocateString(char *chars, int length) {
  sObjString *string = ALLOCATE_OBJ(sObjString, OBJ_TYPE::OBJ_STRING);
  string->length = length;
  string->chars = chars;
  return string;
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
