# Plan 9: editor architecture

On `master`, `Scene`, a `QGraphicsScene`, holds the document (items, ICs, simulation, undo, autosave), the editing
session (selection, hover, view toggles, routing) and the interaction; ports, wires and labels are graphics items; click
selection and drag live partly in Qt's own item handlers. This plan splits the code into six layers enforced by build
targets; makes ports, wires and labels plain objects drawn and hit-tested by layers over spatial grids; gives the model
neutral events, neutral queries and one change funnel with a document listener; splits `Scene` into a document and an
editing session, with a controller taking the interaction; adds a new view whose viewport is the rendering backend's
widget, and a CPU backend with painters, a sprite cache and an export mode; and then deletes the old view and scene,
after which the element is a plain class. It draws what `master` draws, and plan 1's goldens pin it on both views.

- **Depends on:** plans 1 to 8 and the Qt floor's plan, landed, since the plans run one at a time, in their
  order (the plans' rules, §1): plan 3 (one element class with its descriptors, the presenters, the appearance key,
  the element painter, the outline and decorations, and the include check of the model's direction, which this plan
  extends to six layers); plan 6 (the core, with its suspend count in the model, which the document takes, and the
  runner with its changed slots, which the application layer and the renderers take); plan 8 (exporters on the
  netlist builder, which no longer include the scene, and the IC as one element class with no internal elements);
  plan 4 (the IC preview cache, whose renderer moves to the application here, and the IC registry's file cache); plan
  1 (the pointer, wire-geometry, stacking, view and wire-pixel goldens, run on the old and the new view; the probe that
  times a load and its first frames); plan 2's fixes this plan keeps (the wheel zoom that keeps the point under the
  cursor, the hit test that answers the topmost item, MCP's empty-scene error, a connected input that stays on its
  wire).
- **Lands before:** the model-cost round, which works on the new model; then plan 10, whose GPU renderer is a second
  backend behind this plan's render interface, set as the view's viewport, on the model as the round leaves it.
- **Branch and ledger:** `09-editor`, from `08-export` once plan 8 has landed, in slices (the plans' rules, §1 and
  §3). `LEDGER_09_editor.md` has a row for every step, check and open question below.
- **Rules every plan shares** are the plans' rules (`PLAN_rules.md`) and `CLAUDE.md`'s "Building and testing". This
  file states plan 9's own: every behaviour Qt carries for items today is implemented in the editor before the flag or
  base class that carries it goes; a replacement runs beside what it replaces until plan 1's goldens pass on both; a
  layer rule is enforced from the landing where its layer's code is clean. Its decisions are numbered P9.D1 on.

## 1. Where `master` stands

### 1.1 `Scene` and its items

