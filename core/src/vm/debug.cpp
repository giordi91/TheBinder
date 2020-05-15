#include "binder/log/log.h"
#include "binder/vm/debug.h"
#include "stdio.h"

namespace binder::vm {
static char buffer[1024];

#define LOG(logger, toPrint, ...)                                              \
  do {                                                                         \
    sprintf(buffer, (toPrint), ##__VA_ARGS__);                                 \
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
  LOG(logger, "\n");
  return offset + 2;
}

void disassambleChunk(const Chunk *chunk, const char *name, log::Log *logger) {
  LOG(logger, "== %s ==\n", name);

  for (int offset = 0; offset < chunk->m_code.size();) {

    offset = disassambleInstruction(chunk, offset, logger);
  }
}
int disassambleInstruction(const Chunk *chunk, int offset, log::Log *logger) {

  // first we write the line offset, which we zero extend to 4 digits
  LOG(logger, "%04d ", offset);
  // next we write the source code lines the line comes from
  // since many instruction often expand from a single line we
  // use a | to identify the line is same as above otherwise
  // just write the digit of the line it comes from
  if (offset > 0 && chunk->m_lines[offset] == chunk->m_lines[offset - 1]) {
    LOG(logger, "   | ");
  } else {
    LOG(logger, "%4d ", chunk->m_lines[offset]);
  }

  // next we process the actual instruction
  auto instruction = static_cast<OP_CODE>(chunk->m_code[offset]);
  switch (instruction) {
  case OP_CODE::OP_CONSTANT:
    return constantInstruction("OP_CONSTANT", chunk, offset, logger);
  case OP_CODE::OP_ADD:
    return simpleInstruction("OP_ADD", offset, logger);
  case OP_CODE::OP_SUBTRACT:
    return simpleInstruction("OP_SUBTRACT", offset, logger);
  case OP_CODE::OP_MULTIPLY:
    return simpleInstruction("OP_MULTIPLY", offset, logger);
  case OP_CODE::OP_DIVIDE:
    return simpleInstruction("OP_DIVIDE", offset, logger);
  case OP_CODE::OP_NEGATE:
    return simpleInstruction("OP_NEGATE", offset, logger);
  case OP_CODE::OP_RETURN:
    return simpleInstruction("OP_RETURN", offset, logger);
  default:
    LOG(logger, "Unknown opcode %d\n", instruction);
    return offset + 1;
  }
}

#undef LOG

} // namespace binder::vm
