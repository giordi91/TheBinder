#pragma once

#include "binder/memory/resizableVector.h"
#include "tokens.h"

namespace binder {
class BinderContext;

class Scanner {
 public:
  explicit Scanner(BinderContext* context)
      : m_tokens(1024), m_context(context) {}
  ~Scanner() = default;

  void scan(const char* source);
  [[nodiscard]] const memory::ResizableVector<Token>& getTokens() const {
    return m_tokens;
  }

 private:
  [[nodiscard]] bool isAtEnd() const;
  char advance();

  void scanToken();
  void addToken(TOKEN_TYPE token);
  bool match(char expected);
  [[nodiscard]] char peek() const;
  [[nodiscard]] char peekNext() const;
  void scanString();
  void scanNumber();
  void scanIdentifier();
  static bool isDigit(const char c);
  static bool isAlpha(const char c);
  static bool isAlphaNumeric(const char c);
  void addStringToken(uint32_t startIdx, uint32_t endIdx);
  void addNumberToken(uint32_t startIdx, uint32_t endIdx);

 private:
  memory::ResizableVector<Token> m_tokens;
  BinderContext* m_context;

  // input data
  const char* m_source = nullptr;
  uint32_t m_sourceLength = 0;

  // tracking data
  uint32_t start = 0;
  uint32_t current = 0;
  uint32_t line = 0;
};

}  // namespace binder
