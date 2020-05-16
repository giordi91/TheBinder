#include "binder/log/log.h"
#include "binder/vm/value.h"

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
  }
}

} // namespace binder::vm
