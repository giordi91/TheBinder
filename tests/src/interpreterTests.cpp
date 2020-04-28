#include "binder/context.h"
#include "binder/interpreter.h"
#include "binder/log/bufferLog.h"
#include "binder/parser.h"
#include "binder/printer/jsonASTPrinter.h"
#include "binder/scanner.h"
#include "stdlib.h"

#include "catch.h"

class SetupInterpreterTestFixture {
public:
  SetupInterpreterTestFixture()
      : context({32, binder::LOGGER_TYPE::BUFFERED, 5}), scanner(&context),
        parser(&context), interpreter(&context) {}

  binder::RuntimeValue *interpret(const char *source) {

    // flush the logger
    context.getLogger()->flush();

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

  const char *getOutput() {
    return ((binder::log::BufferedLog *)context.getLogger())->getBuffer();
  };

protected:
  binder::BinderContext context;
  binder::Scanner scanner;
  binder::Parser parser;
  binder::ASTInterpreter interpreter;
};

inline uint32_t VoidtoIndex(void *ptr) {

  uint32_t result;
  memcpy(&result, &ptr, sizeof(uint32_t));
  return result;
}
double randfrom(double min, double max) {
  double range = (max - min);
  double div = RAND_MAX / range;
  return min + (rand() / div);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "print single number",
                 "[interpreter]") {

  const char *source = "print 12;";
  interpret(source);
  const char *out = getOutput();
  REQUIRE(strcmp(out, "12.00000\n") == 0);
}
TEST_CASE_METHOD(SetupInterpreterTestFixture, "basic variable ",
                 "[interpreter]") {

  const char *source = "var a = 12;";
  interpret(source);
  binder::RuntimeValue *value = interpreter.getRuntimeVariable("a");
  REQUIRE(value != nullptr);
  REQUIRE(value->number == Approx(12.0));
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "basic variable and print",
                 "[interpreter]") {

  const char *source = "var a = 12; print a;";
  interpret(source);
  const char *out = getOutput();
  REQUIRE(strcmp(out, "12.00000\n") == 0);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "assign and print re-assign",
                 "[interpreter]") {

  const char *source = "var a = 1; print a = 2;";
  interpret(source);
  const char *out = getOutput();
  REQUIRE(strcmp(out, "2.00000\n") == 0);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "single unary", "[interpreter]") {

  const char *source = "var test = -111;";
  interpret(source);
  binder::RuntimeValue *value = interpreter.getRuntimeVariable("test");
  REQUIRE(value != nullptr);
  REQUIRE(value->number == Approx(-111.0));
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "random unary", "[interpreter]") {

  for (int i = 0; i < 2000; ++i) {
    double value = randfrom(-10000.0, 10000.0);
    char source[100];
    // TODO expand to random variable and variable legnth?
    snprintf(source, 100, "var x = -%f;\n", value);
    interpret(source);
    binder::RuntimeValue *result = interpreter.getRuntimeVariable("x");
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
    snprintf(source, 150, "var ff = %f * %f;", left, right);
    interpret(source);
    binder::RuntimeValue *result = interpreter.getRuntimeVariable("ff");
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
    snprintf(source, 200, "var fdsf = -((%f * %f) + %f);", left, right, add);
    interpret(source);
    binder::RuntimeValue *result = interpreter.getRuntimeVariable("fdsf");
    REQUIRE(result->type == binder::RuntimeValueType::NUMBER);
    REQUIRE(result->number == Approx(-((left * right) + add)));
    interpreter.flushMemory();
  }
}
TEST_CASE_METHOD(SetupInterpreterTestFixture, "greater ", "[interpreter]") {

  interpret("var myExpr = 10 > 1;");
  binder::RuntimeValue *result = interpreter.getRuntimeVariable("myExpr");
  REQUIRE(result->type == binder::RuntimeValueType::BOOLEAN);
  REQUIRE(result->boolean== true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "less", "[interpreter]") {

  interpret("var myExpr = 10 < 1;");
  binder::RuntimeValue *result = interpreter.getRuntimeVariable("myExpr");
  REQUIRE(result->type == binder::RuntimeValueType::BOOLEAN);
  REQUIRE(result->boolean== false);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "greater equal", "[interpreter]") {

  interpret("var myExpr = 10 >= 1;");
  binder::RuntimeValue *result = interpreter.getRuntimeVariable("myExpr");
  REQUIRE(result->type == binder::RuntimeValueType::BOOLEAN);
  REQUIRE(result->boolean== true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "less equal", "[interpreter]") {

  interpret("var myExpr = 10 <= 1;");
  binder::RuntimeValue *result = interpreter.getRuntimeVariable("myExpr");
  REQUIRE(result->type == binder::RuntimeValueType::BOOLEAN);
  REQUIRE(result->boolean== false );
}


TEST_CASE_METHOD(SetupInterpreterTestFixture, "expression 1", "[interpreter]") {

  interpret("var myExpr = (-1*3.14)+(--13);");
  binder::RuntimeValue *result = interpreter.getRuntimeVariable("myExpr");
  REQUIRE(result->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(result->number == Approx(9.86));
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "error binary runtime mul bool",
                 "[interpreter]") {

  // here we could log against a specific error but error messages might change
  // a lot so unless specific reason we just expect a gracefull error
  interpret("var myExpr121 = -1 * true;");
  REQUIRE(context.hadError() == true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "error binary runtime mul str",
                 "[interpreter]") {

  interpret("var xs = -1 * \"t\";");
  REQUIRE(context.hadError() == true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "error binary runtime divide",
                 "[interpreter]") {

  interpret("var e2 = 11.2 / \"error\";");
  REQUIRE(context.hadError() == true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "error binary runtime minus",
                 "[interpreter]") {

  interpret("var ss = 10 - \"minus!\";");
  REQUIRE(context.hadError() == true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "error binary runtime add str",
                 "[interpreter]") {

  interpret("var longVariableName = 15.201 + \"letsadd\";");
  REQUIRE(context.hadError() == true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "error binary runtime add str 2",
                 "[interpreter]") {

  interpret("var longVariableNameWithNumber102 =  \"letsadd\" + 2222;");
  REQUIRE(context.hadError() == true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "runtime add str str",
                 "[interpreter]") {

  interpret("var conc =  \"hello \" + \"world\";");
  binder::RuntimeValue *result = interpreter.getRuntimeVariable("conc");
  REQUIRE(result != nullptr);
  REQUIRE(context.hadError() == false);
  REQUIRE(result->type == binder::RuntimeValueType::STRING);
  REQUIRE(strcmp(result->string, "hello world") == 0);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "runtime > str",
                 "[interpreter]") {

  interpret("var err =  12 > \"hello\";");
  REQUIRE(context.hadError() == true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "runtime >= str",
                 "[interpreter]") {

  interpret("var err = 12 >= \"hello\";");
  REQUIRE(context.hadError() == true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "runtime < str",
                 "[interpreter]") {

  interpret(" var err2 = 12 < \"hello\";");
  REQUIRE(context.hadError() == true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "runtime <= str",
                 "[interpreter]") {

  interpret("var newErr=  12 <= \"hello\";");
  REQUIRE(context.hadError() == true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "runtime != str",
                 "[interpreter]") {

  interpret("var errAgain = 12 != \"hello\";");
  REQUIRE(context.hadError() == true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "runtime == str",
                 "[interpreter]") {

  interpret("var error = 12 == \"hello\";");
  REQUIRE(context.hadError() == true);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "assign ", "[interpreter]") {

  interpret("var a = 12; a = 1;");
  REQUIRE(context.hadError() == false);
}
TEST_CASE_METHOD(SetupInterpreterTestFixture, "r value to l value assign", "[interpreter]") {

  interpret("var first = 10;");
  binder::RuntimeValue *first= interpreter.getRuntimeVariable("first");
  REQUIRE(first->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(first->number== Approx(10.0f));
  REQUIRE(first->storage == binder::RuntimeValueStorage::L_VALUE);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "unary with r value", "[interpreter]") {

  //in this case we should be able to resuse the value of the allocation
  //for the R value 10 and steal it for the unary
  interpret("var a =  -10;");
  binder::RuntimeValue *first= interpreter.getRuntimeVariable("a");
  REQUIRE(first->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(first->number== Approx(-10.0f));
  REQUIRE(first->storage == binder::RuntimeValueStorage::L_VALUE);
}
TEST_CASE_METHOD(SetupInterpreterTestFixture, "unary with l value", "[interpreter]") {

  //in this case we need to duplicate the runtime value, such that in the unary operation
  //the variable a does not get modified
  interpret("var a =  10; var b = -a;");
  binder::RuntimeValue *a= interpreter.getRuntimeVariable("a");
  binder::RuntimeValue *b= interpreter.getRuntimeVariable("b");
  REQUIRE(a->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(a->number== Approx(10.0f));
  REQUIRE(a->storage == binder::RuntimeValueStorage::L_VALUE);
  REQUIRE(b->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(b->number== Approx(-10.0f));
  REQUIRE(b->storage == binder::RuntimeValueStorage::L_VALUE);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "binary minus with second r value", "[interpreter]") {

  //in this case we need to duplicate the runtime value, such that in the unary operation
  //the variable a does not get modified
  interpret("var a =  10; var b = a - 1;");
  binder::RuntimeValue *a= interpreter.getRuntimeVariable("a");
  binder::RuntimeValue *b= interpreter.getRuntimeVariable("b");
  REQUIRE(a->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(a->number== Approx(10.0f));
  REQUIRE(a->storage == binder::RuntimeValueStorage::L_VALUE);
  REQUIRE(b->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(b->number== Approx(9.0f));
  REQUIRE(b->storage == binder::RuntimeValueStorage::L_VALUE);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "binary divide with second r value", "[interpreter]") {

  //in this case we need to duplicate the runtime value, such that in the unary operation
  //the variable a does not get modified
  interpret("var a =  10; var b = a / 2;");
  binder::RuntimeValue *a= interpreter.getRuntimeVariable("a");
  binder::RuntimeValue *b= interpreter.getRuntimeVariable("b");
  REQUIRE(a->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(a->number== Approx(10.0f));
  REQUIRE(a->storage == binder::RuntimeValueStorage::L_VALUE);
  REQUIRE(b->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(b->number== Approx(5.0f));
  REQUIRE(b->storage == binder::RuntimeValueStorage::L_VALUE);
}
TEST_CASE_METHOD(SetupInterpreterTestFixture, "binary star with second r value", "[interpreter]") {

  //in this case we need to duplicate the runtime value, such that in the unary operation
  //the variable a does not get modified
  interpret("var a =  10; var b = a * 2;");
  binder::RuntimeValue *a= interpreter.getRuntimeVariable("a");
  binder::RuntimeValue *b= interpreter.getRuntimeVariable("b");
  REQUIRE(a->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(a->number== Approx(10.0f));
  REQUIRE(a->storage == binder::RuntimeValueStorage::L_VALUE);
  REQUIRE(b->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(b->number== Approx(20.0f));
  REQUIRE(b->storage == binder::RuntimeValueStorage::L_VALUE);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "binary + with second r value", "[interpreter]") {

  //in this case we need to duplicate the runtime value, such that in the unary operation
  //the variable a does not get modified
  interpret("var a =  10; var b = a + 2;");
  binder::RuntimeValue *a= interpreter.getRuntimeVariable("a");
  binder::RuntimeValue *b= interpreter.getRuntimeVariable("b");
  REQUIRE(a->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(a->number== Approx(10.0f));
  REQUIRE(a->storage == binder::RuntimeValueStorage::L_VALUE);
  REQUIRE(b->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(b->number== Approx(12.0f));
  REQUIRE(b->storage == binder::RuntimeValueStorage::L_VALUE);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "binary + with second r value string", "[interpreter]") {

  //in this case we need to duplicate the runtime value, such that in the unary operation
  //the variable a does not get modified
  interpret("var a =  \"hello\"; var b = a + \" world\";");
  binder::RuntimeValue *a= interpreter.getRuntimeVariable("a");
  binder::RuntimeValue *b= interpreter.getRuntimeVariable("b");
  REQUIRE(a->type == binder::RuntimeValueType::STRING);
  REQUIRE(strcmp(a->string,"hello")==0 );
  REQUIRE(a->storage == binder::RuntimeValueStorage::L_VALUE);
  REQUIRE(b->type == binder::RuntimeValueType::STRING);
  REQUIRE(strcmp(b->string,"hello world")==0 );
  REQUIRE(b->storage == binder::RuntimeValueStorage::L_VALUE);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "binary < with second r value", "[interpreter]") {

  interpret("var a =  10; var b = a < 2;");
  binder::RuntimeValue *a= interpreter.getRuntimeVariable("a");
  binder::RuntimeValue *b= interpreter.getRuntimeVariable("b");
  REQUIRE(a->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(a->number== Approx(10.0f));
  REQUIRE(a->storage == binder::RuntimeValueStorage::L_VALUE);
  REQUIRE(b->type == binder::RuntimeValueType::BOOLEAN);
  REQUIRE(b->boolean== false );
  REQUIRE(b->storage == binder::RuntimeValueStorage::L_VALUE);
}


TEST_CASE_METHOD(SetupInterpreterTestFixture, "binary >= with second r value", "[interpreter]") {

  interpret("var a =  10; var b = a >= 10;");
  binder::RuntimeValue *a= interpreter.getRuntimeVariable("a");
  binder::RuntimeValue *b= interpreter.getRuntimeVariable("b");
  REQUIRE(a->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(a->number== Approx(10.0f));
  REQUIRE(a->storage == binder::RuntimeValueStorage::L_VALUE);
  REQUIRE(b->type == binder::RuntimeValueType::BOOLEAN);
  REQUIRE(b->boolean== true);
  REQUIRE(b->storage == binder::RuntimeValueStorage::L_VALUE);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "binary <= with second r value", "[interpreter]") {

  interpret("var a =  10; var b = a <= 10;");
  binder::RuntimeValue *a= interpreter.getRuntimeVariable("a");
  binder::RuntimeValue *b= interpreter.getRuntimeVariable("b");
  REQUIRE(a->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(a->number== Approx(10.0f));
  REQUIRE(a->storage == binder::RuntimeValueStorage::L_VALUE);
  REQUIRE(b->type == binder::RuntimeValueType::BOOLEAN);
  REQUIRE(b->boolean== true);
  REQUIRE(b->storage == binder::RuntimeValueStorage::L_VALUE);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "binary == with second r value", "[interpreter]") {

  interpret("var a =  10; var b = a == 10;");
  binder::RuntimeValue *a= interpreter.getRuntimeVariable("a");
  binder::RuntimeValue *b= interpreter.getRuntimeVariable("b");
  REQUIRE(a->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(a->number== Approx(10.0f));
  REQUIRE(a->storage == binder::RuntimeValueStorage::L_VALUE);
  REQUIRE(b->type == binder::RuntimeValueType::BOOLEAN);
  REQUIRE(b->boolean== true);
  REQUIRE(b->storage == binder::RuntimeValueStorage::L_VALUE);
}

TEST_CASE_METHOD(SetupInterpreterTestFixture, "binary != with second r value", "[interpreter]") {

  interpret("var a =  10; var b = a != 10;");
  binder::RuntimeValue *a= interpreter.getRuntimeVariable("a");
  binder::RuntimeValue *b= interpreter.getRuntimeVariable("b");
  REQUIRE(a->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(a->number== Approx(10.0f));
  REQUIRE(a->storage == binder::RuntimeValueStorage::L_VALUE);
  REQUIRE(b->type == binder::RuntimeValueType::BOOLEAN);
  REQUIRE(b->boolean== false);
  REQUIRE(b->storage == binder::RuntimeValueStorage::L_VALUE);
}

/*
TEST_CASE_METHOD(SetupInterpreterTestFixture, "multiple variable", "[interpreter]") {

  interpret("var first = 10; var second  = 17; var c = first + second;");
  binder::RuntimeValue *first= interpreter.getRuntimeVariable("first");
  binder::RuntimeValue *second= interpreter.getRuntimeVariable("second");
  binder::RuntimeValue *c= interpreter.getRuntimeVariable("c");

  REQUIRE(context.hadError() == false);
  REQUIRE(first->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(second->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(c->type == binder::RuntimeValueType::NUMBER);
  REQUIRE(second->number== Approx(17.0f));
  REQUIRE(c->number== Approx(27.0f));
  REQUIRE(first->number== Approx(10.0f));
}
*/
