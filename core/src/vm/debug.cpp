#include "binder/log/log.h"
#include "binder/vm/debug.h"
#include "stdio.h"

namespace binder::vm {
static char buffer[1024];

#define LOG(logger, toPrint, ...)                                              \
  do {                                                                         \
    sprintf(buffer, (toPrint), __VA_ARGS__);                                   \
    logger->print(buffer);                                                     \
  } while (false);

static int simpleInstruction(const char *name, int offset, log::Log *logger) {
  LOG(logger, "%s\n", name);
  return offset + 1;
}
static int constantInstruction(const char *name, const Chunk *chunk, int offset,
                               log::Log *logger) {
  uint8_t constant = chunk->m_code[offset + 1];
  LOG(logger, "%-16s %4d '", name, constant);
  printValue(chunk->m_constants[constant], logger);
  LOG(logger,"\n");
  return offset + 2;
}

void disassambleChunk(const Chunk *chunk, const char *name, log::Log *logger) {
  LOG(logger, "== %s ==\n", name);

  for (int offset = 0; offset < chunk->m_code.size();) {
    offset = disassambleInstruction(chunk, offset, logger);
  }
}
int disassambleInstruction(const Chunk *chunk, int offset, log::Log *logger) {

  sprintf(buffer, "%04d ", offset);
  logger->print(buffer);

  auto instruction = static_cast<OP_CODE>(chunk->m_code[offset]);
  switch (instruction) {
  case OP_CODE::OP_CONSTANT:
    return constantInstruction("OP_CONSTANT", chunk, offset, logger);
  case OP_CODE::OP_RETURN:
    return simpleInstruction("OP_RETURN", offset, logger);
  default:
    LOG(logger, "Unknown opcode %d\n", instruction);
    return offset + 1;
  }
}

} // namespace binder::vm
