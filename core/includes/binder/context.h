#pragma once
#include "binder/memory/stringPool.h"

namespace binder {

struct ContextConfig {
  int m_stringPoolSizeInMb = 32;
};
class BinderContext {
public:
  explicit BinderContext(const ContextConfig &config);

  void runFile(const char *filePath);
  void run(const char *code);

  void reportError(const int line, const char *message) {
    m_hadError = true;
    if (m_errorReportingEnabled) {
      report(line, "", message);
    }
  };

  memory::StringPool &getStringPool() { return m_stringPool; }

  bool hadError() const { return m_hadError; }
  void setErrorReportingEnabled(bool value) { m_errorReportingEnabled = value; }

private:
  static void report(const int line, const char *location, const char *message);

private:
  const ContextConfig m_config;
  memory::StringPool m_stringPool;
  bool m_hadError = false;
  bool m_errorReportingEnabled = true;
};

} // namespace binder
