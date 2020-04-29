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

inline void *toVoid(uint32_t index) {
  // using same type pointer rather than void* at least i am sure
  // alignment matches, next make sure the pointer is 0, otherwise
  // behaviour changes based on compiler, for example msvc, in debug ,
  // sets a not initialized pointer to 0xcccccc, but in general, good to
  // initialize it, i can't write afterwards to the hight 32bits due
  // to targetting WASM which currently 32bit
  uint32_t *result = 0;
  // just making sure i can actually fit a 32 bit value in here, probably
  // overkill but better be safe than sorry
  assert(sizeof(uint32_t *) >= 4);
  memcpy(&result, &index, sizeof(index));
  return result;
}

inline bool isAligned(void *pointer, size_t byte_count) {
  return (uintptr_t)pointer % byte_count == 0;
}

inline uint32_t toIndex(void *ptr) {

  uint32_t result;
  memcpy(&result, &ptr, sizeof(uint32_t));
  return result;
}

// error handling
const char *RuntimeValue::debugToString(BinderContext *context) {
  memory::StringPool &pool = context->getStringPool();
  char valueStr[50];
  const char *finalStrValue = valueStr;
  bool shouldFreeValueStr = false;
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
    finalStrValue = pool.concatenate("\"", "\"", string);
    shouldFreeValueStr = true;
    break;
  }
  default:
    assert(0 &&
           "unhandled value in runtime type, it is INVALID, report as bug");
  }

  char ff = memory::FREE_FIRST_AFTER_OPERATION;
  char fs = memory::FREE_SECOND_AFTER_OPERATION;
  char flags = shouldFreeValueStr ? fs | ff : ff;

  // ok now we have the value so we need to compose a message
  const char *temp = pool.concatenate("Runtime value with type ", " and value ",
                                      RUNTIME_TYPE_NAMES[(int)type]);
  return pool.concatenate(temp, finalStrValue, nullptr, flags);
}
const char *RuntimeValue::toString(BinderContext *context,
                                   bool trailingNewLine) {
  memory::StringPool &pool = context->getStringPool();
  // we might want to append a new line to force a flush in the printf,
  // done here it helps saving and extra concatenation
  if (!trailingNewLine) {
    switch (type) {
    case (RuntimeValueType::NUMBER): {
      char value[50];
      snprintf(value, 50, "%02.*f", context->getConfig().printFloatPrecision,
               number);
      return pool.allocate(value);
    }
    case (RuntimeValueType::BOOLEAN): {
      const char *value = boolean ? "true" : "false";
      return pool.allocate(value);
    }
    case (RuntimeValueType::NIL): {
      const char *value = "nil";
      return pool.allocate(value);
    }
    case (RuntimeValueType::STRING): {
      return pool.concatenate("\"", "\"", string);
    }
    default:
      assert(0 &&
             "unhandled value in runtime type, it is INVALID, report as bug");
    }
  } else {
    switch (type) {
    case (RuntimeValueType::NUMBER): {
      char value[50];
      snprintf(value, 50, "%02.*f\n", context->getConfig().printFloatPrecision,
               number);
      return pool.allocate(value);
    }
    case (RuntimeValueType::BOOLEAN): {
      const char *value = boolean ? "true\n" : "false\n";
      return pool.allocate(value);
    }
    case (RuntimeValueType::NIL): {
      const char *value = "nil\n";
      return pool.allocate(value);
    }
    case (RuntimeValueType::STRING): {
      return pool.concatenate("\"", "\"\n", string);
    }
    default:
      assert(0 &&
             "unhandled value in runtime type, it is INVALID, report as bug");
    }
  }
  return nullptr;
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

  const char *leftValue = left->debugToString(context);
  const char *rightValue = right->debugToString(context);

  const char ff = memory::FREE_FIRST_AFTER_OPERATION;
  const char fj = memory::FREE_JOINER_AFTER_OPERATION;
  const char fs = memory::FREE_SECOND_AFTER_OPERATION;

  const char *temp = pool.concatenate(base, " and \n left value: \n\t",
                                      getLexemeFromToken(op));
  // we can free temp and joiner
  temp = pool.concatenate(temp, "\n right value:\n\t", leftValue, ff | fj);
  // we can free both sides since are result of concatenationor build
  // by the debugToString (which result is in the pool)
  temp = pool.concatenate(temp, "\n", rightValue, ff | fs);

  return temp;
}

