#pragma once
#include "binder/log/log.h"
#include "binder/memory/resizableVector.h"
#include "stdio.h"

namespace binder::log {

class BufferedLog : public Log {
public:
  BufferedLog() : m_buffer(256){m_buffer[0]='\n';};
  virtual ~BufferedLog() = default;

  // making sure you can't copy etc around
  BufferedLog(const BufferedLog &) = delete;
  BufferedLog &operator=(const BufferedLog &) = delete;
  BufferedLog(BufferedLog &&) = delete;
  BufferedLog &operator=(BufferedLog &&) = delete;

  // interface
  void print(const char *toLog) override {
    int size = m_buffer.size();
    size_t inputLen = strlen(toLog);
    if (size == 0) {
      //+1 so we get the null terminator
      m_buffer.resize(static_cast<uint32_t>(inputLen) + 1);
      memcpy(m_buffer.data(), toLog, inputLen + 1);
    } else {

      //if we are here the size is not zero, meaning we already have some 
      //input in here, we need to make sure we override the null terminator
      //in the buffer

      //we don't add 1 becase the nullterminator is in the buffer already, 
      //just needs to be moved around
      m_buffer.resize(static_cast<uint32_t>(size + inputLen));

      //offsetting for the null terminator in the buffer
      memcpy(m_buffer.data() + size-1, toLog, inputLen + 1);
    }
  }

  // no need to flush anything
  void flush() override { m_buffer.clear(); };
  const char* getBuffer() const {return m_buffer.data();}


private:
  memory::ResizableVector<char> m_buffer;
};

} // namespace binder::log
