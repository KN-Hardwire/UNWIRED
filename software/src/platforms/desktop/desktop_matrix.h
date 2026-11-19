#ifndef UNWIRED_PLATFORM_DESKTOP_MATRIX_H
#define UNWIRED_PLATFORM_DESKTOP_MATRIX_H

#include <stdbool.h>
#include <stdint.h>

#include "core/limits.h"
#include "pal/matrix.h"

/* Capacity of the simulated matrix.
 *
 * PAL treats matrix capacity as an implementation capability, so these values
 * belong to the platform rather than to pal/matrix.h. The desktop backend
 * models no hardware limits of its own, so it derives its capacity from
 * core/limits.h instead of mirroring the literals. The platform is the layer
 * that may read Core capacities (see docs/app_lifecycle.md, startup step 2).
 */
#define DESKTOP_MATRIX_PORT_COUNT      ((uint32_t)CORE_MAX_PORTS)
#define DESKTOP_MATRIX_MAX_CONNECTIONS ((uint32_t)CORE_MAX_CONNECTIONS)

/* Simulated matrix state, owned by the platform for the whole App lifetime.
 * App and UI see only the PalMatrix table from desktop_matrix_interface().
 */
typedef struct {
    PalMatrixConnection connections[DESKTOP_MATRIX_MAX_CONNECTIONS];
    uint32_t connection_count;
    bool initialized;
} DesktopMatrix;

/* Reset the simulated matrix to a known empty routing state. Must be called
 * before desktop_matrix_interface(). */
void desktop_matrix_init(DesktopMatrix *matrix);

/* Build the injectable PAL table. The returned context points at *matrix,
 * which must outlive all App/UI use. */
PalMatrix desktop_matrix_interface(DesktopMatrix *matrix);

/* Inspection helpers for tests and for later simulator visualisation. They
 * report committed routing only and never change state. */
uint32_t desktop_matrix_connection_count(const DesktopMatrix *matrix);
const PalMatrixConnection *desktop_matrix_connections(const DesktopMatrix *matrix);
bool desktop_matrix_is_connected(const DesktopMatrix *matrix,
                                 PalPortNumber source,
                                 PalPortNumber destination);

#endif
