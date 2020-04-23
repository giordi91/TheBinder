#pragma once
#include "binder/memory/stringHashMap.h"

namespace binder {
struct RuntimeValue;
class Enviroment {
public:
  // TODO we will need the hashmap to be resizable to be resizable
  Enviroment() : m_values(1024){};
  ~Enviroment() = default;
  // TODO block copy/assigment constructor etc

  void define(const char* variable, RuntimeValue* value)
  {
      m_values.insert(variable,value);
  }

private:
  memory::HashMap<const char *, RuntimeValue *, hashString32> m_values;
};

} // namespace binder
