#include "binder/log/bufferLog.h"
#include "binder/log/consoleLog.h"
#include "binder/memory/stringIntern.h"
#include "binder/vm/compiler.h"
#include "binder/vm/object.h"

#include "../catch.h"

class SetupVmParserTestFixture {
public:
  SetupVmParserTestFixture() : intern(1024), compiler(&intern) {}
  ~SetupVmParserTestFixture() { binder::vm::freeAllocations(); }
  const binder::vm::Chunk *compile(const char *source, bool debug = false) {
    result = compiler.compile(source, &m_log);

    m_chunk = compiler.getCompiledChunk();
    if (debug && m_chunk != nullptr) {
      binder::vm::disassambleChunk(m_chunk, "debug", &m_debugLog);
    }

    return m_chunk;
  }

  void compareInstruction(int offset, binder::vm::OP_CODE expected) {
    compareInstruction(offset, static_cast<uint8_t>(expected));
  }
  void compareInstruction(uint32_t offset, int expected) {
      compareInstruction(offset,static_cast<uint8_t>(expected));
  }

  void compareInstruction(uint32_t offset, uint8_t expected) {
    REQUIRE(offset <= m_chunk->m_code.size());
    REQUIRE(m_chunk->m_code[offset] == expected);
  }
  void compareConstantValue(uint32_t offset, int idx, double value) {
    REQUIRE(m_chunk->m_code[offset] == idx);
    REQUIRE(m_chunk->m_constants[idx].as.number == Approx(value));
  }
  void compareConstant(uint32_t offset, int idx, double value) {
    compareInstruction(offset, binder::vm::OP_CODE::OP_CONSTANT);
    compareConstantValue(offset + 1, idx, value);
  }

