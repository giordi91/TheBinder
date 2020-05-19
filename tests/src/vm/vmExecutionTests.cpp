
#include "binder/log/bufferLog.h"
#include "binder/log/consoleLog.h"
#include "binder/vm/compiler.h"
#include "binder/vm/vm.h"

#include "../catch.h"

class SetupVmExecuteTestFixture {
public:
  SetupVmExecuteTestFixture() : m_log(), 
#ifdef DEBUG_TRACE_EXECUTION
    m_vm(&m_log,&m_debugLog) 
#else
    m_vm(&m_log) 
#endif
    {}

  binder::vm::INTERPRET_RESULT interpret(const char *source) {
    binder::vm::INTERPRET_RESULT result = m_vm.interpret(source);
    return result;
  }

  void printOutput()
  {
      printf("%s\n",m_log.getBuffer());
  }
  void printDebugOutput()
  {
      printf("%s\n",m_debugLog.getBuffer());
  }
  
  
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

  const char *source = "1 + 2";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("3\n")==0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm divide", "[vm-parser]") {

  const char *source = "10 / 2";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("5\n")==0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm subract", "[vm-parser]") {

  const char *source = "10 -3";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("7\n")==0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm mult", "[vm-parser]") {

  const char *source = "10*20";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("200\n")==0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm str cmp exec", "[vm-parser]") {

  const char *source = "\"hello world\" == \"hello world\"";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("true\n")==0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm str cmp exec 2", "[vm-parser]") {

  const char *source = "\"hello world\" == \"hello world0\"";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("false\n")==0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm str cmp exec 3", "[vm-parser]") {

  const char *source = "\"hello world\" != \"hello world0\"";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("true\n")==0);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm str concatentation", "[vm-parser]") {

  const char *source = "\"hello \" + \"world\"";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_OK);
  REQUIRE(compareLog("hello world\n")==0);
}


TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm add error 1", "[vm-parser]") {

  const char *source = "\"hello \" + 1.2";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_RUNTIME_ERROR);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm add error 2", "[vm-parser]") {

  const char *source = "2  + \"hello \" ";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_RUNTIME_ERROR);
}
TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm add error 3", "[vm-parser]") {

  const char *source = "2 + 1 - \"hello \" ";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_RUNTIME_ERROR);
}

TEST_CASE_METHOD(SetupVmExecuteTestFixture, "vm add error 4", "[vm-parser]") {

  const char *source = "\"hello \" + \"world\" *10 ";
  binder::vm::INTERPRET_RESULT result = interpret(source);
  REQUIRE(result == binder::vm::INTERPRET_RESULT::INTERPRET_RUNTIME_ERROR);
}
