# Quick vs. Widgets UI/Feature Parity Audit

**Date**: 2026-09-03
**Method**: Multi-agent workflow audit (`quick-widgets-parity-audit`, run `wf_5d547713-e59`). One
"find" agent per UI area read the Widgets reference implementation (`/home/torres/wiredpanda`,
branch `master`) against the Qt Quick rewrite (this worktree, branch `qtquick-rewrite-v2`) and
proposed candidate parity gaps with file:line citations in both trees. An independent "verify"
agent per area then re-read every citation directly, searched broadly across the whole Quick tree
for evidence the feature actually exists elsewhere before accepting an absence claim, and marked
each finding CONFIRMED or REFUTED. All findings below survived verification (0 refuted).

**Result**: 49 confirmed parity gaps across 9 UI areas: element property editor, element
right-click context menu, Clock/Length dialogs, element palette, IC embed/extract UI, file
dialogs & tab/workspace management, update checker, BeWavedDolphin waveform viewer, minimap.

This is an audit only -- no fixes are implemented yet. See the fix plan (tracked separately) for
sequencing.

---

## High severity (7)

### File-menu keyboard shortcuts (Ctrl+N/Ctrl+O/Ctrl+S/Ctrl+Shift+S/Ctrl+F5) are not wired up
*Area: File dialogs & tab/workspace management*
- **Widgets**: `App/UI/MainWindowUI.cpp` `retranslateUi()` sets `actionOpen`/`actionSave`/
  `actionSaveAs`/`actionNew`/`actionReloadFile` shortcuts to `Ctrl+O`/`Ctrl+S`/`Ctrl+Shift+S`/
  `Ctrl+N`/`Ctrl+F5`.
- **Quick**: no `Main.qml` File-menu `MenuItem` declares a `shortcut:` property; repo-wide grep
  for `shortcut:` across `App/QuickShell/` returns zero hits outside the unrelated
  `TourOverlay.qml` Escape shortcut. `CanvasItem::keyPressEvent()`'s own comment admits "no chrome
  menu/QAction shortcut layer exists yet." The in-app "Shortcuts and Tips" dialog
  (`QuickAppController::shortcutsHelpHtml()`) actively documents Ctrl+N/O/S as working, which they
  don't.
- **Impact**: none of New/Open/Save/Save As/Reload File respond to their muscle-memory shortcuts.

### No WASM/browser-specific file Open/Save flow
*Area: File dialogs & tab/workspace management*
- **Widgets**: `App/UI/WorkspaceManager.cpp` has `#ifdef Q_OS_WASM` branches in `openFile()`/
  `saveFile()`/`saveFileAs()` using `QFileDialog::getOpenFileContent()`/`saveFileContent()` to
  route through browser upload/download APIs instead of a real filesystem path.
- **Quick**: `QuickWorkspaceManager.cpp`'s equivalents have zero WASM/Emscripten branching and
  unconditionally assume a real local path from the file dialog. `App/QuickShell/CMakeLists.txt`
  builds this exact target under `EMSCRIPTEN`, and `.github/workflows/wasm.yml` actively builds it
  in CI -- this is a live, shipped build target, not a hypothetical one.
- **Impact**: file Open/Save would be non-functional in the deployed WASM build today.

### IC toolbar controls stay active on an inline IC tab and silently no-op
*Area: IC embed/extract UI*
- **Widgets**: `MainWindow::setICButtonsVisible(!newTab->isInlineIC())` hides the entire
  Add/Remove/Make-Self-Contained button group while editing an inline IC tab, since those actions
  need a real project file/directory an inline tab doesn't have.
