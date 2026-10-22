# Plan 10: GPU canvas

On `master` the canvas is a `QGraphicsView` painted by Qt's raster engine on the CPU, and plan 9 replaces it with a view
whose viewport is a rendering backend's widget and a CPU backend that draws what `master` draws. This plan adds the
second backend: a renderer on `QRhi`, hosted by a `QRhiWidget` as the view's viewport, drawing the circuit with
instanced draws from a mirror of the document (one instance buffer per layer with stable slots, an edit uploading
only its dirty ranges), wires as strips whose curve the vertex shader rebuilds, status colours from a style table,
bodies from sprite atlases per zoom level, labels from a glyph atlas, ports and the background in shaders, and 4×
multisampling. Frames follow the mirror's changes; a hidden tab releases its GPU resources; a lost device is rebuilt
from the mirror; a pipeline failure switches the view to the CPU backend. A setting (Automatic, Hardware, Software)
with an environment override chooses the backend; Automatic, the default everywhere, prefers the GPU when a startup
probe finds a hardware adapter. CI checks the mirror on `QRhi`'s Null backend in every job and compares parity renders
with the CPU backend on Mesa and on Metal.

- **Depends on:** plans 1 to 9, the Qt floor's plan and the model-cost round, landed, since the plans run one
  at a time, in their order (the plans' rules, §1): plan 9 (the render target and the `RenderBackend` interface,
  the view whose viewport is the backend's widget and its `viewportReplaced`, the overlay state as data, the document
  listener, the sprite key and cache with the rule for fractional pixel ratios, the ports holding their slots in the
  core, and the CPU backend as the fallback and the parity reference); the model-cost round (the model as it leaves
  it, with the listener's events settled for the mirror, MC.11, and the probe's selection and move phases); plan 6
  (the interface through which the renderers read the core's status arrays, P6.D1, and the runner's changed slots);
  the Qt floor's plan, since `QRhiWidget` first appeared in Qt 6.7; plan 3 (the appearance key and the
  presenters its sprites draw from); plan 1 (the wire-pixel goldens, the CPU reference its parity renders build on,
  and the probe that times a load and its first frames).
