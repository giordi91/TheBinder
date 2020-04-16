#pragma once

#include "binder/autogen/astgen.h"
#include "binder/memory/stringPool.h"
#include "binder/tokens.h"

namespace binder::printer {
class JSONASTPrinter : autogen::Visitor {
public:
  JSONASTPrinter(memory::StringPool &pool) : autogen::Visitor(), m_pool(pool){};
  virtual ~JSONASTPrinter() = default;
  // interface
  void *acceptBinary(autogen::Binary *expr) override {
    return parenthesize("binary", getLexemeFromToken(expr->op), expr->left,
                        expr->right);
  }
  void *acceptGrouping(autogen::Grouping *expr) override {
    return parenthesize("group", "group", expr->expr, nullptr);
  }
  void *acceptLiteral(autogen::Literal *expr) override {

    const char *title = m_pool.concatenate("{ \"type\" : \"", "\"",
                                           getLexemeFromToken(expr->type));
    const char* value = m_pool.concatenate(",\"value\" :\"", "\"}", expr->value);
    return (char*) m_pool.concatenate(title,value);
  }
  void *acceptUnary(autogen::Unary *expr) override {
    return parenthesize("unary", getLexemeFromToken(expr->op), expr->right,
                        nullptr);
  }

  const char *print(autogen::Expr *expr) {
    return (const char *)expr->accept(this);
  }

  char *parenthesize(const char *type, const char *op, autogen::Expr *expr1,
                     autogen::Expr *expr2) {

    // write the name
    const char *title = m_pool.concatenate("{ \"type\" : \"", "\"", type);
    if (op != nullptr) {
      const char* opStr = m_pool.concatenate(",\"op\" :\"", "\"", op);
      title = m_pool.concatenate(title, opStr);
    }

    const char *expr1str = (char *)expr1->accept(this);
    // const char flags = binder::memory::FREE_FIRST_AFTER_OPERATION |
    //                   binder::memory::FREE_SECOND_AFTER_OPERATION;
    const char *left = m_pool.concatenate(title, ",\"left\": ");
    const char *expr1done = m_pool.concatenate(left, expr1str);
    if (expr2 != nullptr) {
      // result from second expression
      char *expr2str = (char *)expr2->accept(this);

      // now we join with a space the first and second expression
      const char *right = m_pool.concatenate(expr1done, ",\"right\":");
      const char *expr2join = m_pool.concatenate(right, expr2str);
      // finally we add a closing parent
      const char *expr2done = m_pool.concatenate(expr2join, "}");
      return (char *)expr2done;
    }
    return (char *)m_pool.concatenate(expr1done, "}");
  }

private:
  memory::StringPool &m_pool;
};

} // namespace binder::printer
