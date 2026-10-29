# Ledger: the Qt 6.8 floor

Tracks the items of `.claude/PLAN_qt_floor.md`. The plan says what and why; this file says where each item stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`QF.4a`, `QF.4b`, ...) and a line in the log. Status: `todo`, `doing`, `done`
(committed), `blocked` (on what, in the notes) or `dropped` (why, with the user's decision). The Commits column lists
every commit of the item. Every status change gets a dated line in the log.

**Branch:** `02a-qt-floor`, from `02-master-fixes` at `f73422ab7`, created 2026-10-28. One slice of one commit.

**Open decisions:** none.

Each row's notes say how its check is shown to fail; "as on the branch" marks a break `gpu-canvas` ran and saw fail.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| QF.1 | CMake: `find_package` asks for 6.8, WebAssembly keeps 6.9; the `lupdate` options and plan 1's translatable-strings target no longer wait for 6.7 (§2.1) | done | `4c54ccc8a` | Shown to fail: configuring against Ubuntu 24.04's apt Qt (6.4.2) stops at `find_package`, naming the version Done: configuring the floor against Ubuntu 24.04's apt Qt, in a bare `ubuntu:24.04` container, stops at `find_package`, naming Qt 6.4.2 against the 6.8 asked for. `translatable_strings_lupdate` builds unconditionally. |
| QF.2 | The dead guards: `Common.h`'s `qCDebug` override, `ThemeManager`'s palette-lightness guess and its comments, its three 6.8 blocks made unconditional, the colour-scheme tests' and the macOS palette tests' skips, `QVERIFY_THROWS` (37 uses to `QVERIFY_THROWS_EXCEPTION`), `LanguageManager.cpp`'s comment, the developer guide's two passages (§2.2) | done | `4c54ccc8a` | Shown to fail: with the `colorSchemeChanged` connection removed, the colour-scheme test that expects a reaction fails; a `QVERIFY_THROWS_EXCEPTION` whose expression throws nothing fails its test Done, on relwithdebinfo: with the connection taken out, `testSystemColorSchemeChangeReactsWhenThemeIsSystem` fails; `QVERIFY_THROWS_EXCEPTION(std::exception, (void) 0)` fails `testExportToArduinoThrowsOnEmptyFileName` ("Expected exception of type std::exception to be thrown but no exception caught"). |
| QF.3 | The documents: Qt 6.8 in the READMEs, the build documents, the developer guide and the dev container's README, in three languages; which releases have it and which use the aqt recipe (§2.3) | done | `4c54ccc8a` | Checked by the fresh distro workflow, started by hand on the pushed branch, since a push never runs it (QF.5, §4) Done: aqtinstall at Qt 6.9.3 first, a distribution's packages the fallback where they ship 6.8 or later (QF.D6, the user's ruling of 2026-10-28). The distributions' Qt read from their images: Debian 12 6.4.2, Debian 13 6.8.2, Ubuntu 22.04 6.2.4, 24.04 6.4.2, 25.04 6.8.3 and 25.10 6.9.2 (both end of life, not named), 26.04 6.10.2, Fedora 6.11.2, Arch 6.12.0. |
| QF.4 | The aqt recipe: a compiler, Qt Multimedia and Qt Image Formats, the plugins' run-time libraries, no `libxcb-xinerama0` (§2.4) | done | `4c54ccc8a` | Shown to fail: the recipe as on `master`, in a bare Ubuntu 24.04 container, fails to configure, Qt Multimedia missing (as on the branch); the `ldd` check fails with one of the plugins' libraries left out of the recipe Done, in bare `ubuntu:24.04` containers: the recipe as on `master` fails to configure, Qt Multimedia missing (a compiler came with `python3-pip`); the new one, taken from `BUILD.md` as written, builds, its `ldd` check passes and `wiredpanda --version` runs offscreen; with `libxcb-cursor0` removed the `ldd` check fails, naming `libxcb-cursor.so.0`. The FFmpeg backend tries PipeWire at run time and works without it. The macOS recipe's architecture and modules read from `aqt list-qt`; it runs first in CI's `aqt-macos` job. |
| QF.5 | CI: the matrix's comment naming the floor's 6.8 entries; the fresh distro jobs on Debian trixie and Ubuntu 26.04; the aqt job with its `ldd` check; the minimum CMake on Debian trixie (§2.5) | done | `4c54ccc8a` | Shown to fail: the fresh distro workflow's old `ubuntu:24.04` apt job, run locally against the raised floor, stops at `find_package` Done: the old `ubuntu:24.04` apt job, run locally against the floor, stops at `find_package` (QF.1). The `aqt` and `aqt-macos` jobs take their recipes out of `BUILD.md`; a changed line in `BUILD_es.md`'s recipe fails the translations' check, run locally. The fresh distro workflow is started by hand on the pushed branch. |
| QF.6 | Which of the 11 test files include `TestUtils.h` only for `QVERIFY_THROWS`: four on the branch, checked on `master` by compiling each alone without the include (§6) | done | `4c54ccc8a` | Shown to fail: a file that uses another helper fails to compile without the include Done: three files drop the include (`TestDolphinExporter`, `TestDolphinModelBuilder`, `TestCircuitExporter`, the last using `TestUtils::ScenePointer` from `Goldens.h`); `TestFileUtils`, one of the branch's four, now uses `TestUtils::` and keeps it, as do the other seven. |

## Log

- 2026-10-21: the plan file and this ledger written on `gpu-canvas`, every row `todo`.
- 2026-10-21: the floor raised from 6.7 to 6.8 at the user's ruling ("go with 6.8"), after RS.12's pass over the
  floor's decisions found that no maintained distribution ships 6.7, Qt's support for 6.7 ended in March 2025 while
  6.8 is supported until 2029, and CI's oldest Qt is already 6.8. QF.D1 now says 6.8; QF.D5 (the 6.8 guards stay)
  became "no new CI build", the 6.8 guards joining the dead ones (QF.2). The rows for a local 6.7.3 suite run (QF.6)
  and the first Windows and macOS 6.7.3 runs (QF.7) went, with no 6.7.3 build left to run; the include check, QF.8,
  is now QF.6. QF.5's break is now the old apt job against the raised floor, since no 6.7.3 job exists to break.
- 2026-10-28: branch created from `02-master-fixes` at `f73422ab7`, plan 2 done. QF.1 to QF.6 doing (`4c54ccc8a`):
  the floor, its dead guards, the documents, the recipes and CI in one commit; tip checked, suite 227/227 on
  relwithdebinfo, every changed source compiled alone and at `-O3`, `fix_style.py` clean.
- 2026-10-28: QF.D6 and QF.D7 added at the user's rulings: aqtinstall first with a distribution's packages the
  fallback; the floor and the ceiling separate plans, the ceiling's (Qt 6.12.0, the drag picture for issue #367) right
  after this one. The macOS recipe and its CI job added with them.
- 2026-10-28: QF.1 to QF.6 done: CI green on `48244a14e`, Build 40798106953, Sanitizers 40798106948, Lint
  40798107241, API docs 40798107236, and the fresh distro workflow, started by hand, 40798106406 (Debian trixie,
  Ubuntu 26.04, the minimum CMake, aqtinstall on Ubuntu 24.04 and on macOS 15). The floor has landed.
