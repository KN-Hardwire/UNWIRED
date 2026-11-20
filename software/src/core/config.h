#ifndef UNWIRED_CORE_CONFIG_H
#define UNWIRED_CORE_CONFIG_H

#include "core_limits.h"

typedef uint16_t CoreModuleId;
typedef uint16_t CorePortId;

typedef enum : uint8_t {
    CORE_PORT_INPUT  = 0,
    CORE_PORT_OUTPUT
} CorePortDirection;

typedef struct {
    char name[CORE_NAME_CAPACITY];
    CorePortId id;
    CorePortDirection direction;
} CorePort;

typedef struct {
    char name[CORE_NAME_CAPACITY];
    CorePortId port_ids[CORE_MAX_PORTS_PER_MODULE];
    CoreModuleId id;
    uint16_t port_count;
} CoreModule;

typedef struct {
    CorePort ports[CORE_MAX_PORTS];
    CoreModule modules[CORE_MAX_MODULES];
    uint16_t module_count;
    uint16_t port_count;
} CoreConfig;

#endif
