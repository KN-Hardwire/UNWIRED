#ifndef UNWIRED_CORE_PRESET_H
#define UNWIRED_CORE_PRESET_H

#include "core_limits.h"
#include "routing.h"

typedef struct {
    CoreRouting routing;
    char name[CORE_NAME_CAPACITY];
} CorePreset;

#endif
