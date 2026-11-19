#ifndef UNWIRED_CORE_ROUTING_H
#define UNWIRED_CORE_ROUTING_H

#include <stddef.h>

#include "config.h"
#include "limits.h"

typedef struct {
    CorePortId source;
    CorePortId destination;
} CoreConnection;

typedef struct {
    CoreConnection connections[CORE_MAX_CONNECTIONS];
    size_t connection_count;
} CoreRouting;

#endif
