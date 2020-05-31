#pragma once
#include "binder/enviroment.h"
#include "binder/memory/resizableVector.h"
#include "binder/memory/sparseMemoryPool.h"

namespace binder {

namespace autogen {
class Expr;
class Stmt;
class Function;
} // namespace autogen

// used to keep track of the type of our runtime
enum class RuntimeValueType { INVALID = 0, NUMBER, STRING, NIL, BOOLEAN };
enum class RuntimeValueStorage { INVALID = 0, L_VALUE = 1, R_VALUE };

class BinderContext;

// this is our generic runtime value and can be any
// of the supported types.
// not a huge fan of crazy modern bananas c++ but I guess this might
// be a chance for std::any to shine?
struct RuntimeValue {
  union {
    double number;
    const char *string;
    bool boolean;
  };
  RuntimeValueType type = RuntimeValueType::INVALID;
  RuntimeValueStorage storage = RuntimeValueStorage::INVALID;

  const char *debugToString(BinderContext *context) const;
  const char *toString(BinderContext *context, bool trailingNewLine = false) const;
};

class AstInterpreterVisitor;

class Callable {
public:
  virtual void *call(AstInterpreterVisitor *interpreter,
                     memory::ResizableVector<void *> &arguments) = 0;
  virtual int arity()=0;
};

class BinderFunction : public Callable {

public:
  BinderFunction(autogen::Function *declaration) : m_declaration(declaration){};
  void *call(AstInterpreterVisitor *interpreter,
             memory::ResizableVector<void *> &arguments) override;
  int arity()override;

private:
  autogen::Function *m_declaration;
};

// NOTE possibly have an abstract class at the base as
// interface to allow to bolt different interpreters if needed
class ASTInterpreter {
  typedef uint32_t RuntimeValueHandle;

public:
  // poolSize is in number of elements stored in the pool;
  ASTInterpreter(BinderContext *context, int poolSize = 1000)
      : m_context(context), m_pool(poolSize){};
  ~ASTInterpreter() = default;

  void interpret(const binder::memory::ResizableVector<autogen::Stmt *> &stmts);
  void flushMemory() {
    m_pool.clear();
    m_enviroment.clear();
  };
  void setSuppressPrint(bool value) { m_suppressPrints = value; }

  RuntimeValue *getRuntimeVariable(const char *variableName);

private:
  BinderContext *m_context;
  Enviroment m_enviroment;
  memory::SparseMemoryPool<RuntimeValue> m_pool;
  bool m_suppressPrints = false;
};

} // namespace binder
