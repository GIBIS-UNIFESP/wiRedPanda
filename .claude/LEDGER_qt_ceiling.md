# Ledger: the Qt ceiling

Tracks the items of `.claude/PLAN_qt_ceiling.md`. The plan says what and why; this file says where each item stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`QC.2a`, `QC.2b`, ...) and a line in the log. Status: `todo`, `doing`, `done`
(committed), `blocked` (on what, in the notes) or `dropped` (why, with the user's decision). The Commits column lists
every commit of the item. Every status change gets a dated log line.

**Branch:** `02b-qt-ceiling`, from `02a-qt-floor`.

**Open decisions:** the user's: QC.3, what issue #367 is told; QC.2a, whether the loader guards against Qt's
`qUncompress()` defect on the floor; QC.2b, `master`'s required checks.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| QC.1 | The scene draws the drag's picture: `DragPreview`, the ghost at the element's size and drop position, both drags, its tests (§2.1) | done | `e576c59f6` | Prototype `b459df797` on `probe/drag-ghost`, checked on this machine with Qt 6.10.2: `QDrag`'s picture shows nothing, the scene's follows the cursor. Landed: the ghost at the drop's snapped position and the element's scene size, both drags, the copy-drag's picture shown again (the user's ruling); four tests, each broken once; tip: 227/227, compile-alone and -O3 clean |
| QC.2 | Qt 6.12.0: CI, the releases, the documents' recipes, the dev container, `CLAUDE.md`, what 6.12 needs in the code (§2.2) | doing | `babdebb06` | §1.3 lists every place; §6's claims checked in it. Done locally on Qt 6.12.0 (aqtinstall 3.3.0): relwithdebinfo with `-Werror` builds with no warning; suite 227/227 once the two truncated-stream tests accept 6.12's qUncompress warning (each failed on 6.12 before, the break); the translatable-strings catalog extracted with 6.12's lupdate and its golden passing; `BUILD.md`'s recipe, taken out by the workflow's awk in a bare `ubuntu:24.04`, installs 6.12.0, builds, passes the `ldd` check (now `libqwayland.so`: 6.12 merged the Wayland plugins, and the old glob matched nothing) and runs `--version` offscreen. Waiting on CI and the fresh distro workflow |
| QC.2a | Qt 6.8.3 to 6.11.2 reach a `Q_UNREACHABLE` in `qUncompress()` on a truncated stream (QTBUG-150081, fixed in 6.12.0 and 6.11.3, picked to 6.8): undefined behaviour in a release build of Qt, which the project's loader reaches with a truncated `.panda` file | todo | | Found in QC.2. The releases bundle 6.12.0, so they are fixed; builds against the floor's Qt rely on their Qt carrying the fix. Whether the loader should avoid handing Qt a truncated stream is open |
| QC.2b | `master`'s branch protection requires the build jobs by name, six of them `Qt6.9.3`; once the ceiling reaches `master` they never report, so the required checks must name the `Qt6.12.0` jobs | todo | | Found in QC.2. A repository setting, the user's to change, when the plans reach `master` |
| QC.3 | Issue #367's resolution, and whether to post on QTBUG-142809 and Launchpad (§6) | todo | | The user's to decide and post |
| QC.4 | A copy-drag after a palette drop jumped the view as the copy landed: `resizeScene()` restored scroll values that do not say where the view is while Qt centres a rect smaller than the viewport | done | `7c5442de0` | Reported by the user. Reproduced offscreen: the copy drop moved the view's corner from (-68,-48) to (7,7); a move drag after a palette drop moved it too. Fixed: the corner's scene point kept in both branches and in a right-to-left view; the palette drop and double-click/search add resize the scene. Three tests; each break (the old `resizeScene()`, no right-to-left sign, no resize on the palette drop) fails one; the view behaviours' golden read and regenerated (zoom-out from step 13 no longer drifts). Tip: 227/227, compile-alone and -O3 clean |
| QC.4a | A zoom step that shows more than the scene rect moves the view: `scale()` clamps its pivot to the old rect before `zoomChanged` lets `resizeScene()` grow it (the view behaviours' golden, zoom-out step 10: 7 pixels, back on the next step) | done | `fb1332b73` | Found in QC.4; fixed at the user's ruling ("fix the zoom clamp too"). Every zoom pins its pivot's scene point, grows the scene over what it will show first (`Scene::growSceneOver()`), then scrolls the point back; the buttons and keys pivot on the viewport's centre, not Qt's stale last centre (their first steps had moved the centre to the scene's origin). A new test zooms a tight scene out and in from each corner, by wheel and button, in both layout directions; each break (no growth, no scroll back, no right-to-left sign, the pivot always the centre) fails it or the wheel test; the golden read and regenerated. Tip: 227/227, compile-alone and -O3 clean |

## Log

- 2026-10-28: the plan file and this ledger written, every row `todo`, at the user's ruling that the floor and the
  ceiling are separate plans; the exception boundaries move from 2b to 2c.
- 2026-10-28: QC.1 done (`e576c59f6`): the branch made from `02a-qt-floor`; the copy-drag's picture, at opacity 0
  since 2022, shown again at the user's ruling ("Show it").
- 2026-10-28: QC.2 doing (`babdebb06`): Qt 6.12.0 everywhere 6.9.3 was, CI on a pinned commit of aqtinstall's
  master (QC.D4); checked locally, CI pending. QC.2a and QC.2b found, both `todo`.
- 2026-10-29: QC.4, reported by the user, done (`7c5442de0`): resizing the scene keeps the view where it is. QC.4a
  found, `todo`.
- 2026-10-29: QC.4a done (`fb1332b73`) at the user's ruling: a zoom keeps the point it pivots on still.
