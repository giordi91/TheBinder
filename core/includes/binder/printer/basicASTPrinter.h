#pragma once
#include "binder/autogen/astgen.h"
#include "binder/memory/stringPool.h"
#include "binder/tokens.h"

namespace binder::printer {
class BasicASTPrinter : autogen::Visitor<const char *> {
public:
  BasicASTPrinter(memory::StringPool &pool)
      : autogen::Visitor<const char *>(), m_pool(pool){};
  virtual ~BasicASTPrinter() = default;
  // interface
  const char *acceptBinary(autogen::Binary<const char *> *expr) override {
    return parenthesize(getLexemeFromToken(expr->op), expr->left, expr->right);
  }
  const char *acceptGrouping(autogen::Grouping<const char *> *expr) override {
    return parenthesize("grouping", expr->expr, nullptr);
  }
  const char *acceptLiteral(autogen::Literal<const char *> *expr) override {
    return expr->value;
  }
  const char *acceptUnary(autogen::Unary<const char *> *expr) override {
    return parenthesize(getLexemeFromToken(expr->op), expr->right, nullptr);
  }

  const char *print(autogen::Expr<const char *> *expr) {
    return expr->accept(this);
  }

  const char *parenthesize(const char *name, autogen::Expr<const char *> *expr1,
                           autogen::Expr<const char *> *expr2) {

    // write the name
    const char *title = m_pool.concatenate("(", " ", name);
    const char *expr1str = expr1->accept(this);
    const char *expr1done = m_pool.concatenate(title, expr1str);
    if(expr2 != nullptr)
    {
        const char *expr2str = expr2->accept(this);
        const char *expr2done = m_pool.concatenate( expr1done,")",expr2str);
        return expr2done;
    }
    return m_pool.concatenate(expr1done,")");
  }

private:
  memory::StringPool &m_pool;
};

} // namespace binder::printer
