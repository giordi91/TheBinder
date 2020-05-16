#include "binder/log/bufferLog.h"
#include "binder/log/consoleLog.h"
#include "binder/vm/compiler.h"

#include "../catch.h"

class SetupVmParserTestFixture {
public:
  SetupVmParserTestFixture() {}
  const binder::vm::Chunk *compile(const char *source, bool debug = false) {
    compiler.compile(source, &m_log);

    chunk = compiler.getCompiledChunk();
    if (debug) {
      binder::vm::disassambleChunk(chunk, "debug", &m_debugLog);
    }

    return chunk;
  }

  void compareNextToken(binder::vm::TokenType type, int len, const char *start,
                        const char *expectedVal) {
    /*
    parser.advance();
    REQUIRE(parser.current.type == type);
    REQUIRE(parser.current.length == len);
    REQUIRE(parser.current.start == start);
    REQUIRE(strncmp(parser.current.start,expectedVal,len)==0);
    */
  }

  void compareInstruction(int offset, binder::vm::OP_CODE expected) {
    compareInstruction(offset, static_cast<uint8_t>(expected));
  }

  void compareInstruction(int offset, uint8_t expected) {
    REQUIRE(offset <= chunk->m_code.size());
    REQUIRE(chunk->m_code[offset] == expected);
  }
  void compareConstant(int offset, int idx, double value)
  {
      REQUIRE(chunk->m_code[offset] == idx);
      REQUIRE(chunk->m_constants[idx] ==Approx(value));

  }

protected:
  binder::log::BufferedLog m_log;
  binder::log::ConsoleLog m_debugLog;
  binder::vm::Compiler compiler;
  const binder::vm::Chunk *chunk;
};

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm basic number parse",
                 "[vm-parser]") {

  const char *source = "12345.5";
  auto *chunk = compile(source, false);
  // op const + constant + op return
  REQUIRE(chunk->m_code.size() == 3);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  //should be the first constant in there
  compareConstant(1,0,12345.5);
  compareInstruction(2, binder::vm::OP_CODE::OP_RETURN);
  delete chunk;
}

TEST_CASE_METHOD(SetupVmParserTestFixture, "vm basic multiply", "[vm-parser]") {

  const char *source = "77 *323.2;";
  auto *chunk = compile(source, false);
  // op const + constant + op return
  REQUIRE(chunk->m_code.size() == 6);
  compareInstruction(0, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(1,0,77);

  compareInstruction(2, binder::vm::OP_CODE::OP_CONSTANT);
  compareConstant(3,1,323.2);

  compareInstruction(4, binder::vm::OP_CODE::OP_MULTIPLY);
  compareInstruction(5, binder::vm::OP_CODE::OP_RETURN);
  delete chunk;
}
