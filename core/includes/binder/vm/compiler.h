#pragma once
#include "binder/memory/stringIntern.h"
#include "binder/tokens.h"
#include "binder/vm/chunk.h"
#include "binder/vm/debug.h"
#include "common.h"

#include "assert.h"
#include "stdio.h"
#include "string.h"

namespace binder {
namespace log {
class Log;
}

namespace vm {

struct Chunk;

// TODO we have two token representation, the one here, in the vm is the correct
// one, the one in the interpreter needs to be changed to not use string
// directly but pointer + len, not null terminated strings
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

enum FunctionId {
  NULLID,
  GROUPING,
  UNARY,
  BINARY,
  NUMBER,
  LITERAL,
  STRING,
  VARIABLE
};

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

struct Local {
  Token name;
  int depth;
};

#define UINT8_COUNT (UINT8_MAX + 1)

struct LocalPool {
  Local locals[UINT8_COUNT];
  int localCount = 0;
  int scopeDepth = 0;
};

class Compiler {
public:
  Compiler(memory::StringIntern *intern) : m_intern(intern) {}
  bool compile(const char *source, log::Log *logger);
  const Chunk *getCompiledChunk() const { return m_chunk; };

private:
  void consume(TOKEN_TYPE type, const char *message) {
    if (parser.current.type == type) {
      parser.advance();
      return;
    }
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
  // we are going to rely on the auto deduction of the template param
  // for using this, this should be used mostly for constants and OP_CODES
  template <typename T, typename P> void emitBytes(T byte, P byte2) const {
    emitByte(byte);
    emitByte(byte2);
  }

  void endCompilation(log::Log *) { emitByte(OP_CODE::OP_RETURN); }
  // emit instructions
  void parsePrecedence(Precedence precedence);

  void number(bool canAssign);
  void emitConstant(Value value);
  uint8_t makeConstant(Value value);
  void grouping(bool canAssign);
  void unary(bool canAssign);
  void binary(bool canAssign);
  void literal(bool canAssign);
  void string(bool canAssign);
  void variable(bool canAssign);
  void namedVariable(const Token &token, bool canAssign);
  int resolveLocal(const Token& name);

  // statements
  void expression();
  void declaration();
  void varDeclaration();
  uint8_t parseVariable(const char *error);
  uint8_t identifierConstant(const Token *token);
  void defineVariable(uint8_t globalId);
  void markInitialized();
  void declareVariable();
  void addLocal(const Token& name);
  void statement();
  void printStatement();
  void expressionStatement();

  //block
  void beginScope();
  void block();
  void endScope();

  //identifiers
  bool identifierEqual(const Token& a, const Token& b)const
  {
      //first we check the len, and if it is the same then we can
      //check the actual memory
      if(a.length != b.length) return false;
      return memcmp(a.start, b.start, a.length) ==0;
  }


  void dispatchFunctionId(FunctionId id, bool canAssign);

  bool match(TOKEN_TYPE type) {
    if (!check(type))
      return false;
    parser.advance();
    return true;
  }

  bool check(TOKEN_TYPE type) const { return parser.current.type == type; }

private:
  Scanner scanner;
  Parser parser;
  LocalPool m_localPool;
  memory::StringIntern *m_intern;
  Chunk *m_chunk = nullptr;
};

} // namespace vm
} // namespace binder
