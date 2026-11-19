#ifndef UNWIRED_PAL_INPUT_H
#define UNWIRED_PAL_INPUT_H

#include "status.h"
#include <stdint.h>

typedef uint16_t PalControlId;

typedef enum {
    PAL_INPUT_BUTTON_PRESSED,
    PAL_INPUT_BUTTON_RELEASED,
    PAL_INPUT_ROTATED,
    PAL_INPUT_VALUE_CHANGED,
    PAL_INPUT_OVERFLOW,
    PAL_INPUT_TOUCH_DOWN,
    PAL_INPUT_TOUCH_MOVE,
    PAL_INPUT_TOUCH_UP,
    PAL_INPUT_TOUCH_CANCEL,
} PalInputEventType;

typedef struct {
    PalInputEventType type;
    PalControlId control;
    union {
        int32_t rotation; /* Signed detents; positive means clockwise. */
        uint16_t value;   /* Absolute normalized value, 0 through UINT16_MAX. */
        struct {
            uint16_t x;
            uint16_t y;
        } touch;
    } data;
} PalInputEvent;

typedef struct {
    void *context;
    /* Nonblocking FIFO poll. Writes event only on PAL_OK.
     * PAL_NO_EVENT means empty. Overflow is an event; its control/data are ignored.
     * See docs/pal_contracts.md for event and queue guarantees.
     */
    PalStatus (*poll)(void *context, PalInputEvent *event);
} PalInput;

#endif
