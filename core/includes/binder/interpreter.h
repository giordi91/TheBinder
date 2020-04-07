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

  void reportError(const int line, const char* message) {
    report(line, "", message);
    m_hadError = true;
  };

 private:
  static void report(const int line, const char* location, const char* message);

 private:
  const ContextConfig m_config;
  memory::StringPool m_stringPool;
  bool m_hadError = false;
};

}  // namespace binder
