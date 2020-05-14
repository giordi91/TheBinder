#include "binder/log/log.h"
#include "binder/vm/debug.h"
#include "binder/vm/value.h"
#include "binder/vm/vm.h"

namespace binder::vm {

#define BINARY_OP(op)                                                          \
  do {                                                                         \
    double b = stackPop();                                                     \
    double a = stackPop();                                                     \
    stackPush(a op b);                                                         \
  } while (false)

void VirtualMachine::init() { resetStack(); }

void VirtualMachine::stackPush(Value value) {
  *m_stackTop = value;
  ++m_stackTop;
}

Value VirtualMachine::stackPop() {
  --m_stackTop;
  return *m_stackTop;
}

INTERPRET_RESULT VirtualMachine::intepret(const Chunk *chunk) {
  m_chunk = chunk;
  m_ip = chunk->m_code.data();
  return INTERPRET_RESULT::INTERPRET_OK;
}
#define DEBUG_TRACE_EXECUTION

INTERPRET_RESULT VirtualMachine::run() {
  for (;;) {

#ifdef DEBUG_TRACE_EXECUTION
    // show the stack  before each instruction
    // this is going to spam!
    // TODO wrap this into another ifdef so we can turn it on/off independnetly
    m_logger->print("          ");
    for (Value *slot = m_stack; slot < m_stackTop; slot++) {
      m_logger->print("[ ");
      printValue(*slot, m_logger);
      m_logger->print(" ]");
    }
    m_logger->print("\n");

    disassambleInstruction(m_chunk, (int)(m_ip - m_chunk->m_code.data()),
                           m_logger);
#endif

    OP_CODE instruction;
    switch (instruction = static_cast<OP_CODE>(readByte())) {
    case OP_CODE::OP_RETURN: {
      printValue(stackPop(), m_logger);
      m_logger->print("\n");
      return INTERPRET_RESULT::INTERPRET_OK;
    }
    case OP_CODE::OP_CONSTANT: {
      Value constant = readConstant();
      // temprarely we print the constant
      stackPush(constant);
      break;
    }
    case OP_CODE::OP_ADD: {
      BINARY_OP(+);
      break;
    }
    case OP_CODE::OP_SUBTRACT: {
      BINARY_OP(-);
      break;
    }
    case OP_CODE::OP_MULTIPLY: {
      BINARY_OP(*);
      break;
    }
    case OP_CODE::OP_DIVIDE: {
      BINARY_OP(/);
      break;
    }
    case OP_CODE::OP_NEGATE: {
      stackPush(-stackPop());
      break;
    }
    }
  }
}

} // namespace binder::vm
