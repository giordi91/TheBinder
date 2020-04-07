#pragma once

#include "binder/memory/resizableVector.h"
#include "tokens.h"

namespace binder {

class Scanner {
 public:
  Scanner() = default;
  ~Scanner() = default;

  void scan(const char* source);

 private:
  [[nodiscard]] bool isAtEnd() const; 
  [[nodiscard]] char advance() const {return m_source[current];}

  void scanToken();
  void addToken(TOKEN_TYPE token);

 private:
  memory::ResizableVector<Token> m_tokens;

  //input data
  const char* m_source;
  uint32_t m_sourceLength;

  // tracking data
  uint32_t start = 0;
  uint32_t current = 0;
  uint32_t line = 0;
};

}  // namespace binder
