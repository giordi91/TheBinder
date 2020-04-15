#include "binder/interpreter.h"
#include "binder/scanner.h"
#include <iostream>


extern "C"
{
    void bindExecute(const char* source)
    {
      binder::ContextConfig config{};
      config.m_stringPoolSizeInMb = 1;
      binder::BinderContext context(config);
      binder::Scanner scanner(&context);

      scanner.scan(source);
      const binder::memory::ResizableVector<binder::Token>& tokens =
          scanner.getTokens();
      std::cout<<"TheBinder scanner produced "<<tokens.size()<<" tokens"<<std::endl;
    }
}
