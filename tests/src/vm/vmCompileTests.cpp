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

    chunk = compiler.getCompiledChunk();
    if (debug && chunk != nullptr) {
      binder::vm::disassambleChunk(chunk, "debug", &m_debugLog);
    }

    return chunk;
  }

  void compareInstruction(int offset, binder::vm::OP_CODE expected) {
    compareInstruction(offset, static_cast<uint8_t>(expected));
  }

  void compareInstruction(uint32_t offset, uint8_t expected) {
    REQUIRE(offset <= chunk->m_code.size());
    REQUIRE(chunk->m_code[offset] == expected);
  }
  void compareConstant(uint32_t offset, int idx, double value) {
    REQUIRE(chunk->m_code[offset] == idx);
    REQUIRE(chunk->m_constants[idx].as.number == Approx(value));
  }

  void compareConstant(uint32_t offset, int idx, bool value) {
    REQUIRE(chunk->m_code[offset] == idx);
    REQUIRE(chunk->m_constants[idx].as.boolean == value);
  }

  void compareConstant(uint32_t offset, int idx, const char *toCompare) {
    REQUIRE(chunk->m_code[offset] == idx);
    // REQUIRE(chunk->m_constants[idx].as.number == Approx(value));
    binder::vm::Value value = chunk->m_constants[idx];
    REQUIRE(binder::vm::isValueObj(value));
    REQUIRE(binder::vm::isObjType(value, binder::vm::OBJ_TYPE::OBJ_STRING));
    REQUIRE(strcmp(binder::vm::valueAsCString(value), toCompare) == 0);
  }
  void compareJumpOffset(uint32_t offset, uint16_t expectedJump) {
    auto jump = static_cast<uint16_t>(chunk->m_code[offset] << 8);
    jump |= chunk->m_code[offset + 1];
    REQUIRE(jump == expectedJump);
  }

  void printOutput() { printf("%s\n", m_log.getBuffer()); }

