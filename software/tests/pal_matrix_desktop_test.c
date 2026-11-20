#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "platforms/desktop/desktop_matrix.h"

/* Contract tests for the desktop PalMatrix backend. Contract:
 * docs/pal_contracts.md, "Matrix". Decisions: docs/desktop_platform.md. */

static unsigned failures = 0u;

static void check(const char *name, bool ok) {
    printf("%-54s %s\n", name, ok ? "pass" : "FAIL");
    if (!ok) {
        failures++;
    }
}

/* The "previous routing" that every rejection must preserve. */
static const PalMatrixConnection baseline[] = {
    {1u, 2u},
    {3u, 4u},
    {5u, 63u},
};
static const uint32_t baseline_count = 3u;

static void commit_baseline(PalMatrix pal, DesktopMatrix *matrix) {
    PalMatrixResult result = pal.replace(pal.context, baseline, baseline_count);
    check("baseline committed",
          result.status == PAL_OK
              && desktop_matrix_connection_count(matrix) == baseline_count);
}

static bool baseline_preserved(const DesktopMatrix *matrix) {
    return desktop_matrix_connection_count(matrix) == baseline_count
        && desktop_matrix_is_connected(matrix, 1u, 2u)
        && desktop_matrix_is_connected(matrix, 3u, 4u)
        && desktop_matrix_is_connected(matrix, 5u, 63u);
}

static void test_context(PalMatrix pal) {
    check("null context -> INVALID_ARGUMENT",
          pal.replace(NULL, NULL, 0u).status == PAL_INVALID_ARGUMENT);

    DesktopMatrix uninitialized = {
        .connection_count = 99u,
        .initialized = false,
    };
    PalMatrix raw = desktop_matrix_interface(&uninitialized);
    check("uninitialized context -> INVALID_STATE",
          raw.replace(raw.context, NULL, 0u).status == PAL_INVALID_STATE);
}

static void test_empty_set(PalMatrix pal, DesktopMatrix *matrix) {
    PalMatrixResult result = pal.replace(pal.context, NULL, 0u);
    check("empty set with null array -> OK",
          result.status == PAL_OK
              && desktop_matrix_connection_count(matrix) == 0u);

    commit_baseline(pal, matrix);
    result = pal.replace(pal.context, NULL, 0u);
    check("empty set clears committed routing",
          result.status == PAL_OK
              && desktop_matrix_connection_count(matrix) == 0u
              && !desktop_matrix_is_connected(matrix, 1u, 2u));
}

static void test_rejections_preserve_routing(PalMatrix pal,
                                             DesktopMatrix *matrix) {
    commit_baseline(pal, matrix);

    PalMatrixResult result = pal.replace(pal.context, NULL, 2u);
    check("null array with count > 0 -> INVALID_ARGUMENT, UNCHANGED",
          result.status == PAL_INVALID_ARGUMENT
              && result.failure_effect == PAL_MATRIX_STATE_UNCHANGED);
    check("  routing preserved", baseline_preserved(matrix));

    const PalMatrixConnection reversed[] = {{7u, 8u}, {8u, 7u}};
    result = pal.replace(pal.context, reversed, 2u);
    check("reversed duplicate pair -> INVALID_ARGUMENT, UNCHANGED",
          result.status == PAL_INVALID_ARGUMENT
              && result.failure_effect == PAL_MATRIX_STATE_UNCHANGED);
    check("  routing preserved", baseline_preserved(matrix));

    const PalMatrixConnection repeated[] = {{7u, 8u}, {7u, 8u}};
    result = pal.replace(pal.context, repeated, 2u);
    check("exact duplicate pair -> INVALID_ARGUMENT, UNCHANGED",
          result.status == PAL_INVALID_ARGUMENT
              && result.failure_effect == PAL_MATRIX_STATE_UNCHANGED);
    check("  routing preserved", baseline_preserved(matrix));

    const PalMatrixConnection self_connection[] = {{5u, 5u}};
    result = pal.replace(pal.context, self_connection, 1u);
    check("source == destination -> INVALID_ARGUMENT, UNCHANGED",
          result.status == PAL_INVALID_ARGUMENT
              && result.failure_effect == PAL_MATRIX_STATE_UNCHANGED);
    check("  routing preserved", baseline_preserved(matrix));

    const PalMatrixConnection unmappable[] = {
        {0u, (PalPortNumber)DESKTOP_MATRIX_PORT_COUNT},
    };
    result = pal.replace(pal.context, unmappable, 1u);
    check("port outside the map -> INVALID_ARGUMENT, UNCHANGED",
          result.status == PAL_INVALID_ARGUMENT
              && result.failure_effect == PAL_MATRIX_STATE_UNCHANGED);
    check("  routing preserved", baseline_preserved(matrix));
}

