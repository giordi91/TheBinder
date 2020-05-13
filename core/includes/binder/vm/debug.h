#pragma once
#include "binder/vm/chunk.h"

namespace binder {
namespace log {

class Log;
}

namespace vm {

void disassambleChunk(const Chunk *chunk, const char *name,
                      log::Log *logger);
int disassambleInstruction(const Chunk *chunk, int offset, log::Log *logger);
} // namespace vm
} // namespace binder
