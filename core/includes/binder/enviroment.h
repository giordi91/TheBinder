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

  void define(const char *variable, RuntimeValue *value) {
    m_values.insert(variable, value);
  }

  bool assign(const char *variable, RuntimeValue *value)
  {
      if(m_values.containsKey(variable))
      {
          m_values.insert(variable,value);
          return true;
      }
      return false;
  }

  bool get(const char *variable, RuntimeValue**outValue) const {
    bool result = m_values.get(variable, *outValue);
    return result;
  }

  void clear()
  {
      m_values.clear();

  }

private:
  memory::HashMap<const char *, RuntimeValue *, hashString32> m_values;
};

} // namespace binder
