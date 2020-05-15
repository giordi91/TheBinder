#pragma once
#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "common.h"
#include "binder/vm/chunk.h"

namespace binder {
namespace log {
class Log;
}

namespace vm {

struct Chunk;

enum TokenType {
  // Single-character tokens.
  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_COMMA,
  TOKEN_DOT,
  TOKEN_MINUS,
  TOKEN_PLUS,
  TOKEN_SEMICOLON,
  TOKEN_SLASH,
  TOKEN_STAR,

  // One or two character tokens.
  TOKEN_BANG,
  TOKEN_BANG_EQUAL,
  TOKEN_EQUAL,
  TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER,
  TOKEN_GREATER_EQUAL,
  TOKEN_LESS,
  TOKEN_LESS_EQUAL,

  // Literals.
  TOKEN_IDENTIFIER,
  TOKEN_STRING,
  TOKEN_NUMBER,

  // Keywords.
  TOKEN_AND,
  TOKEN_CLASS,
  TOKEN_ELSE,
  TOKEN_FALSE,
  TOKEN_FOR,
  TOKEN_FUN,
  TOKEN_IF,
  TOKEN_NIL,
  TOKEN_OR,
  TOKEN_PRINT,
  TOKEN_RETURN,
  TOKEN_SUPER,
  TOKEN_THIS,
  TOKEN_TRUE,
  TOKEN_VAR,
  TOKEN_WHILE,

  TOKEN_ERROR,
  TOKEN_EOF
};

struct Token {
  TokenType type;
  const char *start;
  int length;
  int line;
};

struct Scanner {
  const char *start;
  const char *current;
  int line;

  void init(const char *source) {
    start = source;
    current = start;
    line = 0;
  }

  Token scanToken();

private:
  bool isAtEnd() const { return *current == '\0'; }
  char advance() {
    current++;
    return current[-1];
  }
  bool match(char expected) {
    // TODO we can probably make this branchless
    if (isAtEnd())
      return false;
    if (*current != expected)
      return false;
    current++;
    return true;
  }

  char peek() const { return *current; }
  char peekNext() const {
    // TODO compile probably optimizes this already but might
    // be worth to make branchless
    if (isAtEnd())
      return '\0';
    return current[1];
  }

  Token makeToken(TokenType type) const {
    return {type, start, static_cast<int>(current - start), line};
  }
  Token errorToken(const char *message) const {
    return {TokenType::TOKEN_ERROR, message, static_cast<int>(strlen(current)),
            line};
  }

  void skipWhiteSpace();
  bool isDigit(char c) const { return (c >= '0') & (c <= '9'); }
  bool isAlpha(char c) const {
    return ((c >= 'a') & (c <= 'z')) | ((c >= 'A') & (c <= 'Z')) | (c == '_');
  }
  TokenType identifierType();
  TokenType checkKeyword(int start, int length, const char *rest,
                         TokenType type);
  Token string();
  Token number();
  Token identifier();
};

class Parser {

public:
  Parser() = default;

  void init(Scanner *scanner, log::Log *logger) {
    m_scanner = scanner;
    m_logger = logger;
    hadError = false;
    panicMode = false;
    current = {};
    previous = {};
  }
  void advance() {
    previous = current;

    for (;;) {
      current = m_scanner->scanToken();
      if (current.type != TokenType::TOKEN_ERROR)
        break;
      errorAtCurrent(current.start);
    }
  }

  bool getHadError() const { return hadError; }
  void errorAtCurrent(const char *message) { errorAt(&current, message); }

public:
  Token current;
  Token previous;

private:
  void error(const char *message) { errorAt(&previous, message); }

  void errorAt(Token *token, const char *message);

private:
  Scanner *m_scanner = nullptr;
  log::Log *m_logger = nullptr;
  bool hadError = false;
  bool panicMode = false;
};

class Compiler {
public:
  bool compile(const char *source, log::Log *logger);

private:
  void consume(TokenType type, const char *message) {
    if (parser.current.type == type) {
      parser.advance();
    }
    return;

    parser.errorAtCurrent(message);
  }

  void emitByte(uint8_t byte) const
  {
      assert(m_chunk!=nullptr);
      m_chunk->write(byte,parser.previous.line);
  }
  void emitByte(OP_CODE byte) const
  {
      assert(m_chunk!=nullptr);
      m_chunk->write(byte,parser.previous.line);
  }
  void emitByte(OP_CODE byte, uint8_t byte2) const
  {
      emitByte(byte);
      emitByte(byte2);
  }

  void endCompilation()
  {
      emitByte(OP_CODE::OP_RETURN);

  }
private:
  Scanner scanner;
  Parser parser;
  Chunk* m_chunk =nullptr;
};

} // namespace vm
} // namespace binder
