#include "binder/context.h"
#include "binder/interpreter.h"
#include "binder/parser.h"
#include "binder/printer/jsonASTPrinter.h"
#include "binder/scanner.h"
#include "stdlib.h"

#include "catch.h"

// TODO here we shoulud have possibly a print handler
// that would allow us to actually redirect the out and read it
// for now is good enough
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
    const binder::memory::ResizableVector<binder::autogen::Stmt *> &stmts =
        parser.getStmts();
    REQUIRE(stmts.size() != 0);
    // TODO not pretty the const cast, need to see what I can do about it
    interpreter.interpret(stmts);
    return nullptr;
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

TEST_CASE_METHOD(SetupInterpreterTestFixture, "print single number",
                 "[interpreter]") {

  interpreter.setSuppressPrint(true);
  const char *source = "print 12;";
  interpret(source);
  interpreter.setSuppressPrint(false);
}
TEST_CASE_METHOD(SetupInterpreterTestFixture, "basic variable ",
                 "[interpreter]") {

  interpreter.setSuppressPrint(true);
  const char *source = "var a = 12;";
  interpret(source);
  interpreter.setSuppressPrint(false);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "basic variable and print",
                 "[interpreter]") {

  interpreter.setSuppressPrint(true);
  const char *source = "var a = 12; print a;";
  interpret(source);
  interpreter.setSuppressPrint(false);
}

/*
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
    interpreter.flushMemory();
  }
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "random mult", "[interpreter]") {

  for (int i = 0; i < 2000; ++i) {
    double left = randfrom(-10000.0, 10000.0);
    double right = randfrom(-10000.0, 10000.0);
    char source[150];
    snprintf(source, 150, "%f * %f", left, right);
    binder::RuntimeValue *result = interpret(source);
    REQUIRE(result->type == binder::RuntimeValueType::NUMBER);
    REQUIRE(result->number == Approx(left * right));
    interpreter.flushMemory();
  }
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "random mad unary",
                 "[interpreter]") {

  for (int i = 0; i < 2000; ++i) {
    double left = randfrom(-10000.0, 10000.0);
    double right = randfrom(-10000.0, 10000.0);
    double add = randfrom(-10000.0, 10000.0);
    char source[200];
    snprintf(source, 200, "-((%f * %f) + %f)", left, right, add);
    binder::RuntimeValue *result = interpret(source);
    REQUIRE(result->type == binder::RuntimeValueType::NUMBER);
    REQUIRE(result->number == Approx(-((left * right) + add)));
    interpreter.flushMemory();
  }
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "expression 1", "[interpreter]") {

  binder::RuntimeValue *result = interpret("(-1*3.14)+(--13)");
  REQUIRE(result->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(result->number == Approx(9.86));
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "error binary runtime mul bool",
                 "[interpreter]") {

  context.setErrorReportingEnabled(false);
  binder::RuntimeValue *result = interpret("-1 * true");
  REQUIRE(result == nullptr);
  REQUIRE(context.hadError() == true);
  context.setErrorReportingEnabled(true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "error binary runtime mul str",
                 "[interpreter]") {

  context.setErrorReportingEnabled(false);
  binder::RuntimeValue *result = interpret("-1 * \"t\"");
  REQUIRE(result == nullptr);
  REQUIRE(context.hadError() == true);
  context.setErrorReportingEnabled(true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "error binary runtime divide",
                 "[interpreter]") {

  context.setErrorReportingEnabled(false);
  binder::RuntimeValue *result = interpret("11.2 / \"error\"");
  REQUIRE(result == nullptr);
  REQUIRE(context.hadError() == true);
  context.setErrorReportingEnabled(true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "error binary runtime minus",
                 "[interpreter]") {

  context.setErrorReportingEnabled(false);
  binder::RuntimeValue *result = interpret("10 - \"minus!\"");
  REQUIRE(result == nullptr);
  REQUIRE(context.hadError() == true);
  context.setErrorReportingEnabled(true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "error binary runtime add str",
                 "[interpreter]") {

  context.setErrorReportingEnabled(false);
  binder::RuntimeValue *result = interpret("15.201 + \"letsadd\"");
  REQUIRE(result == nullptr);
  REQUIRE(context.hadError() == true);
  context.setErrorReportingEnabled(true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "error binary runtime add str 2",
                 "[interpreter]") {

  context.setErrorReportingEnabled(false);
  binder::RuntimeValue *result = interpret(" \"letsadd\" + 2222");
  REQUIRE(result == nullptr);
  REQUIRE(context.hadError() == true);
  context.setErrorReportingEnabled(true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "runtime add str str",
                 "[interpreter]") {

  binder::RuntimeValue *result = interpret(" \"hello \" + \"world\"");
  REQUIRE(result != nullptr);
  REQUIRE(context.hadError() == false);
  REQUIRE(result->type == binder::RuntimeValueType::STRING);
  REQUIRE(strcmp(result->string, "hello world") == 0);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "runtime > str",
                 "[interpreter]") {

  context.setErrorReportingEnabled(false);
  binder::RuntimeValue *result = interpret(" 12 > \"hello\"");
  REQUIRE(result == nullptr);
  REQUIRE(context.hadError() == true);
  context.setErrorReportingEnabled(true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "runtime >= str",
                 "[interpreter]") {

  context.setErrorReportingEnabled(false);
  binder::RuntimeValue *result = interpret(" 12 >= \"hello\"");
  REQUIRE(result == nullptr);
  REQUIRE(context.hadError() == true);
  context.setErrorReportingEnabled(true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "runtime < str",
                 "[interpreter]") {

  context.setErrorReportingEnabled(false);
  binder::RuntimeValue *result = interpret(" 12 < \"hello\"");
  REQUIRE(result == nullptr);
  REQUIRE(context.hadError() == true);
  context.setErrorReportingEnabled(true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "runtime <= str",
                 "[interpreter]") {

  context.setErrorReportingEnabled(false);
  binder::RuntimeValue *result = interpret(" 12 <= \"hello\"");
  REQUIRE(result == nullptr);
  REQUIRE(context.hadError() == true);
  context.setErrorReportingEnabled(true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "runtime != str",
                 "[interpreter]") {

  context.setErrorReportingEnabled(false);
  binder::RuntimeValue *result = interpret(" 12 != \"hello\"");
  REQUIRE(result == nullptr);
  REQUIRE(context.hadError() == true);
  context.setErrorReportingEnabled(true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "runtime == str",
                 "[interpreter]") {

  context.setErrorReportingEnabled(false);
  binder::RuntimeValue *result = interpret(" 12 == \"hello\"");
  REQUIRE(result == nullptr);
  REQUIRE(context.hadError() == true);
  context.setErrorReportingEnabled(true);
}

*/
