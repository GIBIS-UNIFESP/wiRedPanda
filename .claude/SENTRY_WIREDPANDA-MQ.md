# WIREDPANDA-MQ — use-after-free in InlineLabelEditor

- **Sentry**: https://rodrigo-torres.sentry.io/issues/WIREDPANDA-MQ
- **Title**: `std::_Atomic_integral<T>::fetch_add` (MSVC STL frame name)
- **Error**: `EXCEPTION_ACCESS_VIOLATION_WRITE / 0xd00000002`, fatal, minidump
- **Release**: wiredpanda@5.2.2, Qt 6.9.3, Windows 10.0.26200 x86_64
- **Seen**: 1 event, 1 user, 2026-10-04T03:32:45Z
- **Other issues with `InlineLabelEditor` in the stack (90d)**: none

## Crashing stack (main thread, first-party frames)

```
QGraphicsView::mouseDoubleClickEvent
Scene::mouseDoubleClickEvent
QGraphicsScenePrivate::mousePressEventHandler
GraphicElement::inlineEditRequested            (emitted from GraphicElement::mouseDoubleClickEvent)
Scene::addItem lambda -> InlineLabelEditor::start
InlineLabelEditor::commit                      (start(): `if (m_target) commit();`)
GraphicElement::label                          (returns m_label by value)
QString copy -> QArrayData::ref -> fetch_add   (WRITE to freed memory -> AV)
```

The `QBitArray`/`QByteArray` frames are MSVC identical-COMDAT folding of the
`QArrayDataPointer` copy constructor; the real call is the `QString` copy in
`GraphicElement::label()`.

## Root cause

`InlineLabelEditor` (App/Scene/InlineLabelEditor.{h,cpp}) stores the element under edit
as a raw `GraphicElement *m_target` that stays set for as long as the on-canvas
`QLineEdit` is open. Only `commit()`/`cancel()` close the editor, and they run only from
the line edit's `returnPressed`/`editingFinished` signals, Escape, or the next `start()`.
Nothing ties the editor to the element's lifetime.

Elements are freed outright: `CommandUtils::deleteItems()` does
`scene->removeItem(item); delete item;` (Delete, Cut, undo of Add/Paste/Duplicate,
Morph, ...). If the target is freed while the editor is open, `m_target` dangles, and
the next `commit()` reads freed memory: `m_target->label()`, then `save()`/`setLabel()`
if the text differs. That can happen from focus-out or from the next `start()`, as in
this crash.

## Breadcrumb timeline (last 100)

- `03:28:52.214` Drop event -> `Add 1 elements`
- `03:28:52.532` Drag started: 2 (press on the newly dropped, already-selected element)
- `03:28:53.719` Wire cancelled (press on a non-element: empty canvas, the editor box, or right button)
- `03:28:54.766` **Delete -> `Delete 1 elements`** (the only deletion in the window)
- `03:28:56` .. `03:32:31` 13x Play toggled, no canvas presses
- `03:32:35` .. `03:32:41` single clicks on elements (pairs "Drag started: 1" then "2")
- `03:32:45.240` press + release, then the double-click -> crash

Reading notes:
- Widgets deliver only `MouseButtonDblClick` for a double-click's second press (Qt
  suppresses the press, QTBUG-25831, `qwidgetwindow.cpp`), so a double-click shows up as
  one "Drag started"/"Drag ended" pair.
- "Drag started: 2" = press on an element that's already selected (the clicked element
  is appended to `selectedElements()` again in `SceneInteraction::mousePress`).

So the editor was open on an element that was deleted at 03:28:54.766, and no
`commit()`/`cancel()` ran between then and the crash 4 minutes later.

## Paths checked empirically (debug build, offscreen QPA, shown + active WorkSpace)

| Path while editing | Result |
| --- | --- |
| Double-click a labelled element | proxy + QLineEdit get scene focus (focus widget stays the view) |
| Right press on the editor box | `Scene::itemAt` skips the proxy -> canvas/element menu `exec()`; popup focus-out **commits** |
| `QContextMenuEvent` to the editor (Menu key / WM_CONTEXTMENU) | QLineEdit's own menu is embedded in the scene as a child proxy; editor loses focus, `activePopupWidget()` is null -> **commits** |
| Delete key with the line edit focused | swallowed by `QLineEdit` ShortcutOverride (as are Undo/Redo/Cut/Paste) |

