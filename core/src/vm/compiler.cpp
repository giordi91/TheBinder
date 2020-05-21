#include "binder/log/log.h"
#include "binder/vm/compiler.h"
#include "binder/vm/object.h"
#include "stdlib.h"

namespace binder::vm {

struct ParseRule {
  FunctionId prefix;
  FunctionId infix;
  Precedence precedence;
};

// Pratt parser
// this is the heart of the parser, it tells us what to do when we
// hit a specific token what to do for infix and prefix also what the
// precedence level is
ParseRule rules[] = {
    {GROUPING, NULLID, PREC_NONE},     // LEFT_PAREN
    {NULLID, NULLID, PREC_NONE},       // RIGHT_PAREN
    {NULLID, NULLID, PREC_NONE},       // LEFT_BRACE
    {NULLID, NULLID, PREC_NONE},       // RIGHT_BRACE
    {NULLID, NULLID, PREC_NONE},       // COMMA
    {NULLID, NULLID, PREC_NONE},       // DOT
    {UNARY, BINARY, PREC_TERM},        // MINUS
    {NULLID, BINARY, PREC_TERM},       // PLUS
    {NULLID, NULLID, PREC_NONE},       // SEMICOLON
    {NULLID, BINARY, PREC_FACTOR},     // SLASH
    {NULLID, BINARY, PREC_FACTOR},     // STAR
    {UNARY, NULLID, PREC_NONE},        // BANG
    {NULLID, BINARY, PREC_EQUALITY},   // BANG_EQUAL
    {NULLID, NULLID, PREC_NONE},       // EQUAL
    {NULLID, BINARY, PREC_EQUALITY},   // EQUAL_EQUAL
    {NULLID, BINARY, PREC_COMPARISON}, // GREATER
    {NULLID, BINARY, PREC_COMPARISON}, // GREATER_EQUAL
    {NULLID, BINARY, PREC_COMPARISON}, // LESS
    {NULLID, BINARY, PREC_COMPARISON}, // LESS_EQUAL
    {VARIABLE, NULLID, PREC_NONE},     // IDENTIFIER
    {STRING, NULLID, PREC_NONE},       // STRING
    {NUMBER, NULLID, PREC_NONE},       // NUMBER
    {NULLID, NULLID, PREC_NONE},       // AND
    {NULLID, NULLID, PREC_NONE},       // CLASS
    {NULLID, NULLID, PREC_NONE},       // ELSE
    {LITERAL, NULLID, PREC_NONE},      // BOOL_FALSE
    {NULLID, NULLID, PREC_NONE},       // FOR
    {NULLID, NULLID, PREC_NONE},       // FUN
    {NULLID, NULLID, PREC_NONE},       // IF
    {LITERAL, NULLID, PREC_NONE},      // NIL
    {NULLID, NULLID, PREC_NONE},       // OR
    {NULLID, NULLID, PREC_NONE},       // PRINT
    {NULLID, NULLID, PREC_NONE},       // RETURN
    {NULLID, NULLID, PREC_NONE},       // SUPER
    {NULLID, NULLID, PREC_NONE},       // THIS
    {LITERAL, NULLID, PREC_NONE},      // TRUE
    {NULLID, NULLID, PREC_NONE},       // VAR
    {NULLID, NULLID, PREC_NONE},       // WHILE
    {NULLID, NULLID, PREC_NONE},       // TOKEN_ERROR
    {NULLID, NULLID, PREC_NONE},       // END_OF_FILE
};

ParseRule *getRule(TOKEN_TYPE type) { return &rules[static_cast<int>(type)]; }

// this function uses an id to figure out which function to dispatch,
// we could have done it with function pointers but would have required a
// runtime table to do the binding with the instances, this is simpler and
// should be the same level of indirection
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
  case LITERAL:
    literal();
    break;
  case STRING:
    string();
    break;
  case VARIABLE:
    variable();
    break;
  default:
    assert(false && "unsupported function id for pratt parser");
  }
}

