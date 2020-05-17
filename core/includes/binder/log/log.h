#pragma once

#include "stdarg.h"
#include "stdio.h"

namespace binder::log{

static char logBuffer[1024];

//abstract interface for the Log/Printing system
class Log {
public:
  Log() = default;
  virtual ~Log() = default;

  //making sure you can't copy etc around
  Log(const Log &) = delete;
  Log &operator=(const Log &) = delete;
  Log(Log &&) = delete;
  Log &operator=(Log &&) = delete;

  //interface
  //produces an output, whatever it can be on the log, console, file etc
  virtual void print(const char* toLog) =0;
  //gives a change to the logger to clean up if there are pending operations
  //aswell as clearing temporary memory etc
  virtual void flush() =0;
};

inline void LOG(log::Log *logger, const char* format, ...)
{
    va_list args;
    va_start (args,format);
    vsprintf(logBuffer,format,args);
    va_end(args);
    logger->print(logBuffer);                                                     \
}
}
