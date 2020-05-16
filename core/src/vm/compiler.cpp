#include "binder/log/log.h"
#include "binder/vm/compiler.h"
#include "stdlib.h"

namespace binder::vm {

static char vmBuffer[1024];

#define LOG(logger, toPrint, ...)                                              \
  do {                                                                         \
    sprintf(vmBuffer, (toPrint), ##__VA_ARGS__);                               \
    logger->print(vmBuffer);                                                   \
  } while (false);

struct ParseRule {
  FunctionId prefix;
  FunctionId infix;
  Precedence precedence;
};

ParseRule rules[] = {
    {GROUPING, NULLID, PREC_NONE}, // TOKEN_LEFT_PAREN
    {NULLID, NULLID, PREC_NONE},   // TOKEN_RIGHT_PAREN
    {NULLID, NULLID, PREC_NONE},   // TOKEN_LEFT_BRACE
    {NULLID, NULLID, PREC_NONE},   // TOKEN_RIGHT_BRACE
    {NULLID, NULLID, PREC_NONE},   // TOKEN_COMMA
    {NULLID, NULLID, PREC_NONE},   // TOKEN_DOT
    {UNARY, BINARY, PREC_TERM},    // TOKEN_MINUS
    {NULLID, BINARY, PREC_TERM},   // TOKEN_PLUS
    {NULLID, NULLID, PREC_NONE},   // TOKEN_SEMICOLON
    {NULLID, BINARY, PREC_FACTOR}, // TOKEN_SLASH
    {NULLID, BINARY, PREC_FACTOR}, // TOKEN_STAR
    {NULLID, NULLID, PREC_NONE},   // TOKEN_BANG
    {NULLID, NULLID, PREC_NONE},   // TOKEN_BANG_EQUAL
    {NULLID, NULLID, PREC_NONE},   // TOKEN_EQUAL
    {NULLID, NULLID, PREC_NONE},   // TOKEN_EQUAL_EQUAL
    {NULLID, NULLID, PREC_NONE},   // TOKEN_GREATER
    {NULLID, NULLID, PREC_NONE},   // TOKEN_GREATER_EQUAL
    {NULLID, NULLID, PREC_NONE},   // TOKEN_LESS
    {NULLID, NULLID, PREC_NONE},   // TOKEN_LESS_EQUAL
    {NULLID, NULLID, PREC_NONE},   // TOKEN_IDENTIFIER
    {NULLID, NULLID, PREC_NONE},   // TOKEN_STRING
    {NUMBER, NULLID, PREC_NONE},   // TOKEN_NUMBER
    {NULLID, NULLID, PREC_NONE},   // TOKEN_AND
    {NULLID, NULLID, PREC_NONE},   // TOKEN_CLASS
    {NULLID, NULLID, PREC_NONE},   // TOKEN_ELSE
    {NULLID, NULLID, PREC_NONE},   // TOKEN_FALSE
    {NULLID, NULLID, PREC_NONE},   // TOKEN_FOR
    {NULLID, NULLID, PREC_NONE},   // TOKEN_FUN
    {NULLID, NULLID, PREC_NONE},   // TOKEN_IF
    {NULLID, NULLID, PREC_NONE},   // TOKEN_NIL
    {NULLID, NULLID, PREC_NONE},   // TOKEN_OR
    {NULLID, NULLID, PREC_NONE},   // TOKEN_PRINT
    {NULLID, NULLID, PREC_NONE},   // TOKEN_RETURN
    {NULLID, NULLID, PREC_NONE},   // TOKEN_SUPER
    {NULLID, NULLID, PREC_NONE},   // TOKEN_THIS
    {NULLID, NULLID, PREC_NONE},   // TOKEN_TRUE
    {NULLID, NULLID, PREC_NONE},   // TOKEN_VAR
    {NULLID, NULLID, PREC_NONE},   // TOKEN_WHILE
    {NULLID, NULLID, PREC_NONE},   // TOKEN_ERROR
    {NULLID, NULLID, PREC_NONE},   // TOKEN_EOF
};

ParseRule *getRule(TokenType type) { return &rules[type]; }

void Compiler::dispatchFunctionId(FunctionId id) {
  switch (id) {
  case GROUPING:
    grouping();
    break;
  case UNARY:
    unary();
    break;
  case BINARY:
    binary();
    break;
  case NUMBER:
    number();
    break;
  default:
    assert(false && "unsupported function id for pratt parser");
  }
}

void Compiler::parsePrecedence(Precedence precedence) {
  parser.advance();
  FunctionId prefixRule = getRule(parser.previous.type)->prefix;
  if (prefixRule == FunctionId::NULLID) {
    parser.error("Expect expression.");
    return;
  }

  dispatchFunctionId(prefixRule);

  while (precedence <= getRule(parser.current.type)->precedence) {
    parser.advance();
    FunctionId infixRule = getRule(parser.previous.type)->infix;
    dispatchFunctionId(infixRule);
  }
}

void Compiler::number() {
  double value = strtod(parser.previous.start, NULL);
  emitConstant(value);
}

void Compiler::emitConstant(Value value) {
  emitBytes(OP_CODE::OP_CONSTANT, makeConstant(value));
}

uint8_t Compiler::makeConstant(Value value) {
  int constant = m_chunk->addConstant(value);
  if (constant > UINT8_MAX) {
    parser.error("Too many constants in one chunk.");
    return 0;
  }
  return static_cast<uint8_t>(constant);
}

void Compiler::grouping() {
  expression();
  consume(TokenType::TOKEN_RIGHT_PAREN, "Expected ')' after expression.");
}

void Compiler::unary() {
  TokenType operatorType = parser.previous.type;

  // compiler the operand
  parsePrecedence(PREC_UNARY);

  switch (operatorType) {
  case TokenType::TOKEN_MINUS: {
    emitByte(OP_CODE::OP_NEGATE);
    break;
  }
  default:
    break;
  }
}

void Compiler::binary() {
  TokenType operatorType = parser.previous.type;

  // compile right operand
  ParseRule *rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  // emit the operator insturction
  switch (operatorType) {
  case TokenType::TOKEN_PLUS:
    emitByte(OP_CODE::OP_ADD);
    break;
  case TokenType::TOKEN_MINUS:
    emitByte(OP_CODE::OP_SUBTRACT);
    break;
  case TokenType::TOKEN_STAR:
    emitByte(OP_CODE::OP_MULTIPLY);
    break;
  case TokenType::TOKEN_SLASH:
    emitByte(OP_CODE::OP_DIVIDE);
    break;
  default:
    return; // unreachable
  }
}

void Compiler::expression() { parsePrecedence(PREC_ASSIGNMENT); }

void Scanner::skipWhiteSpace() {
  // we keep chew until we find a non white space

  // TODO would be interesting to see if we can use
  // avx to speed this up
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      advance();
      break;
    case '\n':
      line++;
      advance();
      break;
    case '/':
      // checking for double /
      if (peekNext() == '/') { // we need to skip until end of the line
        while ((peek() != '\n') & (!isAtEnd()))
          advance();
      } else {
        return;
      }
      break;

    default:
      return;
    }
  }
}

