# Core Model

Core defines the modular synthesizer domain model.

Port:
- name
- number
- direction (in/out)

Module:
- name
- ports

Connection:
- source port
- destination port

Preset:
- name
- connections

## Module and Port Identity

Each module has a stable `id` that uniquely identifies it within the device's
logical configuration. Each physical port is identified by a stable `number`
that is unique across the device. A module references its assigned ports by
these numbers; module identity is not part of port identity.

Module:
- `id`: stable module identifier
- `name`: editable display label
- `ports`: references to the ports assigned to the module, identified by number

Port:
- `number`: stable, device-wide unique identifier for the physical port
- `name`: editable display label
- `direction`: input or output, defined by software configuration rather than
  fixed by the physical port

Connection:
- `source`: number of a port configured as an output
- `destination`: number of a port configured as an input

Renaming a module or port does not change its identity or existing connections.
Changing a port's module assignment or configured direction does not change its
number. Module creation assigns existing physical ports rather than creating
new physical ports.

Module identifiers and port numbers remain stable across saving and loading
presets so that stored connections reference the same logical ports.

Physical matrix addresses are mapped separately from device-wide port numbers.
The platform's PAL matrix implementation owns this mapping. App translates Core
connections into PAL connection records containing port numbers; module
identifiers are not needed for matrix operations.

## Data Ownership and Capacity

`CoreState` owns a `CoreConfig` and committed `CoreRouting`. Configuration owns
module and port records. Modules reference ports by number rather than owning
copies. Routing owns connection records, each referring to its endpoints by
port number. A connection is selected by its source/destination pair rather
than a separate connection identifier.

The C headers in `src/core/` use fixed-capacity arrays. Each count describes the
occupied prefix of its array and must not exceed its capacity. Names are
null-terminated and name capacity includes that terminator. The numeric
`CorePortId` in C represents the device-wide port number defined above.

`src/core/limits.h` is the source of truth for capacities, including port
capacity. These are current software limits, not physical hardware claims.
Changing how limits are sourced is deferred. Core and UI operation signatures,
configuration validation rules and their tests belong to the next development
stage.

Core owns no hardware synchronization status; App tracks whether committed
routing is confirmed to match the matrix. Prepared routing changes describe
the full target connection set, which App sends to PAL for replacement before
committing. Details of the prepared-change C type remain for the operation
interface stage.

## Presets

Preset modeling and behavior are deferred. The following describes initial
intent; it is not a complete preset specification or an implementation priority
for the initial simulator. The existing `CorePreset` header is an initial data
sketch, not a storage format.

A **preset** is a named, saved set of connections. Editing active connections
does not automatically modify a saved preset.

**Saving a preset** stores the current active connections.

**Loading a preset** validates its connections and applies them to the hardware
through App and PAL. After hardware confirms success, Core commits those
connections as active. App reports any failure to UI.

## Connecting Two Ports

A connection request identifies a source port and a destination port.
Core does not call PAL or UI - App coordinates the operation.

1. App asks Core to prepare the connection.
2. Core validates the request, including checking that the source port is an
   output and the destination port is an input. Validation does not change
   committed domain state.
3. If validation fails, Core returns the reason to App. App does not request
   a hardware change and provides the failure to UI for presentation.
4. If validation succeeds, Core returns the proposed routing change to App.
5. App translates the routing change into a matrix request and executes it
   through PAL.
6. After PAL confirms successful completion, App asks Core to commit the
   prepared connection. App then provides the updated state to UI.
7. If the hardware operation fails, App does not commit the connection and
   provides the failure to UI.

App completes or resolves the current routing operation before starting another
operation that could invalidate the prepared change. Committing a successfully
prepared change must not introduce a new domain validation failure.

A hardware failure does not necessarily mean the matrix is unchanged. If PAL
cannot guarantee the resulting hardware state, App must treat it as unknown;
the unchanged Core state must not be presented as confirmed hardware state.

## Deferred Programmable Control Bindings

The device has five user-programmable controls sampled through the
microcontroller's ADC. A future binding associates a control with a module
target; it is distinct from a connection between ports. Binding data, target
semantics, value delivery and preset inclusion are deferred. No control-binding
types or operations are required for initial port-routing development. Physical
readings enter through PAL, never through ADC access in Core.
