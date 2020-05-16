#include "binder/log/bufferLog.h"
#include "binder/log/consoleLog.h"
#include "binder/vm/compiler.h"

#include "../catch.h"

class SetupVmParserTestFixture {
public:
  SetupVmParserTestFixture() {}
  const binder::vm::Chunk *compile(const char *source, bool debug = false) {
    result = compiler.compile(source, &m_log);

    chunk = compiler.getCompiledChunk();
    if (debug) {
      binder::vm::disassambleChunk(chunk, "debug", &m_debugLog);
    }

    return chunk;
  }

  void compareInstruction(int offset, binder::vm::OP_CODE expected) {
    compareInstruction(offset, static_cast<uint8_t>(expected));
  }

  void compareInstruction(int offset, uint8_t expected) {
    REQUIRE(offset <= chunk->m_code.size());
    REQUIRE(chunk->m_code[offset] == expected);
  }
  void compareConstant(int offset, int idx, double value) {
    REQUIRE(chunk->m_code[offset] == idx);
    REQUIRE(chunk->m_constants[idx].as.number == Approx(value));
  }

protected:
  binder::log::BufferedLog m_log;
  binder::log::ConsoleLog m_debugLog;
  binder::vm::Compiler compiler;
  const binder::vm::Chunk *chunk;
  bool result;
};

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm basic number parse",
                 "[vm-parser]") {

  const char *source = "12345.5";
  auto *chunk = compile(source, false);
  // op const + constant + op return
  REQUIRE(chunk->m_code.size() == 3);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  // should be the first constant in there
  compareConstant(1, 0, 12345.5);
  compareInstruction(2, binder::vm::OP_CODE::OP_RETURN);
  delete chunk;
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm basic multiply", "[vm-parser]") {

  const char *source = "77 *323.2;";
  auto *chunk = compile(source, false);
  // op const + constant + op return
  REQUIRE(chunk->m_code.size() == 6);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(1, 0, 77);

  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(3, 1, 323.2);

  compareInstruction(4, binder::vm::OP_CODE::OP_MULTIPLY);
  compareInstruction(5, binder::vm::OP_CODE::OP_RETURN);
  delete chunk;
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm unary precedence",
                 "[vm-parser]") {

  const char *source = "-1+ 5;";
  auto *chunk = compile(source, false);
  REQUIRE(chunk->m_code.size() == 7);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(1, 0, 1);
  compareInstruction(2, binder::vm::OP_CODE::OP_NEGATE);
  compareInstruction(3, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(4, 1, 5);
  compareInstruction(5, binder::vm::OP_CODE::OP_ADD);
  compareInstruction(6, binder::vm::OP_CODE::OP_RETURN);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm MAD 1", "[vm-parser]") {
  const char *source = "(144.4*3.14)+12;";
  auto *chunk = compile(source, false);
  REQUIRE(chunk->m_code.size() == 9);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(1, 0, 144.4);
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(3, 1, 3.14);
  compareInstruction(4, binder::vm::OP_CODE::OP_MULTIPLY);
  compareInstruction(5, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(6, 2, 12);
  compareInstruction(7, binder::vm::OP_CODE::OP_ADD);
  compareInstruction(8, binder::vm::OP_CODE::OP_RETURN);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm MAD 1 no grouping",
                 "[vm-parser]") {
  const char *source = "144.4*3.14+12;";
  auto *chunk = compile(source, false);
  REQUIRE(chunk->m_code.size() == 9);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(1, 0, 144.4);
  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(3, 1, 3.14);
  compareInstruction(4, binder::vm::OP_CODE::OP_MULTIPLY);
  compareInstruction(5, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(6, 2, 12);
  compareInstruction(7, binder::vm::OP_CODE::OP_ADD);
  compareInstruction(8, binder::vm::OP_CODE::OP_RETURN);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm MAD 2", "[vm-parser]") {
  const char *source = "(-1*3.14)+(--13);";
  auto *chunk = compile(source, false);
  REQUIRE(chunk->m_code.size() == 12);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(1, 0, 1);
  compareInstruction(2, binder::vm::OP_CODE::OP_NEGATE);
  compareInstruction(3, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(4, 1, 3.14);
  compareInstruction(5, binder::vm::OP_CODE::OP_MULTIPLY);
  compareInstruction(6, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(7, 2, 13);
  compareInstruction(8, binder::vm::OP_CODE::OP_NEGATE);
  compareInstruction(9, binder::vm::OP_CODE::OP_NEGATE);
  compareInstruction(10, binder::vm::OP_CODE::OP_ADD);
  compareInstruction(11, binder::vm::OP_CODE::OP_RETURN);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm expression error 1",
                 "[vm-parser]") {
  const char *source = "1 ** 1;";
  auto *chunk = compile(source, false);
  //check for failure
  REQUIRE(chunk == nullptr);
  REQUIRE(result == false);
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm expression error 2",
                 "[vm-parser]") {
  const char *source = "(1 ** 1;";
  auto *chunk = compile(source, false);
  //check for failure
  REQUIRE(chunk == nullptr);
  REQUIRE(result == false);
}
