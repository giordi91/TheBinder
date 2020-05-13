#pragma once
#include "binder/memory/resizableVector.h"
#include "binder/vm/common.h"


namespace binder::vm {

enum class OP_CODE { OP_RETURN };

struct Chunk {
  memory::ResizableVector<uint8_t> m_code;
  memory::ResizableVector<uint8_t> m_constants;

  void write(OP_CODE op) { m_code.pushBack(static_cast<uint8_t>(op)); };
};

} // namespace binder::vm
