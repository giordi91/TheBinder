#include "binder/context.h"
#include "binder/parser.h"
#include <stdio.h>

namespace binder {

void Parser::parse(const memory::ResizableVector<Token> *tokens) {
  current = 0;
  m_tokens = tokens;
  m_stmts.clear();

  try {
    while (!isAtEnd()) {
      m_stmts.pushBack(statement());
    }
  } catch (ParserException e) {
  }
}

autogen::Expr *Parser::expression() { return equality(); }

// equality → comparison ( ( "!=" | "==" ) comparison )* ;
autogen::Expr *Parser::equality() {
  autogen::Expr *expr = this->comparison();

  TOKEN_TYPE types[] = {TOKEN_TYPE::BANG_EQUAL, TOKEN_TYPE::EQUAL_EQUAL};
  while (match(types, 2)) {
    Token op = previous();
    autogen::Expr *right = comparison();
    // TODO  deal with this allocation
    autogen::Binary *binary = new autogen::Binary();
    binary->left = expr;
    binary->op = op.m_type;
    binary->right = right;
    expr = binary;
  }
  return expr;
}

autogen::Expr *Parser::comparison() {
  autogen::Expr *expr = addition();

  TOKEN_TYPE types[] = {TOKEN_TYPE::GREATER, TOKEN_TYPE::GREATER_EQUAL,
                        TOKEN_TYPE::LESS, TOKEN_TYPE::LESS_EQUAL};
  while (match(types, 4)) {
    Token op = previous();
    autogen::Expr *right = addition();

    autogen::Binary *binary = new autogen::Binary();
    binary->left = expr;
    binary->op = op.m_type;
    binary->right = right;
    expr = binary;
  }
  return expr;
}

autogen::Expr *Parser::addition() {
  autogen::Expr *expr = multiplication();

  TOKEN_TYPE types[] = {TOKEN_TYPE::MINUS, TOKEN_TYPE::PLUS};
  while (match(types, 2)) {
    Token op = previous();
    autogen::Expr *right = addition();

    autogen::Binary *binary = new autogen::Binary();
    binary->left = expr;
    binary->op = op.m_type;
    binary->right = right;
    expr = binary;
  }
  return expr;
}
autogen::Expr *Parser::multiplication() {
  autogen::Expr *expr = unary();

  TOKEN_TYPE types[] = {TOKEN_TYPE::STAR, TOKEN_TYPE::SLASH};
  // TODO change for array len
  while (match(types, 2)) {
    Token op = previous();
    autogen::Expr *right = addition();

    autogen::Binary *binary = new autogen::Binary();
    binary->left = expr;
    binary->op = op.m_type;
    binary->right = right;
    expr = binary;
  }
  return expr;
}
autogen::Expr *Parser::unary() {
  TOKEN_TYPE types[] = {TOKEN_TYPE::BANG, TOKEN_TYPE::MINUS};
  // TODO change for array len
  if (match(types, 2)) {
    Token op = previous();
    autogen::Expr *right = addition();

    // find a way for brace init
    auto *unary = new autogen::Unary();
    unary->op = op.m_type;
    unary->right = right;
    return unary;
  }

  return primary();
}

autogen::Expr *Parser::primary() {

  if (match(TOKEN_TYPE::BOOL_FALSE)) {
    auto *expr = new autogen::Literal();
    expr->value = "false";
    expr->type = TOKEN_TYPE::BOOL_FALSE;
    return expr;
  }
  if (match(TOKEN_TYPE::BOOL_TRUE)) {
    auto *expr = new autogen::Literal();
    expr->value = "true";
    expr->type = TOKEN_TYPE::BOOL_TRUE;
    return expr;
  }
  if (match(TOKEN_TYPE::NIL)) {
    auto *expr = new autogen::Literal();
    expr->value = nullptr;
    expr->type = TOKEN_TYPE::NIL;
    return expr;
  }

  TOKEN_TYPE types[] = {TOKEN_TYPE::NUMBER, TOKEN_TYPE::STRING};
  if (match(types, 2)) {

    auto *expr = new autogen::Literal();
    expr->value = previous().m_lexeme;
    expr->type = previous().m_type;
    return expr;
  }

  if (match(TOKEN_TYPE::LEFT_PAREN)) {
    autogen::Expr *expr = expression();
    consume(TOKEN_TYPE::RIGHT_PAREN, "Expected ')' after expresion.");

    auto *grouping = new autogen::Grouping();
    grouping->expr = expr;
    return grouping;
  }

  throw error(peek(), "Expected primary expression.");
}

autogen::Stmt *Parser::statement() {
  if (match(TOKEN_TYPE::PRINT)) {
    return printStatement();
  }
  return expressionStatement();
}

autogen::Stmt *Parser::printStatement() {
  autogen::Expr *value = expression();
  consume(TOKEN_TYPE::SEMICOLON, "Expect ';' after print expression.");
  auto *stmt = new autogen::Print();
  stmt->expression = value;
  return stmt;
}
autogen::Stmt *Parser::expressionStatement() {
  autogen::Expr *value = expression();
  consume(TOKEN_TYPE::SEMICOLON, "Expect ';' after expression.");
  auto *stmt = new autogen::Expression();
  stmt->expression = value;
  return stmt;
}
// utility functions

bool Parser::match(const TOKEN_TYPE *types, const int size) {
  for (int i = 0; i < size; ++i) {
    if (check(types[i])) {
      advance();
      return true;
    }
  }
  return false;
}

bool Parser::match(const TOKEN_TYPE type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

bool Parser::check(TOKEN_TYPE type) {
  if (isAtEnd())
    return false;
  // looking ahead without consuming
  return peek().m_type == type;
}

bool Parser::isAtEnd() const {
  return peek().m_type == TOKEN_TYPE::END_OF_FILE;
}
const Token &Parser::advance() {
  if (!isAtEnd())
    current++;
  return previous();
}

const Token &Parser::peek() const { return (*m_tokens)[current]; };
const Token &Parser::previous() const { return (*m_tokens)[current - 1]; };

ParserException Parser::error(const Token &token, const char *message) {
  m_context->reportError(token.m_line, message);
  return ParserException();
}

const Token &Parser::consume(TOKEN_TYPE type, const char *message) {
  if (check(type)) {
    return advance();
  }

  throw error(peek(), message);
}

} // namespace binder
