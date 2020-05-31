#pragma once

#include "binder/memory/resizableVector.h"
#include "binder/vm/common.h"
#include "binder/vm/value.h"

namespace binder::vm {

enum class OP_CODE {
  OP_CONSTANT,
  OP_NIL,
  //since bool need only two values it can get
  //so is a bit of a waste story it in the constant table
  //so we directly encode it as instruction, so if we want to push
  //the literal instead we use a dedicated instruction
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_GET_LOCAL,
  OP_GET_GLOBAL,
  OP_DEFINE_GLOBAL,
  OP_SET_LOCAL,
  OP_SET_GLOBAL,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NOT,
  OP_NEGATE,
  OP_PRINT,
  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_LOOP,
  OP_RETURN,
};

struct Chunk {
  memory::ResizableVector<uint8_t> m_code;
  memory::ResizableVector<uint16_t> m_lines;
  memory::ResizableVector<Value> m_constants;

  void write(const OP_CODE op, const uint16_t line) {
    m_code.pushBack(static_cast<uint8_t>(op));
    m_lines.pushBack(line);
  };
  void write(const uint8_t byte, const uint16_t line) {
    m_code.pushBack(static_cast<uint8_t>(byte));
    m_lines.pushBack(line);
  };
  int addConstant(const Value value) {
    m_constants.pushBack(value);
    return static_cast<int>(m_constants.size()) - 1;
  };
};

} // namespace binder::vm
