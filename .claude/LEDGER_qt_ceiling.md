# Ledger: the Qt ceiling

Tracks the items of `.claude/PLAN_qt_ceiling.md`. The plan says what and why; this file says where each item stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`QC.2a`, `QC.2b`, ...) and a line in the log. Status: `todo`, `doing`, `done`
(committed), `blocked` (on what, in the notes) or `dropped` (why, with the user's decision). The Commits column lists
every commit of the item. Every status change gets a dated log line.

**Branch:** `02b-qt-ceiling`, from `02a-qt-floor` once the floor has landed; not created yet.

**Open decisions:** QC.3, what issue #367 is told, the user's.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| QC.1 | The scene draws the drag's picture: `DragPreview`, the ghost at the element's size and drop position, both drags, its tests (§2.1) | todo | | Prototype `b459df797` on `probe/drag-ghost`, checked on this machine with Qt 6.10.2: `QDrag`'s picture shows nothing, the scene's follows the cursor |
| QC.2 | Qt 6.12.0: CI, the releases, the documents' recipes, the dev container, `CLAUDE.md`, what 6.12 needs in the code (§2.2) | todo | | §1.3 lists every place; §6's claims checked in it |
| QC.3 | Issue #367's resolution, and whether to post on QTBUG-142809 and Launchpad (§6) | todo | | The user's to decide and post |

## Log

- 2026-10-28: the plan file and this ledger written, every row `todo`, at the user's ruling that the floor and the
  ceiling are separate plans; the exception boundaries move from 2b to 2c.
