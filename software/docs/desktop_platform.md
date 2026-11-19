# Desktop Platform

This document records the implementation decisions of the desktop platform. It
does not restate the contracts; those live in [PAL Contracts](pal_contracts.md).
Where a contract leaves a choice to the implementation, the choice taken here is
documented below together with its reason.

The desktop platform runs the simulator on a host machine. Its PAL
implementations live in `src/platforms/desktop/`, one translation unit per
service, named after the service it implements. SDL is used only by services
that need host windowing, input or timing; the matrix needs none of them.

## Matrix

`desktop_matrix.c` implements `PalMatrix` as an in-memory simulated matrix. It
switches no signals: it validates a requested connection set and stores the
committed one, so the shared application can exercise routing and so the
simulator can later visualise it. Physical crosspoint addressing belongs to the
firmware backend.

### Capacity

| Value | Source |
| --- | --- |
| `DESKTOP_MATRIX_PORT_COUNT` | `CORE_MAX_PORTS` |
| `DESKTOP_MATRIX_MAX_CONNECTIONS` | `CORE_MAX_CONNECTIONS` |

PAL treats matrix capacity as an implementation capability, so these constants
belong to the platform rather than to `src/pal/matrix.h`. The simulator models
no hardware limits of its own, so it derives both values from
`src/core/limits.h` instead of duplicating the literals; the platform is the
layer allowed to read Core capacities, as described in
[Application Lifecycle](app_lifecycle.md). Deriving them also keeps the
simulator from rejecting a set that Core is able to express at all.

The values are reached through the platform's own names, never by using
`CORE_MAX_*` inside the implementation. When hardware capacity stops matching
Core's storage limits, only these two definitions change. The firmware backend
will declare its own capacity from the crosspoint topology.

### Port Map

The simulated port map is flat: valid port numbers are
`[0, DESKTOP_MATRIX_PORT_COUNT)`. Any other number is unmappable and rejected
with `PAL_INVALID_ARGUMENT`.

### Completion and Failure Effect

Completion is established by the in-memory state itself, never by electrical
readback. Validation always finishes before the commit step, so every failure
reports `PAL_MATRIX_STATE_UNCHANGED`. This backend never reports
`PAL_MATRIX_STATE_UNKNOWN`, which means App's recovery path cannot be reached
through the desktop matrix and must be exercised by a substituted PAL
implementation in tests.

### Implementation Choices Left Open by the Contract

| Case | Decision | Reason |
| --- | --- | --- |
| `(a, b)` and `(b, a)` in one set | Duplicate, `PAL_INVALID_ARGUMENT` | A crosspoint closure is bidirectional, so both records name the same route |
| `source == destination` | `PAL_INVALID_ARGUMENT` | A port has no crosspoint to itself, so the endpoint pair is unmappable |
| A port used by several connections | Accepted | A crosspoint supports fan-out; see the known simplification below |

Rejecting the reversed pair cannot reject a request App is able to make.
[Core Model](core_model.md) defines a connection as an output source and an
input destination, and a port holds one configured direction, so `(a, b)` and
`(b, a)` can never both pass Core validation. The rule therefore guards against
a malformed request, not against legitimate routing.

`desktop_matrix_is_connected()` matches that same unordered pair, so it reports
a committed route in either argument order. It describes the simulated closure,
not the domain's direction; Core and UI remain the source of truth for which
endpoint is the source.

### Validation Order

1. Null context: `PAL_INVALID_ARGUMENT`.
2. Context not initialised: `PAL_INVALID_STATE`.
3. `count == 0`: success, and a null array is permitted.
4. Null array with `count > 0`: `PAL_INVALID_ARGUMENT`.
5. `count` above capacity: `PAL_UNSUPPORTED`.
6. Each record unmappable or self-connected: `PAL_INVALID_ARGUMENT`.
7. Duplicate pairs: `PAL_INVALID_ARGUMENT`.

Capacity is checked before the per-record scan, ahead of the order the contract
suggests for an otherwise valid set. A consequence is that an oversized set
containing an invalid record reports `PAL_UNSUPPORTED` rather than
`PAL_INVALID_ARGUMENT`.

This ordering is defensive, not a contract requirement. The contract already
obliges the caller to pass a valid array for a nonempty set, so a `count` that
overstates the array is a caller defect. Checking capacity first means such a
defect is rejected instead of walking the array according to an unchecked
length. The same check makes the later `count * sizeof(PalMatrixConnection)`
calculation provably free of overflow, because `count` is bounded by capacity
by then.

### Known Simplifications

The hardware review in `hardware/connection_matrix/verification_report.md`
records a topology rule for the physical matrix: two driven AUX lines must never
reach one Y pad. The simulated matrix does not model it, so a set that connects
two sources to one destination is accepted here and would be rejected by
hardware. Encoding that rule is firmware backend work, and a shared rule would
need a home outside `src/pal/`, which holds contracts only.

### Tests

`tests/pal_matrix_desktop_test.c` holds the contract tests. It compiles
`desktop_matrix.c` directly, without the entry point or SDL, and runs through
CTest as `pal-matrix-desktop`.