- **Lands before:** nothing; it is the last in the order.
- **Branch and ledger:** `10-gpu-canvas`, from `09a-model-cost` once the model-cost round has landed, in slices (the
  plans' rules, §1 and §3). `LEDGER_10_gpu_canvas.md` has a row for every step, check and open question below.
- **Rules every plan shares** are the plans' rules (`PLAN_rules.md`) and `CLAUDE.md`'s "Building and testing". This
  file states plan 10's own: all `QRhi` use stays in the GPU target, so a source change in a later Qt minor release is
  fixed in one place; the mirror changes only from the model's change notifications, and no per-frame pass walks every
  element, port or wire; a cache is correct only while its key names everything the cached value depends on; CPU per
  frame is measured on a settled, attached capture, blocking on the presented frame, never by frame rate; nothing CI
  renders may need Metal 3 or an Apple GPU family; a parity test fails instead of skipping where the job requires a
  backend; and Automatic picks the GPU only once every layer passes its parity renders. Its decisions are numbered
  P10.D1 on.

## 1. Where `master` stands, and what was measured

### 1.1 Painting on the CPU

`master` has no `QRhi` code; it paints the canvas through `QGraphicsView` with Qt's raster engine: wires as stroked
paths with adaptive antialiasing (off while a paint pass is over its 50 ms budget), elements from Qt's per-item device
cache, ports and labels as child items. On the 8,000-element circuit at 1280×800, on the `perf/load-hotspots` prototype
branch (`master`'s view, but wires never antialiased and ports drawn by one layer item), three frames at 1:1 and three
fitted cost 5.96 billion instructions: `drawPolyline` 69.5% (the stroker 1.45 billion, the scan converter about 2.5
billion), reduced-detail wires 11.7%, cached element pixmaps 5.1%, ports 4.9%, the wire curve's own code 3.1%, element
painting 1.5%. A 1:1 window holds 7,284 of the 14,800 wires, 5.74 million device pixels of stroke, about 1,500
instructions per pixel of wire length; Qt pays a function call, a clip intersection and a blend setup per span of a
3-pixel non-antialiased wire. Wall time per frame for the alternatives (8 threads, a virtual GPU):

| | Full rebuild, dense window | Same, sparse (600 wires) | Pan, nothing changed | Every wire recoloured, dense |
|---|---|---|---|---|
| Raster, Qt antialiased | 1,200–1,900 ms | 51 ms | as a rebuild without tiles | as a rebuild |
| Raster, Qt non-antialiased | 400–470 ms | 16 ms | blit 0.6–1.4 ms | 400–470 ms |
| `QOpenGLWidget` + `QPainter`, 4× multisampling | 156 ms every frame | | 156 ms | 156 ms |
| CPU, own antialiased filler, 1 thread | 87–160 ms | 3.2–4.3 ms | blit 0.6–1.4 ms | 87–160 ms |
| The same, 8 threads | 22–32 ms | 1.4–1.5 ms | blit 0.6–1.4 ms | 22–32 ms |
| Qt Quick scene graph, one batched node, palette texture | 0.7–1.2 ms CPU | 1.3–1.7 ms CPU | 0.7–1.2 ms CPU | 1.2 ms CPU |

A CPU alternative keeps a buffer of wire ids and coverage and colours it with a palette pass on every status change (66
ms to rasterize the dense ids once, 2.6 ms per palette pass on one thread, 1.1 to 1.6 on several), but a pixel then
holds one wire, so the antialiased fringe of a wire over another is not blended (a mean difference of 22.7 of 255 on the
touched pixels of the dense window), and it scales with the viewport (about 10 ms single-threaded at 4K). The whole
scene as four batched nodes in one scene graph (14,800 wires as 5.35 million vertices, 8,001 element quads, 44,005 port
quads, 8,001 labels of six glyph quads) cost 1.0 ms of CPU per frame static, panning or zooming, and 1.2 ms with every
wire and port recoloured each frame (palette uploads of 59 and 176 KB): a fixed cost per frame, not per object. The user
chose the GPU on 2026-10-03, as the only route measured at about 1 ms of CPU per frame with everything changing. What
that prototype left out, and a real renderer needs: atlas management with per-element re-rendering on a state change
(displays, ICs, selection), text shaping and caching, port glyph shapes, overlays (rubber band, hover chips, the wire
being dragged, tooltips), reduced-detail variants, high-DPI atlas scaling, incremental geometry on edits (11.6 µs per
dense wire plus a partial upload), and a fallback, since Qt Quick's software adaptation does not run custom materials.

An earlier synthetic prototype in Qt Quick (2,000 gates batched into one geometry node, curved antialiased wires, 4×
multisampling) used about 4.4% of a core where the widgets view's rendering alone used about 25%, and sixteen times more
wire geometry with multisampling cost it nothing measurable: the curve flattening and antialiasing that dominate the CPU
path moved to the GPU.

### 1.2 What the spikes found

On the same circuit with the old `Scene` as a headless model and the mouse forwarded as scene events, a Qt Quick scene
graph renderer cost 2.3 ms of CPU per frame static or panning, 3.0 panning and zooming, 3.7 with every wire and port
recoloured, 4.4 with a real simulation tick and real statuses each frame (reading every status 0.8 ms), 4.5 dragging an
ordinary element, and 12.2 dragging the clock (about 87 ms per moved frame, 17 of it the renderer refilling 7,200 wire
slots, the rest the scene's own move). A rubber band cost 158 ms of the scene's event handling and a wire drag 6.5 ms,
model costs independent of the renderer. Other circuits (pan, recolour): the IC-heavy circuit 2.7 and 4.1 ms; the
20,000-element combinational one 9.2 and 7.9 ms, 2.7 of it polling for moves and 1.2 scanning statuses, neither needed
with change notifications. What the spikes had to learn:

- **One node per layer re-uploads its whole vertex buffer** (12 MB to move one element), so the spike split layers into
  chunks (256 wires, 2,048 quads) with fixed-size slots per object (a wire 32 samples, 66 vertices; a port, element or
  label 6), an edit rewriting its slots in place and uploading only the chunks it touched.
- **A `DrawTriangles` node with a custom material** rendered as one long strip, zig-zagging across neighbouring ports,
  though it reported triangles; the cause was not found, and strips with a degenerate join vertex at each end of a slot
  worked.
- **Bodies drawn in code** (ICs, multiplexers) were painted once into the atlas and pictures copied in; labels were
  rendered with `QStaticText` into the atlas, one entry per distinct text.
- **Moves were found by comparing** every element's position and rotation each frame (1.2 ms at 8,000); the renderer
  needs the model's change notifications instead. Adding or removing an item was a full rebuild (30 ms); incremental
  appends need spare slots.
- **Hosts,** CPU per frame (static, pan, recolour, drag): `QQuickWindow` 2.4, 2.3, 3.7, 4.5 ms; `QQuickWidget` 0.3 to
  0.7 ms more; a window container 3.4, 2.2, 3.6, 4.5 ms. A `QRhiWidget` renderer of the wires cost the same as the Qt
  Quick item (2.5, 2.1, 2.0 ms), so Qt Quick adds nothing measurable; the plain widget's price is writing text, atlases
  and picking.
- **Beside other widgets:** as a scroll area's viewport with both scroll bars and a label, the `QRhiWidget` cost 1.6 to
  2.8 ms per frame; an unrelated scroll bar's repaint cost 8.3 to 11.7 ms there and 7.5 to 10.4 ms in a raster window
  with no texture widget (`perf` put 69% of the samples in the kernel's task switching, the virtual machine's thread
  wake-ups for the raster engine's pooled fills), so the texture widget adds 0 to 3 ms. A window with a texture child
  composes at most about once per refresh, since Qt turns a `repaint()` within a frame of the last compose into a
  deferred update, and `frameSubmitted` fires after every compose, re-rendered or not. An earlier figure of +22 ms per
  repaint beside a `QQuickWidget` was mostly that same flush cost. The overlays above the `QRhiWidget` (tours,
  exercises, the floating minimap, tooltips, the inline editor) and drag and drop worked; a window container broke both.
- **A `QOpenGLWidget` as the old view's viewport** kept the CPU stroking and was slower than raster (static 8.7 against
  7.7 ms, pan 28.9 against 21.5, every wire recoloured 1,289 ms of CPU against 358): rejected.
- **Without a display,** the offscreen platform cannot create an OpenGL `QRhi` and falls back to the Null backend;
  `xvfb-run` gets Mesa's llvmpipe. A Qt Quick window proves nothing about compositing a texture widget among widgets.
- **Measuring:** spinning on events inflated the CPU figures fivefold; a frame is measured by blocking on the presented
  frame.

### 1.3 What the Qt Quick rewrite's canvas taught

The `qtquick-rewrite` interface drew its canvas on the GPU, and its first real profile (the 2,000-element clocked
circuit, 20 s after a matched settle, `perf stat -e task-clock`) cost 56.9% of a core against the widgets view's 37.0%.
Its `updatePaintNode()` rebuilt, for every element on every frame, an appearance key from a chain of `QString::arg()`
calls (22.9% of the samples) and the element's bounds by walking its ports (17% and 16%), and every wire's port
positions through `scenePos()` (7 to 9% per function), driven by a refresh timer that called `update()` every 16 ms
whether anything had changed or not. Two caches fixed it: per element, its bounds and atlas tile, kept while a cheap
fingerprint matched (the picture's `qint64` cache key, rotation, flips, selection and port counts, and for the displays
each input's status); per port, its scene position, cleared whenever the spatial index was rebuilt (after every move,
add, delete, paste, undo, redo, rotation, flip and morph). It then cost 31.5%, 15% below the widgets view, the rest
being frame submission and idle waiting. The rules it gives are this plan's: a GPU frame is cheap only while the CPU
side does work in proportion to what changed; a cache key that misses a field the frame path reads is a silent stale
draw, so the mirror's dirty marking has a test per kind of change; CPU per frame is measured on a settled, attached
capture, never by frame rate.

### 1.4 Qt, CI and packaging

`master` asks CMake for Qt 6.2 (6.9 for WebAssembly); its build matrix runs 6.8.3 and 6.9.3 (macOS 15 on 6.8.3, macOS
26 on 6.9.3) and releases on 6.9.3; the Debian i386 job builds against the distribution's Qt. `QRhi` is public since
Qt 6.6 with limited compatibility guarantees (no source or binary promise; breaks kept minimal, in minor releases only),
used by linking `Qt::GuiPrivate` and including `<rhi/qrhi.h>`; `QRhiWidget` exists since 6.7. The private headers come
in `qt6-base-private-dev` (Debian, Ubuntu) and `qt6-qtbase-private-devel` (Fedora); aqt and Arch's `qt6-base` include
them. The release's AppImage already bundles the xcb GL integrations and the Wayland graphics integration, and
`windeployqt` deploys the Direct3D compiler. The Qt floor's plan, outside the ten plans, raises the minimum to
6.8 right after plan 2, the oldest Qt CI already builds; this plan starts on that floor.

GitHub closed the request for GPU access on its hosted runners as impossible by their design
(`actions/runner-images#7085`), yet the paravirtual GPU of its Apple-silicon virtual machines runs Metal. Measured on
2026-10-21 by a probe workflow (run 39637751948) on CI's macOS and Qt pairs (macOS 15 with Qt 6.7.3 and 6.8.3, macOS 26
with 6.9.3): each runner is a virtual machine ("Apple M1 (Virtual)") with one Metal device, the "Apple Paravirtual
device", in the `mac2` family only (not `apple7`, not Metal 3). A Metal draw with shaders compiled at run time read
back exact pixels, and a `QRhi` Metal offscreen frame read back its exact clear colour under both the cocoa and the
offscreen platform plugins; `system_profiler SPDisplaysDataType` prints nothing there, so it is no test. The parity
renders therefore run on Metal. They prove output, not speed or an Apple GPU's driver.

### 1.5 A route not taken

Qt Canvas Painter, a technology preview in Qt 6.11, is an imperative 2D painter on `QRhi` with a `QRhiWidget`-based
widget and paths cached on the GPU, which would fit wires whose geometry rarely changes while their colours do. As read
in July 2026 (not checked since), it needs a Qt 6.11 floor, has no API or ABI stability yet, and lacks dashed strokes,
clipping to shapes other than rectangles, and filter effects. It was noted and never measured.

## 2. What plan 10 builds

### 2.1 The renderer and its host

`GpuRenderer` works on a `QRhi`, a render target and a command buffer it is given, and knows no widget;
`GpuCanvasWidget`, a `QRhiWidget`, hosts it as the view's viewport (`initialize`, `render`, `releaseResources`), and
tests drive the same renderer on an offscreen `QRhi`. Both live in the GPU target (`render_gpu`, between plan 9's
render and canvas layers), which holds all `QRhi` use and has its own layer rule in plan 9's include check. The view
hands the viewport's paint and resize events to the widget (plan 9 gives the view that path), and a backend switch
replaces the viewport through plan 9's `viewportReplaced`.

The host's first step checks real input on it: the mouse, palette drag and drop, cursors, tooltips, the overlays above
it (tours, exercises, the floating minimap, the inline editor), and high DPI. It also settles two questions no spike
answered: whether the host widget must stay alive across tab switches, and whether adding or removing a window's only
texture widget on a tab switch recreates the native window (P10.20).

### 2.2 The mirror

Plain C++, testable without a GPU: one instance buffer per layer with stable slots (a free list and a high-water mark);
an edit rewrites its records and uploads the dirty byte ranges, with no chunking. The mirror changes only from plan 9's
document listener (an element or wire added or removed, a wire moved from its ends, a port moved, a label changed, the
theme changed, a bulk scope) and from the status publications (§2.4); no frame walks every element, port or wire. A
bulk scope's changes reach the mirror once, at its end. The mirror's dirty marking has a test per kind of change, and a
coverage test drives every change the listener reports and checks the mirror's records against a mirror built from
scratch.

### 2.3 Wires

A static strip of 33 samples by two sides (a 129-sample strip above a length threshold set from the parity renders);
the instance holds the end points, from which the vertex shader rebuilds the control points exactly as plan 9's
`WireCurve` does and evaluates the cubic, and a flags index. Widths as `master` draws them: 3, Error and selected 5, at
least one device pixel at far zoom; the blue highlight halo (a 10-unit pen on `master`) is a second instanced draw.
The wire being routed is drawn among the wires, from plan 9's overlay state. Moving the clock's 7,200 wires rewrites
7,200 records of about 20 bytes (about 144 KB) instead of re-tessellating 7,200 × 66 vertices.

### 2.4 Status bytes and the style table

One status byte per wire and per port, gathered into a per-instance attribute (about 15 KB for wires and 44 KB for
ports on the 8,000-element circuit); colours come from a style table in a uniform buffer indexed by status and flags,
rebuilt on a theme change. The gather reads the core's status arrays through plan 6's interface, at the slots plan 9's
ports hold, taking the changed slots of each of the runner's publications, which name the ports and wires whose status
changed (P10.D9).

### 2.5 Ports

One quad per port (position, kind, flags) and one status byte; drawn analytically in the fragment shader, the input
circle and the output triangle with `master`'s pens and brushes and the hover highlight, glyphs within the hit square as
plan 9's port painter draws them. Ports are drawn above every element (plan 9's ordering).

### 2.6 Elements and sprites

Two quads per element, the selection and then the body, with the rectangle, the transform, the sprite rectangle and a
selected flag; the renderer draws the selection (P10.D7). Sprites per picture key per zoom level at device size, in
atlas textures with texels 1:1 at quarter turns, from plan 3's presenters and plan 9's sprite key, under plan 9's rule
for fractional pixel ratios (the rule its sprite probe, P9.13, settles). A zoom step re-renders the visible sprites
under a per-frame budget, the previous level scaled meanwhile (P10.D5); eviction is least recently used. Every class of
picture key is covered (pictures, bodies drawn in code, displays, ICs, custom appearances), and the atlases' memory is
measured on the IC-heavy and the 20,000-element circuits.

### 2.7 Labels

Glyphs shaped once with `QTextLayout` and `QGlyphRun`, their bitmaps from `QRawFont::alphaMapForGlyph` in a glyph
atlas, one quad per glyph, at the label's scene transform from plan 9; right-to-left and CJK text checked.

### 2.8 The background, the overlays and the draw order

The background is one quad with the grid dots in its shader. The overlays come from plan 9's overlay state: the rubber
band, the wire being routed, the hover chips. The draw order is the same in both backends: the background and the grid;
the wires in stacking order with their halos, the wire being routed among them; the rubber band; the elements in
stacking order, each its selection and then its body; the ports; the labels; the hover chips.

### 2.9 Frames and the lifecycle

4× multisampling through `QRhiWidget::setSampleCount` (P10.D4). Frames on mirror changes, coalesced and paced by the
simulation's display rate. A hidden tab releases its GPU resources and its mirror (a rebuild costs about 30 ms on the
8,000-element circuit), one renderer per window on the visible tab. Device loss and `releaseResources` rebuild from the
mirror. A pipeline or shader failure switches that view to the CPU backend, after which a test resizes the window and
checks that the minimap's view rectangle still follows it.

