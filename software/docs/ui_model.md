# UI Model

This document defines UI data and state. It does not prescribe screen layout or
C operation signatures; those will be developed with the initial simulator.
UI does not include Core headers or access Core state directly.

## Presentation Data

App builds a read-only presentation view from committed domain state and App's
runtime status. UI receives its own presentation types, not Core structs.

| Record | Fields |
| --- | --- |
| Module view | Module identifier, display name, assigned port numbers |
| Port view | Device-wide port number, display name, configured input/output direction |
| Connection view | Source port number, destination port number |
| Device view | Module/port/connection collections with explicit counts, routing synchronization status |
| Operation feedback | Operation kind, success/failure category and relevant identifiers |

Domain collections are owned by App/Core. App owns any mapped presentation
storage. UI borrows views only during a call and stores stable identifiers for
selection rather than retaining pointers into those views. Display names are
null-terminated; all views remain unchanged during input handling or rendering.
No presentation view may be reused after App changes its backing data.

Routing synchronization distinguishes confirmed active connections from Core's
last committed connections when the hardware state is unknown. UI must expose
this condition rather than presenting unconfirmed routing as physically active.

## UI State

UI owns the following presentation state, separate from Core:

| State | Contents |
| --- | --- |
| Navigation | Current screen and navigation history |
| Selection | Module identifier, port number or connection endpoint pair, and scroll position |
| Editing | Draft name/direction/assignment values and the item being edited |
| Connection editing | Selected source and destination, not yet committed |
| Feedback | Visible operation result or notification |
| Input interaction | Transient held-button/gesture state |

Selections and editing targets may be absent. UI clears selections whose targets
disappear from a new view. Draft edits are not active configuration. Screen
identifiers, menu layout, text-entry mechanics and concrete state structs are
left to UI implementation.

## User Actions

Navigation, selection and cancellation are local UI behavior. Domain intentions
are actions returned to App, with these conceptual payloads:

| Action | Payload |
| --- | --- |
| Create module | Name and assigned port numbers |
| Edit module | Module identifier and proposed name/port assignments |
| Remove module | Module identifier |
| Configure port | Port number and proposed name/direction |
| Connect | Source and destination port numbers |
| Disconnect | Source and destination port numbers |
| Recover routing | No domain payload; asks App to restore synchronization |

Actions carry proposed values, never pointers to mutable Core state. Their
concrete C representation and operation interfaces are deferred. App invokes
Core validation and returns operation feedback; UI may perform convenience
checks but is not the authority for domain validity. Editing/removal effects on
existing routes will be specified with Core operation interfaces.

Preset management actions and presentation are deferred until preset behavior
is specified in its own development stage.

## Input and Display

App delivers normalized [PAL input events](pal_contracts.md); UI does not poll
the input queue. A device control map associates stable control identifiers
with UI functions without exposing ADC channels, desktop key codes or GPIOs.
The initial simulator supplies its UI control mapping during implementation.

Touch follows the single-contact [PAL touch contract](pal_contracts.md#touch).
UI interprets down/move/up coordinates for hit testing and gestures. Touch cancel
and input overflow clear transient touch state without activating the target.
The simulator supplies equivalent events from the primary mouse button.

On an input I/O failure, App also requests immediate cancellation of transient
interaction state. Recovery follows the [PAL input recovery contract](pal_contracts.md#input-recovery).
Cancellation clears held buttons, repeats and gestures without producing domain
actions; it preserves navigation, selection and draft edits. Fresh absolute
control events replace stale readings after the reset marker.

UI draws directly through PAL Display primitives.
UI owns widgets, layout, shared bitmap font assets, text measurement and drawing
decisions; PAL owns raster operations and display transport. UI uses the
implementation-defined dimensions advertised by PAL Display for layout and
drawing; no particular resolution or orientation is required by the contract.

A complete framebuffer is not required. UI may compose pixel regions using
bounded scratch storage whose ownership and capacity are established during
initialization. PAL borrows region and font data only during each drawing call.
UI redraws changed areas, including erasing obsolete content, and draws the
complete screen initially and after a display failure. It returns rendering
failures to App. The platform controls presentation timing.

## Deferred Programmable Controls

The device has five user-programmable controls sampled through the
microcontroller's ADC. Their module bindings are a separate domain concept from
port routing. Binding models, value delivery to modules, assignment UI and
preset inclusion are deferred. PAL can report absolute control values without
requiring those features now. Their readings do not implicitly become UI
navigation input or port connections.
