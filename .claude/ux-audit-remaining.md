# wiRedPanda UX Audit — Remaining Tasks

## Context
Tracking doc for the tail of the UX-audit work on branch `ux-improvements`. The audit's 4 bugs
(B1–B4), test/CI hardening (#42/#43), Tier 1 high-value UX, and all small/medium Tier 2–4 items
are DONE and release-verified — 40 commits banked locally (unpushed). Only the heavy,
design-driven features below remain. Full findings/rationale live in the plan file
`i-m-having-this-issue-purrfect-sunset.md`; item numbers match it.

## Remaining (heavy features, roughly smallest → largest)

- **#7 / #38 — Inline double-click rename/edit.** Double-click a plain element → inline rename;
  double-click a Text element → inline edit + a visible empty-state hint. Reuses existing
  label/rename plumbing. Smallest, most self-contained. Risk: low.
- **#16 — Align / distribute tools.** New Align (left/right/top/bottom/center) + Distribute
  actions for multi-selections, as an undoable `AlignCommand`; group-centroid math already exists
  (rotation pivots on it). New menu/toolbar entries. Risk: low–med.
- **#8 — Menu consolidation.** Merge single-item menus: Exercises + Tours → a "Learn" menu; move
  Report-Translation under Help. Structural UI reshuffle; touches the translation catalog (new
  menu title). Risk: low.
- **#14 — Accessibility sweep.** Add `setAccessibleName`/`setWhatsThis` across canvas, palette,
  minimap, drop zones (currently zero anywhere); fix hardcoded `color:gray`/`11px` in the
  Exercise/Tour browser UIs so they respect theme + font scale. Broad but shallow. Risk: low.
- **#19 — Waveform-editor undo/redo.** New `QUndoStack` subsystem for beWavedDolphin: wrap each
  destructive edit op (Set 0/1, Invert, Clear, Merge/Split) as a `QUndoCommand`, wire Ctrl+Z/Y.
  Largest of the set. Risk: med–high.

## Done since the audit (not remaining)
- **#13 — First-run onboarding: DONE (already on master, commit `524b9609a`).** `MainWindow::show()`
  auto-starts `:/Tours/ui-overview.json` on first-ever launch, gated on `Application::interactiveMode`
  + one-time `Settings::welcomeTourShown()`. No further work.

## Deferred / out of scope
- **#23** — exercise partial-progress feedback: a pedagogy choice, not a defect.
- **#11** — Save-As toolbar button: needs a distinct icon asset first.

## Working constraints (carried from CLAUDE.md / memory)
- One self-contained commit per fix; each fix gets a locking test.
- Build in `build/`; verify release build (`-Werror`) + full `ctest` before pushing.
- Do NOT run `update_translations` mid-branch (catalogs resync at release); the exercise-catalog
  generator is separate and IS run when exercise content changes.
- master is protected — commit/push only when asked.
- New test classes need the four-place registration.