### 2.10 The setting, the probe and the override

A setting with three values, Automatic, Hardware and Software, and an environment override. Automatic, the default
everywhere (macOS included, P10.D3), means the GPU when a startup probe creates a `QRhi` on the platform's default API
(Direct3D 11, Metal or OpenGL; Vulkan selectable) whose adapter is not a CPU device, and builds the pipelines; else the
CPU backend. Until every layer passes its parity renders (P10.12), the setting is not offered, Automatic picks the CPU
backend, and the GPU is reached by the override alone. Crash reports are tagged with the graphics API, the adapter and
the backend. Where the setting is offered (`master`'s View menu holds the theme and fast mode) is shown to the user
(P10.25).

### 2.11 Shaders

Background, wire, port, sprite, glyph and solid shaders through `qt_add_shaders`, compiled at build time, with no
committed `.qsb` (P10.D6).

### 2.12 Build and packaging

- **The option** `WIREDPANDA_GPU_RENDERER`, on by default when `Qt6::ShaderTools` is found and off for WebAssembly. CI,
  the release and the translation workflow pass it explicitly, so a missing module fails the job instead of shipping a
  CPU-only release, and so the translation workflow, which configures with only the multimedia and image-format
  modules, still extracts the GPU target's strings.
- **The modules to add** (`qtshadertools`): the `modules:` lists of the build, release, sanitizer, coverage and CodeQL
  (both jobs) workflows and the translation workflow's one job that configures the project, not the WebAssembly
  workflow (CPU only); the apt lines of the fresh-dependencies workflow and the Debian i386 container
  (`qt6-shadertools-dev`, `qt6-base-private-dev`, both in trixie for i386); the dev container's `Dockerfile`; the AUR
  `PKGBUILD`'s `makedepends`; the build documents' apt and pacman lines and the aqt recipe (`-m qtshadertools`);
  `CONTRIBUTING.md`; `DEVELOPER_GUIDE.md`.
- **Runtime libraries:** none new. `QRhi` is in Qt Gui and `QRhiWidget` in Qt Widgets; the AppImage and `windeployqt`
  already deploy what it needs (§1.4).
- **Private API:** a binary works only with the Qt it was built against. Releases bundle theirs; distribution packages
  rebuild when their Qt changes (an AUR `pkgrel` bump on a Qt update).

### 2.13 CI for the GPU tests

The mirror's logic runs on `QRhi`'s Null backend in every job. The parity renders compare the GPU backend with the CPU
backend: the Linux jobs install `xvfb` and Mesa's `libgl1-mesa-dri` and run them under `xvfb-run` with
`WIREDPANDA_REQUIRE_RHI=opengl`, so a missing backend fails instead of skipping; the macOS jobs set `metal` and run on
the hosted runners' paravirtual device (§1.4). Nothing CI renders may need Metal 3 or an Apple GPU family.

### 2.14 Budgets and measurement

Measured on real hardware, on Linux and Windows, after the plan: pan, zoom and recolouring every wire under 3 ms of
CPU; an ordinary drag frame under 6 ms; the first frame of the 8,000-element circuit under 100 ms (P10.17). CPU per
frame on a settled, attached capture, blocking on the presented frame. macOS is not measured (no machine): its default
rests on the probe, the fallback and the crash tags (P10.D3).

### 2.15 Documentation and the manual pass

The build documents and the developer guide describe the option, the modules, the setting and its override, the GPU
tests and their environment variable. A pass by hand covers what this plan changes: the canvas on the GPU in both
themes, high DPI at 1, 1.25, 1.5 and 2, many tabs and tab switches, the large fixtures, the overlays (tours, exercises,
the floating minimap, tooltips, the inline editor), palette drag and drop, right-to-left layouts, the setting's three
values and the fallback. Release notes are written at the release.

## 3. Pitfalls

- **A cache key that misses a field** the frame path reads is a silent stale draw (the Qt Quick canvas's lesson): the
  mirror's dirty marking has a test per kind of change, and a test compares the mirror with one built from scratch.
- **A per-frame pass over every element** (the Qt Quick canvas's 16 ms poll, the spike's move detection at 1.2 ms per
  frame) costs in proportion to the circuit; the mirror follows notifications.
- **A whole-layer upload** for a small edit (12 MB to move one element in the spike): slots and dirty ranges.
- **Adding or removing an item as a full rebuild** (30 ms in the spike): the free list and the high-water mark.
- **Triangles drawn as one strip:** strips with a degenerate join vertex at each end of a slot, as the spike found
  working where a `DrawTriangles` node with a custom material zig-zagged across neighbouring ports.
- **The offscreen platform has no OpenGL `QRhi`**: tests that need a real backend run under `xvfb-run`, and a job that
  requires one fails instead of skipping on Null.
- **A Qt Quick window proves nothing** about compositing a texture widget among widgets; the host is checked as the
  view's viewport.
- **A window with a texture child composes at most about once per refresh,** and `frameSubmitted` fires after every
  compose, re-rendered or not: a frame counter is no proof of a render.
- **Spinning on events** inflated the CPU figures fivefold: measure by blocking on the presented frame.
- **`system_profiler` prints nothing on the hosted macOS runners;** only a rendered read-back proves Metal there.
- **`QRhi` may change in a Qt minor release:** all of it in one target, and CI on 6.8 and 6.9.

## 4. Steps and slices

| Row | Step | Slice |
|---|---|---|
| P10.1 | Build plumbing: the option, the shaders, CI's modules, the dev container, packaging, the documents' lines; the renderer's and the widget's shells in the GPU target (§2.1, §2.11, §2.12) | 1 |
| P10.2 | The render pass, the pipelines and bindings; the offscreen and Null test harness, failing where CI requires a backend (§2.13) | 1 |
| P10.3 | The host as the view's viewport, with its real-input check and the two tab-switch questions (§2.1) | 2 |
| P10.4 | The per-tab lifecycle: one renderer per window on the visible tab; a hidden tab releases its resources (§2.9) | 2 |
| P10.5 | The startup probe, the setting with its override (Automatic still the CPU), and the fallback (§2.10) | 2 |
| P10.6 | The mirror: slots, the free list, dirty ranges, the listener's changes, the dirty-marking and from-scratch tests (§2.2) | 3 |
| P10.7 | Wires: the curve in the shader, the status gather, the style table, selection, the halo, the wire being routed, the long-wire threshold, parity renders (§2.3, §2.4) | 3 |
| P10.8 | Ports, with parity renders (§2.5) | 4 |
| P10.9 | Elements: the selection and body quads, sprite atlases per zoom level, the budgeted re-render, eviction, every picture-key class, memory on the IC-heavy and 20,000-element circuits, parity renders (§2.6) | 5 |
| P10.10 | Labels: shaping, the glyph atlas, right-to-left and CJK, parity renders (§2.7) | 6 |
| P10.11 | The background shader and the overlays, in the shared draw order, parity renders (§2.8) | 6 |
| P10.12 | Automatic picks the GPU, every layer passing its parity renders (§2.10) | 7 |
| P10.13 | Device loss, pipeline and shader failure with the minimap check, the Sentry tags (§2.9, §2.10) | 7 |
| P10.14 | The GPU target's include rules enforced (§2.1) | 7 |
| P10.15 | The build documents and the developer guide (§2.15) | 7 |
| P10.16 | The manual pass (§2.15) | after |

The budgets, the checks and the open questions of §6 are rows too (P10.17 to P10.25).

## 5. Decisions

Each ends with its basis: measured or tested (a run backs it), read (the code, with nothing run), or a ruling (the
user's choice between workable options).

- **P10.D1. A `QRhiWidget` as the view's viewport;** `createWindowContainer` rejected. Basis: measured in the spikes
  (the `QRhiWidget` cost 1.6 to 2.8 ms per frame of panning beside scroll bars, and the overlays above it and drag and
  drop worked; a window container broke both; an OpenGL widget as the old view's viewport kept the CPU stroking and was
  slower than raster, 1,289 ms of CPU against 358 with every wire recoloured); the user's ruling of 2026-10-04.
- **P10.D2. The GPU preferred, behind an Automatic setting** (Automatic, Hardware, Software), Automatic the default
  everywhere. Basis: measured on one Linux machine with a virtual GPU (§1.1: 69.5% of 5.96 billion instructions in
  stroking polylines; a full rebuild of the dense window 400 to 1,900 ms on the raster engine; a GPU scene graph
  holding the whole scene 1.0 ms of CPU per frame, 1.2 ms with everything recoloured); the user's ruling of
  2026-10-03/04, the GPU chosen on 2026-10-03.
- **P10.D3. macOS gets the GPU default without a speed measurement.** **Unverified for speed:** no Mac was measured.
  The output is checked on the hosted runners' paravirtual Metal device (run 39637751948), so the parity renders run on
  Metal; the startup probe and the CPU fallback cover a failure, and crash reports carry the API, the adapter and the
  backend. Basis: measured for output; the user's ruling of 2026-10-03/04, made knowing the speed was unmeasured.
- **P10.D4. Wires with 4× multisampling on the GPU;** the CPU backend keeps `master`'s adaptive antialiasing (plan 9).
  Basis: measured on the synthetic Qt Quick prototype (§1.1: 4.4% of a core against 25%, and sixteen times the wire
  geometry at no measurable cost); the user's acceptance of 2026-10-03/04.
- **P10.D5. Far zoom-out through sprites per zoom level,** a zoom step re-rendering the visible sprites under a
  per-frame budget, the previous level scaled meanwhile, with eviction; the atlas churn is measured with the plan.
  Basis: the user's ruling of 2026-10-03/04.
- **P10.D6. Shaders through `qt_add_shaders`, no committed `.qsb`:** compiled at build time, so Qt's shader tools are
  needed only to build and no binary shader enters the repository. Basis: the user's ruling of 2026-10-03/04.
- **P10.D7. The renderer draws the selection;** a picture's key leaves it out, so one sprite serves a selected and an
  unselected element. Basis: tested on the branch (every body state by orientation by theme: one key per drawing and
  one drawing per key); the user's ruling of 2026-10-03/04.
- **P10.D8. Qt 6.8 for the whole project,** raised by its own plan before this one (`QRhiWidget` needs 6.7);
  users of distributions with an older Qt install it with the documented aqt recipe. Basis: read (Qt 6.8 or later
  ships in Debian 13, Ubuntu 25.04 and later, Fedora, Alpine and Arch; Ubuntu 22.04 and 24.04 and Debian 12 ship
  6.4.2 or older; no maintained distribution ships 6.7, whose support Qt ended in March 2025); the user's rulings of
  2026-10-14 (one floor for the whole project rather than for GPU builds only) and 2026-10-21 (6.8, not 6.7).
- **P10.D9. The renderers read the core's status arrays through an interface plan 6 defines,** at the slots plan 9's
  ports hold, taking the changed slots the runner publishes. Basis: the user's agreement of 2026-10-20 for the
  interface; read for the rest (plan 6's core and runner and plan 9's ports land before this plan starts, since the
  plans run one at a time, in their order, by the user's ruling of 2026-10-21).

## 6. Claims to check and open questions

- **The budgets** (P10.17), measured on real hardware on Linux and Windows: pan, zoom and recolouring every wire under 3
  ms of CPU, an ordinary drag frame under 6 ms, the 8,000-element circuit's first frame under 100 ms.
- **The macOS speed** (P10.18): unmeasured, with no machine; the output is checked on CI's Metal device.
- **Whether plan 9's listener is enough for the mirror** (P10.19): stable slots and dirty ranges from its events.
  Plan 9's P9.29 settles it against this plan's mirror (§2.2) before plan 9's listener is final, and the model-cost
  round's MC.11 again for an optimisation that changes the listener's events, both before this plan starts; the
  mirror's coverage test (P10.6) confirms the answer.
- **The host across tab switches** (P10.20): whether the host widget must stay alive across tab switches, and whether
  adding or removing a window's only texture widget recreates the native window; the first step settles both.
- **The long-wire threshold** (P10.21): the length above which a wire takes the 129-sample strip, set from the parity
  renders.
- **What fast mode does on the GPU** (P10.22): on `master` it turns off antialiasing, text antialiasing and smooth
  pixmap scaling; on the GPU, whether it turns multisampling off or does nothing, decided once the budgets show what a
  frame costs.
- **The atlases' memory and churn** (P10.23): on the IC-heavy and 20,000-element circuits, and zooming over many custom
  bodies.
- **The GPU path on real drivers** (P10.24): a manual pass on Linux, Windows and a Mac where one is at hand, beside the
  parity renders on llvmpipe and Metal.
- **What the user sees new** (P10.25): wires multisampled at every zoom, the setting and where it is offered, shown to
  the user before Automatic picks the GPU.

## 7. Risks

| Risk | Mitigation |
|---|---|
| The macOS GPU path is the default, but its speed is unmeasured | The startup probe and the CPU fallback; parity renders on the hosted runners' paravirtual Metal device; the crash tags |
| A Qt minor release changes `QRhi`, or a distribution's Qt update breaks a private-API binary (mostly AUR) | All `QRhi` code in one target; CI on 6.8 and 6.9; an AUR `pkgrel` bump on Qt updates; bundled releases unaffected |
| Driver differences across platforms | Parity renders on llvmpipe and Metal; a manual pass |
| Atlas churn when zooming with many custom bodies | The per-frame budget, the previous level scaled meanwhile, least-recently-used eviction |
| A change that does not reach the mirror draws stale | The mirror follows plan 9's one change funnel; a dirty-marking test per kind of change and a from-scratch comparison |
| Load stays as slow as `master`'s once paint is fixed | The model-cost round, which lands before this plan, measures load on plan 9's model and cuts what the measurement shows |
| A missing shader module ships a CPU-only release | CI, the release and the translation workflow pass the option explicitly |

## 8. What other plans take from plan 10

Nothing: it is the last plan in the order. The model-cost round, which takes the model's own costs (§1.2), comes
before it.

## 9. Evidence

The branch `gpu-canvas` never started this plan; the evidence is the spikes, the Qt Quick rewrite's canvas, the probe
and the floor as the branch raised it. A hash is its branch's, kept read-only.

| What | Where |
|---|---|
| The Qt Quick scene graph rendering a headless `Scene` with palette-texture status colours | `spike/gpu-scene`: `b214546bf` |
| The `QRhiWidget` wire renderer, and the raster and OpenGL viewport comparisons | `spike/gpu-scene`: `4c7906614` |
| A `QQuickWidget` painting over the view, which keeps the input | `spike/gpu-scene`: `e7ea176c2` |
| A `QRhiWidget` as a scroll area's viewport beside scroll bars and a label, and a raster baseline | `spike/gpu-scene`: `853865204` |
| The Qt Quick canvas's two caches (56.9% to 31.5% of a core) | `qtquick-rewrite`: `6e688df50` |
| The Qt Quick canvas's blind 16 ms repaint poll removed | `qtquick-rewrite`: `ba187e9d4` |
| The Qt floor raised to 6.7 on the branch, its documents and CI, which the 6.8 floor's plan follows | `gpu-canvas`: `c4b545131` |
| Metal on the hosted macOS runners | GitHub Actions run 39637751948 |
