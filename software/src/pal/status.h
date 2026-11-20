#ifndef UNWIRED_PAL_STATUS_H
#define UNWIRED_PAL_STATUS_H

#include <stdint.h>

typedef enum : uint8_t {
    PAL_OK = 0,
    PAL_NO_EVENT,
    PAL_INVALID_ARGUMENT,
    PAL_INVALID_STATE,
    PAL_IO_ERROR,
    PAL_NOT_FOUND,
    PAL_BUFFER_TOO_SMALL,
    PAL_UNSUPPORTED,
} PalStatus;

#endif
