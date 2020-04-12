#include "binder/interpreter.h"
#include "binder/scanner.h"
#include <iostream>


extern "C"
{
    int test(int v)
    {
        return v*2;
    }

}

int main() {
  binder::ContextConfig config{};
  config.m_stringPoolSizeInMb = 1;
  binder::BinderContext context(config);
  binder::Scanner scanner(&context);

  const char *toScan = "(){}}{(,.-+*";
  scanner.scan(toScan);
  const binder::memory::ResizableVector<binder::Token>& tokens =
      scanner.getTokens();
  std::cout<<"hello scanner "<<tokens.size()<<std::endl;

  return 0;
}
