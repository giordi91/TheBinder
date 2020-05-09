#pragma once

#include "binder/autogen/astgen.h"
#include "binder/memory/resizableVector.h"
#include "binder/tokens.h"

#include <exception>

struct ParserException : public std::exception {

  const char *what() const throw() { return "Parser exception"; }
};

namespace binder {
class BinderContext;

class Parser {
public:
  Parser(BinderContext *context) : m_context(context), m_stmts(10){};
  ~Parser() = default;

  void parse(const memory::ResizableVector<Token> *tokens);
  // const autogen::Expr* getRoot() const {return m_root;}
  const memory::ResizableVector<autogen::Stmt *> &getStmts() const {
    return m_stmts;
  }

private:
  // private interface
  autogen::Expr *expression();
  autogen::Expr *assignment();
  autogen::Expr *orExpr();
  autogen::Expr *andExpr();
  autogen::Expr *equality();
  autogen::Expr *comparison();
  autogen::Expr *addition();
  autogen::Expr *multiplication();
  autogen::Expr *unary();
  autogen::Expr *primary();
  autogen::Stmt *statement();
  autogen::Stmt *declaration();
  autogen::Stmt *ifStatement();
  autogen::Stmt *printStatement();
  autogen::Stmt *whileStatement();
  autogen::Stmt *blockStatement();
  autogen::Stmt *expressionStatement();
  autogen::Stmt *varDeclaration();

  void syncronize();
  bool match(const TOKEN_TYPE *types, const int size);
  bool match(const TOKEN_TYPE type);
  bool check(TOKEN_TYPE type);
  const Token &advance();
  bool isAtEnd() const;
  const Token &peek() const;
  const Token &previous() const;
  const Token &consume(TOKEN_TYPE type, const char *message);
  ParserException error(const Token &token, const char *message);

private:
  int current = 0;
  const memory::ResizableVector<Token> *m_tokens;
  BinderContext *m_context = nullptr;
  memory::ResizableVector<autogen::Stmt *> m_stmts;
};

} // namespace binder
