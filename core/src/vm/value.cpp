#include "binder/vm/value.h"
#include "binder/log/log.h"

namespace binder::vm
{

void printValue(Value value, log::Log *logger)
{
    char valueBuffer[64];
    sprintf(valueBuffer,"%g", valueAsNumber(value));
    logger->print(valueBuffer);
}

}
