#pragma once
#include "binder/memory/stringHashMap.h"

namespace binder {
struct RuntimeValue;
class Enviroment {
public:
  // TODO we will need the hashmap to be resizable to be resizable
  explicit Enviroment(Enviroment* enclosing ) : m_values(1024), m_enclosing(enclosing){};
  Enviroment( ) : m_values(1024), m_enclosing(nullptr){};
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

    if(m_enclosing != nullptr)
    {
        bool result = m_enclosing->assign(variable,value);
        return result;
    }

      return false;
  }

  bool get(const char *variable, RuntimeValue**outValue) const {
    bool result = m_values.get(variable, *outValue);
    if(result)
    {
        return result;
    }

    //let us check the enclosing enviroment 
    if(m_enclosing != nullptr)
    {
        result = m_enclosing->get(variable,outValue);
        return result;
    }
    return false;
  }

  void clear()
  {
      m_values.clear();

  }

private:
  memory::HashMap<const char *, RuntimeValue *, hashString32> m_values;
  Enviroment* m_enclosing;
};

} // namespace binder