/* Distinct unordered pairs; the port map provides far more than capacity. */
static void fill_unique_pairs(PalMatrixConnection *connections, uint32_t count) {
    uint32_t written = 0u;
    for (uint32_t source = 0u;
         source < DESKTOP_MATRIX_PORT_COUNT && written < count;
         source++) {
        for (uint32_t destination = source + 1u;
             destination < DESKTOP_MATRIX_PORT_COUNT && written < count;
             destination++) {
            connections[written].source = (PalPortNumber)source;
            connections[written].destination = (PalPortNumber)destination;
            written++;
        }
    }
}

static void test_capacity(PalMatrix pal, DesktopMatrix *matrix) {
    static PalMatrixConnection oversized[DESKTOP_MATRIX_MAX_CONNECTIONS + 1u];

    commit_baseline(pal, matrix);

    fill_unique_pairs(oversized, DESKTOP_MATRIX_MAX_CONNECTIONS + 1u);
    PalMatrixResult result =
        pal.replace(pal.context, oversized, DESKTOP_MATRIX_MAX_CONNECTIONS + 1u);
    check("one over capacity -> UNSUPPORTED, UNCHANGED",
          result.status == PAL_UNSUPPORTED
              && result.failure_effect == PAL_MATRIX_STATE_UNCHANGED);
    check("  routing preserved", baseline_preserved(matrix));

    /* A count far beyond capacity must be rejected without reading the array. */
    const PalMatrixConnection single[] = {{1u, 2u}};
    result = pal.replace(pal.context, single, UINT32_MAX);
    check("count = UINT32_MAX -> UNSUPPORTED, UNCHANGED",
          result.status == PAL_UNSUPPORTED
              && result.failure_effect == PAL_MATRIX_STATE_UNCHANGED);
    check("  routing preserved", baseline_preserved(matrix));

    result = pal.replace(pal.context, oversized, DESKTOP_MATRIX_MAX_CONNECTIONS);
    check("exactly capacity -> OK",
          result.status == PAL_OK
              && desktop_matrix_connection_count(matrix)
                     == DESKTOP_MATRIX_MAX_CONNECTIONS);
}

static void test_replacement(PalMatrix pal, DesktopMatrix *matrix) {
    commit_baseline(pal, matrix);

    const PalMatrixConnection replacement[] = {{10u, 11u}};
    PalMatrixResult result = pal.replace(pal.context, replacement, 1u);
    check("replacement removes routes absent from the request",
          result.status == PAL_OK
              && desktop_matrix_connection_count(matrix) == 1u
              && desktop_matrix_is_connected(matrix, 10u, 11u)
              && !desktop_matrix_is_connected(matrix, 1u, 2u));

    result = pal.replace(pal.context, replacement, 1u);
    check("replacing with the same set is idempotent",
          result.status == PAL_OK
              && desktop_matrix_connection_count(matrix) == 1u
              && desktop_matrix_is_connected(matrix, 10u, 11u));

    check("a committed route is reported in either order",
          desktop_matrix_is_connected(matrix, 10u, 11u)
              && desktop_matrix_is_connected(matrix, 11u, 10u));
    check("an absent route is not reported",
          !desktop_matrix_is_connected(matrix, 10u, 12u));
}

static void test_accessors_reject_null(void) {
    check("connection_count(NULL) == 0",
          desktop_matrix_connection_count(NULL) == 0u);
    check("connections(NULL) == NULL",
          desktop_matrix_connections(NULL) == NULL);
    check("is_connected(NULL, ...) == false",
          !desktop_matrix_is_connected(NULL, 1u, 2u));
}

int main(void) {
    DesktopMatrix matrix;
    desktop_matrix_init(&matrix);
    PalMatrix pal = desktop_matrix_interface(&matrix);

    check("init leaves routing empty",
          desktop_matrix_connection_count(&matrix) == 0u);
    check("interface exposes the matrix as its context",
          pal.context == &matrix && pal.replace != NULL);

    test_context(pal);
    test_empty_set(pal, &matrix);
    test_rejections_preserve_routing(pal, &matrix);
    test_capacity(pal, &matrix);
    test_replacement(pal, &matrix);
    test_accessors_reject_null();

    if (failures > 0u) {
        printf("\n%u check(s) failed.\n", failures);
        return EXIT_FAILURE;
    }
    puts("\nAll checks passed.");
    return EXIT_SUCCESS;
}
