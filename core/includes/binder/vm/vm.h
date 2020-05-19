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

#define DEBUG_TRACE_EXECUTION

class VirtualMachine {
public:
  VirtualMachine(log::Log *logger) : m_logger(logger) {}
#ifdef DEBUG_TRACE_EXECUTION
  VirtualMachine(log::Log *logger, log::Log *debugLogger)
      : m_logger(logger), m_debugLogger(debugLogger) {}
#endif
  ~VirtualMachine() = default;

  void init();
  void shutdown(){};
  INTERPRET_RESULT interpret(const char *source);

private:
  INTERPRET_RESULT run();

  // stack
  void resetStack() { m_stackTop = m_stack; };
  void stackPush(Value value);
  Value stackPop();

  //runtime operations
  void concatenate();

  // instructions
  inline uint8_t readByte() { return *m_ip++; }
  inline Value readConstant() { return m_chunk->m_constants[readByte()]; };
  //-1 gives us the first not freevalue and then we subtract the distance
  // since we want to go back in the stack
  inline Value peek(int distance) { return m_stackTop[-1 - distance]; }
  void runtimeError(const char *message);

private:
  static constexpr uint32_t STACK_MAX = 256;
  Value m_stack[STACK_MAX];
  Value *m_stackTop;
  log::Log *m_logger;
  const Chunk *m_chunk;
  uint8_t *m_ip;

#ifdef DEBUG_TRACE_EXECUTION
  log::Log *m_debugLogger = nullptr;
#endif
};

} // namespace vm
} // namespace binder
