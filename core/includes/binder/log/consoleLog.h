#pragma once

#include "stdio.h"
#include "binder/log/log.h"

namespace binder::log{

class ConsoleLog : public Log{
public:
  ConsoleLog() = default;
  virtual ~ConsoleLog() = default;

  //making sure you can't copy etc around
  ConsoleLog(const ConsoleLog &) = delete;
  ConsoleLog &operator=(const ConsoleLog &) = delete;
  ConsoleLog(ConsoleLog &&) = delete;
  ConsoleLog &operator=(ConsoleLog &&) = delete;

  //interface
  void print(const char* toLog)  override
  {
      printf("%s",toLog);
  }

  //no need to flush anything
  void flush() override{};
};
}

