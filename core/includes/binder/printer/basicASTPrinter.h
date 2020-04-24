#pragma once
#include "binder/autogen/astgen.h"
#include "binder/memory/stringPool.h"
#include "binder/tokens.h"

namespace binder::printer {
class BasicASTPrinter : autogen::ExprVisitor {
public:
  BasicASTPrinter(memory::StringPool &pool)
      : autogen::ExprVisitor(), m_pool(pool){};
  virtual ~BasicASTPrinter() = default;
  // interface
  void *acceptBinary(autogen::Binary *expr) override {
    return parenthesize(getLexemeFromToken(expr->op), expr->left, expr->right);
  }
  void *acceptGrouping(autogen::Grouping *expr) override {
    return parenthesize("group", expr->expr, nullptr);
  }
  void *acceptLiteral(autogen::Literal *expr) override {
    return (void *)expr->value;
  }
  void *acceptUnary(autogen::Unary *expr) override {
    return parenthesize(getLexemeFromToken(expr->op), expr->right, nullptr);
  }
  void *acceptVariable(autogen::Variable *expr) override {
    return  (void*)expr->name;
  }

  const char *print(autogen::Expr *expr) {
    return (const char *)expr->accept(this);
  }

  char *parenthesize(const char *name, autogen::Expr *expr1,
                     autogen::Expr *expr2) {

    // write the name
    const char *title = m_pool.concatenate("(", " ", name);
    char *expr1str = (char *)expr1->accept(this);
    const char flags = binder::memory::FREE_FIRST_AFTER_OPERATION |
                       binder::memory::FREE_SECOND_AFTER_OPERATION;
    const char *expr1done = m_pool.concatenate(title, expr1str, nullptr, flags);
    if (expr2 != nullptr) {
      // result from second expression
      char *expr2str = (char *)expr2->accept(this);

      //now we join with a space the first and second expression
      const char *expr2join =
          m_pool.concatenate(expr1done, expr2str, " ", flags);
      //finally we add a closing parent
      const char *expr2done = m_pool.concatenate(
          expr2join, ")", nullptr, binder::memory::FREE_FIRST_AFTER_OPERATION);
      return (char *)expr2done;
    }
    return (char *)m_pool.concatenate(
        expr1done, ")", nullptr, binder::memory::FREE_FIRST_AFTER_OPERATION);
  }

private:
  memory::StringPool &m_pool;
};

} // namespace binder::printer
