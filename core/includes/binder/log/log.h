#pragma once

namespace binder::log{
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
}
