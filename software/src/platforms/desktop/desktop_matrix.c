#include "desktop_matrix.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "pal/matrix.h"
#include "pal/status.h"

/* In-memory PalMatrix backend for the desktop simulator.
 *
 * Contract: docs/pal_contracts.md, "Matrix". replace() receives the complete
 * desired connection set, validates it fully before touching committed state,
 * and reports PAL_OK only for a complete replacement. Because the committed
 * set lives in memory, every rejection can honestly claim
 * PAL_MATRIX_STATE_UNCHANGED; completion is established by the in-memory state
 * itself, not by electrical readback.
 *
 * TODO: implementation pass. Every function below is a stub.
 */

static PalMatrixResult make_result(PalStatus status,
                                   PalMatrixFailureEffect failure_effect) {
    PalMatrixResult result = {
        .status = status,
        .failure_effect = failure_effect,
    };
    return result;
}

/* True when the port number exists in the simulated port map. */
static bool port_is_mappable(PalPortNumber port) {
    (void)port;
    /* TODO: flat map, valid range [0, DESKTOP_MATRIX_PORT_COUNT). */
    return false;
}

/* Per-connection validity, independent of the rest of the set.
 * TODO: decide whether source == destination is rejected here. */
static bool connection_is_valid(PalMatrixConnection connection) {
    /* TODO: self-connection rule. */
    return port_is_mappable(connection.source)
        && port_is_mappable(connection.destination);
}

/* True when two records describe the same route.
 * TODO: decide whether (a, b) and (b, a) count as the same pair. */
static bool connections_are_equal(PalMatrixConnection first,
                                  PalMatrixConnection second) {
    (void)first;
    (void)second;
    return false;
}

static bool set_has_duplicates(const PalMatrixConnection *connections,
                               uint32_t count) {
    (void)connections;
    (void)count;
    /* TODO: pairwise scan over connections_are_equal(). */
    (void)connections_are_equal;
    return false;
}

/* Full argument and capability validation, performed before any state change.
 * Returns PAL_OK when the set may be committed as-is. */
static PalMatrixResult validate_request(const DesktopMatrix *matrix,
                                        const PalMatrixConnection *connections,
                                        uint32_t count) {
    (void)matrix;
    (void)connections;
    (void)count;
    (void)connection_is_valid;
    (void)set_has_duplicates;
    /* TODO, in order:
     *   - matrix NULL or not initialized
     *   - count == 0 accepts connections == NULL
     *   - count > 0 requires connections != NULL
     *   - count * sizeof(PalMatrixConnection) fits before any size_t use
     *   - count > DESKTOP_MATRIX_MAX_CONNECTIONS -> PAL_UNSUPPORTED
     *   - every record valid, no duplicates -> PAL_INVALID_ARGUMENT
     * All rejections report PAL_MATRIX_STATE_UNCHANGED.
     */
    return make_result(PAL_UNSUPPORTED, PAL_MATRIX_STATE_UNCHANGED);
}

/* Replace committed routing with an already validated set. Copies the borrowed
 * array; routes absent from it are removed. */
static void commit_request(DesktopMatrix *matrix,
                           const PalMatrixConnection *connections,
                           uint32_t count) {
    (void)matrix;
    (void)connections;
    (void)count;
    /* TODO: copy count records, store connection_count. */
}

static PalMatrixResult desktop_matrix_replace(void *context,
                                              const PalMatrixConnection *connections,
                                              uint32_t count) {
    (void)context;
    (void)connections;
    (void)count;
    (void)validate_request;
    (void)commit_request;
    /* TODO: cast context to DesktopMatrix *, validate_request(), then
     * commit_request() and return PAL_OK on success. */
    return make_result(PAL_UNSUPPORTED, PAL_MATRIX_STATE_UNCHANGED);
}

void desktop_matrix_init(DesktopMatrix *matrix) {
    (void)matrix;
    /* TODO: empty routing, initialized = true. */
}

PalMatrix desktop_matrix_interface(DesktopMatrix *matrix) {
    PalMatrix interface = {
        .context = matrix,
        .replace = desktop_matrix_replace,
    };
    return interface;
}

uint32_t desktop_matrix_connection_count(const DesktopMatrix *matrix) {
    (void)matrix;
    return 0;
}

const PalMatrixConnection *desktop_matrix_connections(const DesktopMatrix *matrix) {
    (void)matrix;
    return NULL;
}

bool desktop_matrix_is_connected(const DesktopMatrix *matrix,
                                 PalPortNumber source,
                                 PalPortNumber destination) {
    (void)matrix;
    (void)source;
    (void)destination;
    return false;
}