// this function is going to tell us how to proceed in the parsing
// if we should give precedence or not
void Compiler::parsePrecedence(Precedence precedence) {
  // here we parse the next token and we look up the correspoinding
  // rule, the one we are interested in is in the previous token
  parser.advance();
  FunctionId prefixRule = getRule(parser.previous.type)->prefix;
  if (prefixRule == FunctionId::NULLID) {
    parser.error("Expect expression.");
    return;
  }

  // here we dispatch the prefix rule
  dispatchFunctionId(prefixRule);

  // here we go in a loop where we keep parsing if we have a higher precedence
  while (precedence <= getRule(parser.current.type)->precedence) {
    // get next token
    parser.advance();
    // if there is a higher precedence we process as infix
    FunctionId infixRule = getRule(parser.previous.type)->infix;
    dispatchFunctionId(infixRule);
  }
}

void Compiler::number() {
  double value = strtod(parser.previous.start, NULL);
  emitConstant(makeNumber(value));
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
  consume(TOKEN_TYPE::RIGHT_PAREN, "Expected ')' after expression.");
}

void Compiler::unary() {
  TOKEN_TYPE operatorType = parser.previous.type;

  // compiler the operand
  parsePrecedence(PREC_UNARY);

  switch (operatorType) {
  case TOKEN_TYPE::BANG: {
    emitByte(OP_CODE::OP_NOT);
    break;
  }
  case TOKEN_TYPE::MINUS: {
    emitByte(OP_CODE::OP_NEGATE);
    break;
  }
  default:
    break;
  }
}

void Compiler::binary() {
  TOKEN_TYPE operatorType = parser.previous.type;

  // compile right operand
  ParseRule *rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  // emit the operator insturction
  switch (operatorType) {
  case TOKEN_TYPE::BANG_EQUAL:
    // a != b is the same as !(a == b)
    // TODO we might want to optimize this with dedicated instructions
    emitBytes(OP_CODE::OP_EQUAL, OP_CODE::OP_NOT);
    break;
  case TOKEN_TYPE::EQUAL_EQUAL:
    emitByte(OP_CODE::OP_EQUAL);
    break;
  case TOKEN_TYPE::GREATER:
    emitByte(OP_CODE::OP_GREATER);
    break;
  case TOKEN_TYPE::GREATER_EQUAL:
    // a >= b is the same as !(a<b)
    // TODO as != we might wanty to optimize with dedicated instructions
    emitBytes(OP_CODE::OP_LESS, OP_CODE::OP_NOT);
    break;
  case TOKEN_TYPE::LESS:
    emitByte(OP_CODE::OP_LESS);
    break;
  case TOKEN_TYPE::LESS_EQUAL:
    emitBytes(OP_CODE::OP_GREATER, OP_CODE::OP_NOT);
    break;
  case TOKEN_TYPE::PLUS:
    emitByte(OP_CODE::OP_ADD);
    break;
  case TOKEN_TYPE::MINUS:
    emitByte(OP_CODE::OP_SUBTRACT);
    break;
  case TOKEN_TYPE::STAR:
    emitByte(OP_CODE::OP_MULTIPLY);
    break;
  case TOKEN_TYPE::SLASH:
    emitByte(OP_CODE::OP_DIVIDE);
    break;
  default:
    return; // unreachable
  }
}

// to parse an expression is quite simple, we just parse anything that
// has higher precedence than assigment
void Compiler::expression() { parsePrecedence(PREC_ASSIGNMENT); }

void Compiler::declaration() {
  if (match(TOKEN_TYPE::VAR)) {
    varDeclaration();
  } else {
    statement();
  }
}

void Compiler::varDeclaration() {
  uint8_t global = parseVariable("Expected variable name");

  if (match(TOKEN_TYPE::EQUAL)) {
    expression();
  } else {
    emitByte(OP_CODE::OP_NIL);
  }
  consume(TOKEN_TYPE::SEMICOLON, "Expected ';' after variabled declaration.");
  defineVariable(global);
}

