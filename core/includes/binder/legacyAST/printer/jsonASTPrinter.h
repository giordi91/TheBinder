#pragma once

#include "binder/legacyAST/autogen/astgen.h"
#include "binder/memory/stringPool.h"
#include "binder/tokens.h"

namespace binder::printer {
class JSONASTPrinter : autogen::ExprVisitor, autogen::StmtVisitor {
public:
  JSONASTPrinter(memory::StringPool &pool)
      : autogen::ExprVisitor(), m_pool(pool){};
  virtual ~JSONASTPrinter() = default;
  // interface
  void *acceptAssign(autogen::Assign *expr) override {

    const char ff = binder::memory::FREE_FIRST_AFTER_OPERATION;
    const char fj = binder::memory::FREE_JOINER_AFTER_OPERATION;
    const char *tmp =
        m_pool.concatenate("{ \"type\" : \"assign\",\n \"expr\" : ", ",\n",
                           (char *)expr->value->accept(this),fj);

    tmp = m_pool.concatenate(tmp,expr->name,"\"name\" : \"" ,ff);
    tmp = m_pool.concatenate(tmp,"\"}\n",nullptr,ff);
    return (char *)tmp;
  };
  void *acceptBinary(autogen::Binary *expr) override {
    return parenthesize("binary", getLexemeFromToken(expr->op), expr->left,
                        expr->right);
  }
  void *acceptGrouping(autogen::Grouping *expr) override {
    return parenthesize("group", "group", expr->expr, nullptr);
  }
  void *acceptLiteral(autogen::Literal *expr) override {

    const char ff = binder::memory::FREE_FIRST_AFTER_OPERATION;

    const char *title = m_pool.concatenate("{ \"type\" : \"", "\"",
                                           getLexemeFromToken(expr->type));
    const char *value =
        m_pool.concatenate(",\"value\" :\"", "\"}", expr->value);
    return (char *)m_pool.concatenate(title, value, nullptr, ff);
  }
  void *acceptUnary(autogen::Unary *expr) override {
    return parenthesize("unary", getLexemeFromToken(expr->op), expr->right,
                        nullptr);
  }

  void *acceptVariable(autogen::Variable *expr) override {

    const char *body = m_pool.concatenate("{ \"type\" : \"var\", \"value\" :\"",
                                          "\"}", expr->name);
    return (char *)body;
  }

  const char *print(autogen::Stmt *stmt) {
    return (const char *)stmt->accept(this);
  }

  char *parenthesize(const char *type, const char *op, autogen::Expr *expr1,
                     autogen::Expr *expr2) {

    const char sf = binder::memory::FREE_SECOND_AFTER_OPERATION;
    const char ff = binder::memory::FREE_FIRST_AFTER_OPERATION;

    // write the type
    const char *title = m_pool.concatenate("{ \"type\" : \"", "\"", type);
    if (op != nullptr) {
      const char *opStr = m_pool.concatenate(",\"op\" :\"", "\"", op);
      title = m_pool.concatenate(title, opStr, nullptr, ff | sf);
    }

    const char *expr1str = (char *)expr1->accept(this);
    const char *left = m_pool.concatenate(title, ",\"left\": ", nullptr, ff);
    const char *expr1done =
        m_pool.concatenate(left, expr1str, nullptr, ff | sf);
    // TODO should clear this up and possibly remove the return in the if
    if (expr2 != nullptr) {
      // result from second expression
      char *expr2str = (char *)expr2->accept(this);

      // now we join with a space the first and second expression
      const char *right =
          m_pool.concatenate(expr1done, ",\"right\":", nullptr, ff);
      const char *expr2join =
          m_pool.concatenate(right, expr2str, nullptr, ff | sf);
      // finally we add a closing parent
      const char *expr2done = m_pool.concatenate(expr2join, "}", nullptr, ff);
      return (char *)expr2done;
    }
    return (char *)m_pool.concatenate(expr1done, "}", nullptr, ff);
  }

  void *acceptExpression(autogen::Expression *stmt) override {

    const char ff = binder::memory::FREE_FIRST_AFTER_OPERATION;
    const char fj = binder::memory::FREE_JOINER_AFTER_OPERATION;

    const char *expr = (char *)stmt->expression->accept(this);
    const char *tmp = m_pool.concatenate("{\"expr\" : ", ",\n", expr, fj);
    return (char *)m_pool.concatenate(tmp, "\"name\" :\"statement\"}\n",
                                      nullptr, ff);
  };
  void *acceptPrint(autogen::Print *stmt) override {

    const char ff = binder::memory::FREE_FIRST_AFTER_OPERATION;
    const char fj = binder::memory::FREE_JOINER_AFTER_OPERATION;

    const char *expr = (char *)stmt->expression->accept(this);
    const char *tmp = m_pool.concatenate("{\"expr\" : ", ",\n", expr, fj);
    return (char *)m_pool.concatenate(tmp, "\"name\" :\"print\"}\n", nullptr,
                                      ff);
  };

  void *acceptVar(autogen::Var *stmt) override {

    const char ff = binder::memory::FREE_FIRST_AFTER_OPERATION;
    const char fj = binder::memory::FREE_JOINER_AFTER_OPERATION;

    const char *expr = (char *)stmt->initializer->accept(this);
    const char *tmp = m_pool.concatenate("{\"expr\" : ", ",\n", expr, fj);
    return (char *)m_pool.concatenate(tmp, "\"name\" :\"var\"}\n", nullptr, ff);
  };

private:
  memory::StringPool &m_pool;
};

} // namespace binder::printer
