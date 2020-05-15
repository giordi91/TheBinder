#include "binder/log/log.h"
#include "binder/vm/compiler.h"

namespace binder::vm {

static char vmBuffer[1024];

#define LOG(logger, toPrint, ...)                                              \
  do {                                                                         \
    sprintf(vmBuffer, (toPrint), ##__VA_ARGS__);                               \
    logger->print(vmBuffer);                                                   \
  } while (false);

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
      }

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

  return errorToken("Unexpected character.");
}

bool Compiler::compile(const char *source, log::Log *logger) {

  m_chunk = new Chunk;


  scanner.init(source);
  // setupping the pump
  parser.init(&scanner, logger);
  parser.advance();
  // expression();
  consume(TOKEN_EOF, "expected end of expression");
  endCompilation();
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
