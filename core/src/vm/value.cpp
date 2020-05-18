#include "binder/log/log.h"
#include "binder/vm/value.h"
#include "binder/vm/object.h"

namespace binder::vm {

void printValue(Value value, log::Log *logger) {
  char valueBuffer[64];
  switch (value.type) {
  case VALUE_TYPE::VAL_NUMBER: {
    sprintf(valueBuffer, "%g", valueAsNumber(value));
    logger->print(valueBuffer);
    break;
  }
  case VALUE_TYPE::VAL_BOOL: {
    sprintf(valueBuffer, "%s", valueAsBool(value) ? "true" : "false");
    logger->print(valueBuffer);
    break;
  }
  case VALUE_TYPE::VAL_NIL: {
    sprintf(valueBuffer, "nil");
    logger->print(valueBuffer);
    break;
  }
  case VALUE_TYPE::VAL_OBJ: {
    printObject(&value,logger);
    break;
  }
  }
}

bool valuesEqual(Value a, Value b) {
  if (a.type != b.type)
    return false;

  switch (a.type) {
  case VALUE_TYPE::VAL_BOOL:
    return valueAsBool(a) == valueAsBool(b);
  case VALUE_TYPE::VAL_NIL:
    return true;
  case VALUE_TYPE::VAL_NUMBER:
    return valueAsNumber(a) == valueAsNumber(b);
  case VALUE_TYPE::VAL_OBJ: {
    // temporary string cast, we need to check proper type
    ObjString *aString = valueAsString(a);
    ObjString *bString = valueAsString(b);
    return aString->length == bString->length &&
           memcmp(aString->chars, bString->chars, aString->length) == 0;
  }
  default: // unreacheable
    assert(0);
    return false;
  }
}

} // namespace binder::vm