// TODO this will need to become graceful errors
void assertBinaryFull(RuntimeValue *left, RuntimeValue *right) {
  assert((left->type == RuntimeValueType::NUMBER) |
         (left->type == RuntimeValueType::STRING));
  assert((right->type == RuntimeValueType::NUMBER) |
         (right->type == RuntimeValueType::STRING));
}

bool isEqual(RuntimeValue *left, RuntimeValue *right) {
  // TODO handle null
  return left->number == right->number;
}

bool areBothNumbers(RuntimeValue *left, RuntimeValue *right) {
  return (left->type == RuntimeValueType::NUMBER) &
         (right->type == RuntimeValueType::NUMBER);
}

// visitor to evaluate  the code
class ASTInterpreterVisitor : public autogen::ExprVisitor,
                              public autogen::StmtVisitor {
public:
  ASTInterpreterVisitor(
      BinderContext *context,
      memory::SparseMemoryPool<RuntimeValue> *runtimeValuePool,
      Enviroment *enviroment)
      : autogen::ExprVisitor(), m_context(context),
        m_runtimeValuePool(runtimeValuePool), m_enviroment(enviroment){};

  virtual ~ASTInterpreterVisitor() = default;

  void setSuppressPrint(bool value) { m_suppressPrints = value; }

  // interface
  void *acceptAssign(autogen::Assign *expr) override {
    auto value = (RuntimeValue *)(evaluate(expr->value));
    RuntimeValue *runtime = getRuntime(toIndex(value));

    // if we have an R_value it will get stored in the variable
    // becoming an L_value
    if (runtime->storage == RuntimeValueStorage::R_VALUE) {
      runtime->storage = RuntimeValueStorage::L_VALUE;
    }

    bool result = m_enviroment->assign(expr->name, value);
    if (!result) {
      auto &pool = m_context->getStringPool();
      const char *message =
          pool.concatenate("Undefined variable: \"", "\"", expr->name);
      error(m_context, message);
    }
    return value;
  }

  void *acceptBinary(autogen::Binary *expr) override {

    // keeping as indexes until needed, in this way
    // we avoid pointer invalidation due to pool re-allocations
    uint32_t leftIdx = toIndex(evaluate(expr->left));
    uint32_t rightIdx = toIndex(evaluate(expr->right));

    RuntimeValue *left = getRuntime(leftIdx);
    RuntimeValue *right = getRuntime(rightIdx);

    // RuntimeValue *left = (RuntimeValue *)evaluate(expr->left);
    // RuntimeValue *right = (RuntimeValue *)evaluate(expr->right);

    // TODO handle null value
    // TODO after this operation we can probably deallocate the
    // right?
    assertBinaryFull(left, right);

    uint32_t index;
    RuntimeValue *returnValue =
        getReturnValueForBinary(leftIdx, left, rightIdx, right, index);
    switch (expr->op) {
    case (TOKEN_TYPE::MINUS): {
      if (!areBothNumbers(left, right)) {
        throw error(m_context, buildBinaryOperationError(m_context, left, right,
                                                         TOKEN_TYPE::MINUS));
      }
      returnValue->number = (left->number) - (right->number);
      returnValue->type = RuntimeValueType::NUMBER;
      freeBinaryValuesIfNecessary(returnValue, leftIdx, left, rightIdx, right);
      return toVoid(index);
    }
    case (TOKEN_TYPE::SLASH): {
      if (!areBothNumbers(left, right)) {
        throw error(m_context, buildBinaryOperationError(m_context, left, right,
                                                         TOKEN_TYPE::SLASH));
      }
      returnValue->number = (left->number) / (right->number);
      returnValue->type = RuntimeValueType::NUMBER;
      freeBinaryValuesIfNecessary(returnValue, leftIdx, left, rightIdx, right);
      return toVoid(index);
    }
    case (TOKEN_TYPE::STAR): {
      if (!areBothNumbers(left, right)) {
        throw error(m_context, buildBinaryOperationError(m_context, left, right,
                                                         TOKEN_TYPE::STAR));
      }
      returnValue->number = (left->number) * (right->number);
      returnValue->type = RuntimeValueType::NUMBER;
      freeBinaryValuesIfNecessary(returnValue, leftIdx, left, rightIdx, right);
      return toVoid(index);
    }
    case (TOKEN_TYPE::PLUS): {
      if (areBothNumbers(left, right)) {
        returnValue->number = (left->number) + (right->number);
      returnValue->type = RuntimeValueType::NUMBER;
        freeBinaryValuesIfNecessary(returnValue, leftIdx, left, rightIdx,
                                    right);
        return toVoid(index);
      } else if ((left->type == RuntimeValueType::STRING) &
                 (right->type == RuntimeValueType::STRING)) {
        // TODO figure out if it safe to free the strings
        // how to keep track of a concatenated string should i just
        // flush ad the end and not track for runtime concatenated strings?
        returnValue->string =
            m_context->getStringPool().concatenate(left->string, right->string);
        returnValue->type = RuntimeValueType::STRING;
        freeBinaryValuesIfNecessary(returnValue, leftIdx, left, rightIdx,
                                    right);
        return toVoid(index);
      } else {
        throw error(m_context, buildBinaryOperationError(m_context, left, right,
                                                         TOKEN_TYPE::PLUS));
      }
    }
      // comparison operatrions
    case (TOKEN_TYPE::GREATER): {
      if (!areBothNumbers(left, right)) {
        throw error(m_context, buildBinaryOperationError(m_context, left, right,
                                                         TOKEN_TYPE::GREATER));
      }
      returnValue->boolean = (left->number) > (right->number);
      returnValue->type = RuntimeValueType::BOOLEAN;
      freeBinaryValuesIfNecessary(returnValue, leftIdx, left, rightIdx, right);
      return toVoid(index);
    }
    case (TOKEN_TYPE::GREATER_EQUAL): {
      if (!areBothNumbers(left, right)) {
        throw error(m_context,
                    buildBinaryOperationError(m_context, left, right,
                                              TOKEN_TYPE::GREATER_EQUAL));
      }
      returnValue->boolean = (left->number) >= (right->number);
      returnValue->type = RuntimeValueType::BOOLEAN;
      freeBinaryValuesIfNecessary(returnValue, leftIdx, left, rightIdx, right);
      return toVoid(index);
    }
    case (TOKEN_TYPE::LESS): {
      if (!areBothNumbers(left, right)) {
        throw error(m_context, buildBinaryOperationError(m_context, left, right,
                                                         TOKEN_TYPE::LESS));
      }
      returnValue->boolean = (left->number) < (right->number);
      returnValue->type = RuntimeValueType::BOOLEAN;
      freeBinaryValuesIfNecessary(returnValue, leftIdx, left, rightIdx, right);
      return toVoid(index);
    }
    case (TOKEN_TYPE::LESS_EQUAL): {
      if (!areBothNumbers(left, right)) {
        throw error(m_context,
                    buildBinaryOperationError(m_context, left, right,
                                              TOKEN_TYPE::LESS_EQUAL));
      }
      returnValue->boolean = (left->number) <= (right->number);
      returnValue->type = RuntimeValueType::BOOLEAN;
      freeBinaryValuesIfNecessary(returnValue, leftIdx, left, rightIdx, right);
      return toVoid(index);
    }
    case (TOKEN_TYPE::BANG_EQUAL): {
      if (!areBothNumbers(left, right)) {
        throw error(m_context,
                    buildBinaryOperationError(m_context, left, right,
                                              TOKEN_TYPE::BANG_EQUAL));
      }
      returnValue->boolean = !isEqual(left, right);
      returnValue->type = RuntimeValueType::BOOLEAN;
      freeBinaryValuesIfNecessary(returnValue, leftIdx, left, rightIdx, right);
      return toVoid(index);
    }
    case (TOKEN_TYPE::EQUAL_EQUAL): {
      if (!areBothNumbers(left, right)) {
        throw error(m_context,
                    buildBinaryOperationError(m_context, left, right,
                                              TOKEN_TYPE::EQUAL_EQUAL));
      }
      returnValue->boolean = isEqual(left, right);
      returnValue->type = RuntimeValueType::BOOLEAN;
      freeBinaryValuesIfNecessary(returnValue, leftIdx, left, rightIdx, right);
      return toVoid(index);
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

    uint32_t index = 0;
    RuntimeValue &value = m_runtimeValuePool->getFreeMemoryData(index);
    value.storage = RuntimeValueStorage::R_VALUE;

    // we need to figure out what we are dealing with
    switch (expr->type) {
    case (TOKEN_TYPE::NUMBER): {
      value.number = strtod(expr->value, nullptr);
      value.type = RuntimeValueType::NUMBER;
      break;
    }

    case (TOKEN_TYPE::STRING): {
      value.string = expr->value;
      value.type = RuntimeValueType::STRING;
      break;
    }
    default: {
      throw error(m_context, buildLiteralErrorMessage(m_context, expr));
      break;
    }
    }

    return toVoid(index);
  }
  void *acceptUnary(autogen::Unary *expr) override {
    // we have an expression to evaluate, the right hand side
    uint32_t rightIdx = toIndex((evaluate(expr->right)));

    RuntimeValue *rightValue = getRuntime(rightIdx);
    // now we need to check the storage of the runtime
    // if is an R_ value we can steal it and re use the storage
    // otherwise we need to allocate
    RuntimeValue *returnValue = rightValue;
    if (rightValue->storage == RuntimeValueStorage::L_VALUE) {
      // we override the right idx so that will be returned
      // and the retuurn value will be used to set the reult of the
      // operation
      RuntimeValue &value = m_runtimeValuePool->getFreeMemoryData(rightIdx);
      // by default setting as R_VALUE since this will hold the result of the r
      // value
      value.storage = RuntimeValueStorage::R_VALUE;
      returnValue = &value;
    }

    switch (expr->op) {
    case (TOKEN_TYPE::MINUS): {
      // TODO temporary assert until we have proper runtime errors
      assert(rightValue->type == RuntimeValueType::NUMBER);
      returnValue->number = -rightValue->number;
      returnValue->type = rightValue->type;
      break;
    }
    case (TOKEN_TYPE::BANG): {
      bool result = isTruthy(rightValue);
      // TODO what to do if i am converting a string value to bool?
      // should i dealloc it? investigate
      returnValue->boolean = !result;
      returnValue->type = RuntimeValueType::BOOLEAN;
      break;
    }
    default: {
      assert(0 && "unhandled unary operator type");
      break;
    }
    }
    // no need to free anything we re-used the same value, modified in place
    return toVoid(rightIdx);
  }

  void *acceptVariable(autogen::Variable *expr) override {
    // we just straight up return the value which again
    // is a index in the pool masked as void*
    // whoever uses this value will properly convert back
    // to index and extract the real runtime value from it
    RuntimeValue *toReturn = nullptr;
    bool result = m_enviroment->get(expr->name, &toReturn);
    assert(result);
    return toReturn;
  }

  // statements
  void *acceptExpression(autogen::Expression *stmt) override {
    // we eval the side effect and free the expression
    uint32_t index = toIndex(evaluate(stmt->expression));
    releaseRuntime(index);
    return nullptr;
  };
  void *acceptPrint(autogen::Print *stmt) override {
    uint32_t index = toIndex(evaluate(stmt->expression));
    RuntimeValue *value = getRuntime(index);

    if (!m_suppressPrints) {
      const char *str = value->toString(m_context, true);
      m_context->print(str);
      m_context->getStringPool().free(str);
      releaseRuntime(index);
    }
    return nullptr;
  };

  void *acceptVar(autogen::Var *stmt) override {

    RuntimeValue *value = nullptr;
    if (stmt->initializer != nullptr) {
      // now this is really important, we don't deal with runtime value pointers
      // directly ever, unless we actually evaluate the value, when that happens
      // pointer is converted to pool index. This pointer should not be
      // dereferenced also check acceptVariable() to see usage
      value = (RuntimeValue *)evaluate(stmt->initializer);
    }
    // what is the storage of the variable?
    RuntimeValue *runtime = getRuntime(toIndex(value));
    // if we have an R_value it will get stored in the variable
    // becoming an L_value, if is already an L vlaue we store as
    // it is, meaning we point to the same runtime, for example
    // var a = 1;
    // var c = a;
    // a = 2;
    // c should be equal to 2, since they reference the same runtime value
    if (runtime->storage == RuntimeValueStorage::R_VALUE) {
      runtime->storage = RuntimeValueStorage::L_VALUE;
    }

    m_enviroment->define(stmt->token.m_lexeme, value);

    return nullptr;
  };

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

  RuntimeValue *getRuntime(uint32_t poolIdx) {
    return &(*m_runtimeValuePool)[poolIdx];
  }
  RuntimeValue *getReturnValueForBinary(uint32_t leftIdx, RuntimeValue *left,
                                        uint32_t rightIdx, RuntimeValue *right,
                                        uint32_t &index) {
    RuntimeValueStorage leftStorage = left->storage;
    RuntimeValueStorage rightStorage = right->storage;

    // if any of the two is R value we can re-use it
    if (leftStorage == RuntimeValueStorage::R_VALUE) {
      index = leftIdx;
      return left;
    }

    if (rightStorage == RuntimeValueStorage::R_VALUE) {
      index = rightIdx;
      return right;
    }
    // if none of the two is an rvalue we need to allocate
    RuntimeValue *value = &m_runtimeValuePool->getFreeMemoryData(index);
    value->storage = RuntimeValueStorage::R_VALUE;
    return value;
  }

  void freeBinaryValuesIfNecessary(RuntimeValue *returnValue, uint32_t leftIdx,
                                   RuntimeValue *left, uint32_t rightIdx,
                                   RuntimeValue *right) {

    // this should never happen since if this is the case we would reuse it
    // but just for symmetry
    // it is fine to compare the pointers since there is the chance we are
    // reusing values
    if ((returnValue != left) &
        (left->storage == RuntimeValueStorage::R_VALUE)) {
      m_runtimeValuePool->free(leftIdx);
    }
    if ((returnValue != right) &
        (right->storage == RuntimeValueStorage::R_VALUE)) {
      m_runtimeValuePool->free(rightIdx);
    }
  }

  void releaseRuntime(uint32_t poolIdx) { m_runtimeValuePool->free(poolIdx); }

private:
  BinderContext *m_context;
  memory::SparseMemoryPool<RuntimeValue> *m_runtimeValuePool;
  Enviroment *m_enviroment;
  bool m_suppressPrints = false;
};

RuntimeValue *ASTInterpreter::getRuntimeVariable(const char *variableName) {
  // this is a void* encoded pool index;
  RuntimeValue *runtime = nullptr;
  bool result = m_enviroment.get(variableName, &runtime);
  assert(result);
  uint32_t index = toIndex(runtime);
  // now we can convert to the actual runtime value
  return &m_pool[index];
}

void ASTInterpreter::interpret(
    const binder::memory::ResizableVector<autogen::Stmt *> &stmts) {
  // TODO can I propagate const to the accept? unluckily I don't think I can
  // I need to investigate

  // if issues happened at parser time, we should not reach this point
  // and exit earlier

  uint32_t count = stmts.size();
  assert(stmts.size() != 0);

  // TODO sure, thrwoing is easy to get out of recursion....but what about
  // heap memory? Here probably i want to use a pool to allocate the AST
  // nodes
  try {
    ASTInterpreterVisitor visitor(m_context, &m_pool, &m_enviroment);
    visitor.setSuppressPrint(m_suppressPrints);
    for (uint32_t i = 0; i < count; ++i) {
      stmts[i]->accept(&visitor);
    }
    // uint32_t index = toIndex(ASTRoot->accept(&visitor));
    // return &m_pool[index];
  } catch (RuntimeException e) {
  }
}

} // namespace binder
