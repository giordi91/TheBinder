#include "binder/scanner.h"

#include "binder/interpreter.h"

namespace binder {
void Scanner::scan(const char* source) {
  m_source = source;

  if (m_source == nullptr) {
    // adding EOF token
    addToken(TOKEN_TYPE::END_OF_FILE);
    return;
  }
  m_sourceLength = static_cast<uint32_t>(strlen(m_source));

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

char Scanner::advance() {
  ++current;
  return m_source[current - 1];
}

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
    case '!':
      addToken(match('=') ? TOKEN_TYPE::BANG_EQUAL : TOKEN_TYPE::BANG);
      break;
    case '=':
      addToken(match('=') ? TOKEN_TYPE::EQUAL_EQUAL : TOKEN_TYPE::EQUAL);
      break;
    case '<':
      addToken(match('=') ? TOKEN_TYPE::LESS_EQUAL : TOKEN_TYPE::LESS);
      break;
    case '>':
      addToken(match('=') ? TOKEN_TYPE::GREATER_EQUAL : TOKEN_TYPE::GREATER);
      break;
    case '/':
      if (match('/')) {
        // comments start with / so we check  for an extra /
        while (peek() != '\n' && !isAtEnd()) {
          advance();
        }
      } else {
        addToken(TOKEN_TYPE::SLASH);
      }
      break;
    // whitespaces
    case ' ':
    case '\r':
    case '\t':
      break;
    case '\n':
      ++line;
      break;

    default:
      m_context->reportError(line, "Unexpected character");
      break;
  }
}

void Scanner::addToken(const TOKEN_TYPE token) {
  m_tokens.pushBack({getLexemeFromToken(token), line, token});
}

bool Scanner::match(const char expected) {
  if (isAtEnd()) return false;
  if (m_source[current] != expected) return false;

  // the char matches so we consume it and return true
  ++current;
  return true;
}

char Scanner::peek() const {
  if (isAtEnd()) return '\0';
  return m_source[current];
}
}  // namespace binder
