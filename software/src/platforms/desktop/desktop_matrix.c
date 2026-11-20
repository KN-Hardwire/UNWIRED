#include "desktop_matrix.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "pal/matrix.h"
#include "pal/status.h"

/* In-memory PalMatrix backend: validates a requested connection set and stores
 * the committed one, switching nothing. Contract: docs/pal_contracts.md,
 * "Matrix". Port map, capacity and the rules the contract leaves open:
 * docs/desktop_platform.md.
 */

static PalMatrixResult make_result(PalStatus status,
                                   PalMatrixFailureEffect failure_effect) {
    PalMatrixResult result = {
        .status = status,
        .failure_effect = failure_effect,
    };
    return result;
}

/* failure_effect is meaningful only when status != PAL_OK. */
static PalMatrixResult make_success(void) {
    PalMatrixResult result = {0};
    result.status = PAL_OK;
    return result;
}

static bool port_is_mappable(PalPortNumber port) {
    return (uint32_t)port < DESKTOP_MATRIX_PORT_COUNT;
}

/* Per-connection validity, independent of the rest of the set. */
static bool connection_is_valid(PalMatrixConnection connection) {
    if (connection.source == connection.destination) {
        return false;
    }
    return port_is_mappable(connection.source)
        && port_is_mappable(connection.destination);
}

/* True when two records describe the same route, in either order. */
static bool connections_are_equal(PalMatrixConnection first,
                                  PalMatrixConnection second) {
    if (first.source == second.source
        && first.destination == second.destination) {
        return true;
    }
    return first.source == second.destination
        && first.destination == second.source;
}

static bool set_has_duplicates(const PalMatrixConnection *connections,
                               uint32_t count) {
    for (uint32_t index = 1u; index < count; index++) {
        for (uint32_t other = 0u; other < index; other++) {
            if (connections_are_equal(connections[index], connections[other])) {
                return true;
            }
        }
    }
    return false;
}

/* Full validation before any state change. Capacity is checked ahead of the
 * per-record scan so an unchecked count never walks the caller's array. */
static PalMatrixResult validate_request(const DesktopMatrix *matrix,
                                        const PalMatrixConnection *connections,
                                        uint32_t count) {
    if (matrix == NULL) {
        return make_result(PAL_INVALID_ARGUMENT, PAL_MATRIX_STATE_UNCHANGED);
    }
    if (!matrix->initialized) {
        return make_result(PAL_INVALID_STATE, PAL_MATRIX_STATE_UNCHANGED);
    }
    if (count == 0u) {
        /* An empty set permits a null array and disconnects everything. */
        return make_success();
    }
    if (connections == NULL) {
        return make_result(PAL_INVALID_ARGUMENT, PAL_MATRIX_STATE_UNCHANGED);
    }
    if (count > DESKTOP_MATRIX_MAX_CONNECTIONS) {
        return make_result(PAL_UNSUPPORTED, PAL_MATRIX_STATE_UNCHANGED);
    }
    for (uint32_t index = 0u; index < count; index++) {
        if (!connection_is_valid(connections[index])) {
            return make_result(PAL_INVALID_ARGUMENT,
                               PAL_MATRIX_STATE_UNCHANGED);
        }
    }
    if (set_has_duplicates(connections, count)) {
        return make_result(PAL_INVALID_ARGUMENT, PAL_MATRIX_STATE_UNCHANGED);
    }
    return make_success();
}

/* Copies the borrowed array; routes absent from it are removed. count is
 * bounded by capacity here, so the size calculation cannot overflow. */
static void commit_request(DesktopMatrix *matrix,
                           const PalMatrixConnection *connections,
                           uint32_t count) {
    if (count > 0u) {
        memcpy(matrix->connections,
               connections,
               (size_t)count * sizeof(PalMatrixConnection));
    }
    matrix->connection_count = count;
}

static PalMatrixResult desktop_matrix_replace(void *context,
                                              const PalMatrixConnection *connections,
                                              uint32_t count) {
    DesktopMatrix *matrix = context;

    PalMatrixResult result = validate_request(matrix, connections, count);
    if (result.status != PAL_OK) {
        return result;
    }

    commit_request(matrix, connections, count);
    return make_success();
}

void desktop_matrix_init(DesktopMatrix *matrix) {
    if (matrix == NULL) {
        return;
    }
    matrix->connection_count = 0u;
    matrix->initialized = true;
}

PalMatrix desktop_matrix_interface(DesktopMatrix *matrix) {
    PalMatrix interface = {
        .context = matrix,
        .replace = desktop_matrix_replace,
    };
    return interface;
}

uint32_t desktop_matrix_connection_count(const DesktopMatrix *matrix) {
    if (matrix == NULL) {
        return 0u;
    }
    return matrix->connection_count;
}

const PalMatrixConnection *desktop_matrix_connections(const DesktopMatrix *matrix) {
    if (matrix == NULL) {
        return NULL;
    }
    return matrix->connections;
}

bool desktop_matrix_is_connected(const DesktopMatrix *matrix,
                                 PalPortNumber source,
                                 PalPortNumber destination) {
    if (matrix == NULL) {
        return false;
    }

    PalMatrixConnection wanted = {
        .source = source,
        .destination = destination,
    };
    for (uint32_t index = 0u; index < matrix->connection_count; index++) {
        if (connections_are_equal(matrix->connections[index], wanted)) {
            return true;
        }
    }
    return false;
}
