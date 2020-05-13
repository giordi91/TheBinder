#pragma once
#include "binder/memory/resizableVector.h"
#include "binder/vm/common.h"
#include "binder/vm/value.h"

namespace binder::vm {

enum class OP_CODE {
  OP_CONSTANT,
  OP_RETURN,
};

struct Chunk {
  memory::ResizableVector<uint8_t> m_code;
  memory::ResizableVector<Value> m_constants;

  void write(OP_CODE op) { m_code.pushBack(static_cast<uint8_t>(op)); };
  void write(uint8_t byte) { m_code.pushBack(static_cast<uint8_t>(byte)); };
  int addConstant(Value value) {
    m_constants.pushBack(value);
    return m_constants.size() - 1;
  };
};

} // namespace binder::vm
