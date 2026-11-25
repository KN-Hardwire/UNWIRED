# Application Lifecycle

This document defines runtime sequencing and App's state model. C operation
interfaces and implementations will be added in the next development stage.
Layer dependencies are defined in [Architecture](architecture.md).

## App State

App owns Core state and UI state. It holds the injected PAL interfaces and maps
Core data into UI presentation data. Platform contexts remain owned by the platform.

App tracks these values separately from Core's domain state:

| Value | Meaning |
| --- | --- |
| Lifecycle phase | Starting, running, faulted or stopped |
| Routing synchronization | Unknown or confirmed to match committed Core routing |
| Operation result | Success, domain validation failure or platform failure |
| Platform failure detail | Service and PAL status; matrix failures also carry their effect |

Committed Core routing is not proof of current hardware routing when
synchronization is unknown. UI receives that distinction in presentation data.

## Startup

1. The platform initializes its execution environment and PAL contexts. It
   supplies stable control identifiers and the mapping of device port numbers
   to physical or simulated matrix addresses.
2. The platform constructs PAL interfaces and supplies the initial device
   configuration. Core capacities come from `src/core/core_limits.h`.
3. App checks required callbacks and nonzero display dimensions, initializes
   Core with the supplied configuration and empty routing, and initializes UI.
   Any bounded UI rendering scratch storage is provisioned outside the
   update/render loop; a full framebuffer is not required. Invalid service tables
   or unavailable required scratch storage cause initialization to fail before
   the platform starts update/render calls.
4. App marks routing synchronization unknown and requests an empty matrix set
   through PAL to establish a known initial state. Success confirms empty
   routing and enters the running phase. Failure leaves App faulted with routing
   changes disabled.
5. App provides presentation data and startup results to UI. The platform may
   continue updating and rendering a faulted App to show the failure.

Startup does not implicitly load a preset or assume hardware starts disconnected.

## Update

The platform pumps its native events, then invokes an App update. PAL and shared
application calls execute serially on the application thread.

1. App polls PAL Input and passes events to UI in their received order.
   Polling stops when the queue is empty, an input error occurs, or a bounded
   per-update event budget is reached. The budget is selected during implementation.
2. UI updates navigation/editing state or returns a user action for App.
3. App processes each action before moving to the next input event. Actions
   requiring domain changes go through Core; UI never modifies Core directly.
4. For a routing action, Core validates and prepares the target connection set
   without committing it. App translates that set to PAL matrix records and
   requests complete replacement.
5. On success, App commits the prepared change through Core and marks routing
   confirmed. On failure, it leaves Core uncommitted, records the result and
   updates synchronization according to PAL's failure effect.
6. App supplies current presentation data and operation feedback to UI.

Only one routing change is prepared/executed at a time. App prevents intervening
configuration changes from invalidating it. Core/UI function signatures and
validation details are deferred to their implementation stage.

## Faults and Recovery

A matrix failure marked unchanged preserves a previously confirmed routing
state. It does not turn an unknown state into a confirmed one. Any other matrix
failure marks routing unknown. App blocks domain actions that change routing or
its configuration until synchronization is restored; UI navigation and rendering
remain available.

To recover, App requests complete replacement with the committed Core routing.
Only success confirms synchronization, enters the running phase and enables
routing changes. Failed
recovery leaves routing unknown. Recovery is an explicit application operation;
the first version does not repeatedly retry on every update.

Input overflow tells UI to discard transient held-button/gesture state without
triggering actions. On an input I/O error, App immediately clears the same UI
state, records the failure and stops polling until a later update. The platform
then follows [PAL input recovery](pal_contracts.md#input-recovery): a reset marker,
fresh absolute values, release gating for buttons/touch and a fresh encoder
baseline. Other input errors are reported as input failures. Input, display and
storage errors do not by themselves invalidate matrix synchronization. Display
recovery uses a complete redraw as described below. Storage recovery will be
added with its implementation.

## Rendering

Rendering is separate from updating; the platform decides its scheduling.

1. App supplies UI with current presentation data and access to PAL Display.
2. UI draws changed areas directly through primitives or
   RGB565 pixel-region transfers. It draws the complete screen initially and
   after a display failure. Shared UI code owns widgets and font assets.
3. UI returns the first rendering failure, or success, to App for service-status
   tracking.

Drawing calls consume borrowed buffers before returning, allowing scratch storage
to be reused immediately. Drawing does not promise atomic visible changes.
UI requires no application-side full framebuffer.

The desktop platform may composite the device frame with its virtual panel after
`app_render()` returns.
Window events and desktop rendering resources remain platform concerns.

## Shutdown

The platform stops invoking updates/renders, then releases application storage
and platform resources after no PAL call is active. PAL borrows buffers only
during calls, so it retains no application buffers to drain. Automatic saving,
matrix reset on exit and persistent startup restoration are not specified yet.
