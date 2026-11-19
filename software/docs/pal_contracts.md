# PAL Contracts

The headers in `src/pal/` define platform-independent C contracts. They depend
only on standard C and other PAL headers. SDL, STM32 HAL, Core and UI types must
not appear in them.

## Injection and Execution

The platform constructs `Pal` (`pal/pal.h`) and injects it into App. Input,
display and matrix callbacks are required. Storage is optional: a null storage
pointer means persistence is unavailable; a supplied table has both callbacks.
UI receives access only to the display service it needs, not the full PAL bundle.

Each service has an opaque context pointer owned by the platform. Contexts and
the optional storage table remain valid until application use ends. Callback
tables may be copied; contexts are never copied or freed by App/UI.

Calls are synchronous and serialized on the application thread. Implementations
must not reenter App/UI. Except for input polling, calls may perform bounded
blocking work; implementations must not wait indefinitely for devices. A return
value describes completed work, not an accepted asynchronous request. This
initial contract has no pending operations or completion callbacks.

Arguments and buffers are borrowed for the duration of a call only. A platform
using DMA or a native asynchronous API must finish accessing them before return
or copy into its own storage. Initialization, native event pumping, scheduling
and resource teardown stay in the platform entry point.

## Results

`PalStatus` (`pal/status.h`) contains:

| Result | Meaning |
| --- | --- |
| `PAL_OK` | Operation completed successfully |
| `PAL_NO_EVENT` | Input queue is empty; valid only for input polling |
| `PAL_INVALID_ARGUMENT` | Invalid argument or unsupported value within an argument |
| `PAL_INVALID_STATE` | Operation is not allowed in the service's current state |
| `PAL_IO_ERROR` | Service could not complete the requested I/O |
| `PAL_NOT_FOUND` | Storage object does not exist; valid only for storage reads |
| `PAL_BUFFER_TOO_SMALL` | Storage read buffer is too small |
| `PAL_UNSUPPORTED` | Requested service behavior is unavailable |

Domain validation errors belong to Core, not PAL. Matrix operations additionally
report whether failure preserved previous routing. Other services make no
rollback guarantee unless stated below.

## Input

`PalInput.poll(context, event)` (`pal/input.h`) is nonblocking. It returns the
oldest queued event, or `PAL_NO_EVENT` if empty. Only `PAL_OK` writes an event.
The event pointer is required. FIFO order is preserved for accepted events.

Control identifiers are stable device control identifiers supplied by the
platform control map, not ADC channels or desktop key codes. Values do not
depend on the current screen or module binding.

| Event | Payload |
| --- | --- |
| Button pressed/released | Control identifier; union payload ignored |
| Rotated | Control identifier and signed detent count; positive is clockwise |
| Value changed | Control identifier and absolute value normalized to 0–65535 |
| Overflow/input reset | No payload; accepted input history is incomplete |
| Touch down/move/up | Touch surface control identifier and `data.touch.x`, `data.touch.y` |
| Touch cancel | Touch surface control identifier; union payload ignored |

The platform performs debouncing and normalization. It may coalesce consecutive
absolute-value changes for the same control if no intervening event is crossed.
Queue capacity and sampling/filtering are implementation choices. Lost events
trigger the recovery sequence below. The first sampling of an absolute control
also produces a value event.

Reporting a value from one of the five programmable controls does not implement
its binding to a module. App can leave those events unused until binding support
is implemented.

### Input Recovery

`PAL_INPUT_OVERFLOW` is the existing input-reset marker: it means accepted input
history is incomplete, whether caused by queue overflow or recovery from an
input I/O failure. Its control identifier and union payload are ignored. No
separate recovery callback is required.

On queue overflow, the platform discards queued events and delivers this marker
before any subsequently accepted input. UI cancels held-button state, repeats,
drags and touch gestures without triggering release actions or widget activation.
Selections, navigation and committed application state are preserved.

