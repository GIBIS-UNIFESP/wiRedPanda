# Ledger: plan 9, editor architecture

Tracks the items of `.claude/PLAN_09_editor.md`. The plan says what and why; this file says where each item stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`P9.4a`, `P9.4b`, ...) and a line in the log. Status: `todo`, `doing`, `done`
(committed), `blocked` (on what, in the notes) or `dropped` (why, with the user's decision). The Commits column lists
every commit of the item. Every status change gets a dated line in the log.

**Branch:** `09-editor`, from `08-export` once plan 8 has landed; not created yet.

**Open decisions:** none yet; P9.13's probe settles the sprite rule (P9.D11), and P9.28 needs the user once the ports
and labels above overlapping elements, and any redrawn glows, can be shown.

Each row's notes say how its check is shown to fail; "as on the branch" marks a break `gpu-canvas` ran and saw fail.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| P9.1 | The layer targets, the include check with its test and break checks, per-layer precompiled headers (§2.1, §2.2) | todo | | Shown to fail: a later layer's header included from a lower layer fails the check's test tree, naming the file and its owner (as on the branch); a bare `<QGraphicsItem>` in the model fails as surely as the module-qualified form |
| P9.2 | The untangling of the layers' crossings (§2.3) | todo | | Each crossing's removal kept by its layer's include check; the translatable-strings golden shows no context moved |
| P9.3 | Orientation as one transform; the element sends scene position changes (§2.4) | todo | | Shown to fail: a plain move of an element whose ports are not child items leaves its wires behind |
| P9.4 | The circuit's bounds from the registry, every reader moved (§2.4) | todo | | Shown to fail: an overlay or a routed wire's free end counted in the bounds; MCP's image export of an empty scene, whose error plan 2 makes reachable |
| P9.5 | Ports as plain objects in a port layer (§2.5) | todo | | Shown to fail: a port past 2^24 units from the origin not drawn (as on the branch, for ports, wires and labels); two ports within reach answering the lower one; setting a port's default cutting a connected input's wire |
| P9.6 | Wires as plain objects in a wire layer, the curve grid, the selection API (§2.5) | todo | | Shown to fail: a wire 7 × 10^7 units long sampled before overflow, 6.4 s and 323 MB (as on the branch); a hidden wire keeping the mouse (as on the branch); a double-click on crossing wires splitting the lower one (as on the branch); a wire that joins a grid missed by its next query |
| P9.7 | Labels as data drawn by a label layer (§2.5) | todo | | Shown to fail: turning a labelled Text element leaves stale index entries (as on the branch) |
| P9.8 | Neutral events (§2.6) | todo | | Shown to fail: a double-click that Qt delivers as a press on an input does not operate it (as on the branch) |
| P9.9 | Neutral queries (§2.6) | todo | | Shown to fail: a caller of a hidden Qt query does not compile until it picks a neutral query (as on the branch); `selectInArea()` emitting `selectionChanged()` when nothing changed |
| P9.10 | One change funnel, the document listener, the coverage test and every bulk scope's test (§2.7) | todo | | Shown to fail: a change kind taken out of the funnel fails the coverage test; paste, a blob load and the IC reload each one bulk change (five of the eight scopes were in no test on the branch) |
| P9.11 | The painters out of the layer items; presentation state off ports and wires (§2.12) | todo | | Plan 1's goldens unchanged on the old view |
| P9.12 | The sprite cache; bodies as sprites in a view and vectors in exports; Qt's device cache gone (§2.12) | todo | | Shown to fail: a sprite off whole device pixels at 1.25 and 1.5 (as on the branch); each cache test fails when the part it covers is broken (as on the branch); a sprite drawn through `combinedTransform()` at double size at ratio 2; a wrong sprite returned through 1,000 evictions |
| P9.13 | The sprite probe at 1.1, 1.25, 1.5 and 2, and what it decides (§2.13) | todo | | Its answer recorded with the rule it supports; the glows redrawn only if it shows the filters are the cause, and shown to the user first (P9.28) |
| P9.14 | The inventory of `Scene`'s callers by group (§2.9) | todo | | Taken when the step starts |
| P9.15 | The editing session; Qt's click selection and drag in the interaction code, then the item flags off (§2.8) | todo | | Plan 1's pointer goldens on both; shown to fail: a Ctrl press that selects without dragging a clone |
| P9.16 | The document; the commands on it; `Scene`'s callers migrated by group (§2.9) | todo | | Commands compile against the document only; the stacking goldens unchanged |
| P9.17 | The controller: the state machines, routing as an overlay, snapping as policy, overlay state as data (§2.10) | todo | | Shown to fail: a half-connected wire that autosave sees; a loaded or MCP-created off-grid position snapped |
| P9.18 | Controller tests in place of view-driven ones (§2.10) | todo | | |
| P9.19 | The new view and the CPU backend beside the old view, the goldens on both (§2.11, §2.14) | todo | | Shown to fail: a GPU viewport whose paint and resize the view keeps never renders (as in a spike) |
| P9.20 | The callers switched to the new view (§2.14) | todo | | Shown to fail: a backend switch without `viewportReplaced` leaves the minimap's view rectangle behind |
| P9.21 | The view-bound tests migrated (§2.14) | todo | | |
| P9.22 | The old view, the layer items, the proxy editor and the scene deleted (§2.14) | todo | | Every caller compiler-checked |
| P9.23 | `Element` a plain class; pointers to elements become ids (§2.14) | todo | | A headless document (command-line exports, MCP, tests) gets `BodyMeasure` installed |
| P9.24 | The developer guide and the build documents (§2.15) | todo | | |
| P9.25 | The CPU backend measured against `master` (§2.15) | todo | | On plan 1's fixtures: a pan, a zoom, a status change, a load's first frames |
| P9.26 | The manual pass (§2.15) | todo | | |
| P9.27 | Whether today's `master` still pays Qt's BSP index rebuild after a load (§6) | todo | | Plan 1's probe |
| P9.28 | What the user sees new (§6) | todo | | Needs the user: ports and labels above overlapping elements; any redrawn glows |
| P9.29 | Whether the listener's events are enough for plan 10's mirror (§6) | todo | | Checked against plan 10's mirror before the listener is final |

## Log

- 2026-10-21: the plan file and this ledger written on `gpu-canvas`, every row `todo`.