uint8_t Compiler::parseVariable(const char *error) {
  consume(TOKEN_TYPE::IDENTIFIER, error);
  return identifierConstant(&parser.previous);
}

uint8_t Compiler::identifierConstant(const Token *token) {
  // so here we build the constant, which is allocated as a string into an
  // object this goes in the constant array, this will allow us to do the look
  // up easily with an index
  return makeConstant(makeObject(copyString(token->start, token->length)));
}

void Compiler::defineVariable(uint8_t globalId) {
  emitBytes(OP_CODE::OP_DEFINE_GLOBAL, globalId);
}

void Compiler::statement() {
  if (match(TOKEN_TYPE::PRINT)) {
    printStatement();
  } else {
    expressionStatement();
  }
}

void Compiler::printStatement() {
  // printing requires an expression and a semicolon, then we print it
  expression();
  consume(TOKEN_TYPE::SEMICOLON, "Expected ';' after value.");
  emitByte(OP_CODE::OP_PRINT);
}

void Compiler::expressionStatement() {
  expression();
  consume(TOKEN_TYPE::SEMICOLON, "Expected ';' after expression");
  emitByte(OP_CODE::OP_POP);
}

void Compiler::literal() {
  // since parse precedence already consumed the token, we just need to look
  // into previous and emit the instruction
  switch (parser.previous.type) {
  case TOKEN_TYPE::BOOL_FALSE:
    emitByte(OP_CODE::OP_FALSE);
    break;
  case TOKEN_TYPE::BOOL_TRUE:
    emitByte(OP_CODE::OP_TRUE);
    break;
  case TOKEN_TYPE::NIL:
    emitByte(OP_CODE::OP_NIL);
    break;
  default:
    assert(0);
    return; // unreachable
  }
}

void Compiler::string() {

  // let us intern the string
  int len = parser.previous.length - 2;
  const char *interned =
      m_intern->intern(parser.previous.start + 1, parser.previous.length - 2);
  sObjString *obj = allocateString(interned, len);
  Value value = makeObject((sObj *)obj);
  emitConstant(value);
}

void Compiler::variable() { namedVariable(parser.previous); }

void Compiler::namedVariable(const Token &token) {
  uint8_t arg = identifierConstant(&token);
  emitBytes(OP_CODE::OP_GET_GLOBAL, arg);
}

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
  return makeToken(TOKEN_TYPE::STRING);
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

  return makeToken(TOKEN_TYPE::NUMBER);
}

TOKEN_TYPE Scanner::checkKeyword(int startIdx, int length, const char *rest,
                                 TOKEN_TYPE type) {
  // first we check if the length is the same second we check if the memory is
  // the same
  if ((current - start) == (startIdx + length) &&
      memcmp(start + startIdx, rest, length) == 0) {
    return type;
  }
  return TOKEN_TYPE::IDENTIFIER;
}

