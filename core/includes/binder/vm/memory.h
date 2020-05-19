#pragma once
#include "stdlib.h"

#define ALLOCATE(type, count)                                                  \
  (type *)reallocate(nullptr, 0, sizeof(type) * (count))

#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

#define FREE_ARRAY(type, pointer, oldCount)                                    \
  reallocate(pointer, sizeof(type) * (oldCount), 0)

// void *reallocate(void *previous, size_t oldSize, size_t newSize) {
inline void *reallocate(void *previous, size_t, size_t newSize) {
  if (newSize == 0) {
    free(previous);
    return nullptr;
  }

  return realloc(previous, newSize);
}
