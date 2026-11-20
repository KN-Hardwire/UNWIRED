# Repository Structure

The `src/` directory contains the application and its platform-specific
implementations. The structure separates shared functionality from
platform-dependent code.

## `src/app/`

Platform-independent application orchestration. Coordinates the application
lifecycle, input polling and delivery, UI actions, Core operations and
platform-dependent services. Polls normalized input events through PAL Input
and passes them to UI for interpretation. Acts as the central communication layer
between UI, Core and PAL. It is responsible for coordinating operations
and maintaining consistency between the logical device state and external
devices.

## `src/core/`

Platform-independent domain logic and data models, including modules,
ports, connections, routing and presets. Defines the device's logical
state and behavior independently of the execution environment. Exposes
operations and data to App without depending on App, UI, PAL or platform-specific
code.

## `src/ui/`

Platform-independent user interface logic, including screen navigation,
input handling and presentation state. Receives input events and application
data through App. Produces UI actions representing user intentions, which
are processed by App. Uses PAL Display interfaces for rendering and PAL
input event types for interpretation. UI does not poll PAL Input, invoke
Core operations or modify Core state directly.

## `src/pal/`

Platform Abstraction Layer. Defines common interfaces for the display,
user input, switching matrix, and persistent storage, without depending
on any specific implementation. Defines normalized input event types and
operation results without depending on App, Core or UI.

`pal.h` collects injected service tables; `input.h`, `display.h`, `matrix.h` and
`storage.h` define individual services, and `status.h` defines common results.
Behavior and buffer ownership are specified in [PAL Contracts](pal_contracts.md).

## `src/platforms/`

Platform-specific implementations of the PAL interfaces and application
startup. Platform entry points initialize the execution environment and PAL
implementations, start App and own the execution loop that invokes App updates
and rendering.

- `desktop/` - Runs the desktop simulator using desktop graphics, input,
  file storage, and simulated hardware.
- `stm32/` - (not implemented) Runs the firmware on the microcontroller
  using physical peripherals and hardware drivers. `stm32/stm32_pinout.h`
  holds the microcontroller pin assignments taken from the schematics in
  `hardware/microcontroller/`.

The desktop platform places each implemented PAL service in its own translation
unit, named after the service and prefixed with the platform, as in
`desktop/desktop_matrix.c`. Decisions the contracts leave to that
implementation are recorded in [Desktop Platform](desktop_platform.md).

### Dependency Rule

`app/`, `core/`, `ui/` and `pal/` must not depend on `platforms/`. App depends
on UI, Core and PAL. UI depends on PAL contracts for input event types and
display operations, but not on App or Core. Core is independent of App, UI
and PAL; PAL is independent of App, Core and UI. Dependencies must not form
cycles. Platform-specific code implements the contracts defined in `pal/`,
allowing the same shared application code to be used by both targets.

## `tests/`

Tests live in the top-level `tests/` directory, alongside `src/`. They run on
the desktop host through CTest.

## `third_party/`

External source dependencies. `third_party/SDL/` is the SDL3 Git submodule,
pinned to a specific upstream commit. CMake builds it for the desktop target.
