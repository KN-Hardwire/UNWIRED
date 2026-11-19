#ifndef UNWIRED_PAL_DISPLAY_H
#define UNWIRED_PAL_DISPLAY_H

#include "status.h"
#include <stdint.h>

/* Numeric RGB565: red 15..11, green 10..5, blue 4..0. */
typedef uint16_t PalColor;

typedef struct {
    int32_t x;
    int32_t y;
    uint16_t width;
    uint16_t height;
} PalDisplayRect;

/* Fixed-cell, 1-bit bitmap font supplied by shared UI code. Glyphs cover the
 * inclusive ASCII range first..last. Each glyph contains height rows of
 * ceil(width / 8) bytes, leftmost pixel in the most significant bit.
 * Glyphs are consecutive; unused bits at the end of each row are ignored.
 * The cell width is also the horizontal advance. No platform font IDs.
 */
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t first;
    uint8_t last;
    const uint8_t* bits;
    uint32_t byte_count;
} PalDisplayFont;

/* All callbacks are required, synchronous and serialized. Geometry is clipped
 * to the display. The platform controls presentation timing.
 * Buffers/font data are borrowed only during the call. See docs/pal_contracts.md
 * for validation, text layout and failure recovery guarantees.
 */
typedef struct {
    void *context;
    uint16_t width;
    uint16_t height;
    PalStatus (*fill_rect)(void *context, PalDisplayRect rect, PalColor color);
    PalStatus (*draw_line)(void *context, int32_t x0, int32_t y0,
                           int32_t x1, int32_t y1, PalColor color);
    PalStatus (*draw_text)(void *context, int32_t x, int32_t y,
                           const PalDisplayFont *font, PalColor color,
                           const char *text, uint32_t length);
    PalStatus (*write_pixels)(void *context, PalDisplayRect rect,
                              const PalColor *pixels, uint32_t pixel_count,
                              uint32_t stride_pixels);
} PalDisplay;

#endif
