#pragma once

#include "binder/memory/stringHashMap.h"

namespace binder::memory {

class StringIntern {
 public:
  StringIntern(int bucketCount) : m_values(bucketCount) {}

  const char *intern(const char *string, bool copy = true) {
    int len = static_cast<int>(strlen(string));
    return intern(string, len, copy);
  }
  const char *intern(const char *string, int len, bool copy = true) {
    const char *toReturn;

    bool result = m_values.get(string, len, toReturn);
    if (result) {
      // string is present we intern it
      return toReturn;
    } else {
      // we need to copy the string and insert it
      char *toInsert = (char *)string;

      if (copy) {
        toInsert = new char[len + 1];
        memcpy(toInsert, string, len);
        toInsert[len] = '\0';
      }
      m_values.insert(toInsert, toInsert);
      return toInsert;
    }
  }

 private:
  memory::HashMap<const char *, const char *, hashString32> m_values;
};
}  // namespace binder::memory
