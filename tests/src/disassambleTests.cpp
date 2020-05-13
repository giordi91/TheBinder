#include "binder/vm/debug.h"
#include "binder/log/bufferLog.h"

#include "catch.h"

class SetupDisassamblerTestFixture {
public:
  SetupDisassamblerTestFixture(){}


protected:
  binder::log::BufferedLog m_log;
};

TEST_CASE_METHOD(SetupDisassamblerTestFixture,"nop", "[disassambler]") {

    //writing simple return op
    binder::vm::Chunk chunk;
    chunk.write(binder::vm::OP_CODE::OP_RETURN);
    REQUIRE(chunk.m_code.size()==1);

    binder::vm::disassambleChunk(&chunk,"test",&m_log);
    const char* buff = m_log.getBuffer();
    REQUIRE(strcmp("== test ==\n0000 OP_RETURN\n",buff)==0);

}
