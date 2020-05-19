#pragma once 
#include "stdlib.h"

#define ALLOCATE(type, count)                                                  \
  (type *)reallocate(nullptr, 0, sizeof(type) * (count))

//void *reallocate(void *previous, size_t oldSize, size_t newSize) {
inline void *reallocate(void *previous, size_t , size_t newSize) {
  if (newSize == 0) {
    free(previous);
    return nullptr;
  }

  return realloc(previous, newSize);
}
