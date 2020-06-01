#pragma once
#include "binder/log/log.h"
#include "binder/memory/stringPool.h"

namespace binder {

enum class LOGGER_TYPE { CONSOLE, BUFFERED };

struct ContextConfig {
  int m_stringPoolSizeInMb = 32;
  LOGGER_TYPE loggerType = LOGGER_TYPE::CONSOLE;
  int printFloatPrecision = 5;
};
class BinderContext {
public:
  explicit BinderContext(const ContextConfig &config);
  ~BinderContext();

  void runFile(const char *filePath);
  //const char* code, suppressed for warning
  void run(const char *){};

  void reportError(const int line, const char *message) {
    m_hadError = true;
    if (m_errorReportingEnabled) {
      report(line, "", message);
    }
  };

  memory::StringPool &getStringPool() { return m_stringPool; }

  bool hadError() const { return m_hadError; }
  void setErrorReportingEnabled(bool value) { m_errorReportingEnabled = value; }
  void print(const char* toPrint);
  log::Log* getLogger(){return m_log;};
  const ContextConfig& getConfig()const{return m_config;}

private:
  void report(const int line, const char *location, const char *message);

private:
  const ContextConfig m_config;
  memory::StringPool m_stringPool;
  bool m_hadError = false;
  bool m_errorReportingEnabled = true;
  log::Log *m_log = nullptr;
};

} // namespace binder
