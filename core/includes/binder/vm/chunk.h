#pragma once
#include "binder/memory/resizableVector.h"
#include "binder/vm/common.h"

enum class OP_CODE { OP_RETURN };

namespace binder::vm {

using Chunk = memory::ResizableVector<uint8_t>;



void disassambleChunk(const Chunk* chunk,  const char* name);
void disassambleInstruction(const Chunk* chunk, int offset); 

}
