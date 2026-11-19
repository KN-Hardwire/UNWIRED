#ifndef UNWIRED_CORE_CONFIG_H
#define UNWIRED_CORE_CONFIG_H

#include <stddef.h>
#include <stdint.h>

#include "limits.h"

typedef uint16_t CoreModuleId;
typedef uint16_t CorePortId;

typedef enum : uint8_t {
    CORE_PORT_INPUT  = 0,
    CORE_PORT_OUTPUT = 1,
} CorePortDirection;

typedef struct {
    CorePortId id;
    char name[CORE_NAME_CAPACITY];
    CorePortDirection direction;
} CorePort;

typedef struct {
    char name[CORE_NAME_CAPACITY];
    CoreModuleId id;
    CorePortId port_ids[CORE_MAX_PORTS_PER_MODULE];
    uint16_t port_count;
} CoreModule;

typedef struct {
    CoreModule modules[CORE_MAX_MODULES];
    size_t module_count;
    CorePort ports[CORE_MAX_PORTS];
    uint16_t port_count;
} CoreConfig;

#endif
