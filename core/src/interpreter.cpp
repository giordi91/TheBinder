#include "binder/autogen/astgen.h"
#include "binder/context.h"
#include "binder/interpreter.h"
#include "binder/memory/stringPool.h"
#include <exception>
#include <stdio.h>
#include <stdlib.h>

namespace binder {

static const char *RUNTIME_TYPE_NAMES[] = {"NUMBER", "STRING", "NIL", "BOOLEAN",
                                           "INVALID"};
// error handling
const char *RuntimeValue::toString(BinderContext *context) {
  memory::StringPool &pool = context->getStringPool();
  char valueStr[50];
  const char *finalStrValue = valueStr;
  switch (type) {
  case (RuntimeValueType::NUMBER): {
    snprintf(valueStr, 50, "%f", number);
    break;
  }
  case (RuntimeValueType::BOOLEAN): {
    finalStrValue = boolean ? "true" : "false";
    break;
  }
  case (RuntimeValueType::NIL): {
    finalStrValue = "nil";
    break;
  }
  case (RuntimeValueType::STRING): {
    finalStrValue = string;
    break;
  }
  default:
    assert(0 &&
           "unhandled value in runtime type, it is INVALID, report as bug");
  }

  // ok now we have the value so we need to compose a message
  const char *temp = pool.concatenate("Runtime value with type ", " and value ",
                                      RUNTIME_TYPE_NAMES[(int)type]);
  return pool.concatenate(temp, finalStrValue, nullptr,
                          memory::FREE_FIRST_AFTER_OPERATION);
}

struct RuntimeException : public std::exception {

  const char *what() const throw() { return "Runtime exception"; }
};

RuntimeException error(BinderContext *context, const char *message) {
  context->reportError(-1, message);
  // freeing the message, must always be pool allocate
  context->getStringPool().free(message);
  return RuntimeException();
}

const char *buildLiteralErrorMessage(BinderContext *context,
                                     autogen::Literal *value) {
  const char *valueType;
  switch (value->type) {
  case (TOKEN_TYPE::NIL): {
    valueType = "NULL";
    break;
  }

  case (TOKEN_TYPE::BOOL_TRUE):
  case (TOKEN_TYPE::BOOL_FALSE): {
    valueType = "BOOL";
    break;
  }
  default:
    assert(0 && "literal unexpected value abort...");
  }

  return context->getStringPool().concatenate(
      "Expected NUMBER or STRING in literal operation got: ", valueType);
}

const char *buildBinaryOperationError(BinderContext *context,
                                      RuntimeValue *left, RuntimeValue *right,
                                      TOKEN_TYPE op) {

  memory::StringPool &pool = context->getStringPool();
  const char *base = "Cannot perform binary operation with operator ";
  assert(left->type != RuntimeValueType::INVALID);
  assert(right->type != RuntimeValueType::INVALID);

  const char *leftValue = left->toString(context);
  const char *rightValue = right->toString(context);

  const char ff = memory::FREE_FIRST_AFTER_OPERATION;
  const char fj = memory::FREE_JOINER_AFTER_OPERATION;
  const char fs = memory::FREE_SECOND_AFTER_OPERATION;

  const char *temp =
      pool.concatenate(base, "and \n left value: \n\t", getLexemeFromToken(op));
  //we can free temp and joiner
  temp = pool.concatenate(temp, "\n right value:\n\t",leftValue, ff|fj);
  //we can free both sides since are result of concatenationor build
  //by the toString (which result is in the pool)
  temp = pool.concatenate(temp, "\n", rightValue, ff|fs);

  return temp;
}

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

bool isEqual(RuntimeValue *left, RuntimeValue *right) {
  // TODO handle null
  return left->number == right->number;
}

// visitor to evaluate  the code
class ASTInterpreterVisitor : public autogen::Visitor {
public:
  ASTInterpreterVisitor(BinderContext *context)
      : autogen::Visitor(), m_context(context){};
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
      // assertBinaryNumber(left, right);
      if (left->type != right->type) {
        throw error(m_context, buildBinaryOperationError(m_context, left, right,
                                                         TOKEN_TYPE::STAR));
      }
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
        left->string =
            m_context->getStringPool().concatenate(left->string, right->string);
        return left;
      } else {
        assert(0 && "cannot mix strings and numbers for + operation");
        return nullptr;
      }
    }
      // comparison operatrions
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
      left->boolean = !isEqual(left, right);
      left->type = RuntimeValueType::BOOLEAN;
      return left;
    }
    case (TOKEN_TYPE::EQUAL_EQUAL): {
      assertBinaryNumber(left, right);
      left->boolean = isEqual(left, right);
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
      value->type = RuntimeValueType::NUMBER;
      break;
    }

    case (TOKEN_TYPE::STRING): {
      value->string = expr->value;
      value->type = RuntimeValueType::STRING;
      break;
    }
    default: {
      throw error(m_context, buildLiteralErrorMessage(m_context, expr));
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
  BinderContext *m_context;
};

RuntimeValue *ASTInterpreter::interpret(autogen::Expr *ASTRoot) {
  // TODO can I propagate const to the accept? unluckily I don't think I can
  // I need to investigate

  // if issues happened at parser time, we should not reach this point
  // and exit earlier
  assert(ASTRoot != nullptr);

  try {
    ASTInterpreterVisitor visitor(m_context);
    return (RuntimeValue *)ASTRoot->accept(&visitor);
  } catch (RuntimeException e) {
    return nullptr;
  }
}

} // namespace binder
