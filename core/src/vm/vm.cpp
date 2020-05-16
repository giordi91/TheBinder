#include "binder/log/log.h"
#include "binder/vm/compiler.h"
#include "binder/vm/debug.h"
#include "binder/vm/value.h"
#include "binder/vm/vm.h"

namespace binder::vm {
static char vmBuffer[1024];

#define BINARY_OP(valueType, op)                                               \
  do {                                                                         \
    if ((!isValueNumber(peek(0))) | (!isValueNumber(peek(1)))) {               \
      runtimeError("Operands must be numbers.");                               \
      return INTERPRET_RESULT::INTERPRET_RUNTIME_ERROR;                        \
    }                                                                          \
    double b = valueAsNumber(stackPop());                                      \
    double a = valueAsNumber(stackPop());                                      \
    stackPush(valueType(a op b));                                              \
  } while (false)

#define LOG(logger, toPrint, ...)                                              \
  do {                                                                         \
    sprintf(vmBuffer, (toPrint), ##__VA_ARGS__);                               \
    m_logger->print(vmBuffer);                                                 \
  } while (false);

void VirtualMachine::init() { resetStack(); }

void VirtualMachine::stackPush(Value value) {
  *m_stackTop = value;
  ++m_stackTop;
}

Value VirtualMachine::stackPop() {
  --m_stackTop;
  return *m_stackTop;
}

void VirtualMachine::runtimeError(const char *message) {
  auto instruction = static_cast<uint32_t>(m_ip - m_chunk->m_code.data() - 1);
  int line = m_chunk->m_lines[instruction];
  LOG("%s\n[line %d] in script\n", message, line);
  resetStack();
}

INTERPRET_RESULT VirtualMachine::intepret(const char *source) {
  Chunk chunk;

  Compiler compiler;

  if (!compiler.compile(source, m_logger)) {
    return INTERPRET_RESULT::INTERPRET_COMPILE_ERROR;
  }

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
      stackPush(constant);
      break;
    }
    case OP_CODE::OP_ADD: {
      BINARY_OP(makeNumber, +);
      break;
    }
    case OP_CODE::OP_SUBTRACT: {
      BINARY_OP(makeNumber, -);
      break;
    }
    case OP_CODE::OP_MULTIPLY: {
      BINARY_OP(makeNumber, *);
      break;
    }
    case OP_CODE::OP_DIVIDE: {
      BINARY_OP(makeNumber, /);
      break;
    }
    case OP_CODE::OP_NEGATE: {
      if (!isValueNumber(peek(0))) {
        runtimeError("Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }
      double negated = -valueAsNumber(stackPop());
      stackPush(makeNumber(negated));
      break;
    }
    }
  }
}

#undef LOG

} // namespace binder::vm