`GraphicElement` is a `QGraphicsObject` and an `ItemWithId`; `Port` is a `QGraphicsPathItem`; `Connection` is a
`QGraphicsPathItem` and an `ItemWithId`; a label is a child text item of its element; a wire's status sets its pen.
`Scene` (a `QGraphicsScene`, the simulation's `SimulationHost`, which plan 6 deletes, and a `ContextDirProvider`) is
1,386 + 553 lines, with collaborators on the session side (`SceneInteraction`, `ConnectionManager`, `ClipboardManager`,
`PropertyShortcutHandler`, `VisibilityManager`, `SceneDropHandler`, `InlineLabelEditor`, `PortHoverLabel`) and on the
document side (`ICRegistry`, `Simulation`, the undo stack, the item registry, the context directory, autosave). In the
C++ sources of `App`, `MCP` and `Tests`, `Scene` occurs as a word
1,373 times in 216 files and `GraphicElement` 1,489 times in 241, so splitting `Scene` is a choice per caller between
document and session, not a rename. `Commands.cpp` calls `simulation()` 16 times, `setCircuitUpdateRequired` 16,
`addItem` 14, `icRegistry` 10, `itemById` 10, `updateItemId` 9, `removeItem` 8, `setAutosaveRequired` 5,
`deserializationContext` 4, `setPropertyUpdateRequired` 3, `truthTableElementChanged` 1, and `setSelected` six times
(the selection after undo and redo). `Scene` emits 11 signals: `circuitHasChanged` and `truthTableElementChanged`
belong to the document; `contextMenuPos`, `icOpenRequested`, `fileDropRequested`, `showStatusMessageRequested`, the
four IC-preview signals and `openTruthTableRequested` to the session or the controller. `QGraphicsScene`'s
`selectionChanged` feeds the property editor and the scene-to-interface binder.

### 1.2 Behaviours the new model reproduces exactly

- **Stacking:** elements at equal z, so insertion order, and an element restored by undo goes on top. An element's ports
  and label are its child items, drawn just after its body, so a later element that overlaps it hides them. Wires sit at
  z −1 under every element, the wire being routed included; the rubber band, added before any element, draws over the
  wires and under every element; the port hover chips draw on top (z 1,000), as does the inline label editor.
- **Hit testing** (`Scene::itemAt`): the items under the point plus those in a small rectangle around it; ports first,
  then elements, then any other custom item (wires), each in the order `items()` returns them (ascending, which plan 2
  fixes to answer the topmost).
- **Grid snap:** `itemChange(ItemPositionChange)` snaps to 8 px only while the element is in a scene, so loaded
  positions (set before insertion) are kept exactly; rotation moves each element about the group's centroid with
  `setPos`, which snaps, as do flip, align, distribute, MCP's move and IC placement; MCP's creation sets the position
  before insertion and keeps it exactly. Undo and redo snap too: the rotation's undo restores the positions it saved
  before its constructor snapped them, and the move's undo and redo set theirs, so an element loaded or created off the
  grid comes back on it.
- **Click selection and drag live in three places.** Qt's default item handlers carry the plain ones: the scene's
  interaction declines element clicks, so `QGraphicsScene` reaches the element; a press on an unselected item selects it
  alone, a release without movement selects the item alone, a drag moves every selected movable item to its press
  position plus the pointer delta through `setPos`, and an empty click clears. `Scene::eventFilter` turns Shift into
  Ctrl, and swallows a Ctrl press on an element or wire to select it and start the clone drag, so a Ctrl press drags a
  clone, and Shift-click toggles through `SceneInteraction`. The `sceneEvent` overrides of `GraphicElement` and
  `Connection` swallow every Ctrl press and release, so Qt's handlers never see one and cannot undo the toggle.
- **Routing:** `ConnectionManager::startFromOutput` and `startFromInput` add a half-connected `Connection` to the scene
  while the user drags, which the model, the simulation and autosave then see.

### 1.3 Where the intended layers are crossed

`ItemWithId` reaches `SceneItemRegistry`; `InputSwitch` reaches `Scene` and `Commands` (its click pushes an
`UpdateCommand`); `ICLoader` reaches `Scene` (for `ICRegistry`'s bytes) and `Application::migrationEnabled`;
`Connection` reaches `Scene` (antialiasing) and `Application::renderingEnabled`; both code generators reach `Scene`
(`wirelessTxInputPorts`, `sortByTopology`) and walk each IC's internal elements; `GraphicElement`, `Port` and
`ElementAppearance` reach `ThemeManager` (which reads `Application` and `Settings`); `ICRenderer` reaches
`ICPreviewPopup`, a widget, and `App/Element` holds widgets (`ElementLabel`, `ICPreviewPopup`); `ICRegistry` reaches
`Scene`, `Commands`, `Simulation`, `SimulationBlocker` and `Application::guardedSlot`; `SimulationBlocker` is used in
`Commands` (16 times), `ClipboardManager` and `ConnectionManager`; the view is reached from `SceneInteraction`,
`ConnectionManager` and `ClipboardManager` (the drag image through `QGraphicsScene::render` and the view's transform);
`RecentFiles` (in `App/IO`) reaches `Settings`; `ArduinoCodeGen` reaches `Simulation`'s settle bound; BeWavedDolphin's
waveform simulator drives the simulation and each element's simulation-state hooks; `Simulation` reaches
`Application::guardedSlot` and `interactiveMode` (widgets) and `QGuiApplication::primaryScreen`, and includes an
unused `<QGraphicsView>`; `Serialization.cpp` includes an unused `<QApplication>`; `App/Core` mixes `Application`,
`ThemeManager`, `UpdateChecker`, `Settings` and `ExerciseTourResources` with model utilities. The source root is a
global include directory, `wiredpanda_lib` links every Qt module, one precompiled header (`pch.h`, written by
`Scripts/update_pch.py`, with `QGraphicsView`, multimedia, network and printing headers) serves the library, the
application and the tests, and the module-qualified form (`#include <QtWidgets/QGraphicsItem>`) appears 15 times, in
five `*UI.cpp` files: a split into targets alone would reject nothing.

The plans before this one remove several of these crossings before it starts. Plan 3's operations leave no input
including the scene or the commands, its presenters draw in the theme they are handed, and the IC renderer's body
moves into the IC's footprint presenter; plan 4's file cache ends the loader's reach into `Scene`; plan 6 deletes
`Simulation`, `SimulationHost` and `SimulationBlocker`, the commands, the clipboard, the wire routing and the IC
registry holding the model's suspend count instead, and BeWavedDolphin sweeps through the runner instead of each
element's hooks; plan 8's exporters read the netlist builder, so neither code generator reaches `Scene`, walks an IC's
internal elements or reads the old engine's settle bound. The rest are this plan's (§2.3).

### 1.4 The view and its painting

`GraphicsView` zooms on a ladder of 1.25^n for n from −9 to +7, with the wheel and the keyboard; plan 2 makes the wheel
keep the point under the cursor. It pans with the middle button and with Space, fits the circuit snapped down to a
step, and sizes its scroll ranges as `resizeScene` computes them. Wire antialiasing is adaptive: it turns off while a
paint pass is over its 50 ms budget and comes back when the canvas has been idle (300 ms after a pass, 3 s after a
flip); each wire reports activity to the scene and asks it whether to antialias. Elements use Qt's device-coordinate
cache: a per-element pixmap of its bounds, used by `scene->render()` too (exports, the minimap, goldens), rendered at
the sub-pixel phase of the paint that fills it and reused at a later phase after a pure translation (up to half a
pixel off); any transform that is not a scale or a quarter turn re-renders it on every paint.

## 2. What plan 9 builds

### 2.1 Six layers as build targets

```
 App: MainWindow, palette, property editor, minimap, dialogs, SimulationRunner, AudioService, MCP
        │ zoomIn, zoomToFit, centerOn, mapToScene, ...  (the names GraphicsView had)
        ▼
 Canvas: CanvasView : QAbstractScrollArea ── viewport = backend host widget; input → neutral events
        │ PointerEvent / KeyEvent / DropEvent (scene coordinates)        │ draws through
        ▼                                                               ▼
 Editor: CanvasController + EditorSession ── Commands ──► Model    Render: RenderBackend (Cpu | Gpu)
        │                                                  │          ▲ overlay state, listener
        └──────────── reads ──────────────► Presentation ◄─┴── DocumentListener
```

| Target | Contents | May depend on |
|---|---|---|
| model | `Document`, `Element`, type descriptors, `Parameters`, `Port`, `Connection`, registries, `ICRegistry`, commands, serializers, code generators, indices, the simulation core and its suspend count; declares `BodyMeasure` | QtCore, QtGui, the Sentry helpers (the document's command push leaves a breadcrumb, as `Scene::receiveCommand` does) |
| presentation | presenters (implementing `BodyMeasure`), assets, label layout, styles, the theme's attributes, the sprite key and cache | model, QtSvg |
| editor | `EditorSession`, `CanvasController`, neutral events, clipboard, shortcuts, routing | model, presentation |
| render | `RenderBackend`, the CPU backend, painters, the label index | model, presentation, editor (overlay state) |
| canvas | `CanvasView`, the inline editor widget, cursors | editor, render, QtWidgets |
| app | widgets, `ThemeManager`, `SimulationRunner`, `AudioService`, MCP, BeWavedDolphin | everything, QtMultimedia |

Plan 10 adds `render_gpu` between render and canvas; a target with no sources cannot exist, so it comes with its code.
A CMake function creates each layer as an `OBJECT` library depending on the layers below it (every Qt library until the
module rules narrow them), which `wiredpanda_lib` links publicly (archiving the objects and passing usage
requirements; whole-archive unchanged). Files move only where a move adds value; the targets carry the layering. On the
branch the placement was: model = `App/Core` but the theme, `App/Element` but the appearance, the IC renderer and the
label metrics, `App/IO`, `App/Simulation`, `App/CodeGen`, `App/Wiring`, the version header, and from `App/Scene` the
commands, `ICRegistry`, the listener and notifier, the item registry and the indices; presentation = the theme and
those three; editor = `Scene` with its interaction, connection, clipboard, shortcut, drop, inline-label, port-hover,
input-event and visibility code; render = the layer items and their bounds; canvas = the view; the workspace stays in
the application.

### 2.2 The include check and the precompiled headers

Each target has a layer rule (no header of a later layer) and a module rule (only its Qt modules). The layer rule is a
CMake script (the build must not require Python) run as a custom command before each layer compiles, with a generated
configuration under `build/IncludeCheck/`: owners from the per-target source lists, every `App/` file owned but
`Main.cpp`; Qt module headers located through each Qt target's `INTERFACE_INCLUDE_DIRECTORIES`, so a bare
`<QGraphicsItem>` is placed as surely as `<QtWidgets/QGraphicsItem>`; each layer's precompiled header held to its
modules. It extends plan 3's check, which already keeps presentation out of the model, and plan 6's, which keeps the
runner out of it. A CTest entry runs the check with `cmake -P` on a generated tree with a lower and an upper target
(twelve cases); a break check per rule: including `Scene.h` from a painter, or `MainWindow.h` from the theme, fails
naming the file and its owner. `Scripts/update_pch.py`, which writes `pch.h` today, writes each layer's precompiled
header and the application's (on the branch the application's had fallen 48 headers behind, and 13 headers only
layers include). The translatable-strings target takes the layers as sources (Qt 6.9's `update_translations` collects
every non-utility target, object libraries included). The repository's `.gitignore` ignores `*.cmake` except under
`cmake/`, so the check lives there, and its test files are excepted too. Each rule is enforced from the landing where
its layer's code is clean. On the branch a ledger row claimed no model file included the theme or presentation while
two wiring files did: layering described and not built was not kept.

### 2.3 The untangling

The crossings of §1.3 that the plans before this one leave go, each a commit of its own, the list taken again on
`master` when this step starts: the serializer writes and reads the element's file tag, type names keyed on tags;
`ItemWithId` holds an item-registry interface that the document's registry implements; file migration becomes a model
setting, off by default and turned on by `main()`, so batch exports migrate as before; the theme splits into a core
`Theme`, its attributes (colours, pens) and the current theme (effective light or dark, its attributes,
`themeChanged()`), while `ThemeManager` keeps the setting, the system scheme, palettes and the style sheet, and
`main()` and the test runner apply the saved theme up front; `RecentFiles` and `ElementLabel` move to the application
with their translation contexts unchanged; the preview's rendering, which `ICRenderer` keeps once plan 3 has moved the
body into the IC's footprint presenter, moves to plan 4's preview cache in the application with the popup's size
limits; `App/Core` keeps model utilities, the theme and the Sentry helpers, while `ThemeManager`, `UpdateChecker`, the
install-relative paths, the exercise and tour resources, `Settings` and `Application` move to the application; the
unused includes go. The IC registry's guarded slot is installed by the application, and the workspace's reads of
`Settings` (autosaves, the minimap's geometry) and of the install-relative paths stay in the application. Plan 3 has
turned the casts of elements to their subclasses into type checks; the pointers to elements that a `QObject` let
callers hold become ids here (§2.14).

### 2.4 Orientation and the circuit's bounds

- **Orientation** is one explicit item transform (turn about the pivot, then flip about the centre), not Qt's rotation
  and transform origin (`ElementOrientation` calls `QGraphicsItem::setRotation` on `master`), so the element's branch
  for `ItemRotationHasChanged`, which nothing sends then, goes. An element sets `ItemSendsScenePositionChanges`, or a
  plain move stops re-routing its wires once ports are not child items (`master`'s element lists
  `ItemScenePositionHasChanged` but never receives it: only its ports send scene position changes).
- **Bounds** come from the registry: the union of elements (with what they draw) and wires; overlays do not count; an
  empty circuit has empty bounds; `setSceneRect(QRectF())` unsets the rect so it grows with the items; a wire being
  routed counts through its attached port; a load leaves the scene rect that `resizeScene` gives, for an empty file
  too. Every reader of Qt's bounds moves to them: the minimap, the exporters, the image and PDF exports, and MCP's image
  export, whose empty-scene error plan 2 makes reachable.

### 2.5 Ports, wires and labels as plain objects

Drawn and hit-tested by one layer item each, over spatial grids (P9.D8), with ports and labels drawn above every element
(P9.D10), which matches the hit test, already ranking ports over bodies; the change shows only where elements overlap.

- **Ports:** a port layer (z 0.5) over a point grid with a clamped edge cell, the theme's port style read when drawn.
  Removing the port's item type makes every check of it fail to compile, so none survives unnoticed. Callers that
  relied on `itemAt()` returning a port ask `portAt()` first (press, double-click, Ctrl-click, the pointer test
  harness). A port has no tooltip (the help event never showed one); it is drawn and hit only while it and its element
  are visible (show-gates hid the gate, and Qt hid the child ports with it); a press reaches a port one unit further,
  as hover does; with two ports within reach, the one drawn on top answers; setting a port's default leaves a connected
  input on its wire.
- **Wires:** the curve as a value (`WireCurve`), a curve grid whose hit shape uses the status pen's width, a wire layer
  at z −1 with the adaptive antialiasing, the theme's pens built once; the registry holds id-ordered maps of elements
  and wires, saved in that order; the selection API (`selection()`, `setSelected`, `clearSelection`, `selectInArea`) and
  a selection batch that emits `selectionChanged` once per operation; a file tag replacing the item type; the
  interaction code delivering press, move, release, double-click and hover to a wire as Qt would, the topmost wire being
  the highest id. The rendering-enabled flag goes: a curve is computed when asked.
- **Labels** are data of their element, drawn by a label layer (z 0.6, above ports): whether shown, the box from the
  label metrics, and the label's scene transform (anchor, its own turn, 90° for the IC, and a counter-orientation that
  keeps a rotating element's label upright).

Defects the branch made in these layers and fixed, each a test here (§3 has more):

- **A wire spanning too many grid cells** belongs in the overflow list whatever its curve does; the check comes before
  sampling (a wire 7 × 10^7 units long took 6.4 s and 323 MB in a debug build), and a sample's repeated cells are
  deduplicated once the list passes a few times the limit.
- **Layer bounds far from the origin:** each layer reports a square about the origin, 2^24 to a side, so the scene
  index holds it once however the scene rect changes; an item beyond it was not drawn, so the square doubles until it
  holds the item, telling Qt before the change, and an empty rectangle (a label with no text) is no area to repaint.
- **A hidden wire kept the mouse:** Qt ungrabs an item hidden while it holds the mouse; a plain wire holding it is
  forgotten the same way, as is one that left the scene, and the scene compares the stored address before reading the
  wire, since an undo restores a deleted wire's id.
- **Two crossing wires:** a double-click where they cross splits the upper one (the layer once answered the other).
- **A wire that joins a grid** is found by the grid's next query: its query stamps are cleared when it joins.

### 2.6 Neutral events and queries

- **Events:** `PointerEvent {scenePos, screenPos, button, buttons, modifiers, timestamp}`, `KeyEvent {key, modifiers,
  autoRepeat, timestamp}` and `DropEvent {positions, buttons, modifiers, timestamp, MIME data}`, positions defaulting to
  `{}` (designated initializers, with `-Wmissing-field-initializers` on). Gestures return a result (unhandled, accepted,
  ignored) instead of the accept flag, which `QGraphicsView` clears before it sends a press, double-click, move or
  release to the scene, so the "ignored" paths behave the same in production. While the old scene lasts it keeps the Qt
  event it handles for the view's pick area (the view can scroll during a gesture), for hover dispatch (which needs a
  widget and the last positions) and for a double-click sent on as a press. The Ctrl clone drag moves from the scene's
  event filter into the interaction code; the Shift-to-Ctrl remap stays while Qt's item handlers still read it. `App`
  had 83 Qt event sites by the branch's count; the hover virtuals skip `QGraphicsItem`'s repaint (only the IC accepts
  hover).
- **Queries:** `hitTest(pos, kinds)`: a port first (the port layer), then the topmost element exactly under the point
  else within its hit area, then the topmost wire likewise; a ports-only lookup for hover, wire completion and the help
  event. `itemsIn(rect or path)`: wires in ascending id, then elements bottom to top. The scene hides `items()`,
  `itemAt()`, `selectedItems()` and `setSelectionArea()` behind deleted declarations (variadic templates catch every
  overload), so every caller has to choose. `selectInArea()` changes elements with the scene's signals blocked and emits
  `selectionChanged()` once, only if something changed, deselecting first so a shared wire stays highlighted
  (`setSelectionArea` reported every stale entry it deselected).

### 2.7 One change funnel and the document listener

Every change to an element goes through one funnel, and the document reports each change of the circuit to its
listeners: element and wire added and removed (also from the registry when an item is destroyed while registered), a
wire moved from its ends, a port moved, a label changed, the theme changed; nested bulk scopes around the workspace's
loads, adding and deleting items (paste, drop, the undo of a delete with the reload of the elements it touched, the redo
of an add), the IC reload, and the blob command's undo and redo; the scene's teardown is not reported. The
`DocumentListener` is a plain interface for hot paths; coarse events stay Qt signals with one owner each (the
document's circuit and truth-table changes; the session's selection and status messages; the controller's context menu,
IC open, file drop, IC preview and truth-table edit requests; the audio service's mute). A coverage test drives every
kind of change and checks it was reported, and each bulk scope asserts one begin and end pair (on the branch five of
the eight, paste among them, were in no test). The IC reload's test checks the bulk change only: what the reload's
undo restores is plan 4's, tested there (its P4.5 and P4.6).
Plan 10's renderer and a pixel-changing change that does not notify are why: one funnel and the coverage test are its
mitigation.

### 2.8 The editing session

`EditorSession`, one per document tab, holds the selection (elements and wires, a batch API, `selectionChanged`), the
highlight derived from the selected elements, hover, the view toggles as a visibility predicate that renderers and hit
tests read (P9.D3, replacing `VisibilityManager`'s `setVisible`), the wire being routed (P9.D5), the paste offset and
the status messages. Qt's click-selection and drag handlers move into the interaction code first, beside its Shift
toggle and clone drag, with the same rules on the session's selection; then `ItemIsSelectable` and `ItemIsMovable` go
off, the element's `sceneEvent` goes, and its Qt selection API is hidden so its callers move at compile time. Commands
carry the ids to select after redo and after undo, which the session applies when the undo stack's index changes; a load
or a bulk event never selects.

### 2.9 The document, and `Scene`'s callers by group

`Document` holds the item registry (iterated in ascending id), `ICRegistry`, plan 6's simulation core and its suspend
count (which the scene holds until this step), the undo stack, the context directory, the serialization contexts, the
modified and autosave flags, the bounds and the spatial indices (a curve grid for wires, a point grid for ports, a
cell grid for elements; the label index belongs to the renderers, since label bounds depend on fonts).
A stacking sequence, assigned on insertion (file order on load, the
top on paste and on an undo's restore), reproduces `QGraphicsScene`'s order. Queries replace `QGraphicsScene::items`:
`hitTest(pos, mask, visible)` with today's priority, slop and wire widths, `itemsIn(rect)`, `itemsIn(path)` (an
element's rectangle or a wire's shape against the path) and `visibleElements(rect)`. Commands depend on the document
only. File tags are named constants of the same values (`FileTag::Connection` 65538, `FileTag::Element` 65539),
replacing the item type enums. An inventory of `Scene`'s callers, by group (document or session), is taken when this
step starts (P9.D9), and the callers move group by group, each move checked by the compiler; the tab creates its
simulation runner, or a test helper does for a bare document.

### 2.10 The controller

`CanvasController` takes over `SceneInteraction`, `ConnectionManager`, `ClipboardManager`, `PropertyShortcutHandler`,
`SceneDropHandler`, `InlineLabelEditor`, `PortHoverLabel` and the interaction half of `Scene`, on neutral events, and
owns no widget. Its state machines are ported one to one: press, drag and move with snapshots for the move command; the
Ctrl clone drag; the rubber band through `itemsIn(path)`; hover (ports, the IC preview's timing); the context menu;
double-click; input operations; nudge; auto-scroll (a request to the view); property shortcuts; align and distribute;
the clipboard, with the selection image through an interface the editor declares and the canvas installs with the CPU
backend's export mode; palette and file drops. The wire being routed is an anchor port and a free end, drawn as an
overlay; completing it is one command that adds the wire, and detaching an end is a preview until release, then one
command; the model, the simulation and autosave never see a half-connected wire. Snapping is controller and command
policy (P9.D6): the controller snaps drag, drop, nudge and paste targets to 8 px; the rotate, flip, align and distribute
commands, the rotation's undo, the move's undo and redo, and MCP's move and IC placement snap explicitly; loading and
MCP's creation keep exact positions; `setPos` stores what it is given, and `itemChange`'s snap goes. The overlay state
the renderers read is data: the selection rectangle, the wire being routed, the hovered port, hover chips, the IC
preview request and the inline-edit request. `Scene` shrinks to an adapter, and controller tests replace the
view-driven ones.

### 2.11 The new view

`CanvasView`, a `QAbstractScrollArea` (P9.D2), keeps `GraphicsView`'s public surface: zoom in and out, reset, zoom to
fit, the can-zoom queries, `zoomChanged`, `setFastMode`, the viewport transform, `mapToScene`, `mapFromScene`,
`centerOn`, `fitInView`, `ensureVisible`, the scroll bars and `transform().m11()`; the zoom ladder with the anchor under
the mouse, zoom to fit snapped down to a step, middle-button and Space panning, wheel zoom, and scroll ranges as
`resizeScene` computes them. The backend's host widget is the viewport. A scroll area's viewport filter takes the
viewport's paint and resize events for the scroll area's own handlers, while a `QRhiWidget` renders from its own
`paintEvent` and resizes its target from its `resizeEvent`, so `viewportEvent` hands both to a GPU viewport, or it never
renders (a spike showed no frame and no `QRhi` created). A backend switch replaces the viewport, and `setViewport`
deletes the old one, so the view emits `viewportReplaced`: the minimap watches the viewport's resizes and reinstalls
its event filter, or its view rectangle stops following the window. The view keeps `GraphicsView`'s accessible name
and What's This text with their translation context. The interaction reaches the view only through the controller's
requests.

### 2.12 The CPU backend

`WirePainter` draws the wires in order (the highlight halo, then the selection or status pen, from the theme it is
handed) with `master`'s adaptive antialiasing (P9.D13); the click target is stroked at a hit width of 3 (5 for Error)
with the stroker's default caps and joins. `PortPainter` draws an input circle or an output triangle, the status pen, a
brush or the hover highlight, glyphs within the hit square. `LabelPainter` draws in the label font and the theme's
label colour at the label's scene transform, with the label index. `ElementPainter` draws the selection while selected,
then the body or its sprite (P9.D12), in the theme handed; the background and the grid; dirty rectangles from the
listener and from the runner's changed slots (plan 6), which name the ports and wires whose status changed,
scrolling by blit, culling by the view rectangle. Two modes:
interactive, with sprites, and export, vectors only, for PDF through `QPrinter`, images, the clipboard image, MCP
screenshots, the IC preview and the minimap. Qt's device-coordinate cache goes. The CPU backend is the fallback and
the parity reference for plan 1's goldens, and it draws what `master` draws.

**The sprite cache.** A sprite's key is the picture key (plan 3), the theme, the area of element coordinates covered,
the device transform's linear part (zoom, turn and flips drawn into the sprite, so drawing is a copy), the place of
its origin in device pixels (§2.13), the pixel ratio and the render hints, compared exactly. A sprite is rendered one
pixel clear of its area on each side, is null past 2,048 device pixels (the caller draws vectors), and is drawn by
copying at whole pixels. Eviction is least recently used by bytes, 64 MiB, the newest kept alone past the budget. The
transform used is the world transform by the window-to-viewport mapping without the pixel ratio
(`combinedTransform()` includes it: a ratio-2 sprite was drawn at double size). Its test covers every case at zoom
steps 0.8^9 to 1.25^7, quarter phases, selection, ratios 1, 1.25, 1.5 and 2 and both themes, within 4 per channel
(translucent layers are rounded twice); the wireless indicator, an unsmoothed scaled pixmap sampled by the whole pixel
it lands on, is compared with vectors moved by whole pixels.

### 2.13 Sprites at fractional pixel ratios

At pixel ratios above 1 a picture's raster was seen to depend on where its origin falls on the logical pixel grid (by
19 to 31 per colour channel at ratio 2). The branch placed sprites in device pixels and keyed them by their
origin's place within a period of the ratio (its numerator in device pixels: 2 at 2, 3 at 1.5, 5 at 1.25); a ratio
whose reduced denominator exceeds 8 (1.1, 1.3, 1.7, ...) had no usable period and drew bodies as vectors. In
instructions per pan the period keys cost +5.2% and +2.7% (fresh and warm) at 1.25, within 1% at 1.5 and 2, and about
three times (+206%, +215%) at 1.1. The decision (P9.D11) is to place a sprite by its device phase alone and redraw the
glows without blur filters, on a premise never checked: that only pictures with a blur filter change with their place
on the logical grid. Its evidence: with sprites placed the old way, one blurred picture (the constant 1) failed the
body snapshots at 1.25, and a synthetic blurred glow failed at 1.25, 1.5 and 2; nothing was rendered at 1.1. Six
element types' pictures use blur filters (clock, LED, switch, constant 1, buzzer, audio box), and the same filters
render differently before Qt 6.8.

So a probe comes first (P9.13): every picture at positions that share a device-pixel phase but sit differently on the
logical grid, at 1.1, 1.25, 1.5 and 2, with and without the filters, and once with the ratio applied as a plain scale.
The glows are redrawn only if the probe shows the filters are the cause and no rendering route keeps the pictures as
they are; the user sees the redrawn pictures before they land. Otherwise the sprites stay keyed by period, the probe's
answer recorded with the rule it supports. Until it is settled, the cache keys by period, which is correct and costs at
most what the branch measured.

### 2.14 The cut

In buildable steps (P9.D7): the new view and the CPU backend beside the old view behind a developer switch, the tests
and plan 1's goldens run on both; the callers switched (`WorkSpace`, `MainWindow`, the minimap, the scene-to-interface
binder, the selection image, BeWavedDolphin, the IC preview, MCP, the exporter); the view-bound tests migrated; the old
view, the layer items, the proxy editor, the `Scene` adapter and its `QGraphicsScene` deleted; and last, `Element` drops
`QGraphicsObject` and `QObject` (P9.D4): its only base holds its id, and the pointers to elements that callers held
through `QObject` become ids. The element's geometry API keeps the names callers use (`pos`, `setPos`, `moveBy`, `x`,
`y`, `rotation`, `setRotation`, `boundingRect`, `sceneBoundingRect`, `sceneTransform` composed as `QTransform` did,
`mapToScene`, `mapFromScene`; `scene()` becomes `document()`); its body shape is a value (`BodyShape { QRectF body;
QPointF centre; }`, since a custom appearance's image size sets the body and the turning centre), measured through
`BodyMeasure`, which presentation implements by reading image and SVG sizes without rasterizing, and which a headless
document (command-line exports, MCP, tests) gets installed too. A `Port` holds its kind, index, name (derived when
asked), default status, required and visible flags, connections, owner, local placement and its slot in the core's
state; a `Connection` its two ports, its curve, visibility and id, and exists only when both ends are attached.
`status()` keeps its name on both and reads plan 6's core through the port's slot, the arrays the renderers read
(P6.D1); the statuses plan 6's runner publishes into the old ports and wires go with them.

### 2.15 Documentation, measurement and the manual pass

The developer guide describes the layers, the document, the session, the controller, the view and the backends; the
build documents the include check. The CPU backend is measured after the plan against `master` on plan 1's fixtures
(a pan, a zoom, a status change, a load's first frames), since it may be slower than today (P9.25). A pass by hand
covers what this plan changes: palette drag, wire drag, the rubber band, the property editor, themes, high DPI at 1.25
and 1.5, many tabs, the large fixtures, tours and exercises, right-to-left layouts.

## 3. Pitfalls

- **A double-click that Qt delivers as a press** must operate the input under the mouse: the scene records which item
  last held the mouse after each press, and a double-click on another element is a press that operates it.
- **Turning a labelled Text element** left stale entries in Qt's item index until a later change fixed them; the
  review's check stays as a regression test for labels as plain objects.
- **A load re-indexed every item twice**, through two scene resizes. Related and unchecked since: in July 2026 a profile
  of the `master` of the time found the first paint of the 8,000-element circuit still inside Qt's BSP index rebuild at
  full CPU 90 seconds after launch; plan 1's probe answers whether today's `master` still pays it (P9.27).
- **Emulating Qt has limits,** listed in the tests while the old scene lasts: an item takes a press by
  `QGraphicsItem`'s rule; there is no modal panel; Qt's record of the button-down item is not updated while a wire holds
  the mouse; a constructed press for a double-click on another item is not spontaneous; a double-click after the
  mouse-holding wire left the scene.
- **Tests of extreme extents** need a second element at the origin; a NaN position is guarded at the origin too.
- **`QHash::erase` moves what is left,** so the sprite cache evicts the oldest before the newest comes in, and a test
  checks that each sprite returned is the one asked for through 1,000 evictions.
- **A sprite at a fractional ratio** is placed and phased in device pixels; tests run at 1.25 and 1.5, not only at 1
  and 2.
- **Translation contexts:** a class moved or renamed keeps its strings' context on purpose; the translatable-strings
  golden shows a context that moved.
- **The model's own costs on large edits** (a rubber band takes 158 ms, moving the clock about 70 ms, on the
  8,000-element circuit) are not rendering's; the model-cost round takes them.

## 4. Steps and slices

| Row | Step | Slice |
|---|---|---|
| P9.1 | The layer targets, the include check with its test and break checks, and per-layer precompiled headers (§2.1, §2.2) | 1 |
| P9.2 | The untangling of the layers' crossings (§2.3) | 1 |
| P9.3 | Orientation as one transform; scene position changes sent (§2.4) | 2 |
| P9.4 | The circuit's bounds from the registry, every reader moved (§2.4) | 2 |
| P9.5 | Ports as plain objects in a port layer (§2.5) | 3 |
| P9.6 | Wires as plain objects in a wire layer, the curve grid, the selection API (§2.5) | 3 |
| P9.7 | Labels as data drawn by a label layer (§2.5) | 3 |
| P9.8 | Neutral events (§2.6) | 4 |
| P9.9 | Neutral queries (§2.6) | 4 |
| P9.10 | One change funnel, the document listener, the coverage test and every bulk scope's test (§2.7) | 4 |
| P9.11 | The painters out of the layer items; presentation state off ports and wires (§2.12) | 5 |
| P9.12 | The sprite cache; bodies as sprites in a view and vectors in exports; Qt's device cache gone (§2.12) | 5 |
| P9.13 | The sprite probe at 1.1, 1.25, 1.5 and 2, and what it decides (§2.13) | 5 |
| P9.14 | The inventory of `Scene`'s callers by group (§2.9) | 6 |
| P9.15 | The editing session; Qt's click selection and drag in the interaction code, then the item flags off (§2.8) | 6 |
| P9.16 | The document; the commands on it; `Scene`'s callers migrated by group (§2.9) | 7 |
| P9.17 | The controller: the state machines, routing as an overlay, snapping as policy, overlay state as data (§2.10) | 8 |
| P9.18 | Controller tests in place of view-driven ones (§2.10) | 8 |
| P9.19 | The new view and the CPU backend beside the old view, the goldens on both (§2.11, §2.14) | 9 |
| P9.20 | The callers switched to the new view (§2.14) | 9 |
| P9.21 | The view-bound tests migrated (§2.14) | 9 |
| P9.22 | The old view, the layer items, the proxy editor and the scene deleted (§2.14) | 10 |
| P9.23 | `Element` a plain class; pointers to elements become ids (§2.14) | 10 |
| P9.24 | The developer guide and the build documents (§2.15) | 10 |
| P9.25 | The CPU backend measured against `master` (§2.15) | after |
| P9.26 | The manual pass (§2.15) | after |

The checks and open questions of §6 are rows too (P9.27 to P9.29).

## 5. Decisions

Each ends with its basis: measured or tested (a run backs it), read (the code, with nothing run), or a ruling (the
user's choice between workable options).

- **P9.D1. Full layering, enforced by build targets:** six layers, each allowed only earlier layers and its own Qt
  modules, each with an include check and a precompiled header of its own. Basis: read on `master` (§1.3: a split
  alone would reject nothing) and on the branch (layering described and not built was not kept); the user's ruling of
  2026-10-03/04.
- **P9.D2. A new view on `QAbstractScrollArea`,** with the old view's surface, its viewport the rendering backend's
  widget. Basis: measured in the spikes (a `QRhiWidget` as a scroll area's viewport, beside both scroll bars and a
  label, cost 1.6 to 2.8 ms per frame of panning; the viewport filter must hand paint and resize to a GPU viewport);
  the user's ruling of 2026-10-03/04.
- **P9.D3. What a view toggle hides is session state:** a visibility predicate that renderers and hit tests read.
  Basis: read on `master` (the visibility manager calls `setVisible` on items); the user's ruling of 2026-10-03/04.
- **P9.D4. The element is a plain class** once the old view goes, its only base holding its id. Basis: read on
  `master` (click selection, multi-element drag, the selection list and change notifications come from Qt's item
  handlers and flags, so each is implemented in the editor before the flags go off, pinned by plan 1's pointer and
  wire-geometry goldens); the user's ruling of 2026-10-03/04.
- **P9.D5. The wire being routed is session state;** the model holds only attached wires, and completing a wire is one
  command. Basis: read on `master` (routing adds a half-connected wire that the model, the simulation and autosave
  see); the user's ruling of 2026-10-03/04.
- **P9.D6. Snapping is editor and command policy; a position stores exactly.** Basis: read on `master` (the item snaps
  in its position-change hook only while in a scene, so a loaded position is kept and every other path snaps through
  `setPos`; the explicit rules give the same positions); the user's ruling of 2026-10-03/04.
- **P9.D7. The old and the new view side by side, then the old deleted.** Basis: the plans' rules (§2: a replacement
  runs beside what it replaces until proven); the user's ruling of 2026-10-03/04.
- **P9.D8. Ports, wires and labels as plain objects, from the intent of the prototype on `perf/load-hotspots`,** drawn
  and hit-tested by layer items over spatial grids, with its element geometry. Basis: tested on the branch, where the
  review found three defects and a missing test, each fixed with a test (§2.5); the user's ruling of 2026-10-04.
- **P9.D9. `Scene` is split by its callers, not renamed,** each caller moved to the document or the session by group,
  checked by the compiler, from an inventory taken where it is used. Basis: measured on `master` (§1.1's counts); the
  user's ruling of 2026-10-04.
- **P9.D10. Ports and labels above every element,** in both backends, matching the hit test. Basis: read on `master`
  (an element's ports and label are child items drawn just after its body, so a later element hides them; the change
  shows only where elements overlap; a per-element order would need depth and an alpha cut-off on the GPU); the user's
  ruling of 2026-10-04.
- **P9.D11. Sprites at every pixel ratio, the glows redrawn without blur filters,** **unverified:** a probe first
  (§2.13), the glows redrawn only if it shows the filters are the cause, and the user sees them before they land.
  Basis: the user's ruling of 2026-10-20, on an unmeasured premise; the user's "ok, i'll wait" of 2026-10-21 for the
  pictures.
- **P9.D12. The renderers draw the selection;** a picture's key leaves it out, so one drawing serves a selected and an
  unselected element. Basis: tested on the branch (every body state by orientation by theme: one key per drawing and one
  drawing per key); the user's ruling of 2026-10-03/04.
- **P9.D13. The CPU backend keeps `master`'s adaptive wire antialiasing;** the GPU's 4× multisampling is plan 10's.
  Basis: read on `master` (§1.4); the user's acceptance of 2026-10-03/04.

## 6. Claims to check and open questions

- **Whether today's `master` still pays Qt's BSP index rebuild after a load** (P9.27), as the July 2026 profile found:
  plan 1's probe answers it, and the plain-object layers and the deleted scene are measured against it (P9.25).
- **What the user sees new** (P9.28): ports and labels above every element where elements overlap, and the redrawn
  glows if the probe leads to them, shown to the user before their slices are finished.
- **The model's notification path for plan 10** (P9.29): whether the listener's events (element, wire, port, label,
  theme, bulk scopes) are enough for a GPU mirror with stable slots and dirty ranges, checked against plan 10's mirror
  before this plan's listener is final.

## 7. Risks

| Risk | Mitigation |
|---|---|
| Moving `Scene`'s callers is larger than it looks (1,373 occurrences in 216 files) | The inventory of callers; migration by group, compiler-checked |
| Behaviour drift in the reimplemented view, hit testing, stacking, snapping or editing | Plan 1's goldens run on the old and the new implementation |
| Behaviour Qt carries for items (click selection, multi-element drag, the selection list, change notifications) is lost when a flag or base class goes | Each implemented in the editor before its flag goes off; pinned by plan 1's pointer and wire-geometry goldens |
| Tests construct and drive what goes: Qt's selection API (about 315 calls) | Shared helpers in `Tests/Common` move first; every deletion compiler-checked, never a silent no-op |
| A pixel-changing change that does not notify | One change funnel; a coverage test over every change |
| A moved or renamed class takes its strings into a new translation context | Contexts kept on purpose; the translatable-strings golden |
| Load and paint stay as slow as `master`'s until the model-cost round | The round, right after this plan, measures load and the CPU backend's painting on this plan's model and cuts what the measurement shows; plan 10's GPU removes the paint cost |
| The CPU backend is slower than today | Measured after the plan (P9.25) |
| The sprite rule rests on an unchecked premise | The probe first; period keys until it settles (§2.13) |

## 8. What other plans take from plan 9

- **The model-cost round,** which follows this plan: the new model (the document with its spatial indices and
  stacking sequence, the change funnel and the listener, the editing session's selection), on which it measures and
  optimises selection, move and load.
- **Plan 10:** the render target and the `RenderBackend` interface; the view whose viewport is the backend's widget, and
  its `viewportReplaced`; the overlay state as data; the document listener; the sprite key and cache; the CPU backend as
  the fallback and the parity reference.

## 9. The branch's work, as evidence

Each step was built on `gpu-canvas`, where its commits show the change and its tests; plan 9 writes its own on
`master`. A hash is the branch's, kept read-only.

| Step | Commits on the branch |
|---|---|
| Orientation as one transform | `3af51137b`, `d1a358392` |
| The circuit's bounds and their readers | `4b7237b45`, `1411e9e4d`, `43336e8a9`, `052fb4f3c`, `fc780d437` |
| Ports as plain objects | `f22fa9ec0`, `990fb896f`, `4d087faa0` |
| Wires as plain objects | `d446e7ebe`, `5edae4e70`, `2538376ad`, `5d3e8d3b8`, `fea5c4d96` |
| Labels drawn by a layer | `76135c99b` |
| Neutral events and queries | `880491f9b`, `8ba09a42a` |
| The untangling | `1002f3967`, `d678e9e64`, `9f8c718b6`, `753e2704c`, `383ebbfdb`, `cca969a6f`, `81be73f47`, `7cbdb54c4`, `65da75ac6`, `9e5802e39`, `429380d6d`, `5004d80be`, `233c34849`, `212eb1be0` |
| The document listener and the change funnel | `1c294058a`, `254aacf4b` |
| The layer targets and their include check | `bc91de8bb` |
| The painters | `05f8915c8`, `dac6ba743`, `7fb8a7d90`, `25029647d`, `360445c7f` |
| The sprite cache and sprites in a view | `e0cc6791e`, `8e7c5e1a6` |
| The fixes to the branch's own layers | `e56830653`, `2a1ca7a7a`, `c61c03a7b`, `05b2086a3`, `860233118` |
| A double-click delivered as a press, folded into the input operations | `099425e53` |
| The pan probe at fractional ratios | `daf94c677`, `25ddb499d` |

The branch never built the session, the document, the controller, the new view or the cut.
