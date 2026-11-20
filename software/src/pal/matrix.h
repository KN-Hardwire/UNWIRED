#ifndef UNWIRED_PAL_MATRIX_H
#define UNWIRED_PAL_MATRIX_H

#include "status.h"

/* Device-wide physical port number; not a Core type or matrix chip address. */
typedef uint16_t PalPortNumber;

typedef struct {
    PalPortNumber source;
    PalPortNumber destination;
} PalMatrixConnection;

typedef enum : uint8_t {
    PAL_MATRIX_STATE_UNKNOWN = 0,
    PAL_MATRIX_STATE_UNCHANGED,
} PalMatrixFailureEffect;

typedef struct {
    PalStatus status;
    /* Used only when status != PAL_OK. */
    PalMatrixFailureEffect failure_effect;
} PalMatrixResult;

typedef struct {
    void* context;
    /* Replace the complete routing set synchronously. PAL_OK confirms the full
     * requested set, including removal of routes absent from this array.
     * count == 0 disconnects everything and permits connections == NULL.
     * Valid sets beyond hardware capacity or topology return PAL_UNSUPPORTED
     * with PAL_MATRIX_STATE_UNCHANGED when detected before hardware writes.
     * Implementations prevalidate known hardware limits before changing routes.
     * The array is borrowed until return; physical addressing stays in platform.
     */
    PalMatrixResult (*replace)(void* context, const PalMatrixConnection* connections, uint32_t count);
} PalMatrix;

#endif
