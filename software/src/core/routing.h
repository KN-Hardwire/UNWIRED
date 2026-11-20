#ifndef UNWIRED_CORE_ROUTING_H
#define UNWIRED_CORE_ROUTING_H

#include "config.h"

typedef struct {
    CorePortId source;
    CorePortId destination;
} CoreConnection;

typedef struct {
    CoreConnection connections[CORE_MAX_CONNECTIONS];
    uint16_t connection_count;
} CoreRouting;

#endif
