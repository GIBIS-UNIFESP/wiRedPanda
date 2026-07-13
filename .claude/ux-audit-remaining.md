# wiRedPanda UX Audit — Status

## Context
Tracking doc for the UX-audit work on branch `ux-improvements`. The audit's 4 bugs (B1–B4),
test/CI hardening (#42/#43), Tier 1 high-value UX, all small/medium Tier 2–4 items, and the 5
remaining heavy/design-driven features below are now all DONE and release-verified — 45 commits
banked locally (unpushed). Full findings/rationale live in the plan file
`i-m-having-this-issue-purrfect-sunset.md`; item numbers match it.

## Heavy features — all DONE
- **#7 / #38 — Inline double-click rename/edit.** `GraphicElement::mouseDoubleClickEvent()` +
  new `InlineLabelEditor` scene collaborator; reuses `UpdateCommand` for undo. `Text` gets a
  faint empty-state hint (the dormant two-appearance pixmap swap was left alone — never wired
  up to begin with).
- **#16 — Align / Distribute tools.** `Scene::alignLeft/Right/Top/Bottom/HorizontalCenter/
  VerticalCenter`/`distributeHorizontally/Vertically`, reusing `MoveCommand` (no new command
  class). New Edit-menu entries.
- **#8 — Menu consolidation.** `menuExercises`/`menuTours` re-parented as submenus under a new
  `menuLearn`; the standalone Translation menu removed, its one action folded into Help.
- **#14 — Accessibility sweep.** `setAccessibleName`/`setWhatsThis` added to the canvas, palette
  (search box + tabs), minimap, and IC drop zones. Separately, `ExerciseOverlay`/`TourOverlay`'s
  hardcoded font-size `px` values now scale with the application font (the audit's original
  "hardcoded `color:gray`" citation didn't hold up on re-check — both overlays already
  theme-switch colors correctly; only the font sizing was stale).
- **#19 — Waveform-editor undo/redo.** New `QUndoStack` + Undo/Redo actions in beWavedDolphin;
  a single generic `SetCellsCommand` (`DolphinCommands.h`) wraps Set 0/1, Invert, Clear,
  clock-wave fill, combinational fill, and the double-click toggle. Merge/Split excluded
  (permanently-disabled placeholders, not live operations); `setLength()`/AutoCrop's
  column-count resize left out of scope (structurally different — would need its own resize-
  aware command).

## Done since the audit (not remaining)
- **#13 — First-run onboarding: DONE (already on master, commit `524b9609a`).** `MainWindow::show()`
  auto-starts `:/Tours/ui-overview.json` on first-ever launch, gated on `Application::interactiveMode`
  + one-time `Settings::welcomeTourShown()`. No further work.

## Deferred / out of scope
- **#23** — exercise partial-progress feedback: a pedagogy choice, not a defect.
- **#11** — Save-As toolbar button: needs a distinct icon asset first.
- **setLength()/AutoCrop undo** (surfaced during #19) — column-count resize can discard trailing
  data; would need a dedicated resize-aware undo command, not the value-only `SetCellsCommand`.

## Working constraints (carried from CLAUDE.md / memory)
- One self-contained commit per fix; each fix gets a locking test.
- Build in `build/`; verify release build (`-Werror`) + full `ctest` before pushing.
- Do NOT run `update_translations` mid-branch (catalogs resync at release); the exercise-catalog
  generator is separate and IS run when exercise content changes.
- master is protected — commit/push only when asked.
- New test classes need the four-place registration.
