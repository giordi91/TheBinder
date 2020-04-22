#include "binder/context.h"
#include "binder/interpreter.h"
#include "binder/parser.h"
#include "binder/printer/jsonASTPrinter.h"
#include "binder/scanner.h"
#include <iostream>

extern "C" {
const char *bindExecute(const char *source) {
  binder::ContextConfig config{};
  // TODO
  // Cannot enlarge memory arrays to size 38825984 bytes (OOM). Either (1)
  // compile with  -s INITIAL_MEMORY=X  with X higher than the current value
  // 16777216, (2) compile with  -s ALLOW_MEMORY_GROWTH=1  which allows
  // increasing the size at runtime, or (3) if you want malloc to return NULL
  // (0) instead of this abort, compile with  -s ABORTING_MALLOC=0 do this so i
  // can allocate the memory i need
  config.m_stringPoolSizeInMb = 1;
  binder::BinderContext context(config);
  binder::Scanner scanner(&context);
  binder::Parser parser(&context);
  binder::ASTInterpreter interpreter(&context);

  scanner.scan(source);
  const binder::memory::ResizableVector<binder::Token> &tokens =
      scanner.getTokens();
  std::cout << "TheBinder scanner produced " << tokens.size() << " tokens"
            << std::endl;
  parser.parse(&tokens);

  const binder::memory::ResizableVector<binder::autogen::Stmt *> &stmts =
      parser.getStmts();
  interpreter.interpret(stmts);
  // const binder::autogen::Expr *expr = parser.getRoot();
  // const char *ast = binder::printer::JSONASTPrinter(context.getStringPool())
  //                      .print((binder::autogen::Expr *)expr);

  //// make a heap copy of the ast
  // int len = strlen(ast);
  // const char *toReturn = new char[len + 1];
  // memcpy((char *)toReturn, ast, len + 1);
  // return toReturn;
  return "{}";
}
}
