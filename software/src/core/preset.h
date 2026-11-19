#ifndef UNWIRED_CORE_PRESET_H
#define UNWIRED_CORE_PRESET_H

#include "limits.h"
#include "routing.h"

typedef struct {
    char name[CORE_NAME_CAPACITY];
    CoreRouting routing;
} CorePreset;

#endif