When an input I/O failure makes tracking unreliable, `poll` returns
`PAL_IO_ERROR` without writing an event. App stops polling for that update,
records the failure and immediately asks UI to clear the same transient state.
The platform discards stale queued events and resumes input only after reliable
sampling is restored. Its first successful event after recovery is
`PAL_INPUT_OVERFLOW`, followed by the resynchronization described below. Repeated
I/O errors must not synthesize user actions; App may poll again on later updates.

After either kind of reset, the platform:

1. Resamples every absolute control and reports `PAL_INPUT_VALUE_CHANGED` for
   each, even if its value matches the last platform reading. These fresh values
   follow the marker and precede ordinary events. Recovery events may be emitted
   across multiple polls; the queue need not hold the entire resampling at once.
2. Suppresses each button already held, or whose state is unknown, until a
   debounced release is observed. That release is not reported as a user action;
   the next press begins a new interaction.
3. Suppresses touch until all contacts are confirmed lifted. No move or up from
   the interrupted gesture is delivered, and the next contact starts with down.
4. Resets encoder decoding to a fresh hardware baseline. Incomplete steps and
   lost detents are discarded; only newly observed complete detents are reported.

Fresh absolute events restore current positions rather than replaying lost
history. Lost button actions and encoder movements are not reconstructed. If
another loss occurs during recovery, the sequence restarts with a new marker.
Malformed poll arguments return `PAL_INVALID_ARGUMENT`; they do not themselves
constitute an input-history reset.

### Touch

`PAL_INPUT_TOUCH_DOWN`, `PAL_INPUT_TOUCH_MOVE`, `PAL_INPUT_TOUCH_UP` and
`PAL_INPUT_TOUCH_CANCEL` describe one active contact per touch surface. `control`
identifies the surface using the platform control map, not a finger or widget.
The same identifier is used throughout a gesture.

`data.touch.x` and `data.touch.y` are `uint16_t` logical display coordinates:
origin at the top left, x increasing right and y increasing down. Coordinates
must satisfy `x < display.width` and `y < display.height`, using the
implementation-defined display dimensions. The platform handles sensor mapping,
rotation and desktop window scaling before reporting events. PAL does not report
raw sensor coordinates.

A gesture starts with down, continues with zero or more moves, and ends with up
or cancel. Down and move carry the contact position; up carries its last known
valid position. Cancel has no coordinate payload and must not be interpreted as
a successful release or click. UI owns hit testing, dragging, gesture recognition
and widget activation.

The initial policy is single-finger interaction. The platform selects the first
contact and tracks it without switching to another finger. Additional contacts
are ignored, and no new down is reported until all contacts have been lifted.
Loss of the tracked contact without a reliable release produces cancel. After
cancellation, reporting remains suppressed until all contacts are lifted.

Consecutive moves for the same surface may be coalesced to the latest position,
but never across another event. Down, up and cancel must not be coalesced away.
If queue overflow loses touch history, the existing overflow event cancels all
active UI interactions; a separate touch cancel is not required. The platform
suppresses new touch gestures until all contacts are lifted, so a gesture cannot
resume after its down or release was lost.

The desktop backend maps the primary mouse button to down/up and mouse motion
while held to move. It ignores presses outside the logical display. During an
active gesture it clamps positions to display bounds, captures the mouse when
supported, and reports release even outside the display. Losing window focus or
capture in a way that prevents reliable tracking produces cancel; a new gesture
requires release followed by a fresh press. Mouse input and physical touch use
the same logical touch-surface identifier in their respective platforms.

## Display

`PalDisplay` (`pal/display.h`) provides drawing primitives and pixel-region
transfers. All callbacks are required. UI owns widget drawing, text layout,
hit testing and interaction state; PAL has no menu, button, keyboard or
navigation concepts. Controller commands and acceleration stay in the backend.

### Geometry and Color

The display advertises implementation-defined, nonzero `width` and `height`,
supplied by the platform and unchanged during an App instance's lifetime.
PAL requires no particular resolution or orientation. UI uses the advertised
dimensions for layout and drawing. A desktop simulator can use the target
hardware's logical dimensions; window resizing is handled by platform scaling
or compositing and does not change the advertised dimensions.

