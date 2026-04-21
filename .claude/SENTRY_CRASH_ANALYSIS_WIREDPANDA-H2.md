# WIREDPANDA-H2 — `Simulation::update` Access Violation (deep analysis)

- **Sentry issue**: [WIREDPANDA-H2](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-H2)
- **Event ID**: `2a29144937e34acdd31cc437e7224603`
- **Seen**: 2026-04-21T00:30:00Z (1 event, 1 user, Brazil)
- **Platform**: Windows 10.0.19045, Qt 6.9.3
- **Release**: `wiredpanda@4.7.0`
- **Mechanism**: `minidump` / `handled: no` (fatal, unhandled)

## Crash signature

```
EXCEPTION_ACCESS_VIOLATION_READ / 0xffffffffffffffff

at Simulation::update (Simulation.cpp:87)
    element->updateLogic();
at QTimer::timeout
at QCoreApplication::notifyInternal2
at Application::notify (Application.cpp:57)
    done = QApplication::notify(receiver, event);
```

Faulting address `0xffffffffffffffff` is the value being **read**, not the pointer
itself — when the compiler emits a virtual call it fetches the vtable slot from
`*element` and jumps through it. A freed `GraphicElement` whose first 8 bytes have
been overwritten with `0xFF`-bytes (the canonical Windows heap scribble on some
heap-poisoning paths) produces exactly this fault. `element` itself is non-null, so
the `if (element)` guard on `Simulation.cpp:86` passes — the classic shape of a
**dangling pointer** still referenced by `m_sortedElements`.

## This is one event in a 23+ event cluster

Same root-cause pattern across multiple signatures (23+ events total, top
unresolved crash class in the project):

| Issue | Site | Events | Release / OS |
| --- | --- | --- | --- |
| **H2** | `Simulation::update` → `updateLogic` | 1 | 4.7.0 Win Qt 6.9.3 |
| **GX** | `Simulation::update` → `Simulation::updatePort` | 1 | 4.3.0 Linux |
| **EX** | `Simulation::updatePort` → `QNEPort::graphicElement` | **9** | 4.3.0 Linux |
| **FT** | `Simulation::updatePort` → `LogicElement::outputValue` → `QArrayData::data` | 2 | 4.3.0 Linux |
| **FX / G0** | `Simulation::updatePort` → `GraphicElement::elementType` | 3 | 4.3.0 Linux |
| **EV / H1** | `Scene::mouseReleaseEvent` → `QGraphicsItem::pos` (offset `0x57` / `0x68`) | 6 | 4.3.0 |
| **EW** | paint path → `QGraphicsSceneFindItemBspTreeVisitor::visit` | 2 | 4.3.0 |
| **FS** | paint path → `QGraphicsItemPrivate::effectiveBoundingRect` | 2 | 4.3.0 |
| **F0** | `DeleteItemsCommand::redo` (producer side) | 5 | 4.3.0 |
| **GP** | `~QGraphicsItem` during `QUndoStack::push` (producer side) | 1 | 4.3.0 |

**Not a Qt 6.9.3 regression** — same pattern on 4.3.0/Linux and 4.7.0/Windows. This
is a logic issue in wiRedPanda code.

## Breadcrumb reconstruction of WIREDPANDA-H2

User had three tabs open (`teste.panda`, `labPanda1.panda`, one other). In the last
~90 seconds there were ~60 `[simulation] stopped/started` transitions — every
`AddItemsCommand` / `DeleteItemsCommand` / tab switch / drop event flips the timer:

```
00:29:24.666  Command: Atualizar 1 elementos                ← UpdateCommand (NO SimulationBlocker)
00:29:26.314  Command: Atualizar 1 elementos
00:29:28.408  Simulation stopped
00:29:31.717  Simulation started
00:29:31.708  Save: teste.panda
00:29:33.303  Tab changed to index 0                        ← switch to labPanda1.panda
...
00:29:45.818  Command: Adicionar 1 elementos (drop)
00:29:46.897  Simulation stopped
00:29:53.856  Simulation started
00:29:58.629  Simulation stopped
00:30:00.448  Simulation started
00:30:00.449  Simulation stopped
00:30:00.450  Save: labPanda1.panda
00:30:00.458  Simulation started
→ CRASH on the very next 1 ms timer tick
```

## Root cause chain

