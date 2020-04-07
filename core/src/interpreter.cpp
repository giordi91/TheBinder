#include "binder/interpreter.h"

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
  m_stringPool.free(fileContent);
}

void BinderContext::run(const char* code) {}

}  // namespace binder
