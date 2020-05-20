#include "binder/log/log.h"
#include "binder/vm/compiler.h"
#include "binder/vm/debug.h"
#include "binder/vm/memory.h"
#include "binder/vm/value.h"
#include "binder/vm/vm.h"

namespace binder::vm {

// TODO can i refactor this? is a bit too big for my likning,
// at the end of the day the only thing that needs the macro is (a op b)
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

void VirtualMachine::init() { resetStack(); }
VirtualMachine::~VirtualMachine() { freeAllocations(); }

void VirtualMachine::stackPush(Value value) {
  *m_stackTop = value;
  ++m_stackTop;
}

Value VirtualMachine::stackPop() {
  --m_stackTop;
  return *m_stackTop;
}

void VirtualMachine::concatenate() {
  ObjString *b = valueAsString(stackPop());
  ObjString *a = valueAsString(stackPop());

  int length = a->length + b->length;
  char *chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  // interning the characters
  chars = (char *)m_intern.intern(chars, length, false);
  ObjString *result = allocateString(chars, length);
  stackPush(makeObject(result));
}

void VirtualMachine::runtimeError(const char *message) {
  auto instruction = static_cast<uint32_t>(m_ip - m_chunk->m_code.data() - 1);
  int line = m_chunk->m_lines[instruction];
  log::LOG(m_logger, "%s\n[line %d] in script\n", message, line);
  resetStack();
}

bool isFalsey(Value value) {
  // first we check wether the value is null, we also check if the value is bool
  // then we also negate the bool value
  return isValueNIL(value) | isValueBool(value) && (!valueAsBool(value));
}

INTERPRET_RESULT VirtualMachine::interpret(const char *source) {

  Compiler compiler(&m_intern);

  if (!compiler.compile(source, m_logger)) {
    return INTERPRET_RESULT::INTERPRET_COMPILE_ERROR;
  }

  m_chunk = compiler.getCompiledChunk();
  m_ip = m_chunk->m_code.data();
  resetStack();
  return run();
}

//#define DEBUG_TRACE_EXECUTION

INTERPRET_RESULT VirtualMachine::run() {
  for (;;) {

#ifdef DEBUG_TRACE_EXECUTION
    assert(m_debugLogger != nullptr);
    // show the stack  before each instruction
    // this is going to spam!
    // TODO wrap this into another ifdef so we can turn it on/off independnetly
    m_debugLogger->print("          ");
    for (Value *slot = m_stack; slot < m_stackTop; slot++) {
      m_debugLogger->print("[ ");
      printValue(*slot, m_debugLogger);
      m_debugLogger->print(" ]");
    }
    m_debugLogger->print("\n");

    disassambleInstruction(m_chunk, (int)(m_ip - m_chunk->m_code.data()),
                           m_debugLogger);
#endif

    OP_CODE instruction;
    switch (instruction = static_cast<OP_CODE>(readByte())) {
    case OP_CODE::OP_PRINT: {
      printValue(stackPop(),m_logger);
      m_logger->print("\n");
      break;
    }
    case OP_CODE::OP_RETURN: {
      return INTERPRET_RESULT::INTERPRET_OK;
    }
    case OP_CODE::OP_CONSTANT: {
      Value constant = readConstant();
      stackPush(constant);
      break;
    }
    case OP_CODE::OP_NIL: {
      stackPush(makeNIL());
      break;
    }
    case OP_CODE::OP_TRUE: {
      stackPush(makeBool(true));
      break;
    }
    case OP_CODE::OP_FALSE: {
      stackPush(makeBool(false));
      break;
    }
    case OP_CODE::OP_POP: {
      stackPop();
      break;
    }
    case OP_CODE::OP_EQUAL: {
      Value b = stackPop();
      Value a = stackPop();
      Value res = makeBool(valuesEqual(a, b));
      stackPush(res);
      break;
    }
    case OP_CODE::OP_GREATER: {
      BINARY_OP(makeBool, >);
      break;
    }
    case OP_CODE::OP_LESS: {
      BINARY_OP(makeBool, <);
      break;
    }
    case OP_CODE::OP_ADD: {
      if (isValueString(peek(0)) & isValueString(peek(1))) {
        concatenate();
      } else if (isValueNumber(peek(0)) & isValueNumber(peek(1))) {
        BINARY_OP(makeNumber, +);
      } else {
        runtimeError("Operands must be two numbers of two strings");
        return INTERPRET_RESULT::INTERPRET_RUNTIME_ERROR;
      }
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
    case OP_CODE::OP_NOT: {
      Value result = makeBool(isFalsey(stackPop()));
      stackPush(result);
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

} // namespace binder::vm