### Bug 1 — `IC::resetInternalState()` leaves `m_sortedInternalElements` dangling

`App/Element/IC.cpp:266`:

```cpp
void IC::resetInternalState()
{
    m_internalInputs.clear();
    m_internalOutputs.clear();
    setInputSize(0);
    setOutputSize(0);
    qDeleteAll(m_internalConnections);
    m_internalConnections.clear();
    qDeleteAll(m_internalElements);       // ← frees the elements
    m_internalElements.clear();
    // m_sortedInternalElements NOT cleared ← still points at freed memory
    // m_boundaryInputElements      NOT cleared
    // m_internalHasFeedback        NOT reset
}
```

Header confirms the sorted vector is a separate member and is never wiped by this
function (`App/Element/IC.h:144`).

### Bug 2 — `IC::updateLogic()` has no null/validity guard

`App/Element/IC.cpp:710`:

```cpp
for (auto *element : std::as_const(m_sortedInternalElements)) {
    element->updateLogic();   // ← no if(element); bad ptr → vtable read of 0xFFFF…FFFF
}
```

This is the site where the access violation originates in H2-style crashes. It is
reached through the outer `Simulation.cpp:87` frame (`element->updateLogic()` where
`element` is the enclosing IC), so the outermost stack frame shows the outer call
site but the *actual* bad dereference can be one level deeper on the nested IC.

### Bug 3 — `IC::loadFile()` **PATH 2** resets before it can fail

`App/Element/IC.cpp:278` — two paths:

```cpp
void IC::loadFile(...) {
    // ...validation, can throw "not found"...

    // PATH 1: bytes cached by ICRegistry → deserializeAndLoad()
    //         (parse-first, then resetInternalState(); safe)
    if (auto *reg = ...; !cached.isEmpty()) {
        deserializeAndLoad(cached, ...);
        return;
    }

    // PATH 2: direct file load (first load OR after cache invalidation)
    resetInternalState();          // ← FREE internals first
    m_file = ...;
    loadFileDirectly(fileInfo);    // ← can throw: circular ref, open fail,
                                   //   deserialize fail, migrateFile fail
}
```

`loadFileDirectly()` has four throw sites (cycle detection, open failure,
`migrateFile` write failures, corrupt stream). If any trips, we return up the stack
with `m_internalElements` empty **and** `m_sortedInternalElements` still holding
freed pointers. Contrast with `deserializeAndLoad()` (`IC.cpp:442`) which parses
first and only resets once parsing is known to succeed — it has the correct shape.

### Bug 4 — `Simulation::restart()` does NOT stop the timer

`App/Simulation/Simulation.cpp:153`:

```cpp
void Simulation::restart()
{
    m_initialized = false;    // flag only
    // m_timer.stop() is NOT called
}
```

The misleading docstring says "Stops and immediately restarts the simulation". It
doesn't. The guard at `Simulation.cpp:55` (`if (!m_initialized && !initialize()) return;`)
saves us on the *next* tick, but only because `m_initialized` is `false` at that
moment. If anything else sets `m_initialized = true` between the two ticks (or if
the next tick calls a virtual that faults before `initialize()` runs — e.g. inside
a nested IC whose sorted vector is dangling), the crash fires.

### Bug 5 — `Scene::setCircuitUpdateRequired()` silently discards failures

`App/Scene/Scene.cpp:241`:

```cpp
void Scene::setCircuitUpdateRequired()
{
    m_visibilityManager.reapply();
    update();
    m_simulation.initialize();    // return value ignored
    m_autosaveRequired = true;
}
```

`initialize()` returns `false` with `m_sortedElements` cleared but `m_initialized`
**unchanged** when the scene has zero or one items (border rect). The existing
flag therefore lies: callers think "topology is fresh" when in fact we're in the
"partial init" state.

### Bug 6 — Commands that mutate topology without `SimulationBlocker`

Verified the following commands call `setCircuitUpdateRequired()` at the end but
do **not** open a `SimulationBlocker` scope around their redo/undo bodies:

- `UpdateCommand::redo/undo` (`App/Scene/Commands.cpp:454,447`)
- `MorphCommand::redo/undo` (`App/Scene/Commands.cpp:669,630`) — frees elements
  directly via `delete oldElm` at line 738
