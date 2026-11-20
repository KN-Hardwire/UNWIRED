#ifndef UNWIRED_CORE_LIMITS_H
#define UNWIRED_CORE_LIMITS_H

#include <stdint.h>

// Software storage limits, not hardware specifications
constexpr uint8_t  CORE_NAME_CAPACITY        = 64;
constexpr uint8_t  CORE_MAX_MODULES          = 32;
constexpr uint8_t  CORE_MAX_PORTS            = 64;
constexpr uint8_t  CORE_MAX_PORTS_PER_MODULE = 16;
constexpr uint16_t CORE_MAX_CONNECTIONS      = 256;

#endif
