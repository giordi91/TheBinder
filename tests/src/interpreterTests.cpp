#include "binder/context.h"
#include "binder/interpreter.h"
#include "binder/parser.h"
#include "binder/printer/jsonASTPrinter.h"
#include "binder/scanner.h"
#include "stdlib.h"

#include "catch.h"

class SetupInterpreterTestFixture {
public:
  SetupInterpreterTestFixture()
      : context({}), scanner(&context), parser(&context),
        interpreter(&context) {}

  binder::RuntimeValue *interpret(const char *source) {

    scanner.scan(source);
    const binder::memory::ResizableVector<binder::Token> &tokens =
        scanner.getTokens();
    parser.parse(&tokens);
    const binder::autogen::Expr *root = parser.getRoot();
    // TODO not pretty the const cast, need to see what I can do about it
    return interpreter.interpret((binder::autogen::Expr *)root);
  }

protected:
  binder::BinderContext context;
  binder::Scanner scanner;
  binder::Parser parser;
  binder::ASTInterpreter interpreter;
};

double randfrom(double min, double max) {
  double range = (max - min);
  double div = RAND_MAX / range;
  return min + (rand() / div);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "single number",
                 "[interpreter]") {

  const char *source = "12";
  binder::RuntimeValue *result = interpret(source);
  REQUIRE(result->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(result->number == Approx(12));
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "single unary", "[interpreter]") {

  const char *source = "-12";
  binder::RuntimeValue *result = interpret(source);
  REQUIRE(result->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(result->number == Approx(-12));
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "random unary", "[interpreter]") {

  for (int i = 0; i < 2000; ++i) {
    double value = randfrom(-10000.0, 10000.0);
    char source[50];
    snprintf(source, 50, "-%f", value);
    binder::RuntimeValue *result = interpret(source);
    REQUIRE(result->type == binder::RuntimeValueType::NUMBER);
    REQUIRE(result->number == Approx(-value));
  }
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "random mult", "[interpreter]") {

  for (int i = 0; i < 2000; ++i) {
    double left = randfrom(-10000.0, 10000.0);
    double right = randfrom(-10000.0, 10000.0);
    char source[150];
    snprintf(source, 150, "%f * %f", left,right);
    binder::RuntimeValue *result = interpret(source);
    REQUIRE(result->type == binder::RuntimeValueType::NUMBER);
    REQUIRE(result->number == Approx(left*right));
  }
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "random mad unary", "[interpreter]") {

  for (int i = 0; i < 2000; ++i) {
    double left = randfrom(-10000.0, 10000.0);
    double right = randfrom(-10000.0, 10000.0);
    double add = randfrom(-10000.0, 10000.0);
    char source[200];
    snprintf(source, 200, "-((%f * %f) + %f)", left,right,add);
    binder::RuntimeValue *result = interpret(source);
    REQUIRE(result->type == binder::RuntimeValueType::NUMBER);
    REQUIRE(result->number == Approx(-((left*right) + add)));
  }
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "expression 1", "[interpreter]") {

  const char *source = "(-1*3.14)+(--13)";
  binder::RuntimeValue *result = interpret(source);
  REQUIRE(result->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(result->number == Approx(9.86));
}