- `ChangePortSizeCommand::redo/undo` (`App/Scene/Commands.cpp:858,904`) — drains
  and deletes connections via `drainPortConnections`
- `SplitCommand::redo/undo` (`App/Scene/Commands.cpp:545,592`) — deletes the
  inserted node and second-segment wire on undo

With `Application::notify()` catching every `std::exception` and spinning a
`QMessageBox::critical` nested event loop (`App/Core/Application.cpp:56`), a throw
anywhere inside these commands leaves the scene torn **and** lets the 1 ms
simulation QTimer fire inside the modal loop — the exact re-entrancy window the
commands are supposed to prevent. Commands that already use `SimulationBlocker`:
`AddItemsCommand`, `DeleteItemsCommand`, `UpdateBlobCommand`,
`ClipboardManager::paste`, `WorkSpace::load`, `WorkSpace::loadFromBlob`,
`WorkSpace::updateEmbeddedICFromInlineTab`, Arduino/SystemVerilog export,
`MainWindow::removeICFile`, `BeWavedDolphin`.

### Bug 7 — `ICRegistry::onFileChanged()` reload has no blocker

`App/Element/ICRegistry.cpp:66`:

```cpp
for (auto *elm : targets) {
    static_cast<IC *>(elm)->loadFile(filePath);   // PATH 2 after invalidate()
}
if (!targets.isEmpty()) {
    m_scene->simulation()->restart();   // see Bug 4 — doesn't stop timer
}
```

`invalidate(filePath)` at line 71 wipes the ICRegistry cache, forcing the
subsequent `loadFile()` onto the buggy **PATH 2**. Re-load errors surface directly
in the signal handler via `PANDACEPTION` → `Application::notify()` QMessageBox →
timer fires mid-modal loop → Bug 2 lands.

### Bug 8 — `iterativeSettle()` has no null check either

`App/Simulation/Simulation.cpp:453`:

```cpp
for (auto *element : std::as_const(elements)) {
    element->clearOutputChanged();   // no null check
    element->updateLogic();           // no null check
}
```

Defensive hole on the feedback-circuit path; not the trigger for H2 but the
matching latent footgun.

### Tab-close race (candidate for future paint-path crashes)

`MainWindow::closeTab` (`App/UI/MainWindow.cpp:1151`) uses `deleteLater()`, and
`Scene::m_simulation` (`App/Scene/Scene.h:346`) is a by-value member — the
QTimer inside it keeps firing until the event-loop gets around to destroying the
`WorkSpace`. Not confirmed as the trigger for H2 specifically, but is the
cleanest explanation for the paint-path variants (EV, EW, FS, H1, GR).

## Why the "save" trigger matters

The last breadcrumb before the crash is `Save: labPanda1.panda` (`[file]`).
`WorkSpace::save()` has **no** `SimulationBlocker`. The two stop/start pairs
surrounding it in the trace come from neighbouring `SimulationBlocker` scopes in
adjacent code (likely commands chained by the save flow / `undoStack->setClean`
side-effects), not from the save itself. If the file being saved was referenced
as an IC by another tab's `ICRegistry`, `QSaveFile::commit()` triggers
`QFileSystemWatcher` → `onFileChanged` → Bug 7. If not, the crash was already
armed earlier by any of Bugs 1, 3, 6, or 7 and the first post-save timer tick
was simply the unlucky one that dereferenced the stale entry.

## Proposed fixes (ordered by payoff)

### Fix A — `IC::resetInternalState()` clears everything it owns

```cpp
void IC::resetInternalState()
{
    m_internalInputs.clear();
    m_internalOutputs.clear();
    setInputSize(0);
    setOutputSize(0);
    qDeleteAll(m_internalConnections);
    m_internalConnections.clear();
    m_sortedInternalElements.clear();     // NEW — drop stale refs before free
    m_boundaryInputElements.clear();      // NEW
    m_internalHasFeedback = false;        // NEW
    qDeleteAll(m_internalElements);
    m_internalElements.clear();
}
```

Clearing the sorted vector **before** `qDeleteAll` is what makes this fix
exception-safe: even if something later throws, there is no stale pointer to
dereference.

### Fix B — `IC::updateLogic()` null-guards the inner loop

```cpp
for (auto *element : std::as_const(m_sortedInternalElements)) {
    if (element) element->updateLogic();
}
```

