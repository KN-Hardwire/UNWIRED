#ifndef UNWIRED_CORE_H
#define UNWIRED_CORE_H

#include "config.h"
#include "preset.h"
#include "routing.h"

typedef struct {
    CoreConfig config;
    CoreRouting routing;
} CoreState;

#endif
