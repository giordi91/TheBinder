#include "binder/log/log.h"
#include "binder/vm/debug.h"
#include "stdio.h"

namespace binder::vm {
static char*buffer[1024];


static int simpleInstruction(const char *name, int offset, log::Log *logger,
                             char *buffer) {
  sprintf(buffer,"%s\n", name);
  logger->print(buffer);
  return offset + 1;
}

void disassambleChunk(const Chunk *chunk, const char *name, log::Log *logger) {
  char buffer[256];
  sprintf(buffer, "== %s ==\n", name);
  logger->print(buffer);

  for (int offset = 0; offset < chunk->m_code.size();) {
    offset = disassambleInstruction(chunk, offset, logger,buffer);
  }
}
int disassambleInstruction(const Chunk *chunk, int offset, log::Log *logger,
                           char *buffer) {

  sprintf(buffer, "%04d ", offset);
  logger->print(buffer);

  auto instruction = static_cast<OP_CODE>(chunk->m_code[offset]);
  switch (instruction) {
  case OP_CODE::OP_RETURN:
    return simpleInstruction("OP_RETURN", offset, logger, buffer);
  default:
    sprintf(buffer,"Unknown opcode %d\n", instruction);
    logger->print(buffer);
    return offset + 1;
  }
}

} // namespace binder::vm
