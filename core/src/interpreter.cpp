#include "binder/autogen/astgen.h"
#include "binder/interpreter.h"
#include "binder/memory/stringPool.h"
#include <stdlib.h>

namespace binder {

class ASTInterpreterVisitor : autogen::Visitor {
public:
  ASTInterpreterVisitor(memory::StringPool &pool)
      : autogen::Visitor(), m_pool(pool){};
  virtual ~ASTInterpreterVisitor() = default;
  // interface
  void *acceptBinary(autogen::Binary *expr) override { return nullptr; }
  void *acceptGrouping(autogen::Grouping *expr) override {
    // simply unwrap the grouping
    return evaluate(expr->expr);
  }
  void *acceptLiteral(autogen::Literal *expr) override {

    // TODO naked alloc
    RuntimeValue *value = new RuntimeValue();
    // we need to figure out what we are dealing with
    switch (expr->type) {
    case (TOKEN_TYPE::NUMBER): {
      value->number = strtod(expr->value, nullptr);
      break;
    }

    case (TOKEN_TYPE::STRING): {
      value->string = expr->value;
      break;
    }
    default: {
      assert(0 && "unhandled literal type");
      break;
    }
    }

    return value;
  }
  void *acceptUnary(autogen::Unary *expr) override {
    // we have an expression to evaluate, the right hand side
    RuntimeValue *rightValue =
        reinterpret_cast<RuntimeValue *>(evaluate(expr->right));

    switch (expr->op) {
    case (TOKEN_TYPE::MINUS): {
        //TODO temporary assert until we have proper runtime errors
        assert(rightValue->type == RuntimeValueType::NUMBER);
        rightValue->number = -rightValue->number;
    }
    case (TOKEN_TYPE::BANG): {
        bool result = isTruthy(rightValue);
        //TODO what to do if i am converting a string value to bool? 
        //should i dealloc it? investigate
        rightValue->boolean = !result;
     }
    default: {
      assert(0 && "unhandled unary operator type");
      break;
    }
    }

    return rightValue;
  }

  const char *interpret(autogen::Expr *expr) {
    return (const char *)expr->accept(this);
  }

private:
  void *evaluate(autogen::Expr *expr) {
    // re-forward the inner expresion to the visitor;
    return expr->accept(this);
  }
  bool isTruthy(RuntimeValue* value)
  {

      if(value->type == RuntimeValueType::NIL)
      {
          return false;
      }
      if(value->type == RuntimeValueType::BOOLEAN)
      {
          return value->boolean;
      }
      //everything else is considered true
      return true;
  }

private:
  memory::StringPool &m_pool;
};

const char *ASTInterpreter::interpret(autogen::Expr *ASTRoot) { return ""; }

} // namespace binder
