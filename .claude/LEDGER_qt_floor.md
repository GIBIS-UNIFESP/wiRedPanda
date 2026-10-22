# Ledger: the Qt 6.8 floor

Tracks the items of `.claude/PLAN_qt_floor.md`. The plan says what and why; this file says where each item stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`QF.4a`, `QF.4b`, ...) and a line in the log. Status: `todo`, `doing`, `done`
(committed), `blocked` (on what, in the notes) or `dropped` (why, with the user's decision). The Commits column lists
every commit of the item. Every status change gets a dated line in the log.

**Branch:** its own, from `master`, once plan 2 has landed; not created yet. One pull request.

**Open decisions:** none.

Each row's notes say how its check is shown to fail; "as on the branch" marks a break `gpu-canvas` ran and saw fail.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| QF.1 | CMake: `find_package` asks for 6.8, WebAssembly keeps 6.9; the `lupdate` options and plan 1's translatable-strings target no longer wait for 6.7 (§2.1) | todo | | Shown to fail: configuring against Ubuntu 24.04's apt Qt (6.4.2) stops at `find_package`, naming the version |
| QF.2 | The dead guards: `Common.h`'s `qCDebug` override, `ThemeManager`'s palette-lightness guess and its comments, its three 6.8 blocks made unconditional, the colour-scheme tests' and the macOS palette tests' skips, `QVERIFY_THROWS` (37 uses to `QVERIFY_THROWS_EXCEPTION`), `LanguageManager.cpp`'s comment, the developer guide's two passages (§2.2) | todo | | Shown to fail: with the `colorSchemeChanged` connection removed, the colour-scheme test that expects a reaction fails; a `QVERIFY_THROWS_EXCEPTION` whose expression throws nothing fails its test |
| QF.3 | The documents: Qt 6.8 in the READMEs, the build documents, the developer guide and the dev container's README, in three languages; which releases have it and which use the aqt recipe (§2.3) | todo | | Checked by the fresh distro workflow, which runs on the pull request since it changes the documents (QF.5) |
| QF.4 | The aqt recipe: a compiler, Qt Multimedia and Qt Image Formats, the plugins' run-time libraries, no `libxcb-xinerama0` (§2.4) | todo | | Shown to fail: the recipe as on `master`, in a bare Ubuntu 24.04 container, fails to configure, Qt Multimedia missing (as on the branch); the `ldd` check fails with one of the plugins' libraries left out of the recipe |
| QF.5 | CI: the matrix's comment naming the floor's 6.8 entries; the fresh distro jobs on Debian trixie and Ubuntu 26.04; the aqt job with its `ldd` check; the minimum CMake on Debian trixie (§2.5) | todo | | Shown to fail: the fresh distro workflow's old `ubuntu:24.04` apt job, run locally against the raised floor, stops at `find_package` |
| QF.6 | Which of the 11 test files include `TestUtils.h` only for `QVERIFY_THROWS`: four on the branch, checked on `master` by compiling each alone without the include (§6) | todo | | Shown to fail: a file that uses another helper fails to compile without the include |

## Log

- 2026-10-21: the plan file and this ledger written on `gpu-canvas`, every row `todo`.
- 2026-10-21: the floor raised from 6.7 to 6.8 at the user's ruling ("go with 6.8"), after RS.12's pass over the
  floor's decisions found that no maintained distribution ships 6.7, Qt's support for 6.7 ended in March 2025 while
  6.8 is supported until 2029, and CI's oldest Qt is already 6.8. QF.D1 now says 6.8; QF.D5 (the 6.8 guards stay)
  became "no new CI build", the 6.8 guards joining the dead ones (QF.2). The rows for a local 6.7.3 suite run (QF.6)
  and the first Windows and macOS 6.7.3 runs (QF.7) went, with no 6.7.3 build left to run; the include check, QF.8,
  is now QF.6. QF.5's break is now the old apt job against the raised floor, since no 6.7.3 job exists to break.
