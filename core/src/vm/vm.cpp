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

INTERPRET_RESULT VirtualMachine::compile(const char *source) {
  Compiler compiler(&m_intern);

  if (!compiler.compile(source, m_logger)) {
    return INTERPRET_RESULT::INTERPRET_COMPILE_ERROR;
  }
  m_chunk = compiler.getCompiledChunk();
  return INTERPRET_RESULT::INTERPRET_OK;
}

INTERPRET_RESULT VirtualMachine::interpret(const char *source) {

  if (compile(source) != INTERPRET_RESULT::INTERPRET_OK) {
    return INTERPRET_RESULT::INTERPRET_COMPILE_ERROR;
  }

  m_ip = m_chunk->m_code.data();
  resetStack();
  return run();
}

INTERPRET_RESULT VirtualMachine::interpret(const Chunk *chunk) {

  assert(chunk != nullptr);
  m_chunk = chunk;

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
      printValue(stackPop(), m_logger);
      m_logger->print("\n");
      break;
    }
    case OP_CODE::OP_JUMP: {
      // unconditional jump
      uint16_t offset = readShort();
      m_ip += offset;
      break;
    }
    case OP_CODE::OP_JUMP_IF_FALSE: {
      uint16_t offset = readShort();
      if (isFalsey(peek(0))) {
        // let us perform the jump
        m_ip += offset;
      }
      break;
    }
    case OP_CODE::OP_LOOP: {
      uint16_t offset = readShort();
      m_ip -= offset;
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
    case OP_CODE::OP_SET_LOCAL: {
      // here we expect the value on top of the stack
      // so we read it and assign it to the corresponiding stack slot
      uint8_t slot = readByte();
      m_stack[slot] = peek(0);
      break;
    }
    case OP_CODE::OP_GET_LOCAL: {
      // hear on top of the stack we have the slot where the variable
      // we want is, so we just read it from it and pop it on top of the stack
      // this is how the stack based machine dances, register machine avoid this
      // by loading and referring registers
      uint8_t slot = readByte();
      stackPush(m_stack[slot]);
      break;
    }
    case OP_CODE::OP_GET_GLOBAL: {
      // reading the identifier from the
      // top of the stack
      ObjString *name = readString();
      Value value;
      // look up the value
      bool result = m_globals.get(name->chars, value);
      if (!result) {
        sprintf(log::tempLogBuffer1, "Undefined variable '%s'.", name->chars);
        runtimeError(log::tempLogBuffer1);
        return INTERPRET_RESULT::INTERPRET_RUNTIME_ERROR;
      }

      // if everything went correctly we can push the looked
      // up value on the stack
      stackPush(value);

      break;
    }

    case OP_CODE::OP_DEFINE_GLOBAL: {

      sObjString *name = readString();
      m_globals.insert(name->chars, peek(0));
      stackPop();
      break;
    }
    case OP_CODE::OP_SET_GLOBAL: {
      // reading the identifier from the
      // top of the stack
      ObjString *name = readString();
      // look up the value
      bool result = m_globals.containsKey(name->chars);
      if (!result) {
        sprintf(log::tempLogBuffer1, "Undefined variable '%s'.", name->chars);
        runtimeError(log::tempLogBuffer1);
        return INTERPRET_RESULT::INTERPRET_RUNTIME_ERROR;
      } else {
        // if the value already exists, meaning the variable has been declared
        // already we insert it
        m_globals.insert(name->chars, peek(0));
      }

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
} // namespace binder::vm

} // namespace binder::vm
