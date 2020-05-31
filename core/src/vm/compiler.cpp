#include "binder/log/log.h"
#include "binder/vm/compiler.h"
#include "binder/vm/object.h"

#include "stdlib.h"

namespace binder::vm {

struct ParseRule {
  FUNCTION_ID prefix;
  FUNCTION_ID infix;
  PRECEDENCE precedence;
};

// Pratt parser
// this is the heart of the parser, it tells us what to do when we
// hit a specific token what to do for infix and prefix also what the
// precedence level is
const ParseRule RULES[] = {
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
    {NULLID, ANDID, PREC_AND},         // AND
    {NULLID, NULLID, PREC_NONE},       // CLASS
    {NULLID, NULLID, PREC_NONE},       // ELSE
    {LITERAL, NULLID, PREC_NONE},      // BOOL_FALSE
    {NULLID, NULLID, PREC_NONE},       // FOR
    {NULLID, NULLID, PREC_NONE},       // FUN
    {NULLID, NULLID, PREC_NONE},       // IF
    {LITERAL, NULLID, PREC_NONE},      // NIL
    {NULLID, ORID, PREC_OR},           // OR
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

const ParseRule *getRule(TOKEN_TYPE type) { return &RULES[static_cast<int>(type)]; }

// this function uses an id to figure out which function to dispatch,
// we could have done it with function pointers but would have required a
// runtime table to do the binding with the instances, this is simpler and
// should be the same level of indirection
void Compiler::dispatchFunctionId(const FUNCTION_ID id, const bool canAssign) {
  switch (id) {
  case GROUPING:
    grouping(canAssign);
    break;
  case UNARY:
    unary(canAssign);
    break;
  case BINARY:
    binary(canAssign);
    break;
  case NUMBER:
    number(canAssign);
    break;
  case LITERAL:
    literal(canAssign);
    break;
  case STRING:
    string(canAssign);
    break;
  case VARIABLE:
    variable(canAssign);
    break;
  case ANDID:
    parseAnd(canAssign);
    break;
  case ORID:
    parseOr(canAssign);
    break;
  default:
    assert(false && "unsupported function id for pratt parser");
  }
}

int Compiler::emitJump(const OP_CODE instruction) const
{
	// first we emit our normal jump
	emitByte(instruction);
	// then we write 16bit for the offset as placeholder
	emitByte(0xff);
	emitByte(0xff);
	// finally we return where in the code the jump is, aka
	// the current size minus two, the two place holder instructions
	return m_chunk->m_code.size() - 2;
}

void Compiler::patchJump(const int offset)
{
	// the offset value is where in the stack the original jump was at
	// so subtracting the current size gives out the delta between
	// jump and current position, the only thing to do is to offset by
	// two bytes, which is the offset itself, since the offset will be
	// eaten by the instruction
	int jump = static_cast<int>(m_chunk->m_code.size() - offset - 2);

	if (jump > UINT16_MAX)
	{
		parser.error("Too much code to jump over in jump instruction");
	}
	// finally we write th\ne high part of the jump into the first byte
	m_chunk->m_code[offset] = (jump >> 8) & 0xff;
	// and the lower part in the second one
	m_chunk->m_code[offset + 1] = jump & 0xff;
} // this function is going to tell us how to proceed in the parsing
// if we should give precedence or not
void Compiler::parsePrecedence(const PRECEDENCE precedence) {
  // here we parse the next token and we look up the corresponding
  // rule, the one we are interested in is in the previous token
  parser.advance();
  FUNCTION_ID prefixRule = getRule(parser.previous.type)->prefix;
  if (prefixRule == FUNCTION_ID::NULLID) {
    parser.error("Expect expression.");
    return;
  }

  bool canAssign = precedence <= PREC_ASSIGNMENT;

  // here we dispatch the prefix rule
  dispatchFunctionId(prefixRule, canAssign);

  // here we go in a loop where we keep parsing if we have a higher precedence
  while (precedence <= getRule(parser.current.type)->precedence) {
    // get next token
    parser.advance();
    // if there is a higher precedence we process as infix
    FUNCTION_ID infixRule = getRule(parser.previous.type)->infix;
    dispatchFunctionId(infixRule, canAssign);
  }

  // if we can assign but nothing consumes the = means is an error
  if (canAssign && match(TOKEN_TYPE::EQUAL)) {
    parser.error("Invalid assigment target.");
  }
}

void Compiler::number(bool) {
  double value = strtod(parser.previous.start, nullptr);
  emitConstant(makeNumber(value));
}

void Compiler::emitConstant(const Value value) {
  emitBytes(OP_CODE::OP_CONSTANT, makeConstant(value));
}

uint8_t Compiler::makeConstant(const Value value) {
  int constant = m_chunk->addConstant(value);
  if (constant > UINT8_MAX) {
    parser.error("Too many constants in one chunk.");
    return 0;
  }
  return static_cast<uint8_t>(constant);
}

void Compiler::grouping(bool) {
  expression();
  consume(TOKEN_TYPE::RIGHT_PAREN, "Expected ')' after expression.");
}

void Compiler::unary(bool) {
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

void Compiler::binary(bool) {
  TOKEN_TYPE operatorType = parser.previous.type;

  // compile right operand
  const ParseRule *rule = getRule(operatorType);
  parsePrecedence(static_cast<PRECEDENCE>(rule->precedence + 1));

  // emit the operator instruction
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
    // TODO as != we might want to optimize with dedicated instructions
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
// has higher precedence than assignment
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
  consume(TOKEN_TYPE::SEMICOLON, "Expected ';' after variable declaration.");
  defineVariable(global);
}

void Compiler::addLocal(const Token &token) {
  if (m_localPool.localCount == UINT8_COUNT) {
    parser.error("Too many local variables in function.");
    return;
  }
  //"allocating" a new local
  Local &local = m_localPool.locals[m_localPool.localCount++];
  // TODO do we need the full token here? ideally we just need the name
  // and the rest can possibly be separated debug information?
  local.name = token;
  local.depth = -1;
}

void Compiler::declareVariable() {
  // here we declare the existence of local variables,
  // this only happens outside global scope, so we
  // get out if we are in global scope
  if (m_localPool.scopeDepth == 0)
    return;
  const Token &name = parser.previous;

  // here we need to check if the variable has not been declared in the local
  // scope already
  for (int i = m_localPool.localCount - 1; i >= 0; i--) {
    // keep walking back, if the scope is lower, means we are one scope above
    // and we should stop
    const Local &local = m_localPool.locals[i];
    if ((local.depth != -1) & (local.depth < m_localPool.scopeDepth)) {
      break;
    }

    if (identifierEqual(name, local.name)) {
      parser.error("Variable with this name already declared in this scope.");
    }
  }

  addLocal(name);
}

uint8_t Compiler::parseVariable(const char *error) {
  consume(TOKEN_TYPE::IDENTIFIER, error);

  declareVariable();
  // so if we have a scope greater than zero it means is not
  // a global variable, this means we can return a dummy id value
  if (m_localPool.scopeDepth > 0)
    return 0;

  return identifierConstant(&parser.previous);
}

uint8_t Compiler::identifierConstant(const Token *token) {
  // so here we build the constant, which is allocated as a string into an
  // object this goes in the constant array, this will allow us to do the look
  // up easily with an index
  return makeConstant(makeObject(copyString(token->start, token->length)));
}
void Compiler::markInitialized() {
  m_localPool.locals[m_localPool.localCount - 1].depth = m_localPool.scopeDepth;
}

void Compiler::defineVariable(uint8_t globalId) {
  // if we are a local variable we don't store anything since local
  // variables are pushed and popped on the stack at runtime
  if (m_localPool.scopeDepth > 0) {
    markInitialized();
    return;
  }
  emitBytes(OP_CODE::OP_DEFINE_GLOBAL, globalId);
}

void Compiler::statement() {
  if (match(TOKEN_TYPE::PRINT)) {
    printStatement();
  } else if (match(TOKEN_TYPE::FOR)) {
    forStatement();
  } else if (match(TOKEN_TYPE::IF)) {
    ifStatement();
  } else if (match(TOKEN_TYPE::WHILE)) {
    whileStatement();
  } else if (match(TOKEN_TYPE::LEFT_BRACE)) {
    beginScope();
    block();
    endScope();
  } else {
    expressionStatement();
  }
}

void Compiler::beginScope() { m_localPool.scopeDepth++; }
void Compiler::endScope() {
  m_localPool.scopeDepth--;

  // we need to clean up the scope
  // we already reduced the scope depth, so everything that
  // has higher scope needs to be popped
  while ((m_localPool.localCount > 0) &&
         (m_localPool.locals[m_localPool.localCount - 1].depth >
          m_localPool.scopeDepth)) {
    // TODO optimization here, we can have a POPN to pop all variables in
    // one go without popping then one at the time
    emitByte(OP_CODE::OP_POP);
    m_localPool.localCount--;
  }
}

void Compiler::block() {
  while ((!check(TOKEN_TYPE::RIGHT_BRACE)) &
         (!check(TOKEN_TYPE::END_OF_FILE))) {
    declaration();
  }
  consume(TOKEN_TYPE::RIGHT_BRACE, "Expected '}' after block.");
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

void Compiler::ifStatement() {
  // first we take care of processing the expression
  consume(TOKEN_TYPE::LEFT_PAREN, "Expected '(' after 'if'.");
  expression();
  consume(TOKEN_TYPE::RIGHT_PAREN, "Expected ')' after 'if'.");

  // after the expression we should have the result on the stack
  // so we can emit our jump
  int thenJump = emitJump(OP_CODE::OP_JUMP_IF_FALSE);
  // need to remove the condition value from the stack
  emitByte(OP_CODE::OP_POP);
  // process the statement
  statement();

  // after evaluating the statement we need an unconditional jump to skip the
  // else branch
  int elseJump = emitJump(OP_CODE::OP_JUMP);

  // finally we know where to jump and we can patch back the value, this is
  // where
  // the possible else branch begins
  patchJump(thenJump);
  // we get here if we skipped the then branch
  // need to remove the condition value from the stack
  emitByte(OP_CODE::OP_POP);

  if (match(TOKEN_TYPE::ELSE)) {
    statement();
  }
  // as of now we have a jump no matter what which in case of missing else would
  // be an empty jump
  patchJump(elseJump);
}

void Compiler::whileStatement() {

  // setting a marker for the loop
  int loopStart = m_chunk->m_code.size();

  // as usual we parse the condition, which will deposit a bool
  // value on the stack
  consume(TOKEN_TYPE::LEFT_PAREN, "Expected '(' after 'while' .");
  expression();
  consume(TOKEN_TYPE::RIGHT_PAREN, "Expected ')' after 'while condition' .");

  // we jump out if the condition is false
  int exitJump = emitJump(OP_CODE::OP_JUMP_IF_FALSE);
  // cleaning up the stack
  emitByte(OP_CODE::OP_POP);
  // here we parse the statements
  statement();

  // doing the loop, this makes us jump all the way before the condition.
  // this is critical since we can re-eval the condition and figure out if we
  // need to exit
  emitLoop(loopStart);

  // finally we exit and cleanup the stack
  patchJump(exitJump);
  emitByte(OP_CODE::OP_POP);
}

//TODO a multi pass compiler should be able to simplify this a lot
//especially the increment case
void Compiler::forStatement() {

  //the loop might declare variables, for example, it might declare a var i=0
  //in the loop condition, to prevent leaking we ensure a scope around the loop
  //such that variable is only valid in that scope
  beginScope();

  consume(TOKEN_TYPE::LEFT_PAREN, "Expected '(' after 'for'.");
  // trying to match variable declaration
  if (match(TOKEN_TYPE::SEMICOLON)) { // we have no initializer
  } else if (match(TOKEN_TYPE::VAR)) {
    // we have a variable declaration of the sort var i = 0
    varDeclaration();
  } else {
    // we can also have a free expression
    expressionStatement();
  }

  int loopStart = m_chunk->m_code.size();

  //conditional clause
  int exitJump = -1;
  if(!match(TOKEN_TYPE::SEMICOLON)){
      expression();
      consume(TOKEN_TYPE::SEMICOLON, "Expected ';' in loop condition.");

      //we need to jump in case the condition says so
      exitJump = emitJump(OP_CODE::OP_JUMP_IF_FALSE);
      emitByte(OP_CODE::OP_POP);//popping the condition
  }

  //finally the increment clause, this is particularly tricky, that is mostly
  //due to the fact that increment clause is evaluated after the loop,
  //but once we get there we already emitted the code forthe increment condition
  //this is a problem for a single pass compiler. The way we solve it is by
  //skipping the condition, run the body jump back to the condition evaluate 
  //and go normally to the rest of the loop
  if(!match(TOKEN_TYPE::RIGHT_PAREN)){
      int bodyJump = emitJump(OP_CODE::OP_JUMP);

      int incrementStart = m_chunk->m_code.size();
      expression();
      emitByte(OP_CODE::OP_POP);
      consume(TOKEN_TYPE::RIGHT_PAREN, "Expected ')' after 'for' clauses.");


      //after we evaluated the increment we jump to the start of the loop
      //since increment happens at the end of the loop
      emitLoop(loopStart);
      //next we patch the loop start, so at the end of the body, when we parse it
      //we are going to jump here, at the increement start before boing to the top of the
      //loop. Bit mind bending but extremely smart
      loopStart = incrementStart;
      //patching the jump over the increment clause
      patchJump(bodyJump);
  }



  statement();

  emitLoop(loopStart);

  //if we had a condition, the offset is differnt than -1, so we can
  //jump here, after the loop jump
  if(exitJump != -1)
  {
      patchJump(exitJump);
      emitByte(OP_CODE::OP_POP);
  }

  //closeing the scope
  endScope();
}

void Compiler::literal(bool) const
{
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

void Compiler::string(bool) {

  // let us intern the string
  int len = parser.previous.length - 2;
  const char *interned =
      m_intern->intern(parser.previous.start + 1, parser.previous.length - 2);
  sObjString *obj = allocateString(interned, len);
  Value value = makeObject((sObj *)obj);
  emitConstant(value);
}

void Compiler::variable(const bool canAssign) {
  namedVariable(parser.previous, canAssign);
}
void Compiler::parseAnd(bool) {
  // here we first emit a jump, that is because if the
  // left hand side is false we short circuit, hence the ump
  int endJump = emitJump(OP_CODE::OP_JUMP_IF_FALSE);

  // we emit a pop, the reason for this is, if we know the left hand side being
  // positive, hence not taking the jump, we don't need that value anymore,
  // the result value will be the right hand side which will be on top of the
  // stack
  emitByte(OP_CODE::OP_POP);
  // parse the rest of the right hand side
  parsePrecedence(PREC_AND);

  // finally we patch the jump
  patchJump(endJump);
}
void Compiler::parseOr(bool) {
  // here we are using more instructions but we can see how we can remap the
  // values to what we want, so we can implement the or instruction with just
  // the instructions we have and not add new ones

  // first we evaluate the left hand side, which would be already on the stack
  // if the or is false we simply skip the right hand side branch.
  // So the first jump if taken lets us skip the following jump that would let
  // us skip the right hand side. Bit confusing but makes sense. looks like this

  /*
   *             left hand side  //eval left side
   *        --   jump if false  //if false we go to eval right hand side
   *      --|--- jump //if true we skip past RHS, LHS is on the top of stack
   *      | |->  pop
   *      |      right hand side
   *      |->    continue...
   */

  // if false we evaluate the right operand
  int elseJump = emitJump(OP_CODE::OP_JUMP_IF_FALSE);
  // if we fall through we skip and leave the op on the stack being our resoult
  int endJump = emitJump(OP_CODE::OP_JUMP);

  patchJump(elseJump);
  // here instead we evaluate the right statement, first we get rid of the LHS
  // form the stack
  emitByte(OP_CODE::OP_POP);

  parsePrecedence(PREC_OR);
  patchJump(endJump);
}

void Compiler::namedVariable(const Token &token, const bool canAssign) {
  OP_CODE getOp;
  OP_CODE setOp;

  // we first try to resolve the variable locally
  // if we don't find one we resolve it globally
  int arg = resolveLocal(token);
  if (arg != -1) {
    getOp = OP_CODE::OP_GET_LOCAL;
    setOp = OP_CODE::OP_SET_LOCAL;
  } else {
    arg = identifierConstant(&token);
    getOp = OP_CODE::OP_GET_GLOBAL;
    setOp = OP_CODE::OP_SET_GLOBAL;
  }

  // here we are dealing with a variable, we need to see
  // if we have a an equal after it, if that is the case
  // it means we want to set  such variable not get it
  if (canAssign & match(TOKEN_TYPE::EQUAL)) {
    expression();
    emitBytes(setOp, arg);
  } else {
    emitBytes(getOp, arg);
  }
}

int Compiler::resolveLocal(const Token &name) {
  // local resolution is fairly straight forward, we walk back
  // until we find a matching variable or we are on a lower level scope
  // aka parent scope
  for (int i = m_localPool.localCount - 1; i >= 0; --i) {
    const Local &local = m_localPool.locals[i];
    if (identifierEqual(name, local.name)) {
      if (local.depth == -1) {
        parser.error("Cannot read local variable in its own initializer");
      }
      return i;
    }
  }
  return -1;
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
  // keep eating until we get a closing quote
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

TOKEN_TYPE Scanner::checkKeyword(const int startIdx, const int length, const char *rest,
                                 const TOKEN_TYPE type) const
{
  // first we check if the length is the same second we check if the memory is
  // the same
  if ((current - start) == (startIdx + length) &&
      memcmp(start + startIdx, rest, length) == 0) {
    return type;
  }
  return TOKEN_TYPE::IDENTIFIER;
}

TOKEN_TYPE Scanner::identifierType() const
{
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
  // setup the pump
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

} // namespace binder::vm
