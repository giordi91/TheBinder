#include "binder/autogen/astgen.h"
#include "binder/interpreter.h"
#include "binder/context.h"
#include "binder/memory/stringPool.h"
#include <stdlib.h>

namespace binder {

// TODO this will need to become graceful errors
void assertBinaryFull(RuntimeValue *left, RuntimeValue *right) {
  assert((left->type == RuntimeValueType::NUMBER) |
         (left->type == RuntimeValueType::STRING));
  assert((right->type == RuntimeValueType::NUMBER |
          right->type == RuntimeValueType::STRING));
}

void assertBinaryNumber(RuntimeValue *left, RuntimeValue *right) {
  assert((left->type == RuntimeValueType::NUMBER));
  assert(right->type == RuntimeValueType::NUMBER);
}

bool isEqual(RuntimeValue *left, RuntimeValue *right)
{
    //TODO handle null
    return left->number == right->number;
}


class ASTInterpreterVisitor : public autogen::Visitor {
public:
  ASTInterpreterVisitor(memory::StringPool &pool)
      : autogen::Visitor(), m_pool(pool){};
  virtual ~ASTInterpreterVisitor() = default;
  // interface
  void *acceptBinary(autogen::Binary *expr) override {
    // TODO proper casting
    RuntimeValue *left = (RuntimeValue *)evaluate(expr->left);
    RuntimeValue *right = (RuntimeValue *)evaluate(expr->right);

    // TODO handle null value
    // TODO after this operation we can probably deallocate the
    // right?
    // for now supporting only numbers
    assertBinaryFull(left, right);
    switch (expr->op) {
    case (TOKEN_TYPE::MINUS): {
      assertBinaryNumber(left, right);
      left->number = (left->number) - (right->number);
      return left;
    }
    case (TOKEN_TYPE::SLASH): {
      assertBinaryNumber(left, right);
      left->number = (left->number) / (right->number);
      return left;
    }
    case (TOKEN_TYPE::STAR): {
      assertBinaryNumber(left, right);
      left->number = (left->number) * (right->number);
      return left;
    }
    case (TOKEN_TYPE::PLUS): {
      if ((left->type == RuntimeValueType::NUMBER) &
          (right->type == RuntimeValueType::NUMBER)) {
        left->number = (left->number) + (right->number);
        return left;
      } else if ((left->type == RuntimeValueType::STRING) &
                 (right->type == RuntimeValueType::STRING)) {
        // TODO figure out if it safe to free the strings
        left->string = m_pool.concatenate(left->string, right->string);
        return left;
      } else {
        assert(0 && "cannot mix strings and numbers for + operation");
        return nullptr;
      }
    }
                             //comparison operatrions
    case (TOKEN_TYPE::GREATER): {
      assertBinaryNumber(left, right);
      left->number = (left->number) > (right->number);
      return left;
    }
    case (TOKEN_TYPE::GREATER_EQUAL): {
      assertBinaryNumber(left, right);
      left->number = (left->number) >= (right->number);
      return left;
    }
    case (TOKEN_TYPE::LESS): {
      assertBinaryNumber(left, right);
      left->number = (left->number) < (right->number);
      return left;
    }
    case (TOKEN_TYPE::LESS_EQUAL): {
      assertBinaryNumber(left, right);
      left->number = (left->number) <= (right->number);
      return left;
    }
    case (TOKEN_TYPE::BANG_EQUAL): {
      assertBinaryNumber(left, right);
      left->boolean= !isEqual(left, right);
      left->type = RuntimeValueType::BOOLEAN;
      return left;
    }
    case (TOKEN_TYPE::EQUAL_EQUAL): {
      assertBinaryNumber(left, right);
      left->boolean= isEqual(left, right);
      left->type = RuntimeValueType::BOOLEAN;
      return left;
    }


    default:
      assert(0 && "operator not supported in binary evaluation");
      return nullptr;
    }
  }

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
      // TODO temporary assert until we have proper runtime errors
      assert(rightValue->type == RuntimeValueType::NUMBER);
      rightValue->number = -rightValue->number;
      break;
    }
    case (TOKEN_TYPE::BANG): {
      bool result = isTruthy(rightValue);
      // TODO what to do if i am converting a string value to bool?
      // should i dealloc it? investigate
      rightValue->boolean = !result;
      break;
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
  bool isTruthy(RuntimeValue *value) {

    if (value->type == RuntimeValueType::NIL) {
      return false;
    }
    if (value->type == RuntimeValueType::BOOLEAN) {
      return value->boolean;
    }
    // everything else is considered true
    return true;
  }

private:
  memory::StringPool &m_pool;
};

RuntimeValue *ASTInterpreter::interpret(autogen::Expr *ASTRoot) { 
    //TODO can I propagate const to the accept? unluckily I don't think I can
    //I need to investigate
    ASTInterpreterVisitor visitor(m_context->getStringPool());
    return (RuntimeValue*)ASTRoot->accept(&visitor);
}

} // namespace binder