Coordinates are signed logical pixels with origin at the top left, x increasing
right and y increasing down. `PalDisplayRect` covers `[x, x + width)` and
`[y, y + height)`. Geometry is clipped to display bounds; off-screen drawing and
zero-area rectangles succeed without changing pixels. Implementations must use
safe arithmetic for coordinate and size calculations, including signed extremes.
There is no mutable transform or clipping state.

`PalColor` is a numeric RGB565 value: red bits 15–11, green 10–5 and blue 4–0.
Physical transport byte order and native format conversion belong to the backend.
Drawing replaces covered pixels without alpha blending or antialiasing.

### Completion and Presentation

Calls execute in order. `PAL_OK` from a drawing call means the operation completed
into platform-owned display state or was transferred to the physical display.
Backends must consume or copy all borrowed data before the drawing call returns,
including with DMA. Physical transfers initiated by a call complete before
return. The platform controls presentation timing; the desktop platform may
composite its display state after `app_render()` returns.

Drawing does not promise double buffering, an atomic visible change or tear-free
output. A backend must preserve pixels outside drawing coverage and between
calls; it must not require a complete frame on every render. Initial contents
are unspecified, so UI draws the complete screen on its first render.
On I/O failure, contents may be partial or unchanged. UI reports the failure
to App and redraws the complete screen on its next rendering
attempt. Invalid arguments are rejected before that operation changes pixels.

### Drawing Operations

| Callback | Behavior |
| --- | --- |
| `fill_rect(context, rect, color)` | Fills the clipped rectangle with one color |
| `draw_line(context, x0, y0, x1, y1, color)` | Draws a one-pixel-wide integer raster line including both endpoints; equal endpoints draw one pixel |
| `draw_text(context, x, y, font, color, text, length)` | Draws a single run of fixed-cell bitmap glyphs, with `(x, y)` at the first cell's top left |
| `write_pixels(context, rect, pixels, pixel_count, stride_pixels)` | Copies a row-major RGB565 region; stride is measured in pixels |

Lines use integer Bresenham rasterization without antialiasing. Clipping must
preserve the visible pixels of the original line. Implementations must bound
work for off-screen geometry rather than walking arbitrarily long hidden spans.
Rectangle outlines can be composed from lines; additional shapes are initially
rendered in shared code and transferred with `write_pixels`.

### Shared Fonts and Text

`PalDisplayFont` describes shared, immutable bitmap assets, not backend-selected
fonts. Width and height are nonzero. `first` and `last` form an inclusive ordered
range of printable ASCII (32–126). Glyphs appear in character order, each using
`height` rows of `(width + 7) / 8` bytes. Rows are most-significant-bit first;
unused trailing bits are ignored. `bits` is required and `byte_count` must cover
all glyphs. Invalid descriptors return `PAL_INVALID_ARGUMENT`.

`draw_text` requires a valid font even for an empty run. `text` is required only
when length is nonzero; length is a byte count, not a null-termination rule.
Every byte must be within the font's range, otherwise the entire call is rejected
before drawing. No encoding conversion, newline processing, wrapping or fallback
font selection occurs in PAL. UI performs those operations before drawing.

Set glyph bits paint the supplied color; unset bits preserve existing pixels.
UI fills the background explicitly when needed. Each character advances by the
font's width, with no kerning. Thus UI measures text using the same font metrics
on every platform, without a separate platform measurement API. Native font
rendering may only be used if it reproduces the supplied glyphs and metrics.

### Pixel Regions and Memory

Font byte counts, text lengths, pixel counts and pixel strides use `uint32_t`
on both desktop and firmware. Required counts must fit within `UINT32_MAX`;
implementations check arithmetic before multiplication or addition and validate
native byte sizes before indexing buffers or converting to native size types.

For a nonempty rectangle, `pixels` is required, `stride_pixels >= rect.width`,
and `pixel_count` must be at least
`(rect.height - 1) * stride_pixels + rect.width`. Invalid or overflowing sizes
return `PAL_INVALID_ARGUMENT`. Validation covers the entire source rectangle,
even when it is partly or wholly off screen. Clipping offsets the source
accordingly; it does not stretch or rescale pixels. For a zero-area rectangle,
a null pointer, zero count and zero stride are allowed.

