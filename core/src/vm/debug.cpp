#include "binder/log/log.h"
#include "binder/vm/debug.h"
#include "stdarg.h"
#include "stdio.h"

namespace binder::vm {

static int simpleInstruction(const char *name, int offset, log::Log *logger) {
  log::LOG(logger, "%s\n", name);
  return offset + 1;
}
static int constantInstruction(const char *name, const Chunk *chunk, int offset,
                               log::Log *logger) {
  uint8_t constant = chunk->m_code[offset + 1];
  log::LOG(logger, "%-16s %4d '", name, constant);
  printValue(chunk->m_constants[constant], logger);
  log::LOG(logger, "\n");
  return offset + 2;
}
static int byteInstruction(const char *name, const Chunk *chunk, int offset,
                           log::Log *logger) {
  uint8_t slot = chunk->m_code[offset + 1];
  log::LOG(logger, "%-16s %4d\n", name, slot);
  return offset + 2;
}
static int jumpInstruction(const char *name, int sign, const Chunk *chunk,
                           int offset, log::Log *logger) {
  auto jump = static_cast<uint16_t>(chunk->m_code[offset + 1] << 8);
  jump |= chunk->m_code[offset + 2];
  log::LOG(logger, "%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
  return offset + 3;
}

void disassambleChunk(const Chunk *chunk, const char *name, log::Log *logger) {
  log::LOG(logger, "== %s ==\n", name);

  for (uint32_t offset = 0; offset < chunk->m_code.size();) {

    offset = disassambleInstruction(chunk, offset, logger);
  }
}
int disassambleInstruction(const Chunk *chunk, int offset, log::Log *logger) {

  // first we write the line offset, which we zero extend to 4 digits
  log::LOG(logger, "%04d ", offset);
  // next we write the source code lines the line comes from
  // since many instruction often expand from a single line we
  // use a | to identify the line is same as above otherwise
  // just write the digit of the line it comes from
  if (offset > 0 && chunk->m_lines[offset] == chunk->m_lines[offset - 1]) {
    log::LOG(logger, "   | ");
  } else {
    log::LOG(logger, "%4d ", chunk->m_lines[offset]);
  }

  // next we process the actual instruction
  auto instruction = static_cast<OP_CODE>(chunk->m_code[offset]);
  switch (instruction) {
  case OP_CODE::OP_CONSTANT:
    return constantInstruction("OP_CONSTANT", chunk, offset, logger);
  case OP_CODE::OP_NIL:
    return simpleInstruction("OP_NIL", offset, logger);
  case OP_CODE::OP_FALSE:
    return simpleInstruction("OP_FALSE", offset, logger);
  case OP_CODE::OP_POP:
    return simpleInstruction("OP_POP", offset, logger);
  case OP_CODE::OP_DEFINE_GLOBAL:
    return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset, logger);
  case OP_CODE::OP_GET_LOCAL:
    return byteInstruction("OP_GET_LOCAL", chunk, offset, logger);
  case OP_CODE::OP_SET_LOCAL:
    return byteInstruction("OP_SET_LOCAL", chunk, offset, logger);
  case OP_CODE::OP_GET_GLOBAL:
    return constantInstruction("OP_GET_GLOBAL", chunk, offset, logger);
  case OP_CODE::OP_SET_GLOBAL:
    return constantInstruction("OP_SET_GLOBAL", chunk, offset, logger);
  case OP_CODE::OP_EQUAL:
    return simpleInstruction("OP_EQUAL", offset, logger);
  case OP_CODE::OP_GREATER:
    return simpleInstruction("OP_GREATER", offset, logger);
  case OP_CODE::OP_LESS:
    return simpleInstruction("OP_LESS", offset, logger);
  case OP_CODE::OP_TRUE:
    return simpleInstruction("OP_TRUE", offset, logger);
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
  case OP_CODE::OP_PRINT:
    return simpleInstruction("OP_PRINT", offset, logger);
  case OP_CODE::OP_JUMP:
    return jumpInstruction("OP_JUMP", 1, chunk, offset, logger);
  case OP_CODE::OP_JUMP_IF_FALSE:
    return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset, logger);
  case OP_CODE::OP_RETURN:
    return simpleInstruction("OP_RETURN", offset, logger);
  default:
    log::LOG(logger, "Unknown opcode %d\n", instruction);
    return offset + 1;
  }
}

} // namespace binder::vm
