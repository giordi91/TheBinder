#pragma once

#include "binder/vm/object.h"
#include "stdio.h"

namespace binder {
namespace log {
class Log;
}
namespace vm {

typedef sObj Obj;
typedef sObjString ObjString;

enum VALUE_TYPE {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
  VAL_OBJ,
};

struct Value {
  VALUE_TYPE type;

  union {
    bool boolean;
    double number;
    Obj *obj;
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

inline Value makeObject(Obj*value) {
  Value outValue{};
  outValue.type = VALUE_TYPE::VAL_OBJ;
  outValue.as.obj= value;
  return outValue;
};
inline Value makeObject(ObjString*value) {
  Value outValue{};
  outValue.type = VALUE_TYPE::VAL_OBJ;
  outValue.as.obj= (Obj*)value;
  return outValue;
};

inline bool valueAsBool(Value value) { return value.as.boolean; }
inline double valueAsNumber(Value value) { return value.as.number; }
inline Obj *valueAsObj(Value value) { return value.as.obj; }
inline OBJ_TYPE getObjType(Value value) { return valueAsObj(value)->type; }

inline bool isValueBool(Value value) {
  return value.type == VALUE_TYPE::VAL_BOOL;
}
inline bool isValueNumber(Value value) {
  return value.type == VALUE_TYPE::VAL_NUMBER;
}
inline bool isValueObj(Value value) {
  return value.type == VALUE_TYPE::VAL_OBJ;
}

inline bool isValueNIL(Value value) {
  return value.type == VALUE_TYPE::VAL_NIL;
}
inline bool isObjType(Value value, OBJ_TYPE type) {
  return isValueObj(value) && valueAsObj(value)->type == type;
}

inline bool isValueString(Value value) {
  return isObjType(value, OBJ_TYPE::OBJ_STRING);
}

inline ObjString *valueAsString(Value value) {
  return (ObjString *)(valueAsObj(value));
}
inline char *valueAsCString(Value value) {
  return ((ObjString *)(valueAsObj(value)))->chars;
}

void printValue(Value value, log::Log *logger);

bool valuesEqual(Value a, Value b); 

} // namespace vm
} // namespace binder