And the mirror fix in `Simulation::iterativeSettle()` (Bug 8).

### Fix C — `IC::loadFile()` PATH 2: parse-before-reset

Refactor PATH 2 to match PATH 1's shape — parse `loadFileDirectly` into a local
item list first, and only call `resetInternalState()` + `processLoadedItems()`
once parsing is known to succeed. Hoist the parse step out of
`loadFileDirectly()` into a pure function. Eliminates the window in Bug 3 entirely.

### Fix D — `ICRegistry::onFileChanged()` wraps reload + reinitializes properly

```cpp
void ICRegistry::onFileChanged(const QString &filePath)
{
    invalidate(filePath);
    if (!m_fileWatcher.files().contains(filePath) && QFileInfo::exists(filePath))
        m_fileWatcher.addPath(filePath);

    const auto targets = findICsByFile(filePath);
    if (targets.isEmpty()) { emit definitionChanged(filePath); return; }

    SimulationBlocker blocker(m_scene->simulation());
    for (auto *elm : targets) static_cast<IC *>(elm)->loadFile(filePath);
    m_scene->setCircuitUpdateRequired();   // full initialize(), not restart()
    emit definitionChanged(filePath);
}
```

### Fix E — `Simulation::restart()` matches its docstring

Option 1 (least invasive): also clear `m_sortedElements`/`m_connections` so
`m_initialized == false` is always paired with empty hot-path vectors:

```cpp
void Simulation::restart()
{
    m_initialized = false;
    m_sortedElements.clear();
    m_connections.clear();
}
```

Option 2 (safer, matches intent): behave like the SimulationBlocker pattern:
`m_timer.stop(); m_initialized = false;` and document that callers must
re-`start()` — touches `MainWindow::on_actionRestart_triggered` and
`ICRegistry::reloadIC`.

### Fix F — wrap the remaining mutating commands in SimulationBlocker

`UpdateCommand`, `MorphCommand`, `ChangePortSizeCommand`, `SplitWireCommand`,
`SpliceNodeCommand` — add `SimulationBlocker blocker(m_scene->simulation());`
at the top of each `redo()` and `undo()`. Brings them in line with
`DeleteItemsCommand` / `AddItemsCommand` and closes the
throw-inside-redo → nested event loop → stale-pointer-tick window.

### Fix G — `Scene::setCircuitUpdateRequired()` handles initialize() failure

```cpp
if (!m_simulation.initialize()) {
    // Empty / single-item scene: drop any flag that might indicate "initialized"
    m_simulation.restart();
}
```

## Test plan for the fixes

- **Unit**: add a test that constructs an IC, populates its sorted vectors,
  calls `resetInternalState()`, and asserts `m_sortedInternalElements` is empty.
  Repeat after a forced `loadFile()` throw (point at a non-existent file).
- **Unit**: add a test that runs `Simulation::update()` after
  `resetInternalState()` without a subsequent `initializeSimulation()` and
  asserts no crash.
- **Integration**: two-tab scenario — Tab A opens `a.panda` which has `b.panda`
  as an IC; Tab B has `b.panda` directly. Save `b.panda` from Tab B while
  Tab A's simulation is running. Should not crash.
- **Stress**: automated rapid add/delete/undo cycle (the real-world trigger in
  H2's breadcrumbs) with the simulation running. Run under ASAN/TSan.

## References

- Sentry: https://rodrigo-torres.sentry.io/issues/WIREDPANDA-H2
- Primary code sites:
  - `App/Element/IC.cpp:266` `resetInternalState()`
  - `App/Element/IC.cpp:710` `IC::updateLogic`
  - `App/Element/IC.cpp:278,327,442` `loadFile / loadFileDirectly / deserializeAndLoad`
  - `App/Simulation/Simulation.cpp:87,153,453` `update / restart / iterativeSettle`
  - `App/Element/ICRegistry.cpp:66` `onFileChanged`
  - `App/Scene/Scene.cpp:241` `setCircuitUpdateRequired`
  - `App/Core/Application.cpp:49` `Application::notify` (the exception-catching
    event loop driver)
- Commands lacking `SimulationBlocker`: `App/Scene/Commands.cpp:447-681,858-918`
- Same-class Sentry issues (cluster): EX, FT, FX, G0, GX, EV, EW, FS, H1, F0, GP
