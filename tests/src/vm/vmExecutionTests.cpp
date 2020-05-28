
#include "binder/log/bufferLog.h"
#include "binder/log/consoleLog.h"
#include "binder/vm/compiler.h"
#include "binder/vm/vm.h"

#include "../catch.h"

class SetupVmExecuteTestFixture {
public:
  SetupVmExecuteTestFixture()
      : m_log(),
#ifdef DEBUG_TRACE_EXECUTION
        m_debugLog(), m_vm(&m_log, &m_debugLog)
#else
        m_vm(&m_log)
#endif
  {
  }

  binder::vm::INTERPRET_RESULT interpret(const char *source) {
    binder::vm::INTERPRET_RESULT result = m_vm.interpret(source);
    return result;
  }

  void printOutput() { printf("%s\n", m_log.getBuffer()); }
  void printDebugOutput() { printf("%s\n", m_debugLog.getBuffer()); }

  int compareLog(const char *expected) {
    return strcmp(m_log.getBuffer(), expected);
  }

protected:
  binder::log::BufferedLog m_log;
  binder::log::BufferedLog m_debugLog;
  binder::vm::VirtualMachine m_vm;

  bool result;
};

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm add ", "[vm-parser]") {

  const char *source = "print 1 + 2;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("3\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm divide", "[vm-parser]") {

  const char *source = "print 10 / 2;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("5\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm subract", "[vm-parser]") {

  const char *source = "print 10 -3;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("7\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm mult", "[vm-parser]") {

  const char *source = "print 10*20;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("200\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm str cmp exec", "[vm-parser]") {

  const char *source = "print \"hello world\" == \"hello world\";";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("true\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm str cmp exec 2",
                 "[vm-parser]") {

  const char *source = "print \"hello world\" == \"hello world0\";";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("false\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm str cmp exec 3",
                 "[vm-parser]") {

  const char *source = "print \"hello world\" != \"hello world0\";";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("true\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm str concatentation",
                 "[vm-parser]") {

  const char *source = "print \"hello \" + \"world\";";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("hello world\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm add error 1", "[vm-parser]") {

  const char *source = "\"hello \" + 1.2;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_RUNTIME_ERROR);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm add error 2", "[vm-parser]") {

  const char *source = "2  + \"hello \" ;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_RUNTIME_ERROR);
}
TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm add error 3", "[vm-parser]") {

  const char *source = "2 + 1 - \"hello \" ;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_RUNTIME_ERROR);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm add error 4", "[vm-parser]") {

  const char *source = "\"hello \" + \"world\" *10 ;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_RUNTIME_ERROR);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm global var print",
                 "[vm-parser]") {

  const char *source = "var test = 10; print test;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("10\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm global var expr print",
                 "[vm-parser]") {

  const char *source = "var test = 10 + 5; print test;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("15\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm global var expr print 2",
                 "[vm-parser]") {

  const char *source = "var test = 10 < 5; print test;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("false\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm global var expr print 3",
                 "[vm-parser]") {

  const char *source = "var test = 0 == 0; print test;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("true\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm global var expr print 4",
                 "[vm-parser]") {

  const char *source = "var test = \"hello\"  + \" world\"; print test;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("hello world\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm exec assigment edge case",
                 "[vm-parser]") {

  const char *source =
      "var breakfast = \"beignets\";\n var beverage = \"cafe "
      "au lait\";\nbreakfast = \"beignets with \" + beverage;\nprint breakfast;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("beignets with cafe au lait\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm exec basic scope",
                 "[vm-parser]") {

  const char *source = "var a = 12;\n {\n var b = 20; print b; \n} print a;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("20\n12\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm exec shadowing scope",
                 "[vm-parser]") {

  //testing that shadowing does not mess up the result
  const char *source = "var a = 12;\n {\n var a = 20; print a; \n} print a;";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("20\n12\n") == 0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm exec simple if else, then taken",
                 "[vm-parser]") {
  const char *source = "var a = 5;\n if(a > 3){\n print 10; \n} else{\n print 20;\n}";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("10\n") == 0);

}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm exec simple if else, else taken",
                 "[vm-parser]") {
  const char *source = "var a = 5;\n if(a < 3){\n print 10; \n} else{\n print 20;\n}";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("20\n") == 0);

}
