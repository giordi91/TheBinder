#include "binder/context.h"

#include "binder/constants.h"

// logs
#include "binder/log/bufferLog.h"
#include "binder/log/consoleLog.h"

namespace binder {

BinderContext::BinderContext(const ContextConfig &config)
    : m_config(config),
      m_stringPool(m_config.m_stringPoolSizeInMb * MB_TO_BYTE) {

  switch (config.loggerType) {
  case (LOGGER_TYPE::CONSOLE): {
    m_log = new log::ConsoleLog();
    break;
  }
  case (LOGGER_TYPE::BUFFERED): {
    m_log = new log::BufferedLog();
    break;
  }
  default:
    assert(0 && "unsupported logger type");
  }
}

BinderContext::~BinderContext() { delete (m_log); }

void BinderContext::runFile(const char *filePath) {
  uint32_t fileSize = 0;
  const char *fileContent = m_stringPool.loadFile(filePath, fileSize);
  assert(fileSize != 0);
  run(fileContent);
  // freeing the file memory
  m_stringPool.free(fileContent);

  // check if everything went alright, or just abort
  if (m_hadError) {
    exit(65);
  }
}
void BinderContext::print(const char *toPrint) { m_log->print(toPrint); }

void BinderContext::report(const int line, const char *location,
                           const char *message) {
  char buffer[512];
  sprintf(buffer, "[ line %i ] Error %s: %s\n", line, location, message);
  m_log->print(buffer);
}
} // namespace binder
