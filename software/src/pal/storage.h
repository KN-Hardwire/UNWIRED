#ifndef UNWIRED_PAL_STORAGE_H
#define UNWIRED_PAL_STORAGE_H

#include "status.h"
#include <stdint.h>

/* Opaque named byte objects. No Core structs or preset format in this contract.
 * Keys are nonempty null-terminated ASCII names using A-Z, a-z, 0-9, '_' and '-'.
 * All arguments are borrowed only for the duration of the synchronous call.
 */
typedef struct {
    void *context;
    /* Reads a whole object. On PAL_OK, size is bytes read; on
     * PAL_BUFFER_TOO_SMALL, size is required capacity. On other errors, size is
     * zero. Buffer contents are usable only on PAL_OK. NULL data requires zero
     * capacity; size must always be non-NULL.
     */
    PalStatus (*read)(void *context, const char *key, void *data,
                      uint32_t capacity, uint32_t *size);
    /* Replaces an object. NULL data is allowed only for size == 0.
     * Failure may leave the stored object changed. No power-loss guarantee.
     */
    PalStatus (*write)(void *context, const char *key, const void *data,
                       uint32_t size);
} PalStorage;

#endif
