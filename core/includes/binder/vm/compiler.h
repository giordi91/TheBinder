#pragma once
#include "assert.h"
#include "binder/tokens.h"
#include "binder/vm/chunk.h"
#include "common.h"
#include "stdio.h"
#include "string.h"
#include "debug.h"

namespace binder {
namespace log {
class Log;
}

namespace vm {

struct Chunk;

//TODO we have two token representation, the one here, in the vm is the correct
//one, the one in the interpreter needs to be changed to not use string directly
//but pointer + len, not null terminated strings
struct Token {
  TOKEN_TYPE type;
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

  Token makeToken(TOKEN_TYPE type) const {
    return {type, start, static_cast<int>(current - start), line};
  }
  Token errorToken(const char *message) const {
    return {TOKEN_TYPE::TOKEN_ERROR, message, static_cast<int>(strlen(current)),
            line};
  }

  void skipWhiteSpace();
  bool isDigit(char c) const { return (c >= '0') & (c <= '9'); }
  bool isAlpha(char c) const {
    return ((c >= 'a') & (c <= 'z')) | ((c >= 'A') & (c <= 'Z')) | (c == '_');
  }
  TOKEN_TYPE identifierType();
  TOKEN_TYPE checkKeyword(int start, int length, const char *rest,
                         TOKEN_TYPE type);
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
      if (current.type != TOKEN_TYPE::TOKEN_ERROR)
        break;
      errorAtCurrent(current.start);
    }
  }

  bool getHadError() const { return hadError; }
  void errorAtCurrent(const char *message) { errorAt(&current, message); }
  void error(const char *message) { errorAt(&previous, message); }

public:
  Token current;
  Token previous;

private:
  void errorAt(Token *token, const char *message);

private:
  Scanner *m_scanner = nullptr;
  log::Log *m_logger = nullptr;
  bool hadError = false;
  bool panicMode = false;
};

enum FunctionId { NULLID, GROUPING, UNARY, BINARY, NUMBER };

enum Precedence {
  PREC_NONE,
  PREC_ASSIGNMENT, // =
  PREC_OR,         // or
  PREC_AND,        // and
  PREC_EQUALITY,   // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_TERM,       // + -
  PREC_FACTOR,     // * /
  PREC_UNARY,      // ! -
  PREC_CALL,       // . ()
  PREC_PRIMARY
};

class Compiler {
public:
  bool compile(const char *source, log::Log *logger);
  const Chunk* getCompiledChunk()const{return m_chunk;};

private:
  void consume(TOKEN_TYPE type, const char *message) {
    if (parser.current.type == type) {
      parser.advance();
    }
    return;

    parser.errorAtCurrent(message);
  }

  void emitByte(uint8_t byte) const {
    assert(m_chunk != nullptr);
    m_chunk->write(byte, parser.previous.line);
  }
  void emitByte(OP_CODE byte) const {
    assert(m_chunk != nullptr);
    m_chunk->write(byte, parser.previous.line);
  }
  void emitBytes(OP_CODE byte, uint8_t byte2) const {
    emitByte(byte);
    emitByte(byte2);
  }

#define DEBUG_PRINT_CODE

  void endCompilation(log::Log* log) {
    emitByte(OP_CODE::OP_RETURN);

#ifdef DEBUG_PRINT_CODE
    if (!parser.getHadError()) {
      disassambleChunk(m_chunk, "code",log);
    }
#endif
  }
  // emit instructions
  void parsePrecedence(Precedence precedence);

  void number();
  void emitConstant(Value value);
  uint8_t makeConstant(Value value);
  void grouping();
  void unary();
  void binary();
  void expression();

  void dispatchFunctionId(FunctionId id);

private:
  Scanner scanner;
  Parser parser;
  Chunk *m_chunk = nullptr;
};

} // namespace vm
} // namespace binder
