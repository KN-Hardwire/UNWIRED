#ifndef UNWIRED_PAL_H
#define UNWIRED_PAL_H

#include "display.h"
#include "input.h"
#include "matrix.h"
#include "storage.h"

/* Constructed by the platform and injected into App. Required callbacks must
 * be non-NULL. Storage is optional; NULL means persistence is unavailable.
 * Platform contexts and the optional storage table outlive App and UI use.
 * Calls are serialized on the application thread and never reenter App/UI.
 */
typedef struct {
    PalInput input;
    PalDisplay display;
    PalMatrix matrix;
    const PalStorage *storage;
} Pal;

#endif
