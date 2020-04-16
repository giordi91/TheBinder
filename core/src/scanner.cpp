#include "binder/scanner.h"

#include "binder/interpreter.h"
#include "binder/memory/stringHashMap.h"

namespace binder {

static memory::HashMap<const char *, TOKEN_TYPE, hashString32>
    STRING_TO_RESERVED(200);

Scanner::Scanner(BinderContext *context) : m_tokens(1024), m_context(context) {
  // populate the static map
  if (STRING_TO_RESERVED.getUsedBins() != 0) {
    return;
  }
  STRING_TO_RESERVED.insert("and", TOKEN_TYPE::AND);
  STRING_TO_RESERVED.insert("class", TOKEN_TYPE::CLASS);
  STRING_TO_RESERVED.insert("else", TOKEN_TYPE::ELSE);
  STRING_TO_RESERVED.insert("false", TOKEN_TYPE::BOOL_FALSE);
  STRING_TO_RESERVED.insert("for", TOKEN_TYPE::FOR);
  STRING_TO_RESERVED.insert("fun", TOKEN_TYPE::FUN);
  STRING_TO_RESERVED.insert("if", TOKEN_TYPE::IF);
  STRING_TO_RESERVED.insert("nil", TOKEN_TYPE::NIL);
  STRING_TO_RESERVED.insert("or", TOKEN_TYPE::OR);
  STRING_TO_RESERVED.insert("print", TOKEN_TYPE::PRINT);
  STRING_TO_RESERVED.insert("return", TOKEN_TYPE::RETURN);
  STRING_TO_RESERVED.insert("super", TOKEN_TYPE::SUPER);
  STRING_TO_RESERVED.insert("this", TOKEN_TYPE::THIS);
  STRING_TO_RESERVED.insert("var", TOKEN_TYPE::VAR);
  STRING_TO_RESERVED.insert("while", TOKEN_TYPE::WHILE);
}

void Scanner::scan(const char *source) {
  m_source = source;
  m_tokens.clear();

  if (m_source == nullptr) {
    // adding EOF token
    addToken(TOKEN_TYPE::END_OF_FILE);
    return;
  }
  m_sourceLength = static_cast<uint32_t>(strlen(m_source));

  // pedantic reset
  start = 0;
  current = 0;
  line = 0;

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
  case '"':
    scanString();
    break;
  default:
    if (isDigit(c)) {
      scanNumber();
    } else if (isAlpha(c)) {
      scanIdentifier();
    } else {
      m_context->reportError(line, "Unexpected character");
    }
    break;
  }
}

void Scanner::addToken(const TOKEN_TYPE token) {
  m_tokens.pushBack({getLexemeFromToken(token), line, token});
}

bool Scanner::match(const char expected) {
  if (isAtEnd())
    return false;
  if (m_source[current] != expected)
    return false;

  // the char matches so we consume it and return true
  ++current;
  return true;
}

char Scanner::peek() const {
  if (isAtEnd())
    return '\0';
  return m_source[current];
}
char Scanner::peekNext() const {
  if (isAtEnd())
    return '\0';
  return m_source[current + 1];
}

void Scanner::scanString() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n') {
      ++line;
    }
    advance();
  }

  if (isAtEnd()) {
    m_context->reportError(line, "Unterminated string");
    return;
  }

  // eat the closing "
  advance();

  // so,the minus two is because by now, the current points to the value
  // after the ", so and the substring method of the pool takes the
  // value up and included to end index, so we need to subtract two
  // since we are passing an end index, m_source[endIdx].
  // there is the edge case, as per the addNumberToken, where if the string
  // is length one would fail, due to startIdx==endIdx, in that case
  // we only subtract one, not ideal might need some refactor
  addStringToken(start + 1, current - 2  );
}

void Scanner::scanNumber() {
  // keep chewing numbers until is done
  while (isDigit(peek()))
    advance();

  // lets check if we have a dot and after the dot we have another series
  // of number to swallow
  if (peek() == '.' && isDigit(peekNext())) {
    advance();
    while (isDigit(peek()))
      advance();
  }

  // since the current is at the token past the number we subtract 1
  // but there is the edge case where we have a single digit,
  // which would give us start == current, in that case we bump by one
  // not ideal, I would love to clean this up and have something more generic
  addNumberToken(start, current - (current - start == 1 ? 0 : 1));
}

void Scanner::scanIdentifier() {
  while (isAlphaNumeric(peek()))
    advance();

  uint32_t endIdx = current - 1;
  const char *newString = nullptr;
  if (endIdx > start) {
    newString = m_context->getStringPool().subString(m_source, start, endIdx);
  }
  assert(newString != nullptr);

  // now that we have the string lets look it up in our map
  TOKEN_TYPE type = TOKEN_TYPE::COUNT;
  bool isReserved = STRING_TO_RESERVED.get(newString, type);
  if (!isReserved) {
    type = TOKEN_TYPE::IDENTIFIER;
  }

  m_tokens.pushBack({newString, line, type});
}

bool Scanner::isDigit(const char c) { return (c >= '0') & (c <= '9'); }
bool Scanner::isAlpha(const char c) {
  return ((c >= 'a') & (c <= 'z')) | ((c >= 'A') & (c <= 'Z')) | (c == '_');
}

bool Scanner::isAlphaNumeric(const char c) { return isAlpha(c) | isDigit(c); }

void Scanner::addStringToken(const uint32_t startIdx, const uint32_t endIdx) {
  const char *newString = "";
  // avoiding empty string
  if (endIdx >= startIdx) {
    newString =
        m_context->getStringPool().subString(m_source, startIdx, endIdx);
  }
  m_tokens.pushBack({newString, line, TOKEN_TYPE::STRING});
}
void Scanner::addNumberToken(const uint32_t startIdx, const uint32_t endIdx) {
  const char *number =
      m_context->getStringPool().subString(m_source, startIdx, endIdx);
  m_tokens.pushBack({number, line, TOKEN_TYPE::NUMBER});
}

} // namespace binder
