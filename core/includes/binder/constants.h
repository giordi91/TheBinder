#pragma once
#include "stdint.h"

namespace binder {
static constexpr uint64_t MB_TO_BYTE = 1024 * 1024;
static constexpr double BYTE_TO_MB_D = 1.0 / MB_TO_BYTE;
static constexpr float BYTE_TO_MB = BYTE_TO_MB_D;
}  // namespace binder