  void compareDefineGlobal(uint32_t offset, int idx, const char *value) {
    compareInstruction(offset, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
    compareConstantValue(offset + 1, idx, value);
  }
  void compareGetGlobal(uint32_t offset, int idx, const char *value) {
    compareInstruction(offset, binder::vm::OP_CODE::OP_GET_GLOBAL);
    compareConstantValue(offset + 1, idx, value);
  }
  void compareSetGlobal(uint32_t offset, int idx, const char *value) {
    compareInstruction(offset, binder::vm::OP_CODE::OP_SET_GLOBAL);
    compareConstantValue(offset + 1, idx, value);
  }
  void compareGetLocal(uint32_t offset, int idx) {
    compareInstruction(offset, binder::vm::OP_CODE::OP_GET_LOCAL);
    compareInstruction(offset+ 1, idx);
  }
  void compareSetLocal(uint32_t offset, int idx) {
    compareInstruction(offset, binder::vm::OP_CODE::OP_SET_LOCAL);
    compareInstruction(offset+ 1, idx);
  }


  void compareConstantValue(uint32_t offset, int idx, bool value) {
    REQUIRE(m_chunk->m_code[offset] == idx);
    REQUIRE(m_chunk->m_constants[idx].as.boolean == value);
  }


  void compareConstantValue(uint32_t offset, int idx, const char *toCompare) {
    REQUIRE(m_chunk->m_code[offset] == idx);
    // REQUIRE(chunk->m_constants[idx].as.number == Approx(value));
    binder::vm::Value value = m_chunk->m_constants[idx];
    REQUIRE(binder::vm::isValueObj(value));
    REQUIRE(binder::vm::isObjType(value, binder::vm::OBJ_TYPE::OBJ_STRING));
    REQUIRE(strcmp(binder::vm::valueAsCString(value), toCompare) == 0);
  }
  void compareJumpOffset(uint32_t offset, uint16_t expectedJump) {
    auto jump = static_cast<uint16_t>(m_chunk->m_code[offset] << 8);
    jump |= m_chunk->m_code[offset + 1];
    REQUIRE(jump == expectedJump);
  }
  void compareJumpFalse(uint32_t offset, uint16_t expectedJump,
                        bool expectPop) {
    compareInstruction(offset, binder::vm::OP_CODE::OP_JUMP_IF_FALSE);
    compareJumpOffset(offset + 1, expectedJump);
    if (expectPop) {
      compareInstruction(offset + 3, binder::vm::OP_CODE::OP_POP);
    }
  }
  void compareJump(uint32_t offset, uint16_t expectedJump, bool expectPop) {
    compareInstruction(offset, binder::vm::OP_CODE::OP_JUMP);
    compareJumpOffset(offset + 1, expectedJump);
    if (expectPop) {
      compareInstruction(offset + 3, binder::vm::OP_CODE::OP_POP);
    }
  }
  void compareLoop(uint32_t offset, uint16_t expectedJump, bool expectPop) {
    compareInstruction(offset, binder::vm::OP_CODE::OP_LOOP);
    compareJumpOffset(offset + 1, expectedJump);
    if (expectPop) {
      compareInstruction(offset + 3, binder::vm::OP_CODE::OP_POP);
    }
  }

  void printOutput() { printf("%s\n", m_log.getBuffer()); }

protected:
  binder::log::BufferedLog m_log;
  binder::log::ConsoleLog m_debugLog;
  binder::memory::StringIntern intern;

  binder::vm::Compiler compiler;
  const binder::vm::Chunk *m_chunk;
  bool result;
};

// NOTE simple expression will have a pop after evaluating, a statement should
// be stack neutral when it comes to push and pop,after a statement execution we
// should be back at the same stack status. if for example we have a multiply we
// push two constants, multiply which pops two and push the result, finally end
// of statement we pop, removing the result we pushed on the stack

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm basic number parse",
                 "[vm-parser]") {

  const char *source = "12345.5;";
  auto *chunk = compile(source, false);
  // op const + constant + op return
  REQUIRE(chunk != nullptr);
  REQUIRE(chunk->m_code.size() == 4);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  // should be the first constant in there
  compareConstantValue(1, 0, 12345.5);
  compareInstruction(2, binder::vm::OP_CODE::OP_POP);
  compareInstruction(3, binder::vm::OP_CODE::OP_RETURN);
  delete chunk;
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm basic multiply", "[vm-parser]") {

  const char *source = "77 *323.2;";
  auto *chunk = compile(source, false);
  // op const + constant + op return
  REQUIRE(chunk != nullptr);
  REQUIRE(chunk->m_code.size() == 7);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 0, 77.0);

  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(3, 1, 323.2);

  compareInstruction(4, binder::vm::OP_CODE::OP_MULTIPLY);
  compareInstruction(5, binder::vm::OP_CODE::OP_POP);
  compareInstruction(6, binder::vm::OP_CODE::OP_RETURN);
  delete chunk;
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm unary precedence",
                 "[vm-parser]") {

  const char *source = "-1+ 5;";
  auto *chunk = compile(source, false);
  REQUIRE(chunk != nullptr);
  REQUIRE(chunk->m_code.size() == 8);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 0, 1.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_NEGATE);
  compareInstruction(3, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(4, 1, 5.0);
  compareInstruction(5, binder::vm::OP_CODE::OP_ADD);
  compareInstruction(6, binder::vm::OP_CODE::OP_POP);
  compareInstruction(7, binder::vm::OP_CODE::OP_RETURN);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm MAD 1", "[vm-parser]") {
  const char *source = "(144.4*3.14)+12;";
  auto *chunk = compile(source, false);
  REQUIRE(chunk != nullptr);
  REQUIRE(chunk->m_code.size() == 10);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 0, 144.4);
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(3, 1, 3.14);
  compareInstruction(4, binder::vm::OP_CODE::OP_MULTIPLY);
  compareInstruction(5, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(6, 2, 12.0);
  compareInstruction(7, binder::vm::OP_CODE::OP_ADD);
  compareInstruction(8, binder::vm::OP_CODE::OP_POP);
  compareInstruction(9, binder::vm::OP_CODE::OP_RETURN);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm MAD 1 no grouping",
                 "[vm-parser]") {
  const char *source = "144.4*3.14+12;";
  auto *chunk = compile(source, false);
  REQUIRE(chunk != nullptr);
  REQUIRE(chunk->m_code.size() == 10);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 0, 144.4);
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(3, 1, 3.14);
  compareInstruction(4, binder::vm::OP_CODE::OP_MULTIPLY);
  compareInstruction(5, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(6, 2, 12.0);
  compareInstruction(7, binder::vm::OP_CODE::OP_ADD);
  compareInstruction(8, binder::vm::OP_CODE::OP_POP);
  compareInstruction(9, binder::vm::OP_CODE::OP_RETURN);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm MAD 2", "[vm-parser]") {
  const char *source = "(-1*3.14)+(--13);";
  auto *chunk = compile(source, false);
  REQUIRE(chunk != nullptr);
  REQUIRE(chunk->m_code.size() == 13);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 0, 1.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_NEGATE);
  compareInstruction(3, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(4, 1, 3.14);
  compareInstruction(5, binder::vm::OP_CODE::OP_MULTIPLY);
  compareInstruction(6, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(7, 2, 13.0);
  compareInstruction(8, binder::vm::OP_CODE::OP_NEGATE);
  compareInstruction(9, binder::vm::OP_CODE::OP_NEGATE);
  compareInstruction(10, binder::vm::OP_CODE::OP_ADD);
  compareInstruction(11, binder::vm::OP_CODE::OP_POP);
  compareInstruction(12, binder::vm::OP_CODE::OP_RETURN);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm expression error 1",
                 "[vm-parser]") {
  const char *source = "1 ** 1;";
  auto *chunk = compile(source, false);
  // check for failure
  REQUIRE(chunk == nullptr);
  REQUIRE(result == false);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm expression error 2",
                 "[vm-parser]") {
  const char *source = "(1 ** 1;";
  auto *chunk = compile(source, false);
  // check for failure
  REQUIRE(chunk == nullptr);
  REQUIRE(result == false);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm basic string", "[vm-parser]") {
  const char *source = "\"hello world\";";
  auto *chunk = compile(source, false);
  // check for failure
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 4);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 0, "hello world");
  compareInstruction(2, binder::vm::OP_CODE::OP_POP);
  compareInstruction(3, binder::vm::OP_CODE::OP_RETURN);

  /*
    == debug ==
    0000    0 OP_CONSTANT         0 'hello world
    0002    | OP_RETURN
  */
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm str cmp", "[vm-parser]") {
  const char *source = "\"hello world\" == \"hello world\";";
  auto *chunk = compile(source, false);
  // check for failure
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 7);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 0, "hello world");
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(3, 1, "hello world");
  compareInstruction(4, binder::vm::OP_CODE::OP_EQUAL);
  compareInstruction(5, binder::vm::OP_CODE::OP_POP);
  compareInstruction(6, binder::vm::OP_CODE::OP_RETURN);

  /*
    == debug ==
    0000    0 OP_CONSTANT         0 'hello world
    0002    | OP_CONSTANT         1 'hello world
    0004    | OP_EQUAL
    0005    | OP_RETURN
  */
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm str cmp  false", "[vm-parser]") {
  const char *source = "\"hello world\" != \"hello world\";";
  auto *chunk = compile(source, false);
  // check for failure
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 8);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 0, "hello world");
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(3, 1, "hello world");
  compareInstruction(4, binder::vm::OP_CODE::OP_EQUAL);
  compareInstruction(5, binder::vm::OP_CODE::OP_NOT);
  compareInstruction(6, binder::vm::OP_CODE::OP_POP);
  compareInstruction(7, binder::vm::OP_CODE::OP_RETURN);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm str concatenation",
                 "[vm-parser]") {
  const char *source = "\"hello \" + \"world\";";
  auto *chunk = compile(source, false);
  // check for failure
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 7);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 0, "hello ");
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(3, 1, "world");
  compareInstruction(4, binder::vm::OP_CODE::OP_ADD);
  compareInstruction(5, binder::vm::OP_CODE::OP_POP);
  compareInstruction(6, binder::vm::OP_CODE::OP_RETURN);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm define global empty",
                 "[vm-parser]") {
  const char *source = "var test;";
  auto *chunk = compile(source, false);
  // check for failure
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 4);
  // first the value of the variable is put on the stack, in this case nil
  compareInstruction(0, binder::vm::OP_CODE::OP_NIL);
  // then we have a global definition, which behaves as a constant,
  // first the opcode then the uin8_t pointing to where it is stored
  compareInstruction(1, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstantValue(2, 0, "test");
  compareInstruction(3, binder::vm::OP_CODE::OP_RETURN);

  /*
== debug ==
0000    0 OP_NIL
0001    | OP_DEFINE_GLOBAL    0 'test
0003    | OP_RETURN
   */
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm define global string",
                 "[vm-parser]") {
  const char *source = "var myVar = \"my first var\";";
  auto *chunk = compile(source, false);
  // check for failure
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 5);

  // first the value of the variable is put on the stack, in this case nil
  // to not that the during the paring the variable name is set first on the
  // stack hence the 1 index of "my first var"
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 1, "my first var");
  // then we have a global definition, which behaves as a constant,
  // first the opcode then the uin8_t pointing to where it is stored
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstantValue(3, 0, "myVar");
  compareInstruction(4, binder::vm::OP_CODE::OP_RETURN);

  /*
  == debug ==
  0000    0 OP_CONSTANT         1 'my first var
  0002    | OP_DEFINE_GLOBAL    0 'myVar
  0004    | OP_RETURN
  */
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm define global int",
                 "[vm-parser]") {
  const char *source = "var myVar = 10;";
  auto *chunk = compile(source, false);
  // check for failure
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 5);

  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 1, 10.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstantValue(3, 0, "myVar");
  compareInstruction(4, binder::vm::OP_CODE::OP_RETURN);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm define global bool",
                 "[vm-parser]") {
  const char *source = "var myVar = false;";
  auto *chunk = compile(source, false);
  // check for failure
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 4);

  compareInstruction(0, binder::vm::OP_CODE::OP_FALSE);
  compareInstruction(1, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstantValue(2, 0, "myVar");
  compareInstruction(3, binder::vm::OP_CODE::OP_RETURN);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm define global bool 2",
                 "[vm-parser]") {
  const char *source = "var myVar = true;";
  auto *chunk = compile(source, false);
  // check for failure
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 4);

  compareInstruction(0, binder::vm::OP_CODE::OP_TRUE);
  compareInstruction(1, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstantValue(2, 0, "myVar");
  compareInstruction(3, binder::vm::OP_CODE::OP_RETURN);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm define global expr 1",
                 "[vm-parser]") {
  const char *source = "var init1 = 1 + 2;";
  auto *chunk = compile(source, false);
  // check for failure
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 8);

  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 1, 1.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(3, 2, 2.0);
  compareInstruction(4, binder::vm::OP_CODE::OP_ADD);
  compareInstruction(5, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstantValue(6, 0, "init1");
  compareInstruction(7, binder::vm::OP_CODE::OP_RETURN);

  /*
== debug ==
0000    0 OP_CONSTANT         1 '1
0002    | OP_CONSTANT         2 '2
0004    | OP_ADD
0005    | OP_DEFINE_GLOBAL    0 'init1
0007    | OP_RETURN
*/
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm get global", "[vm-parser]") {
  const char *source = "var myVar = 10;\n print myVar;";
  auto *chunk = compile(source, false);
  // check for failure
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 8);

  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 1, 10.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstantValue(3, 0, "myVar");
  compareInstruction(4, binder::vm::OP_CODE::OP_GET_GLOBAL);
  compareConstantValue(5, 2, "myVar");
  compareInstruction(6, binder::vm::OP_CODE::OP_PRINT);
  compareInstruction(7, binder::vm::OP_CODE::OP_RETURN);

  /*
  == debug ==
  0000    0 OP_CONSTANT         1 '10
  0002    | OP_DEFINE_GLOBAL    0 'myVar
  0004    1 OP_GET_GLOBAL       2 'myVar
  0006    | OP_PRINT
  0007    | OP_RETURN
  */
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm compile assigment edge case",
                 "[vm-parser]") {

  const char *source = "var breakfast = \"beignets\";\n var beverage = \"cafe "
                       "au lait\";\nbreakfast = \"beignets with \" + beverage;";
  auto *chunk = compile(source, false);
  // check for failure
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 17);

  // defining the first global variable
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 1, "beignets");
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstantValue(3, 0, "breakfast");

  // defining the second global variable
  compareInstruction(4, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(5, 3, "cafe au lait");
  compareInstruction(6, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstantValue(7, 2, "beverage");

  // now that wehave the twovariables we need to perform the assigment
  // which is between first a constant
  compareInstruction(8, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(9, 5, "beignets with ");

  // then the global variable  which we pop on the stack now
  compareInstruction(10, binder::vm::OP_CODE::OP_GET_GLOBAL);
  compareConstantValue(11, 6, "beverage");

  // doing the addition
  compareInstruction(12, binder::vm::OP_CODE::OP_ADD);
  compareInstruction(13, binder::vm::OP_CODE::OP_SET_GLOBAL);
  compareConstantValue(14, 4, "breakfast");
  compareInstruction(15, binder::vm::OP_CODE::OP_POP);
  compareInstruction(16, binder::vm::OP_CODE::OP_RETURN);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm compile basic scope",
                 "[vm-parser]") {

  const char *source = "var a = 12;\n {\n var b = 20;\n print b; \n} print a;";
  auto *chunk = compile(source, false);
  // check for failure
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 14);

  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 1, 12.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstantValue(3, 0, "a");
  compareInstruction(4, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(5, 2, 20.0);
  compareInstruction(6, binder::vm::OP_CODE::OP_GET_LOCAL);
  // comparing the get local slot
  compareInstruction(7, 0);
  compareInstruction(8, binder::vm::OP_CODE::OP_PRINT);
  // we are getting out of scope, so we pop
  compareInstruction(9, binder::vm::OP_CODE::OP_POP);

  // then the global variable
  compareInstruction(10, binder::vm::OP_CODE::OP_GET_GLOBAL);
  compareConstantValue(11, 3, "a");
  compareInstruction(12, binder::vm::OP_CODE::OP_PRINT);
  compareInstruction(13, binder::vm::OP_CODE::OP_RETURN);

  /*
  == debug ==
  0000    0 OP_CONSTANT         1 '12
  0002    | OP_DEFINE_GLOBAL    0 'a
  0004    2 OP_CONSTANT         2 '20
  0006    3 OP_GET_LOCAL        0
  0008    | OP_PRINT
  0009    4 OP_POP
  0010    | OP_GET_GLOBAL       3 'a
  0012    | OP_PRINT
  0013    | OP_RETURN
  */
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm compile shadowing scope",
                 "[vm-parser]") {

  // this test generates the same code as before, which is what we are testing
  // shadowing should not affect outer scope
  const char *source = "var a = 12;\n {\n var a = 20;\n print a; \n} print a;";
  auto *chunk = compile(source, false);
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 14);

  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 1, 12.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstantValue(3, 0, "a");
  compareInstruction(4, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(5, 2, 20.0);
  compareInstruction(6, binder::vm::OP_CODE::OP_GET_LOCAL);
  // comparing the get local slot
  compareInstruction(7, 0);
  compareInstruction(8, binder::vm::OP_CODE::OP_PRINT);
  // we are getting out of scope, so we pop
  compareInstruction(9, binder::vm::OP_CODE::OP_POP);

  // then the global variable
  compareInstruction(10, binder::vm::OP_CODE::OP_GET_GLOBAL);
  compareConstantValue(11, 3, "a");
  compareInstruction(12, binder::vm::OP_CODE::OP_PRINT);
  compareInstruction(13, binder::vm::OP_CODE::OP_RETURN);

  /*
    == debug ==
    0000    0 OP_CONSTANT         1 '12
    0002    | OP_DEFINE_GLOBAL    0 'a
    0004    2 OP_CONSTANT         2 '20
    0006    3 OP_GET_LOCAL        0
    0008    | OP_PRINT
    0009    4 OP_POP
    0010    | OP_GET_GLOBAL       3 'a
    0012    | OP_PRINT
    0013    | OP_RETURN
  */
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm compile simple if",
                 "[vm-parser]") {
  const char *source = "var a = 5;\n if(a > 3){\n print 10; \n}";
  auto *chunk = compile(source, false);
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 21);

  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 1, 5.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstantValue(3, 0, "a");
  compareInstruction(4, binder::vm::OP_CODE::OP_GET_GLOBAL);
  compareConstantValue(5, 2, "a");
  compareInstruction(6, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(7, 3, 3.0);
  compareInstruction(8, binder::vm::OP_CODE::OP_GREATER);
  compareInstruction(9, binder::vm::OP_CODE::OP_JUMP_IF_FALSE);
  compareJumpOffset(10, 7);
  compareInstruction(12, binder::vm::OP_CODE::OP_POP);
  compareInstruction(13, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(14, 4, 10.0);
  compareInstruction(15, binder::vm::OP_CODE::OP_PRINT);
  compareInstruction(16, binder::vm::OP_CODE::OP_JUMP);
  compareJumpOffset(17, 1);
  compareInstruction(20, binder::vm::OP_CODE::OP_RETURN);

  /*
    == debug ==
    0000    0 OP_CONSTANT         1 '5
    0002    | OP_DEFINE_GLOBAL    0 'a
    0004    1 OP_GET_GLOBAL       2 'a
    0006    | OP_CONSTANT         3 '3
    0008    | OP_GREATER
    0009    | OP_JUMP_IF_FALSE    9 -> 19
    0012    | OP_POP
    0013    2 OP_CONSTANT         4 '10
    0015    | OP_PRINT
    0016    3 OP_JUMP            16 -> 20
    0019    | OP_POP
    0020    | OP_RETURN
  */
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm compile simple if else",
                 "[vm-parser]") {
  const char *source =
      "var a = 5;\n if(a > 3){\n print 10; \n} else{\n print 20;\n}";
  auto *chunk = compile(source, false);
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 24);

  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 1, 5.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstantValue(3, 0, "a");
  compareInstruction(4, binder::vm::OP_CODE::OP_GET_GLOBAL);
  compareConstantValue(5, 2, "a");
  compareInstruction(6, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(7, 3, 3.0);
  compareInstruction(8, binder::vm::OP_CODE::OP_GREATER);
  compareInstruction(9, binder::vm::OP_CODE::OP_JUMP_IF_FALSE);
  compareJumpOffset(10, 7);
  compareInstruction(12, binder::vm::OP_CODE::OP_POP);
  compareInstruction(13, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(14, 4, 10.0);
  compareInstruction(15, binder::vm::OP_CODE::OP_PRINT);
  compareInstruction(16, binder::vm::OP_CODE::OP_JUMP);
  compareJumpOffset(17, 4);
  compareInstruction(20, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(21, 5, 20.0);
  compareInstruction(22, binder::vm::OP_CODE::OP_PRINT);
  compareInstruction(23, binder::vm::OP_CODE::OP_RETURN);
  /*
    == debug ==
    0000    0 OP_CONSTANT         1 '5
    0002    | OP_DEFINE_GLOBAL    0 'a
    0004    1 OP_GET_GLOBAL       2 'a
    0006    | OP_CONSTANT         3 '3
    0008    | OP_GREATER
    0009    | OP_JUMP_IF_FALSE    9 -> 19
    0012    | OP_POP
    0013    2 OP_CONSTANT         4 '10
    0015    | OP_PRINT
    0016    3 OP_JUMP            16 -> 23
    0019    | OP_POP
    0020    4 OP_CONSTANT         5 '20
    0022    | OP_PRINT
    0023    5 OP_RETURN
  */
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm compile simple and",
                 "[vm-parser]") {

  const char *source = "var a = 3 > 1 and 5 > 3;";
  auto *chunk = compile(source, false);
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);

  REQUIRE(chunk->m_code.size() == 17);

  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 1, 3.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(3, 2, 1.0);
  compareInstruction(4, binder::vm::OP_CODE::OP_GREATER);
  compareInstruction(5, binder::vm::OP_CODE::OP_JUMP_IF_FALSE);
  compareJumpOffset(6, 6);
  compareInstruction(8, binder::vm::OP_CODE::OP_POP);
  compareInstruction(9, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(10, 3, 5.0);
  compareInstruction(11, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(12, 4, 3.0);
  compareInstruction(13, binder::vm::OP_CODE::OP_GREATER);
  compareInstruction(14, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstantValue(15, 0, "a");
  compareInstruction(16, binder::vm::OP_CODE::OP_RETURN);

  /*
    == debug ==
    0000    0 OP_CONSTANT         1 '3
    0002    | OP_CONSTANT         2 '1
    0004    | OP_GREATER
    0005    | OP_JUMP_IF_FALSE    5 -> 14
    0008    | OP_POP
    0009    | OP_CONSTANT         3 '5
    0011    | OP_CONSTANT         4 '3
    0013    | OP_GREATER
    0014    | OP_DEFINE_GLOBAL    0 'a
    0016    | OP_RETURN
  */
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm compile simple or",
                 "[vm-parser]") {

  const char *source = "var a = 3 < 1 or 5 > 3;";
  auto *chunk = compile(source, false);
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);

  REQUIRE(chunk->m_code.size() == 20);

  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(1, 1, 3.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(3, 2, 1.0);
  compareInstruction(4, binder::vm::OP_CODE::OP_LESS);
  compareInstruction(5, binder::vm::OP_CODE::OP_JUMP_IF_FALSE);
  compareJumpOffset(6, 3);
  compareInstruction(8, binder::vm::OP_CODE::OP_JUMP);
  compareJumpOffset(9, 6);
  compareInstruction(11, binder::vm::OP_CODE::OP_POP);
  compareInstruction(12, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(13, 3, 5.0);
  compareInstruction(14, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstantValue(15, 4, 3.0);
  compareInstruction(16, binder::vm::OP_CODE::OP_GREATER);
  compareInstruction(17, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstantValue(18, 0, "a");
  compareInstruction(19, binder::vm::OP_CODE::OP_RETURN);

  /*
    == debug ==
    0000    0 OP_CONSTANT         1 '3
    0002    | OP_CONSTANT         2 '1
    0004    | OP_LESS
    0005    | OP_JUMP_IF_FALSE    5 -> 11
    0008    | OP_JUMP             8 -> 17
    0011    | OP_POP
    0012    | OP_CONSTANT         3 '5
    0014    | OP_CONSTANT         4 '3
    0016    | OP_GREATER
    0017    | OP_DEFINE_GLOBAL    0 'a
    0019    | OP_RETURN
  */
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm compile simple while",
                 "[vm-parser]") {
  const char *source = "var a = 0; while(a < 5){ print a; a= a+1;}";
  auto *chunk = compile(source, false);
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);

  compareConstant(0, 1, 0.0);
  compareDefineGlobal(2, 0, "a");
  compareGetGlobal(4, 2, "a");
  compareConstant(6, 3, 5.0);
  compareInstruction(8, binder::vm::OP_CODE::OP_LESS);
  compareJumpFalse(9, 15, true);
  compareGetGlobal(13, 4, "a");
  compareInstruction(15, binder::vm::OP_CODE::OP_PRINT);
  compareGetGlobal(16, 6, "a");
  compareConstant(18, 7, 1.0);
  compareInstruction(20, binder::vm::OP_CODE::OP_ADD);
  compareSetGlobal(21, 5, "a");
  compareInstruction(23, binder::vm::OP_CODE::OP_POP);
  compareLoop(24, 23, true);
  compareInstruction(28, binder::vm::OP_CODE::OP_RETURN);
  /*
    == debug ==
    0000    0 OP_CONSTANT         1 '0
    0002    | OP_DEFINE_GLOBAL    0 'a
    0004    | OP_GET_GLOBAL       2 'a
    0006    | OP_CONSTANT         3 '5
    0008    | OP_LESS
    0009    | OP_JUMP_IF_FALSE    9 -> 27
    0012    | OP_POP
    0013    | OP_GET_GLOBAL       4 'a
    0015    | OP_PRINT
    0016    | OP_GET_GLOBAL       6 'a
    0018    | OP_CONSTANT         7 '1
    0020    | OP_ADD
    0021    | OP_SET_GLOBAL       5 'a
    0023    | OP_POP
    0024    | OP_LOOP            24 -> 4
    0027    | OP_POP
    0028    | OP_RETURN
  */
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm compile simple for",
                 "[vm-parser]") {
  const char *source = "for(var a = 0; a < 5;a= a+1){print a;}";
  auto *chunk = compile(source, false);
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  REQUIRE(chunk->m_code.size() == 34);

  //altough similar to a while loop the code generated is different
  //here we define the constant of the a =0;
  compareConstant(0, 0, 0.0);
  //next we get the local variabe, here is where we start to differ,
  //the result of the condition is on a local, right on top of the stack
  //with offset zero
  //TODO investigate if load local 0 can be omitted to save performances
  compareGetLocal(2,0);
  //next we have the other constant which we put on the stack and we do the 
  //condition evaluation
  compareConstant(4, 1, 5);
  compareInstruction(6, binder::vm::OP_CODE::OP_LESS);

  //next we break based on the condtion
  compareJumpFalse(7, 21, true);

  //next we jump over the increment clause with an uncoditional jump
  compareJump(11,11,false);

  //next we have the increment, the +1 and add
  compareGetLocal(14, 0);
  compareConstant(16, 2, 1.0);
  compareInstruction(18, binder::vm::OP_CODE::OP_ADD);
  //next we set the result on the local variable, since the result
  //is on top of the stack
  compareSetLocal(19,0);
  //we get rid of the value left on top of the stack from the operation and loop
  compareInstruction(21, binder::vm::OP_CODE::OP_POP);
  compareLoop(22,23,false);

  //now we actually evaluate the body , this is were we would jump to
  //getting the value on the stack
  compareGetLocal(25,0);
  compareInstruction(27, binder::vm::OP_CODE::OP_PRINT);
  //next we jump to the increment evaluation
  compareLoop(28,17,true);
  //we also need to remove the last value we left on the stack due to the 
  //evaluation
  compareInstruction(32, binder::vm::OP_CODE::OP_POP);
  compareInstruction(33, binder::vm::OP_CODE::OP_RETURN);


  /*
    == debug ==
    0000    0 OP_CONSTANT         0 '0
    0002    | OP_GET_LOCAL        0
    0004    | OP_CONSTANT         1 '5
    0006    | OP_LESS
    0007    | OP_JUMP_IF_FALSE    7 -> 31
    0010    | OP_POP
    0011    | OP_JUMP            11 -> 25
    0014    | OP_GET_LOCAL        0
    0016    | OP_CONSTANT         2 '1
    0018    | OP_ADD
    0019    | OP_SET_LOCAL        0
    0021    | OP_POP
    0022    | OP_LOOP            22 -> 2
    0025    | OP_GET_LOCAL        0
    0027    | OP_PRINT
    0028    | OP_LOOP            28 -> 14
    0031    | OP_POP
    0032    | OP_POP
    0033    | OP_RETURN
  */
}