TOKEN_TYPE Scanner::identifierType() {
  switch (start[0]) {
  case 'a':
    return checkKeyword(1, 2, "nd", TOKEN_TYPE::AND);
  case 'c':
    return checkKeyword(1, 4, "lass", TOKEN_TYPE::CLASS);
  case 'e':
    return checkKeyword(1, 3, "lse", TOKEN_TYPE::ELSE);

  // f is a bit more complicated because the grab branches after the f
  // we can have multiple reserved keywords starting with f
  case 'f':
    if (current - start > 1) {
      switch (start[1]) {
      case 'a':
        return checkKeyword(2, 3, "lse", TOKEN_TYPE::BOOL_FALSE);
      case 'o':
        return checkKeyword(2, 1, "r", TOKEN_TYPE::FOR);
      case 'u':
        return checkKeyword(2, 1, "n", TOKEN_TYPE::FUN);
      }
    }
    break;

  case 'i':
    return checkKeyword(1, 1, "f", TOKEN_TYPE::IF);
  case 'n':
    return checkKeyword(1, 2, "il", TOKEN_TYPE::NIL);
  case 'o':
    return checkKeyword(1, 1, "r", TOKEN_TYPE::OR);
  case 'p':
    return checkKeyword(1, 4, "rint", TOKEN_TYPE::PRINT);
  case 'r':
    return checkKeyword(1, 5, "eturn", TOKEN_TYPE::RETURN);
  case 's':
    return checkKeyword(1, 4, "uper", TOKEN_TYPE::SUPER);
  case 't':
    if (current - start > 1) {
      switch (start[1]) {
      case 'h':
        return checkKeyword(2, 2, "is", TOKEN_TYPE::THIS);
      case 'r':
        return checkKeyword(2, 2, "ue", TOKEN_TYPE::BOOL_TRUE);
      }
    }
    break;
  case 'v':
    return checkKeyword(1, 2, "ar", TOKEN_TYPE::VAR);
  case 'w':
    return checkKeyword(1, 4, "hile", TOKEN_TYPE::WHILE);
  }

  return TOKEN_TYPE::IDENTIFIER;
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
    return makeToken(TOKEN_TYPE::END_OF_FILE);

  char c = advance();

  if (isAlpha(c))
    return identifier();
  if (isDigit(c))
    return number();

  switch (c) {
  // first we check the easy since char
  case '(':
    return makeToken(TOKEN_TYPE::LEFT_PAREN);
  case ')':
    return makeToken(TOKEN_TYPE::RIGHT_PAREN);
  case '{':
    return makeToken(TOKEN_TYPE::LEFT_BRACE);
  case '}':
    return makeToken(TOKEN_TYPE::RIGHT_BRACE);
  case ';':
    return makeToken(TOKEN_TYPE::SEMICOLON);
  case ',':
    return makeToken(TOKEN_TYPE::COMMA);
  case '.':
    return makeToken(TOKEN_TYPE::DOT);
  case '-':
    return makeToken(TOKEN_TYPE::MINUS);
  case '+':
    return makeToken(TOKEN_TYPE::PLUS);
  case '/':
    return makeToken(TOKEN_TYPE::SLASH);
  case '*':
    return makeToken(TOKEN_TYPE::STAR);

  // next we check double char punctuation
  case '!':
    return makeToken(match('=') ? TOKEN_TYPE::BANG_EQUAL : TOKEN_TYPE::BANG);
  case '=':
    return makeToken(match('=') ? TOKEN_TYPE::EQUAL_EQUAL : TOKEN_TYPE::EQUAL);
  case '<':
    return makeToken(match('=') ? TOKEN_TYPE::LESS_EQUAL : TOKEN_TYPE::LESS);
  case '>':
    return makeToken(match('=') ? TOKEN_TYPE::GREATER_EQUAL
                                : TOKEN_TYPE::GREATER);
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

  while (!match(TOKEN_TYPE::END_OF_FILE)) {
    declaration();
  }

  endCompilation(logger);
  bool gotError = parser.getHadError();
  if (gotError) {
    delete m_chunk;
    m_chunk = nullptr;
  }

  return !gotError;
}

void Parser::errorAt(Token *token, const char *message) {
  // if we are already in panic mode we don't want to keep returning
  // errors until we go back to a state where we can start chewing again
  if (panicMode) {
    return;
  }

  panicMode = true;

  LOG(m_logger, "[line %d] Error", token->line);

  if (token->type == TOKEN_TYPE::END_OF_FILE) {
    LOG(m_logger, " at end");
  } else if (token->type == TOKEN_TYPE::TOKEN_ERROR) {
    // Nothing.
  } else {
    LOG(m_logger, " at '%.*s'", token->length, token->start);
  }

  LOG(m_logger, ": %s\n", message);
  hadError = true;
}

#undef LOG

} // namespace binder::vm