Token Scanner::string() {
  // keep eaint until we get a closing quote
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n') {
      line++;
    }
    advance();
  }

  if (isAtEnd())
    return errorToken("Unterminated string.");

  // eating the closing quote
  advance();
  return makeToken(TokenType::TOKEN_STRING);
}

Token Scanner::number() {
  // keep eating until we find numbers
  while (isDigit(peek()))
    advance();

  // if we find a dot there might be a fractional part
  if ((peek() == '.') & isDigit(peekNext())) {
    // we eat the dot and parse as many numbers as we can
    advance();

    while (isDigit(peek()))
      advance();
  }

  return makeToken(TokenType::TOKEN_NUMBER);
}

TokenType Scanner::checkKeyword(int startIdx, int length, const char *rest,
                                TokenType type) {
  // first we check if the length is the same second we check if the memory is
  // the same
  if ((current - start) == (startIdx + length) &&
      memcmp(start + startIdx, rest, length) == 0) {
    return type;
  }
  return TokenType::TOKEN_IDENTIFIER;
}

TokenType Scanner::identifierType() {
  switch (start[0]) {
  case 'a':
    return checkKeyword(1, 2, "nd", TokenType::TOKEN_AND);
  case 'c':
    return checkKeyword(1, 4, "lass", TokenType::TOKEN_CLASS);
  case 'e':
    return checkKeyword(1, 3, "lse", TokenType::TOKEN_ELSE);

  // f is a bit more complicated because the grab branches after the f
  // we can have multiple reserved keywords starting with f
  case 'f':
    if (current - start > 1) {
      switch (start[1]) {
      case 'a':
        return checkKeyword(2, 3, "lse", TokenType::TOKEN_FALSE);
      case 'o':
        return checkKeyword(2, 1, "r", TokenType::TOKEN_FOR);
      case 'u':
        return checkKeyword(2, 1, "n", TokenType::TOKEN_FUN);
      }
    }
    break;

  case 'i':
    return checkKeyword(1, 1, "f", TokenType::TOKEN_IF);
  case 'n':
    return checkKeyword(1, 2, "il", TokenType::TOKEN_NIL);
  case 'o':
    return checkKeyword(1, 1, "r", TokenType::TOKEN_OR);
  case 'p':
    return checkKeyword(1, 4, "rint", TokenType::TOKEN_PRINT);
  case 'r':
    return checkKeyword(1, 5, "eturn", TokenType::TOKEN_RETURN);
  case 's':
    return checkKeyword(1, 4, "uper", TokenType::TOKEN_SUPER);
  case 'v':
    return checkKeyword(1, 2, "ar", TokenType::TOKEN_VAR);
  case 'w':
    return checkKeyword(1, 4, "hile", TokenType::TOKEN_WHILE);
  }

  return TokenType::TOKEN_IDENTIFIER;
}

