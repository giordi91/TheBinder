#include "binder/scanner.h"

namespace binder {
void Scanner::scan(const char* source) {
  m_sourceLength = static_cast<uint32_t>(strlen(source));

  // pedantic reset
  start = 0;
  current = 0;

  while (!isAtEnd()) {
    start = current;
    scanToken();
  }

  // adding EOF token
  addToken(TOKEN_TYPE::END_OF_FILE);
}

bool Scanner::isAtEnd() const { return current >= m_sourceLength; }

void Scanner::scanToken() {
  const char c = advance();
  switch (c) {
    case '(':
      addToken(TOKEN_TYPE ::LEFT_PAREN);
      break;
    case ')':
      addToken(TOKEN_TYPE::RIGHT_PAREN);
      break;
    case '{':
      addToken(TOKEN_TYPE::LEFT_BRACE);
      break;
    case '}':
      addToken(TOKEN_TYPE::RIGHT_BRACE);
      break;
    case ',':
      addToken(TOKEN_TYPE::COMMA);
      break;
    case '.':
      addToken(TOKEN_TYPE::DOT);
      break;
    case '-':
      addToken(TOKEN_TYPE::MINUS);
      break;
    case '+':
      addToken(TOKEN_TYPE::PLUS);
      break;
    case ';':
      addToken(TOKEN_TYPE::SEMICOLON);
      break;
    case '*':
      addToken(TOKEN_TYPE::STAR);
      break;
  }
}

void Scanner::addToken(const TOKEN_TYPE token) {
  m_tokens.pushBack({getLexemeFromToken(token), line, token});
}
}  // namespace binder
