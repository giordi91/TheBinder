#pragma once

#include "stdio.h"

typedef double Value;

namespace binder {
namespace log {
class Log;
}
namespace vm {

void printValue(Value value, log::Log *logger);
}
} // namespace binder
