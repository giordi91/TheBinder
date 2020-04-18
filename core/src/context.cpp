#include "binder/context.h"

#include "binder/constants.h"

namespace binder {

BinderContext::BinderContext(const ContextConfig& config)
    : m_config(config),
      m_stringPool(m_config.m_stringPoolSizeInMb * MB_TO_BYTE) {}

void BinderContext::runFile(const char* filePath) {
  uint32_t fileSize = 0;
  const char* fileContent = m_stringPool.loadFile(filePath, fileSize);
  assert(fileSize != 0);
  run(fileContent);
  //freeing the file memory
  m_stringPool.free(fileContent);

  //check if everything went alright, or just abort
  if (m_hadError) {
    exit(65);
  }
}

void BinderContext::run(const char* code) {}

void BinderContext::report(const int line, const char* location,
                           const char* message) {
  printf("[ line %i ] Error %s: %s", line, location, message);
}
}  // namespace binder
