#include "binder/context.h"
#include "binder/interpreter.h"
#include "binder/parser.h"
#include "binder/printer/jsonASTPrinter.h"
#include "binder/scanner.h"
#include "binder/log/bufferLog.h"

#include "stdlib.h"

#include "catch.h"

class SetupLoggerTestFixture {
public:
  SetupLoggerTestFixture()
      : context({32, binder::LOGGER_TYPE::BUFFERED}), scanner(&context),
        parser(&context), interpreter(&context) {}

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

TEST_CASE_METHOD(SetupLoggerTestFixture, "bufferedLog simple print",
                 "[logger]") {

  const char* source = "hello world!";
  context.print(source);
  auto* logger = dynamic_cast<binder::log::BufferedLog*>(context.getLogger());
  const char* log = logger->getBuffer();
  REQUIRE(strcmp(log,source)==0); 
}

TEST_CASE_METHOD(SetupLoggerTestFixture, "bufferedLog multi print",
                 "[logger]") {

  context.print("hello");
  context.print(" world! !");
  auto* logger = dynamic_cast<binder::log::BufferedLog*>(context.getLogger());
  const char* log = logger->getBuffer();
  REQUIRE(strcmp(log,"hello world! !")==0); 
}
