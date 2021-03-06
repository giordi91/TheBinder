#include "binder/log/bufferLog.h"
#include "binder/vm/compiler.h"
#include "binder/vm/debug.h"
#include "binder/memory/stringIntern.h"

#include "../catch.h"

class SetupDisassamblerTestFixture {
public:
  SetupDisassamblerTestFixture() {}

protected:
  binder::log::BufferedLog m_log;
};

TEST_CASE_METHOD(SetupDisassamblerTestFixture, "return disassamble",
                 "[disassambler]") {

  // writing simple return op
  binder::vm::Chunk chunk;
  chunk.write(binder::vm::OP_CODE::OP_RETURN, 0);
  REQUIRE(chunk.m_code.size() == 1);

  binder::vm::disassambleChunk(&chunk, "test", &m_log);
  const char *buff = m_log.getBuffer();
  REQUIRE(strcmp("== test ==\n0000    0 OP_RETURN\n", buff) == 0);
}

TEST_CASE_METHOD(SetupDisassamblerTestFixture, "constant disassamble",
                 "[disassambler]") {

  binder::vm::Chunk chunk;
  int constant = chunk.addConstant(binder::vm::makeNumber(1.2));
  chunk.write(binder::vm::OP_CODE::OP_CONSTANT, 0);
  chunk.write(static_cast<uint8_t>(constant), 0);
  REQUIRE(chunk.m_code.size() == 2);

  binder::vm::disassambleChunk(&chunk, "test", &m_log);
  const char *buff = m_log.getBuffer();
  REQUIRE(strcmp("== test ==\n0000    0 OP_CONSTANT         0 '1.2\n", buff) ==
          0);
}

TEST_CASE_METHOD(SetupDisassamblerTestFixture, "binary disassamble",
                 "[disassambler]") {

  // writing -((1.2 + 3.4) / 5.6)
  binder::vm::Chunk chunk;
  int constant = chunk.addConstant(binder::vm::makeNumber(1.2));
  chunk.write(binder::vm::OP_CODE::OP_CONSTANT, 123);
  chunk.write(static_cast<uint8_t>(constant), 123);

  constant = chunk.addConstant(binder::vm::makeNumber(3.4));
  chunk.write(binder::vm::OP_CODE::OP_CONSTANT, 123);
  chunk.write(static_cast<uint8_t>(constant), 123);

  chunk.write(binder::vm::OP_CODE::OP_ADD, 123);

  constant = chunk.addConstant(binder::vm::makeNumber(5.6));
  chunk.write(binder::vm::OP_CODE::OP_CONSTANT, 123);
  chunk.write(static_cast<uint8_t>(constant), 123);

  chunk.write(binder::vm::OP_CODE::OP_DIVIDE, 123);
  chunk.write(binder::vm::OP_CODE::OP_NEGATE, 123);
  chunk.write(binder::vm::OP_CODE::OP_RETURN, 123);

  binder::vm::disassambleChunk(&chunk, "test", &m_log);
  const char *buff = m_log.getBuffer();
  REQUIRE(strcmp("== test ==\n0000  123 OP_CONSTANT         0 '1.2\n0002    | "
                 "OP_CONSTANT         1 '3.4\n0004    | OP_ADD\n0005    | "
                 "OP_CONSTANT         2 '5.6\n0007    | OP_DIVIDE\n0008    | "
                 "OP_NEGATE\n0009    | OP_RETURN\n",
                 buff) == 0);

  /*
    == test ==
    0000  123 OP_CONSTANT         0 '1.2
    0002    | OP_CONSTANT         1 '3.4
    0004    | OP_ADD
    0005    | OP_CONSTANT         2 '5.6
    0007    | OP_DIVIDE
    0008    | OP_NEGATE
    0009    | OP_RETURN
*/
}

TEST_CASE_METHOD(SetupDisassamblerTestFixture, "temp", "[disassambler]") {

  const char *source = "1 + 2 * 4;";

  binder::memory::StringIntern intern(1024);
  binder::vm::Compiler comp(&intern);
  bool result= comp.compile(source, &m_log);
  REQUIRE(result == true);
  const binder::vm::Chunk* chunk= comp.getCompiledChunk();

  binder::vm::disassambleChunk(chunk,"code",&m_log);

  const char *buff = m_log.getBuffer();
  REQUIRE(strcmp("== code ==\n0000    0 OP_CONSTANT         0 '1\n"
                 "0002    | OP_CONSTANT         1 '2\n"
                 "0004    | OP_CONSTANT         2 '4\n"
                 "0006    | OP_MULTIPLY\n"
                 "0007    | OP_ADD\n"
                 "0008    | OP_POP\n"
                 "0009    | OP_RETURN\n",
                 buff) == 0);

  /*
  == code ==
  0000    0 OP_CONSTANT         0 '1
  0002    | OP_CONSTANT         1 '2
  0004    | OP_CONSTANT         2 '4
  0006    | OP_MULTIPLY
  0007    | OP_ADD
  0008    | OP_POP
  0009    | OP_RETURN
  */
}
