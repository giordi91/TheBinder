#include "binder/vm/debug.h"
#include "binder/log/bufferLog.h"

#include "catch.h"

class SetupDisassamblerTestFixture {
public:
  SetupDisassamblerTestFixture(){}


protected:
  binder::log::BufferedLog m_log;
};

TEST_CASE_METHOD(SetupDisassamblerTestFixture,"return", "[disassambler]") {

    //writing simple return op
    binder::vm::Chunk chunk;
    chunk.write(binder::vm::OP_CODE::OP_RETURN,0);
    REQUIRE(chunk.m_code.size()==1);

    binder::vm::disassambleChunk(&chunk,"test",&m_log);
    const char* buff = m_log.getBuffer();
    REQUIRE(strcmp("== test ==\n0000    0 OP_RETURN\n",buff)==0);

}

TEST_CASE_METHOD(SetupDisassamblerTestFixture,"constant", "[disassambler]") {

    //writing simple return op
    binder::vm::Chunk chunk;
    int constant = chunk.addConstant(1.2);
    chunk.write(binder::vm::OP_CODE::OP_CONSTANT,0);
    chunk.write(static_cast<uint8_t>(constant),0);
    REQUIRE(chunk.m_code.size()==2);

    binder::vm::disassambleChunk(&chunk,"test",&m_log);
    const char* buff = m_log.getBuffer();
    REQUIRE(strcmp("== test ==\n0000    0 OP_CONSTANT         0 '1.2\n",buff)==0);

}
