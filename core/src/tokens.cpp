#include "binder/tokens.h"

#include <cassert>

static const char* TOKEN_TO_LEXEME[] = {
    "(",      ")",     "{",    "}",    ",",      ".",      "-",   "+",
    ";",      "/",     "*",    "!",    "!=",     "=",      "==",  ">",
    ">=",     "<",     "<=",   "id",   "string", "number", "and", "class",
    "else",   "false", "fun",  "for",  "if",     "nil",    "or",  "print",
    "return", "super", "this", "true", "var",    "while",  ""};

namespace binder {
const char* getLexemeFromToken(const TOKEN_TYPE token) {
  assert(token < TOKEN_TYPE::COUNT);
  return TOKEN_TO_LEXEME[static_cast<uint32_t>(token)];
}
}  // namespace binder
