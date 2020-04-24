#pragma once
#include "binder/memory/sparseMemoryPool.h"
#include "binder/memory/resizableVector.h"
#include "binder/enviroment.h"

namespace binder {

namespace autogen {
class Expr;
class Stmt;
}

// used to keep track of the type of our runtime
enum class RuntimeValueType { NUMBER = 0, STRING, NIL, BOOLEAN, INVALID };

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

  const char *debugToString(BinderContext *context);
  const char *toString(BinderContext *context);
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
  void flushMemory() { m_pool.clear(); };
  void setSuppressPrint(bool value){m_suppressPrints = value;}

private:
  BinderContext *m_context;
  Enviroment m_enviroment;
  memory::SparseMemoryPool<RuntimeValue> m_pool;
  bool m_suppressPrints = false;
};

} // namespace binder
