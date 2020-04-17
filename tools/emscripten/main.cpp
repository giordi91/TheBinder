#include "binder/interpreter.h"
#include "binder/scanner.h"
#include "binder/parser.h"
#include "binder/printer/jsonASTPrinter.h"
#include <iostream>


extern "C"
{
    const char* bindExecute(const char* source)
    {
      binder::ContextConfig config{};
      config.m_stringPoolSizeInMb = 1;
      binder::BinderContext context(config);
      binder::Scanner scanner(&context);
      binder::Parser parser(&context);

      scanner.scan(source);
      const binder::memory::ResizableVector<binder::Token>& tokens =
          scanner.getTokens();
      std::cout<<"TheBinder scanner produced "<<tokens.size()<<" tokens"<<std::endl;
      parser.parse(&tokens);
      const binder::autogen::Expr* expr = parser.getRoot();
      const char* ast = binder::printer::JSONASTPrinter(context.getStringPool()).print((binder::autogen::Expr*)expr);
      //make a heap copy of the ast
      int len = strlen(ast);
      const char* toReturn = new char[len +1]; 
      memcpy((char*)toReturn, ast, len+1);
      return toReturn;
    }
}
