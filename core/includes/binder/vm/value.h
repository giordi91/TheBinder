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
  // NOTE designated initializer are C99 but also then acceptted in C++20
  // would make code a bit better but is it worth adding the dependency to
  // c++20?
  Value outValue{};
  outValue.type = VALUE_TYPE::VAL_BOOL;
  outValue.as.boolean = value;
  return outValue;
};
inline Value makeNumber(double value) {
  Value outValue{};
  outValue.type = VALUE_TYPE::VAL_NUMBER;
  outValue.as.number = value;
  return outValue;
};
inline Value makeNIL() {
  Value outValue{};
  outValue.type = VALUE_TYPE::VAL_NIL;
  outValue.as.number = 0;
  return outValue;
};

inline bool valueAsBool(Value value) { return value.as.boolean; }
inline double valueAsNumber(Value value) { return value.as.number; }

inline bool isValueBool(Value value) {
  return value.type == VALUE_TYPE::VAL_BOOL;
}
inline bool isValueNumber(Value value) {
  return value.type == VALUE_TYPE::VAL_NUMBER;
}
inline bool isValueNIL(Value value) {
  return value.type == VALUE_TYPE::VAL_NIL;
}

void printValue(Value value, log::Log *logger);
} // namespace vm
} // namespace binder