No application-side full framebuffer is required. UI may use bounded reusable
scratch buffers for icons, tiles or software-rendered graphics. The storage
owner guarantees capacity and lifetime; PAL retains no application pointer,
including a font descriptor or its bitmap data. A backend may use a framebuffer
internally but must not impose that allocation on App/UI.

## Matrix

`PalMatrix.replace(context, connections, count)` (`pal/matrix.h`) receives the
complete desired connection set. An empty set disconnects all routes; it permits
a null array. A nonempty set requires a valid array. Array order is irrelevant;
duplicate pairs and unmappable port numbers are invalid arguments.
`count` is a `uint32_t` element count on both desktop and firmware; native array
byte-size calculations must be checked before multiplication or conversion.

Each endpoint is a device-wide port number using PAL's own `PalPortNumber`
type. App explicitly translates Core connections. Platform implementations map
these numbers to physical or simulated matrix addresses; PAL contains no module
identifiers and does not include Core headers. Matrix capacity remains an
implementation capability, while Core storage capacities stay in `limits.h`.

An otherwise valid connection set that exceeds hardware connection capacity or
cannot be realized by the hardware topology returns `PAL_UNSUPPORTED`.
Implementations validate known capacity and topology limits before hardware
writes, returning `PAL_MATRIX_STATE_UNCHANGED` for these rejections. They must
not apply only the subset that fits. If a limitation is discovered only after
writes have begun, the failure effect follows the same guarantees as other
failures: unchanged only when preservation is guaranteed, otherwise unknown.

`PAL_OK` confirms the complete requested set, including removal of routes absent
from the request. Success does not require electrical readback: an implementation
must document whether completion is established by readback or successful device
commands. It must not report success for an incomplete replacement.

On failure, `PalMatrixResult.failure_effect` is:

- `PAL_MATRIX_STATE_UNCHANGED`: previous routing was preserved.
- `PAL_MATRIX_STATE_UNKNOWN`: routing may be partially changed or unverified.

Atomic replacement is not required. An implementation may disconnect/reconnect
sequentially, so transient signal interruption is possible even on success.
Argument validation occurs before hardware writes and returns unchanged routing.
Other failures may claim unchanged only if the implementation can guarantee it.
App uses complete replacement to establish startup state and recover from
unknown routing; recovery requires no separate hardware-specific API.

## Storage

`PalStorage` (`pal/storage.h`) reads and writes opaque named byte objects. Keys
are nonempty null-terminated ASCII names containing only letters, digits,
underscores and hyphens; implementations map them to files or storage slots.
Key/object capacity limits are platform choices. Valid requests that exceed
implementation storage limits return `PAL_UNSUPPORTED`.

Storage capacities, object sizes and the read size output use `uint32_t` byte
counts on both desktop and firmware. Objects larger than `UINT32_MAX` are
unsupported; reads return `PAL_UNSUPPORTED` with size zero. Implementations
validate native size conversions before accessing buffers.

`read(context, key, data, capacity, size)` reads a complete object. `size` is
required. On success it reports bytes read. If capacity is insufficient, it
returns `PAL_BUFFER_TOO_SMALL` and reports the required size. Other failures
set size to zero. Buffer contents are valid only on success. Null data is
allowed only with zero capacity, permitting a size query; a nonempty object
then returns `PAL_BUFFER_TOO_SMALL`.

`write(context, key, data, size)` creates or replaces an object. Null data is
allowed only for an empty object. Success means the service completed its write;
failure may leave the object changed. Atomicity and power-loss durability are
not promised by this initial contract.

App owns encoding/decoding outside PAL. Core struct layout is not a file format.
Preset serialization, persistence policy and stronger durability requirements
will be defined when preset support is implemented.

## Implementation Follow-up

Initial desktop work supplies an input queue, primitive display rendering, an in-memory
matrix and optional file storage behind these interfaces. Contract tests will
be added alongside their implementations. Core/UI operation signatures and
hardware-dependent routing restrictions are outside this contract pass.