Token Scanner::identifier() {
  // here this works because we already matched an alpha, so we
  // know the the the first value aint a digit
  while (isAlpha(peek()) | isDigit(peek())) {
    advance();
  }
  return makeToken(identifierType());
}

Token Scanner::scanToken() {
  skipWhiteSpace();
  start = current;

  if (isAtEnd())
    return makeToken(TokenType::TOKEN_EOF);

  char c = advance();

  if (isAlpha(c))
    return identifier();
  if (isDigit(c))
    return number();

  switch (c) {
  // first we check the easy since char
  case '(':
    return makeToken(TokenType::TOKEN_LEFT_PAREN);
  case ')':
    return makeToken(TokenType::TOKEN_RIGHT_PAREN);
  case '{':
    return makeToken(TokenType::TOKEN_LEFT_BRACE);
  case '}':
    return makeToken(TokenType::TOKEN_RIGHT_BRACE);
  case ';':
    return makeToken(TokenType::TOKEN_SEMICOLON);
  case ',':
    return makeToken(TokenType::TOKEN_COMMA);
  case '.':
    return makeToken(TokenType::TOKEN_DOT);
  case '-':
    return makeToken(TokenType::TOKEN_MINUS);
  case '+':
    return makeToken(TokenType::TOKEN_PLUS);
  case '/':
    return makeToken(TokenType::TOKEN_SLASH);
  case '*':
    return makeToken(TokenType::TOKEN_STAR);

  // next we check double char punctuation
  case '!':
    return makeToken(match('=') ? TokenType::TOKEN_BANG_EQUAL
                                : TokenType::TOKEN_BANG);
  case '=':
    return makeToken(match('=') ? TokenType::TOKEN_EQUAL_EQUAL
                                : TokenType::TOKEN_EQUAL);
  case '<':
    return makeToken(match('=') ? TokenType::TOKEN_LESS_EQUAL
                                : TokenType::TOKEN_LESS);
  case '>':
    return makeToken(match('=') ? TokenType::TOKEN_GREATER_EQUAL
                                : TokenType::TOKEN_GREATER);
  // now checking strings
  case '"':
    return string();
  }

  printf("%c\n", c);
  return errorToken("Unexpected character.");
}

bool Compiler::compile(const char *source, log::Log *logger) {

  m_chunk = new Chunk;

  scanner.init(source);
  // setupping the pump
  parser.init(&scanner, logger);
  parser.advance();
  expression();
  consume(TOKEN_EOF, "expected end of expression");
  endCompilation(logger);
  return !parser.getHadError();
}

void Parser::errorAt(Token *token, const char *message) {
  // if we are already in panic mode we don't want to keep returning
  // errors until we go back to a state where we can start chewing again
  if (panicMode) {
    return;
  }

  panicMode = true;

  LOG(m_logger, "[line %d] Error", token->line);

  if (token->type == TOKEN_EOF) {
    LOG(m_logger, " at end");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    LOG(m_logger, " at '%.*s'", token->length, token->start);
  }

  LOG(m_logger, ": %s\n", message);
  hadError = true;
}

#undef LOG

} // namespace binder::vm
