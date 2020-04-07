#pragma once
#include "binder/memory/stringPool.h"

namespace binder {

struct ContextConfig {
  const int m_stringPoolSizeInMb = 32;
};
class BinderContext {
 public:
  explicit BinderContext(const ContextConfig& config);

  void runFile(const char* filePath);
  void run(const char* code);

 private:
  const ContextConfig m_config;
  memory::StringPool m_stringPool;
};

}  // namespace binder
