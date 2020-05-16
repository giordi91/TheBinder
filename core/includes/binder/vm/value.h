#pragma once

#include "stdio.h"

namespace binder {
namespace log {
class Log;
}
namespace vm {

enum VALUE_TYPE {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
};

struct Value {
  VALUE_TYPE type;

  union {
    bool boolean;
    double number;
  } as;
};

inline Value makeBool(bool value) {
  return Value{VALUE_TYPE::VAL_BOOL, {.boolean = value}};
};
inline Value makeNumber(double value) {
  return Value{VALUE_TYPE::VAL_NUMBER, {.number= value}};
};
inline Value makeNIL() {
  return Value{VALUE_TYPE::VAL_NUMBER, {.number= 0}};
};

inline bool valueAsBool(Value value){return value.as.boolean;}
inline double valueAsNumber(Value value){return value.as.number;}

inline bool isValueBool(Value value){return value.type == VALUE_TYPE::VAL_BOOL;}
inline bool isValueNumber(Value value){return value.type == VALUE_TYPE::VAL_NUMBER;}
inline bool isValueNIL(Value value){return value.type == VALUE_TYPE::VAL_NIL;}

void printValue(Value value, log::Log *logger);
} // namespace vm
} // namespace binder