protected:
  binder::log::BufferedLog m_log;
  binder::log::ConsoleLog m_debugLog;
  binder::memory::StringIntern intern;

  binder::vm::Compiler compiler;
  const binder::vm::Chunk *chunk;
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
  compareConstant(1, 0, 12345.5);
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
  compareConstant(1, 0, 77.0);

  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(3, 1, 323.2);

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
  compareConstant(1, 0, 1.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_NEGATE);
  compareInstruction(3, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(4, 1, 5.0);
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
  compareConstant(1, 0, 144.4);
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(3, 1, 3.14);
  compareInstruction(4, binder::vm::OP_CODE::OP_MULTIPLY);
  compareInstruction(5, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(6, 2, 12.0);
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
  compareConstant(1, 0, 144.4);
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(3, 1, 3.14);
  compareInstruction(4, binder::vm::OP_CODE::OP_MULTIPLY);
  compareInstruction(5, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(6, 2, 12.0);
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
  compareConstant(1, 0, 1.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_NEGATE);
  compareInstruction(3, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(4, 1, 3.14);
  compareInstruction(5, binder::vm::OP_CODE::OP_MULTIPLY);
  compareInstruction(6, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(7, 2, 13.0);
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
  compareConstant(1, 0, "hello world");
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
  compareConstant(1, 0, "hello world");
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(3, 1, "hello world");
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
  compareConstant(1, 0, "hello world");
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(3, 1, "hello world");
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
  compareConstant(1, 0, "hello ");
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(3, 1, "world");
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
  compareConstant(2, 0, "test");
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
  compareConstant(1, 1, "my first var");
  // then we have a global definition, which behaves as a constant,
  // first the opcode then the uin8_t pointing to where it is stored
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstant(3, 0, "myVar");
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
  compareConstant(1, 1, 10.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstant(3, 0, "myVar");
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
  compareConstant(2, 0, "myVar");
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
  compareConstant(2, 0, "myVar");
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
  compareConstant(1, 1, 1.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(3, 2, 2.0);
  compareInstruction(4, binder::vm::OP_CODE::OP_ADD);
  compareInstruction(5, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstant(6, 0, "init1");
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
  compareConstant(1, 1, 10.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstant(3, 0, "myVar");
  compareInstruction(4, binder::vm::OP_CODE::OP_GET_GLOBAL);
  compareConstant(5, 2, "myVar");
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
  compareConstant(1, 1, "beignets");
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstant(3, 0, "breakfast");

  // defining the second global variable
  compareInstruction(4, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(5, 3, "cafe au lait");
  compareInstruction(6, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstant(7, 2, "beverage");

  // now that wehave the twovariables we need to perform the assigment
  // which is between first a constant
  compareInstruction(8, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(9, 5, "beignets with ");

  // then the global variable  which we pop on the stack now
  compareInstruction(10, binder::vm::OP_CODE::OP_GET_GLOBAL);
  compareConstant(11, 6, "beverage");

  // doing the addition
  compareInstruction(12, binder::vm::OP_CODE::OP_ADD);
  compareInstruction(13, binder::vm::OP_CODE::OP_SET_GLOBAL);
  compareConstant(14, 4, "breakfast");
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
  compareConstant(1, 1, 12.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstant(3, 0, "a");
  compareInstruction(4, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(5, 2, 20.0);
  compareInstruction(6, binder::vm::OP_CODE::OP_GET_LOCAL);
  // comparing the get local slot
  compareInstruction(7, 0);
  compareInstruction(8, binder::vm::OP_CODE::OP_PRINT);
  // we are getting out of scope, so we pop
  compareInstruction(9, binder::vm::OP_CODE::OP_POP);

  // then the global variable
  compareInstruction(10, binder::vm::OP_CODE::OP_GET_GLOBAL);
  compareConstant(11, 3, "a");
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
  compareConstant(1, 1, 12.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstant(3, 0, "a");
  compareInstruction(4, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(5, 2, 20.0);
  compareInstruction(6, binder::vm::OP_CODE::OP_GET_LOCAL);
  // comparing the get local slot
  compareInstruction(7, 0);
  compareInstruction(8, binder::vm::OP_CODE::OP_PRINT);
  // we are getting out of scope, so we pop
  compareInstruction(9, binder::vm::OP_CODE::OP_POP);

  // then the global variable
  compareInstruction(10, binder::vm::OP_CODE::OP_GET_GLOBAL);
  compareConstant(11, 3, "a");
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
  compareConstant(1, 1, 5.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstant(3, 0, "a");
  compareInstruction(4, binder::vm::OP_CODE::OP_GET_GLOBAL);
  compareConstant(5, 2, "a");
  compareInstruction(6, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(7, 3, 3.0);
  compareInstruction(8, binder::vm::OP_CODE::OP_GREATER);
  compareInstruction(9, binder::vm::OP_CODE::OP_JUMP_IF_FALSE);
  compareJumpOffset(10, 7);
  compareInstruction(12, binder::vm::OP_CODE::OP_POP);
  compareInstruction(13, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(14, 4, 10.0);
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
  const char *source = "var a = 5;\n if(a > 3){\n print 10; \n} else{\n print 20;\n}";
  auto *chunk = compile(source, false);
  REQUIRE(chunk != nullptr);
  REQUIRE(result == true);
  return;
  REQUIRE(chunk->m_code.size() == 23);

  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(1, 1, 5.0);
  compareInstruction(2, binder::vm::OP_CODE::OP_DEFINE_GLOBAL);
  compareConstant(3, 0, "a");
  compareInstruction(4, binder::vm::OP_CODE::OP_GET_GLOBAL);
  compareConstant(5, 2, "a");
  compareInstruction(6, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(7, 3, 3.0);
  compareInstruction(8, binder::vm::OP_CODE::OP_GREATER);
  compareInstruction(9, binder::vm::OP_CODE::OP_JUMP_IF_FALSE);
  compareJumpOffset(10, 7);
  compareInstruction(12, binder::vm::OP_CODE::OP_POP);
  compareInstruction(13, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(14, 4, 10.0);
  compareInstruction(15, binder::vm::OP_CODE::OP_PRINT);
  compareInstruction(16, binder::vm::OP_CODE::OP_JUMP);
  compareJumpOffset(17, 4);
  compareInstruction(19, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(20, 5, 20.0);
  compareInstruction(21, binder::vm::OP_CODE::OP_PRINT);
  compareInstruction(22, binder::vm::OP_CODE::OP_RETURN);
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