- **Quick**: `ElementPalette.qml`'s IC buttons have no such gating. Traced end-to-end: clicking one
  on an inline IC tab triggers a "Save required" prompt whose Save path
  (`QuickWorkSpace::save()`'s inline-IC branch) never sets `canvas->contextDir()`, so
  `ensureProjectSaved()` returns false and the original action still silently no-ops afterward.
- **Impact**: a real dead end, not just a missing disabled-state -- the user gets no feedback that
  the action failed.

### No keyboard shortcuts anywhere in the BeWavedDolphin waveform window
*Area: BeWavedDolphin*
- **Widgets**: `BeWavedDolphinUI.cpp` wires ~24 shortcuts (Ctrl+L/S/Shift+S/P/X/=/-, Ctrl+Shift+P/
  F/H, Alt+C/W/L/X/A, `0`/`1`/Space for cell edits, QKeySequence::Undo/Redo, F1, Ctrl+Home).
- **Quick**: `DolphinWindow.qml` has no `shortcut` property on any menu item/toolbar button and no
  `Shortcut{}` item anywhere in `App/QuickShell/Dolphin/`.
- **Impact**: every waveform-window action requires a mouse click; none of the ~24 shortcuts work.

### Waveform window is no longer modal to the main circuit
*Area: BeWavedDolphin*
- **Widgets**: `BeWavedDolphin.cpp` explicitly sets `Qt::WindowModal` ("so the user cannot
  interact with the main circuit while the waveform is open").
- **Quick**: `DolphinWindow.qml` sets no modality/flags; `QuickDolphinController::run()`'s own
  comment acknowledges "nothing blocks editing... while this window is open" and relies on a
  defensive `elementsStillLive()` guard instead of preventing the interaction up front.
- **Impact**: editing/deleting circuit elements while the waveform is open can desync or silently
  no-op the waveform instead of being blocked as in Widgets.

### Minimap doesn't swallow the scroll wheel
*Area: Minimap*
- **Widgets**: `MinimapWidget::wheelEvent()` calls `event->accept()` and explicitly does not call
  the base implementation, to prevent the main view from zooming while the cursor is over the
  minimap.
- **Quick**: none of `Minimap.qml`'s 10 `MouseArea`s declare `onWheel`, so unhandled wheel events
  fall through to `CanvasItem::wheelEvent()`, which unconditionally zooms the main view.
- **Impact**: scrolling over the minimap zooms the main circuit instead of doing nothing.

### Minimap shows stale colors after a theme switch
*Area: Minimap*
- **Widgets**: `MinimapWidget.cpp` connects `ThemeManager::instance().themeChanged` to
  `invalidateCache()`.
- **Quick**: `QuickMinimap`'s only regen triggers are undo-stack index changes, canvas
  zoom-changed, and size/visibility changes -- no connection to `ThemeManager::themeChanged`
  anywhere; `QuickAppController` forwards `themeChanged` only to its own QML-facing signal, never
  to the minimap presenter.
- **Impact**: after a light/dark theme switch, the minimap keeps rendering old-theme colors until
  the next circuit edit or pan/zoom regenerates it.

---

## Medium severity (19)

**Element property editor**
- Tab/Shift+Tab element-cycling from editor fields (Widgets' `ElementTabNavigator`) has no Quick
  equivalent anywhere.
- No visual "&lt;Many ...&gt;" placeholder when a multi-selection has divergent property values
  (display-only -- `apply()` still correctly avoids clobbering divergent unedited fields).
- Right-click context menu is missing "Embed sub-circuit"/"Extract to file" (backend methods
  already exist and are wired to `ElementEditor.qml` panel buttons, just never added as
  `MenuItem`s).

**Element right-click context menu**
- "Embed sub-circuit"/"Extract to file" missing (same as above; in Widgets these exist *only* in
  the context menu, with no side-panel equivalent at all -- Quick relocated the feature's sole
  access point).
- "Change color to..."/"Morph to..." submenus render disabled instead of being structurally absent
  when not applicable (e.g. non-colorable gates, non-morphable ICs) -- Widgets never shows the row
  at all in that case.

**Clock/Length dialogs**
- Clock period step is 1 instead of 2, so odd clock periods are reachable and generate a visibly
  uneven duty-cycle waveform, which Widgets' UI made unreachable by construction.

**Element palette**
- Ctrl+F to focus the palette search field is entirely absent (no shortcut mechanism of any kind
  exists in the app for this).
- Search drops 2 of 3 production search passes: matching the untranslated internal type keyword
  (e.g. `label_and`) and matching IC files by full filename/extension. Note: the Quick header's own
  comment claiming these object names are "never set in production" is factually wrong -- they are
  set on every built-in and file-IC label.

**IC embed/extract UI**
- "Add IC..." (file-based) button is never disabled for an unsaved project (falls back to a modal
  prompt instead of communicating the precondition via disabled state).

**File dialogs & tab/workspace management**
- Middle-click doesn't close a tab (only the small "X" button works).
- No "File saved/loaded successfully" status-bar confirmation after Open/Save/Save As, even though
  the exact mechanism (`showStatusMessage`) is already used successfully by sibling controllers --
  `QuickWorkspaceManager` simply isn't given host access to call it.

**Update checker**
- Dialog omits the user's currently-installed version (Widgets shows both new and current).
- Download progress dialog wrongly stays modal for the whole download/finished lifecycle, blocking
  the main window; Widgets' non-modal `QProgressDialog` deliberately keeps it interactive.

**BeWavedDolphin**
- Help menu (About / About Qt) is missing -- notably, `Main.qml` already has both dialogs
  implemented, so this is not a consistent app-wide gap as DolphinWindow.qml's own comment claims.
- Waveform window size/position isn't remembered across sessions (Settings infrastructure already
  exists and is simply unused here).
- Double-clicking a cell only flips the single cell, not the whole multi-cell selection, and isn't
  undoable (Widgets' version snapshots the selection and pushes an undoable command).
- Selecting a waveform row no longer clears canvas selection or highlights the matching circuit
  element.

---

## Low severity (23, cosmetic)

- Truth-table cells toggle on single click instead of double-click.
- Property panel has no section grouping/headers (Identity/Ports/Timing/Sound/Interaction/
  Appearance).
- Missing tooltips on Locked/Wireless/Value/Trigger/Delay controls.
- Volume slider has no tick marks; keyboard step is 1 instead of 5.
- "Appearance for:" caption label above the appearance tile grid is missing.
- "Change color to..." submenu doesn't exclude the element's current color (always lists a
  redundant no-op entry).
- "Set clock period" (Alt+W) and "Set Length" (Alt+L) shortcuts missing in BeWavedDolphin.
- Clock/Length dialog window titles read "Clock"/"Simulation Length" instead of Widgets' actual
  post-construction titles "Clock Period Selection"/"Simulation Length Selection".
- Palette item tooltips are missing the "Drag or double-click to add." usage hint.
- Search field and category tab strip lose their accessible name/What's This text (part of a
  broader, already-tracked app-wide accessibility gap -- zero `Accessible.*` usage anywhere in
  `App/QuickShell/`).
- Hovering the IC preview popup no longer keeps it open (fixed 300ms auto-hide regardless of
  cursor position).
- IC "Add" button labels are identical ("Add IC...") instead of Widgets' distinct "Add IC files"/
  "Embed IC" text.
- Trash drop target lost its dedicated button widget, trash-can icon, and click-for-hint popup
  (drag-to-remove itself still works; documented as a deliberate simplification).
- IC drop zones lost accessible name/description metadata.
- "File Conflict" dialog drops the one-click "Switch to Tab" button (documented deferral --
  `DialogProvider::choice()` has no room for a custom-labeled button yet).
- Tab hover tooltip (full file path) is missing, and there's no data path to even supply it.
- Dialog title and success/failure iconography don't change across update-checker states.
- "Fit to screen" no longer scales cell/header font size in BeWavedDolphin.
- "Save As" for a never-saved waveform doesn't default to the circuit file's folder (inconsistent
  with `load()`, which does have this fallback).
- Minimap has no hover tooltip.
- Minimap has no accessible name/What's This text (part of the same app-wide accessibility gap).
- Right-click/middle-click on the minimap no longer navigates the view (only left-click does).

---

## Notes on methodology

- Every "CONFIRMED" verdict required the verifier agent to independently re-read the exact cited
  file:line ranges in both worktrees and to grep broadly across the whole Quick tree for the
  feature before accepting an absence claim -- several findings note this explicitly caught cases
  where a doc comment's "this is dead code"/"nothing exists yet" claim was stale or simply wrong.
- A few gaps (trash-button icon, double-click multi-cell toggle, File Conflict "Switch to Tab")
  are already called out as deliberate simplifications in the Quick code's own comments. Per the
  audit's scope, a documented simplification is still a real, current, user-visible parity gap and
  is reported as such -- "intentional" and "not a gap" are not the same thing.
- Zero findings were refuted during verification.
