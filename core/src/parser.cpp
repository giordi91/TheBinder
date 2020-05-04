#include "binder/context.h"
#include "binder/parser.h"
#include <stdio.h>

namespace binder {

void Parser::parse(const memory::ResizableVector<Token> *tokens) {
  current = 0;
  m_tokens = tokens;
  m_stmts.clear();

  while (!isAtEnd()) {
    autogen::Stmt *stmt = declaration();
    if (stmt != nullptr) {
      m_stmts.pushBack(stmt);
    }
  }
}

autogen::Expr *Parser::expression() { return assignment(); }

autogen::Expr *Parser::assignment() {
  autogen::Expr *expr = equality();

  if (match(TOKEN_TYPE::EQUAL)) {
    autogen::Expr *value = assignment();

    if (expr->astType == autogen::AST_TYPE::VARIABLE) {
      const char *name = ((autogen::Variable *)expr)->name;
      auto *toReturn = new autogen::Assign();
      toReturn->astType = autogen::AST_TYPE::ASSIGN;
      toReturn->name = name;
      toReturn->value = value;
      return toReturn;
    }

    Token equals = previous();
    error(equals, "Invalid assigment target.");
  }
  return expr;
}

// equality → comparison ( ( "!=" | "==" ) comparison )* ;
autogen::Expr *Parser::equality() {
  autogen::Expr *expr = this->comparison();

  TOKEN_TYPE types[] = {TOKEN_TYPE::BANG_EQUAL, TOKEN_TYPE::EQUAL_EQUAL};
  while (match(types, 2)) {
    Token op = previous();
    autogen::Expr *right = comparison();
    // TODO  deal with this allocation
    autogen::Binary *binary = new autogen::Binary();
    binary->astType = autogen::AST_TYPE::BINARY;
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
    binary->astType = autogen::AST_TYPE::BINARY;
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
    binary->astType = autogen::AST_TYPE::BINARY;
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
    binary->astType = autogen::AST_TYPE::BINARY;
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
    autogen::Expr *right = unary();

    // find a way for brace init
    auto *unary = new autogen::Unary();
    unary->astType = autogen::AST_TYPE::UNARY;
    unary->op = op.m_type;
    unary->right = right;
    return unary;
  }

  return primary();
}

autogen::Expr *Parser::primary() {

  if (match(TOKEN_TYPE::BOOL_FALSE)) {
    auto *expr = new autogen::Literal();
    expr->astType = autogen::AST_TYPE::LITERAL;
    expr->value = "false";
    expr->type = TOKEN_TYPE::BOOL_FALSE;
    return expr;
  }
  if (match(TOKEN_TYPE::BOOL_TRUE)) {
    auto *expr = new autogen::Literal();
    expr->astType = autogen::AST_TYPE::LITERAL;
    expr->value = "true";
    expr->type = TOKEN_TYPE::BOOL_TRUE;
    return expr;
  }
  if (match(TOKEN_TYPE::NIL)) {
    auto *expr = new autogen::Literal();
    expr->astType = autogen::AST_TYPE::LITERAL;
    expr->value = nullptr;
    expr->type = TOKEN_TYPE::NIL;
    return expr;
  }

  TOKEN_TYPE types[] = {TOKEN_TYPE::NUMBER, TOKEN_TYPE::STRING};
  if (match(types, 2)) {

    auto *expr = new autogen::Literal();
    expr->astType = autogen::AST_TYPE::LITERAL;
    expr->value = previous().m_lexeme;
    expr->type = previous().m_type;
    return expr;
  }

  if (match(TOKEN_TYPE::IDENTIFIER)) {
    auto *expr = new autogen::Variable();
    expr->astType = autogen::AST_TYPE::VARIABLE;
    expr->name = previous().m_lexeme;
    return expr;
  }

  if (match(TOKEN_TYPE::LEFT_PAREN)) {
    autogen::Expr *expr = expression();
    consume(TOKEN_TYPE::RIGHT_PAREN, "Expected ')' after expresion.");

    auto *grouping = new autogen::Grouping();
    expr->astType = autogen::AST_TYPE::GROUPING;
    grouping->expr = expr;
    return grouping;
  }

  throw error(peek(), "Expected primary expression.");
}

autogen::Stmt *Parser::statement() {
  if (match(TOKEN_TYPE::PRINT)) {
    return printStatement();
  }
  if (match(TOKEN_TYPE::LEFT_BRACE)) {
    return blockStatement();
  }
  return expressionStatement();
}

autogen::Stmt *Parser::declaration() {

  try {
    if (match(TOKEN_TYPE::VAR)) {
      return varDeclaration();
    }

    return statement();
  } catch (ParserException e) {
    // TODO sync here
    // TODO catch the nullptr outside and do not add it
    // to the list
    syncronize();
    return nullptr;
  }
}

autogen::Stmt *Parser::varDeclaration() {
  Token name = consume(TOKEN_TYPE::IDENTIFIER, "Expected variable name.");

  autogen::Expr *initializer = nullptr;
  if (match(TOKEN_TYPE::EQUAL)) {
    initializer = expression();
  }
  consume(TOKEN_TYPE::SEMICOLON, "Expected ';' after variable declaration.");
  auto *var = new autogen::Var();
  var->astType = autogen::AST_TYPE::VAR;
  var->token = name;
  var->initializer = initializer;
  return var;
}

autogen::Stmt *Parser::printStatement() {
  autogen::Expr *value = expression();
  consume(TOKEN_TYPE::SEMICOLON, "Expect ';' after print expression.");
  auto *stmt = new autogen::Print();
  stmt->astType = autogen::AST_TYPE::PRINT;
  stmt->expression = value;
  return stmt;
}

autogen::Stmt *Parser::blockStatement() {
    auto* block = new autogen::Block();
    //here we keep chewing until we find either a right brance or 
    //we are at the end of the file
    while(!check(TOKEN_TYPE::RIGHT_BRACE) && !isAtEnd())
    {
        block->statements.pushBack(declaration());
    }

    //now that we are done we expect a closing curly otherwise is an error
    consume (TOKEN_TYPE::RIGHT_BRACE, "Expected '}' after block");
    return block;
}

autogen::Stmt *Parser::expressionStatement() {
  autogen::Expr *value = expression();
  consume(TOKEN_TYPE::SEMICOLON, "Expect ';' after expression.");
  auto *stmt = new autogen::Expression();
  stmt->astType = autogen::AST_TYPE::EXPRESSION;
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

void Parser::syncronize() {
  // eating the error
  advance();
  while (!isAtEnd()) {
    // eat until semicolon
    if (previous().m_type == TOKEN_TYPE::SEMICOLON)
      return;
    advance();
  }

  switch (peek().m_type) {
  case TOKEN_TYPE::CLASS:
  case TOKEN_TYPE::FUN:
  case TOKEN_TYPE::VAR:
  case TOKEN_TYPE::FOR:
  case TOKEN_TYPE::IF:
  case TOKEN_TYPE::WHILE:
  case TOKEN_TYPE::PRINT:
  case TOKEN_TYPE::RETURN:
    return;
  default:
    break;
  }
  advance();
}

const Token &Parser::consume(TOKEN_TYPE type, const char *message) {
  if (check(type)) {
    return advance();
  }

  throw error(peek(), message);
}

} // namespace binder
