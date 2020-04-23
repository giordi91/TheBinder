#pragma once
#include <cstdint>
#include <cassert>

namespace binder {
enum class TOKEN_TYPE {
  // Single-character tokens.
  LEFT_PAREN = 0,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  SEMICOLON,
  SLASH,
  STAR,

  // One or two character tokens.
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,

  // Literals.
  IDENTIFIER,
  STRING,
  NUMBER,

  // Keywords.
  AND,
  CLASS,
  ELSE,
  BOOL_FALSE, //like this instead of FALSE thanks to the lovely windows define 
  FUN,
  FOR,
  IF,
  NIL,
  OR,
  PRINT,
  RETURN,
  SUPER,
  THIS,
  BOOL_TRUE, //same as bool false
  VAR,
  WHILE,

  END_OF_FILE,
  COUNT
};

static const char* TOKEN_TO_LEXEME[] = {
    "(",      ")",     "{",    "}",    ",",      ".",      "-",   "+",
    ";",      "/",     "*",    "!",    "!=",     "=",      "==",  ">",
    ">=",     "<",     "<=",   "id",   "string", "number", "and", "class",
    "else",   "false", "fun",  "for",  "if",     "nil",    "or",  "print",
    "return", "super", "this", "true", "var",    "while",  "\0"};

inline const char* getLexemeFromToken(const TOKEN_TYPE token) {
  assert(token < TOKEN_TYPE::COUNT);
  return TOKEN_TO_LEXEME[static_cast<uint32_t>(token)];
}

struct Token {
  const char* m_lexeme{};
  uint32_t m_line{};
  TOKEN_TYPE m_type = TOKEN_TYPE::END_OF_FILE;
};

}  // namespace binder