None of these leaves the editor open with no focus, so the exact click sequence that
let the deletion slip past `commit()` is **not reproduced**. Other things ruled out by
reading the code: Play toggle (doesn't touch focus), F2 rename (moves focus -> commits),
IC preview (ToolTip + `WA_ShowWithoutActivating`), sticky focus (not set; no item
except the editor proxy is focusable), caught exceptions in `commit()` (no other Sentry
events from this user).

The fix doesn't depend on which path it was: the editor must not outlive its target.

## Related bug: Escape didn't cancel an inline edit (fixed, second commit)

`QLineEdit::event` defers ShortcutOverride to
`QWidgetLineControl::processShortcutOverrideEvent`, which doesn't claim `Key_Escape`.
So in the real app Esc fired `MainWindow::actionClearSelection` (shortcut "Esc") instead
of `InlineLabelLineEdit::keyPressEvent`. The edit stayed open, and the next click
committed the text the user meant to throw away. The existing unit test missed it
because it sends the key straight to the QLineEdit, with no window shortcuts present.

- Reproduced with new `TestMainWindowGui::testInlineLabelEditEscapeCancels`: real
  MainWindow, real double-click (click + DblClick + release), Escape sent to
  `QApplication::focusWidget()` (the view, as the platform does). Before the fix it failed
  "Escape must close the inline editor".
- Routing checked in Qt source: QGraphicsView forwards ShortcutOverride to the scene, the
  scene to the focus item, and QGraphicsProxyWidget to the embedded focus widget.
- Fix: `InlineLabelLineEdit::event()` accepts ShortcutOverride for `Key_Escape`, so the
  KeyPress reaches `keyPressEvent()` -> `cancel()`. The selection is kept (Esc cancels
  the edit only).
- While the test was still failing, deleting the visible MainWindow with the edit open
  hit a Qt assert: a `MainWindow` slot ran after `~MainWindow`, via ~QWidget close ->
  focus-out -> `commit()` -> `cleanChanged` -> `WorkspaceManager::titleChanged`. The
  interactive app never deletes its MainWindow (no `WA_DeleteOnClose`), so users can't
  hit it. It's fixed anyway (commit `7bcc998c`, see below), because otherwise any failing
  inline-edit test aborted the whole TestMainWindowGui binary.

## Other keys typed into the inline editor (fixed)

Key presses go view -> `GraphicsView::keyPressEvent` -> `Scene::keyPressEvent` ->
`QGraphicsScene::keyPressEvent` -> focus item (the editor). The app's own handlers act on
every key before the editor gets it. A temporary probe in the real MainWindow, with the
editor open on a selected LED and keys sent to the focus widget, showed:

- **Left arrow** moves the selected element (`Scene::nudgeSelection` consumes it), and
  the text cursor doesn't move.
- **Typing a letter** that's an input's trigger key ('A' on an InputSwitch) toggles that
  input, and the letter is also typed.
- **Holding Space** puts the view into pan mode (`GraphicsView`'s `m_space`,
  ClosedHandCursor), and the space is also typed. While the key is held, plain mouse
  moves scroll the canvas (`mouseMoveEvent` pans on `m_space` with no button needed).

Rule: while a scene item has keyboard focus, keys belong to it. Press-side canvas actions
stand down, and release-side actions only end what a press actually started.

- `7bcc998c` fix(ui): `~MainWindow` calls `hide()` first, so ~QWidget doesn't close a
  visible window after the derived destructor has run. Test
  `testDestroyWindowDuringInlineLabelEdit` hit the QFATAL before the fix.
- `97af94f3` fix(scene): `Scene::keyPressEvent` hands keys straight to `focusItem()` when
  there is one (no nudge, no triggers). `keyReleaseEvent` keeps running trigger releases
  on purpose: a trigger held before the editor opened must still release, and
  `InputButton::setOff()` is idempotent (`setOn(false)`). Tests:
  `testInlineLabelEditArrowKeysStayInEditor` (also checks nudging works again after the
  editor closes), `testInlineLabelEditTypingDoesNotFireTriggers`, and
  `testTriggerHeldIntoInlineLabelEditStillReleases` (guards the release-side decision; it
  passes before and after).
- `fe16a377` fix(view): Space only enters pan mode when `scene()->focusItem()` is null;
  release only ends pan mode if `m_space` was set. Test
  `testInlineLabelEditSpaceDoesNotPan` (scroll position must not change while Space is
  held and the mouse moves; before the fix it scrolled from (-197,-123) to (-256,-256)).

Each new test was run against the old code first and failed for the expected reason.
Final tree: debug 216/216, ASan 216/216.

Left as is on purpose:
- Up/Down/PageUp/PageDown, which QLineEdit ignores, still propagate to
  QAbstractScrollArea and scroll the view. That's standard Qt propagation for a line edit
  inside a scroll area, and it doesn't change the document.
- Modifier shortcuts (Ctrl+D, Ctrl+R, ...) still fire while editing, as window shortcuts
  normally do in Qt apps. `QLineEdit` already claims the text-editing ones (Ctrl+A/C/V/X/Z).

## Fix (branch `fix/inline-label-editor-dangling-target`)

`App/Scene/InlineLabelEditor.{h,cpp}`:
- `m_target` is a `QPointer<GraphicElement>`, so it can't dangle.
- `start()` connects the target's `QObject::destroyed` to `cancel()` (context: the line
  edit), so the editor closes with its element. Without this the box would float over
  empty canvas.
- `close()` and `~InlineLabelEditor()` disconnect it. The destructor disconnect is
  required: `Scene` has no destructor of its own, so `m_inlineLabelEditor` is destroyed
  before `~QGraphicsScene` deletes the items, and a still-open edit's `destroyed()` would
  otherwise call `cancel()` on a dead editor.

## Verification

- New `TestScene::testInlineLabelEditorClosesWhenTargetDeleted`: start(B) -> delete B
  through `Scene::deleteAction()` -> editor must be gone -> `commit()` + `start(A)` must
  not touch B; only the Delete is on the undo stack.
- Before the fix (ASan): fails at "editor must close"; with that assert disabled, ASan
  reports `heap-use-after-free` READ in `QString(const QString&)` <- `GraphicElement::label()`
  <- `InlineLabelEditor::commit()`, freed by `CommandUtils::deleteItems()` <-
  `DeleteItemsCommand::redo()`. Same frames as the Sentry stack.
- After the fix (ASan): TestScene 103/103, TestGraphicElement 23/23, no ASan reports.
- Negative check: removing only the destructor disconnect makes ASan flag a
  heap-use-after-free in the existing
  `testInlineLabelEditorStartWhileEditingCommitsPrevious` (editor destroyed before the
  WorkSpace with an edit open), so that path is already guarded by the suite.
