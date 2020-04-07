#pragma once
#include <cstdint>

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

const char* getLexemeFromToken(const TOKEN_TYPE token);

struct Token {
  const char* m_lexeme{};
  uint32_t m_line{};
  TOKEN_TYPE m_type = TOKEN_TYPE::END_OF_FILE;
  uint16_t padding = 0;
};

}  // namespace binder
