#pragma once
#include "binder/vm/chunk.h"

namespace binder {

namespace log {
class Log;
}

namespace vm {

enum INTERPRET_RESULT {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,
};

class VirtualMachine {
public:
  VirtualMachine(log::Log *logger) : m_logger(logger) {}
  ~VirtualMachine() = default;

  void init();
  void shutdown(){};
  INTERPRET_RESULT intepret(const char* source);

private:
  INTERPRET_RESULT run();

  // stack
  void resetStack() { m_stackTop = m_stack; };
  void stackPush(Value value);
  Value stackPop();

  // instructions
  inline uint8_t readByte() { return *m_ip++; }
  inline Value readConstant() { return m_chunk->m_constants[readByte()]; };


private:
  static constexpr uint32_t STACK_MAX = 256;
  Value m_stack[STACK_MAX];
  Value* m_stackTop;
  log::Log *m_logger;
  const Chunk *m_chunk;
  uint8_t *m_ip;
};

} // namespace vm
} // namespace binder
