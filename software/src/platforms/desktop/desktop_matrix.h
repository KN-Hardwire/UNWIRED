#ifndef UNWIRED_PLATFORM_DESKTOP_MATRIX_H
#define UNWIRED_PLATFORM_DESKTOP_MATRIX_H

#include <stdbool.h>
#include <stdint.h>

#include "core/limits.h"
#include "pal/matrix.h"

/* Capacity is an implementation capability, so it lives here and not in
 * pal/matrix.h. Rationale for deriving it from Core: docs/desktop_platform.md. */
#define DESKTOP_MATRIX_PORT_COUNT      ((uint32_t)CORE_MAX_PORTS)
#define DESKTOP_MATRIX_MAX_CONNECTIONS ((uint32_t)CORE_MAX_CONNECTIONS)

/* Platform-owned state; App and UI see only the PalMatrix table. */
typedef struct {
    PalMatrixConnection connections[DESKTOP_MATRIX_MAX_CONNECTIONS];
    uint32_t connection_count;
    bool initialized;
} DesktopMatrix;

/* Resets routing to empty. Required before desktop_matrix_interface(). */
void desktop_matrix_init(DesktopMatrix *matrix);

/* The returned context points at *matrix, which must outlive App/UI use. */
PalMatrix desktop_matrix_interface(DesktopMatrix *matrix);

/* Committed routing, for tests and simulator visualisation. Never mutates. */
uint32_t desktop_matrix_connection_count(const DesktopMatrix *matrix);
const PalMatrixConnection *desktop_matrix_connections(const DesktopMatrix *matrix);
bool desktop_matrix_is_connected(const DesktopMatrix *matrix,
                                 PalPortNumber source,
                                 PalPortNumber destination);

#endif
