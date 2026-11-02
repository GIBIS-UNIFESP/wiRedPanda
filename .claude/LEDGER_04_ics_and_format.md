# Ledger: plan 4, ICs and the document format

Tracks the items of `.claude/PLAN_04_ics_and_format.md`. The plan says what and why; this file says where each item
stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`P4.4a`, `P4.4b`, ...) and a line in the log. Status: `todo`, `doing`, `done`
(committed), `blocked` (on what, in the notes) or `dropped` (why, with the user's decision). The Commits column lists the code commits of the item; a record commit is named in the log. Every status change gets a dated line in the log.

**Branch:** `04-ics-and-format`, from `03-element-types` at `2fe7e07f9` (the creation point; the merge-base with `03-element-types` is `d8f9e8765`), created 2026-11-02. Hashes from before the restack that no longer resolve were replaced by the commit each row names; those replacements are unverified.

**Open:** P4.10 (the bytes key's cost, to be measured under callgrind, which the Windows machine lacks) and P4.11 (carried by
plan 8's P8.15). D6 is fixed (P4.6, `490983679`), D7 is fixed (P4.2a, `33cc4ceaf`, the nested-file change), D-S1 and D-C12
are fixed on plan 2 (P2.105, P2.106). D3 (a flaky plan 2 test) and D8 (an intermittent unhandled exception, not
reproduced) are discarded, as the user directed.

Each row's notes say how its check is shown to fail; "as on the branch" marks a break `gpu-canvas` ran and saw fail.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| P4.1 | The reload's undo as a failing test on `master`: one input after the undo, the cut wire still cut (§2.1) | done | `6f9f80d1f`, `2713b6bfa`, `a7a538357` | The probe run on plan 1's tip (`952aa6b2c`, the code before plan 2's fixes, with plan 1's MSVC warning flag, C4702, in a throwaway worktree): the one-input undo fails there, as §1.2 says. On plan 2's tip it passes, so plan 2 fixed that case. The reopen defect remains on the tip, marked by a `QEXPECT_FAIL` until P4.6. Edge probes in `TestICRegistry` (the same file): a growing reload, two reloads undone in turn, two instances of one file, undo then redo. Plan 1 fails all four; the tip passes the two instances and the shrinking undo, and fails the growing undo and the second reload's undo, marked until P4.5 |
| P4.2 | The shared definition, its cache keyed by path and bytes for a file and by bytes for a blob, and the loader reading through `ICFileCache` (§2.2) | todo | | Shown to fail: eviction by insertion order, a failed read kept; a key of path, time and size, which reuses the definition of a file rewritten at the same size within the same millisecond |
| P4.2a | A change to a file an IC nests reloads the instances that nest it: the scene watches each file-backed IC's nested files, and a reload watches the files it now reads (D7). The outer definition is keyed by its own bytes (§2.2), so a nested file's change leaves it stale until the outer instance reloads: the same staleness §2.2's key exists for | done | `f5d8395a3` | Shown to fail on this branch, each break once, on relwithdebinfo (`ICRegistry.cpp` and this test's body are unchanged since this commit): the nested reload removed from `ICRegistry::onFileChanged` fails `TestICRegistry::testNestedFileChangeReachesTheOuterInstance` at its `definitionChangedSpy.count() == 1` check (0 instead of 1, line 1578 at this commit and 1579 on the tip); the reload's undo cleared right after it fails the same test at its `canUndo()` check (line 1579 at this commit and 1580 on the tip). The expected failure's mark is removed by the fix, and the test passes on the fix, on relwithdebinfo: the function alone (3 passed) and the class `TestICRegistry` (53 passed, 1 skipped, on the tip, which adds a later test) |
| P4.3 | The preview per definition, rendered on first hover; its guard checks the origin (§2.3) | todo | | Shown to fail: the guard checking the size only (as on the branch); a theme change that keeps previews. Plan 1's preview goldens unchanged |
| P4.4 | Pin names from the circuit in one rule; the header in file order; no pin name saved in the interface language (§2.4) | todo | | Shown to fail: the header built in stacking order after an undo's reorder (as on the branch); a derived name made explicit. Payload goldens holding IC headers or pin names change, each named |
| P4.5 | The reload's undo restores the definition in memory (§2.5) | done | `9610a7615` | Shown to fail: the four probes of `TestICRegistry` that the undo of a reload reaches (a growing reload, two reloads undone in turn, a change after an undo, two writes settled to the last), with their expected-failure marks removed, failed on the tip before the fix. The first fix, in the loader and the command, left them failing: `IC::loadState` skipped the reload when the IC's bytes matched the file's current bytes, so the skip also compares the snapshot's bytes (`ICLoader::fileBytes()`). The reopen part is P4.6 |
| P4.6 | The document's copies of file-backed definitions with their hash; the four cases on open (§2.6) | done | `92a740b56` | The four cases on open, each tested: a file unchanged since the save keeps the copy (`testReopenAfterGrowingUndoKeepsTheRestoredDefinition`, `testReopenedDocumentSavesAgainByteForByte`); a file changed since comes in as an undoable reload (`testReopenChangedFileReloadsWithAnUndo`); a file missing keeps the copy (`testReopenMissingFileKeepsTheCopy`); no copies reads the files, as before (the backward-compatibility fixtures). The copy is kept out of the live registry (`testOpenedFileCopyStaysOutOfTheRegistry`). Each probe fails when broken: copies ignored, the reload skipped, the reload forced, no copy saved, the copy kept in the registry, the loaded file's bytes not recorded. The payload goldens of documents with file-backed ICs changed (180 files, each references an IC file). The format extends the blob registry with a key starting with a NUL, so no earlier document changes; the developer guide's format section is P4.7, still todo |
| P4.7 | The developer guide's IC and format sections; the release notes' format changes | todo | | |
| P4.8 | The reload's undo behaves on `master` as probed on the branch (§5) | done | | Holds on plan 1, the code before plan 2 (P4.1): the one-input undo fails, as §1.2 says. Plan 2 fixed the shrinking case and the two-instance case, not bisected to a commit. §1.2 and §2.5 still describe a defect that the tip does not have for those cases, so their wording is revised in P4.5 to name the growing and repeated cases; the reopen case stands |
| P4.9 | Whether an IC's first reload in a scene always brings another definition (§5) | todo | | Checked here by counting definitions across a first reload; plan 6 checks it for its node ranges |
| P4.10 | The cost of keying a file's definition by its bytes, on the IC-heavy fixture's load (2,000 instances of one file) (§5) | todo | | Measured before P4.2 lands, under callgrind with plan 1's probe; should the profile show the key, it goes back to path, time and size, with the two blind spots stated in the code |
| P4.11 | The hand-over of §2.7: the internal elements, the IC's own serialization and the definitions' move into the document's registry, done by plan 8's last step | todo | | Carried by plan 8 (P8.15); closed here when that step lands |
| P4.12 | The pin order saved in every file as the boundary elements' ids, migrated from positions before any snap; every element snapped on load; landing together (§2.9, P4.D5) | todo | | Added 2026-10-22 from plan 2's snapping rule (P2.42), which leaves the load exact. Shown to fail: the saved order ignored swaps two pins closer than half the grid; the migration taken after the snap swaps them too. P1.14's pin orders unchanged |
| P4.13 | Undo and redo across the application, probed item by item (the section below): the commands, the macros, the stack behaviours and the IC reload | done | `349d8aaa4`, `14c40ef16` | Each item marked checked is broken on purpose once and shown to fail, as the mutation table below records (35 mutations on the App code, 5A, and 2 on the release-only paths, 5B). The reload probes and the waveform's redo are checked by the mutations listed against them. The expected failures that remain hold four defects the probes found (D6, D7, D-S1, D-C12); none is fixed here, and D5 is fixed by P4.5. Plan 2's fixes to the reload (D1 to D4) are P2.104, not here |
| P4.14 | The rollback of a failed IC save over the earlier targets, with a seam (`WorkSpace::saveChildICBlob`) to fail a later one | done | `14c40ef16` | Shown to fail: `seam-rollback-elements-removed` and `seam-rollback-registry-removed` (the seam probe is `testSaveChildICBlobFailureAtALaterTargetRollsBackTheEarlierOnes`). The seam is product code; its inventory rows move with its lines in the same commit |

## Undo and redo probes (P4.13, P4.14)

The probes check undo and redo across the application. Each item is **checked** (a probe fails under a mutation of its
behaviour, in the mutation table), **defect** (an expected failure holds the probe), **existing** (only tests that
predate this work cover it) or a **limit** (no product mutation can reach it).

**Audit first.** Every test that calls `undo()` or `redo()`, or triggers one through an action or key, was read: 482
functions in 24 classes, 233 of them undo tests. Families before this work: A do, undo and redo, 94 of 226 undo tests
covering redo, 132 undoing without redoing; B wires, thin for rotate, move, flip, truth table and blob commands; C several
elements, covered; D new edit after undo, three tests; E save and reopen, one test; F repetition, one test and no ten
cycles. The per-test verdicts are kept outside the repository, not committed.

**Items.**

| Item | Behaviour | Status | Mutation |
|---|---|---|---|
| C1 `AddItemsCommand` | undo | checked | `add-undo-noop` |
| C2 `DeleteItemsCommand` | undo; ten delete cycles; a new edit discards the redo | checked | `delete-undo-noop` |
| C3 `RotateCommand` | undo; ten cycles; wire ends across undo; discard | checked | `rotate-undo-noop` |
| C4 `MoveCommand` | undo; ten cycles; wire ends across undo; discard | checked | `move-undo-noop` |
| C5 `UpdateCommand` | undo | checked | `update-undo-noop` |
| C6 `SetPropertyCommand` | redo rewires the Rx label and the wireless channels | checked | `wireless-redo-no-rebuild` |
| C7 `SplitCommand` | redo; ten cycles; wire ends across undo and redo | checked | `split-redo-noop` |
| C8 `MorphCommand` | redo keeps frequency and colour; ten cycles; discard | checked | `morph-redo-noop` (see limits) |
| C9 `ChangePortSizeCommand` | ten cycles cut and restore the wire; discard | checked | `portsize-no-cut` |
| C10 `FlipCommand` | ten cycles; discard; wire ends | checked | `flip-undo-noop` |
| C11 `ToggleTruthTableOutputCommand` | the wire and the table's size across undo and redo | checked | `truthtable-undo-noop` |
| C12 `RegisterBlobCommand` | redo restores the bytes; undo over a name in use | checked (redo); undo fixed on plan 2 (D-C12, P2.106) | `register-redo-noop` |
| C13 `RemoveBlobCommand` | wire kept across undo and redo; embedded IC removed again on redo | checked (redo) | `removeblob-redo-noop` |
| C14 `RemoveICFileCommand` | trashed again on each redo; a refusal by the trash leaves no step (Windows) | checked (redo) | `trash-redo-noop` |
| C15 `RenameBlobCommand` | nested reference renamed and back; a collision changes nothing | checked | `rename-undo-noop`, `rename-collision-unguarded` |
| C16 `UpdateBlobCommand` | the reload probes below | checked | `update-blob-redo-old`, `update-blob-undo-new` |
| S1 `SetCellsCommand` (waveform fill) | redo fills again | checked (redo); undo fixed on plan 2 (D-S1, P2.105) | `setcells-redo-noop` |
| M1 paste with an embedded IC | existing tests only | existing | none |
| M2 embed | redo restores the blob and the instance | checked | `register-redo-noop` |
| M3 remove an embedded IC | redo removes it again | checked | `removeblob-redo-noop` |
| M4 cycle of element properties | one step each way over several elements; port-count change redo | checked | `cycle-no-macro`, `update-redo-noop`, `portsize-redo-noop` |
| M5 morph of several elements | next and previous morph are each one step | checked | `morph-next-no-macro`, `prev-morph-no-macro` |
| M6 appearance change | the tile grid hidden for a multi-selection (`TestElementEditor`) | checked | `tile-grid-multi` |
| M7 wireless mode change | undo and redo rebuild the channels | checked | `wireless-undo-no-rebuild`, `wireless-redo-no-rebuild` |
| M8 embed or extract with a child reload | a later target's failure rolls back the earlier ones (P4.14); undo of embed and extract | checked | `seam-rollback-elements-removed`, `seam-rollback-registry-removed`, `update-blob-undo-new` |
| U1 dropped command | leaves the stack and the scene as they were | checked | `drop-not-obsolete` |
| U2 a new command discards the redo | seven probes | limit: `QUndoStack` discards the redo itself; the probes push on the stack directly | none (`push-skip-after-undo` fails only the existing test) |
| U3 undo on an empty stack | existing | existing | none |
| U4 save makes the stack clean | undo back to the save is clean | checked | `save-not-clean` |
| U5 reopened document starts empty | six reopen probes | checked | `reopen-pushes-step` |
| U6 autosave leaves the stack as it was | the count, the position and the clean state after an autosave | checked | `autosave-clears-after-write` |
| U7 long history | 200 steps undone and redone | checked | `long-history-limit` |
| 5B release-only failure paths | `CommandUtils::abandon` and the truth table's bound (relwithdebinfo, where `QT_NO_DEBUG` leaves the tests running) | checked | `abandon-not-obsolete`, `truthtable-bounds-unguarded` |

**Reload probes** (`TestICRegistry`, C16, M2, M8 and the file watch): the probes cover a shrinking, growing and repeated
reload, two instances of one file, a change after an undo, a reopen after an undo, a locked, emptied, corrupt or deleted
file, a rename save, a change that goes back to its original, a simulation that runs during a reload, and a nested file.
The file-watch defects D1 to D4 are fixed on plan 2 (P2.104), and their probes are there.

**Defects found by the probes.** Plan 4 leaves no expected failure of its own: D6 is fixed (P4.6), D7 is fixed (`33cc4ceaf`), and D-S1 and D-C12 were fixed on plan 2 (`303c92af3`, `10f274328`); no code mark existed for them here. The one expected failure left in the tests, `TestPortReaders` (`value()` of bits that are not all levels), is outside plan 4.

| ID | Behaviour | Probe | Plan | Status |
|---|---|---|---|---|
| D1 to D4 | a rewrite with identical bytes, a touch, a save by rename and a file made again after deletion | plan 2's probes | plan 2 | fixed (P2.104); D3 is flaky (one failure in three `TestICRegistry` runs on the clean tree), discarded, as the user directed |
| D5 | the undo of a growing reload keeps the added input | expected failures in `TestICRegistry` (the growing undo, two reloads undone in turn, a change after an undo) | plan 4, P4.5 | fixed (P4.5, `d28c93d0e`) |
| D6 | a reopened document brings the file's new definition back after an undo | expected failure in `TestICRegistry` (the reopen part of the shrinking undo, and the growing reopen) | plan 4, P4.6 | fixed (P4.6, `490983679`); mark removed here |
| D7 | a nested IC file's change is not reported to the outer instance | expected failure in `TestICRegistry` (mark removed) | plan 4, P4.2a | fixed (`33cc4ceaf`) |
| D8 | an intermittent "Caught unhandled exception" | none: reproduced once while the nested-file probe ran, not since | plan 2, if on the reload path | discarded, as the user directed; not reproduced |
| D-S1 | undoing a waveform fill keeps the columns it added | expected failure in `TestBewavedDolphinGui` | plan 2, P2.105 (`303c92af3`) | fixed on plan 2 (`303c92af3`); no code mark existed here |
| D-C12 | registering over a name in use loses that name's blob on undo | expected failure in `TestICInline` (two marks) | plan 2, P2.106 (`10f274328`) | fixed on plan 2 (`10f274328`); no code mark existed here |

**Mutation record.** Each mutation changes one behaviour in `App/`, runs the listed classes and restores the file from a
backup. The failing tests are listed; counts are failing lines in the run's log.

| Mutation | Behaviour broken | Classes | Failing |
|---|---|---|---|
| `flip-undo-noop` | `FlipCommand::undo` does nothing | TestSceneUndoredo, TestCommands | 9 and 3 |
| `portsize-no-cut` | a shrink does not cut the wires of removed ports | TestSceneUndoredo | 6 |
| `rename-undo-noop` | `RenameBlobCommand::undo` does not rename back | TestICInline | 3 |
| `rename-collision-unguarded` | the registry's rename guard is removed | TestICInline | 2 |
| `truthtable-undo-noop` | `ToggleTruthTableOutputCommand::undo` does nothing | TestCommands | 2 |
| `removeblob-redo-noop` | `RemoveBlobCommand::redo` does not remove | TestICInline | 6 |
| `tile-grid-multi` | the tile grid shows for any selection | TestElementEditor | the new tile test |
| `save-not-clean` | a save does not mark the stack clean | TestSceneUndoredo, TestWorkspace | 3 and 1 |
| `wireless-redo-no-rebuild` | a redo of a wireless change does not rebuild the channels | TestCommands | 3 |
| `seam-rollback-elements-removed` | a failed later target leaves the earlier targets reloaded | TestICInline | the seam probe |
| `seam-rollback-registry-removed` | a failed save leaves the registry's blob changed | TestICInline | the seam probe and `testChildBlobSavedCorruptBlobTouchesNoTarget` |
| `update-blob-redo-old` | a redo of a file-driven update loads the old definition | TestICRegistry, TestICInline | 10 and 13 |
| `setcells-redo-noop` | a redo of the waveform fill writes nothing | TestBewavedDolphinGui | 8 |
| `trash-redo-noop` | a redo of an IC file removal does not trash the file again | TestICController, TestMainWindowGui | 2 and 1 |
| `morph-next-no-macro` | the next morph of several elements has no macro | TestPropertyShortcutHandler | 2 |
| `autosave-clears-after-write` | the stack is cleared after the autosave file is committed | TestWorkspace | 5 |
| `add-undo-noop` | `AddItemsCommand::undo` does nothing | TestSceneUndoredo, TestCommands | 10 and 3 |
| `delete-undo-noop` | `DeleteItemsCommand::undo` does nothing | TestSceneUndoredo, TestCommands | 13 and 7 |
| `rotate-undo-noop` | `RotateCommand::undo` does nothing | TestSceneUndoredo | 8 |
| `move-undo-noop` | `MoveCommand::undo` does nothing | TestSceneUndoredo | 7 |
| `update-undo-noop` | `UpdateCommand::undo` does nothing | TestCommands, TestSceneUndoredo | 11 and 8 |
| `split-redo-noop` | `SplitCommand::redo` does nothing | TestCommands | 5 |
| `morph-redo-noop` | `MorphCommand::redo` does nothing | TestCommands, TestSceneUndoredo | one unhandled exception per class, which stops the class |
| `register-redo-noop` | `RegisterBlobCommand::redo` does nothing | TestICInline, TestICRegistry | 5 and 3 |
| `update-blob-undo-new` | `UpdateBlobCommand::undo` loads the new definition | TestICRegistry, TestICInline | 13 and 3 |
| `setproperty-redo-noop` | a redo of `SetPropertyCommand` writes nothing | TestCommands | 6 |
| `update-redo-noop` | `UpdateCommand::redo` does nothing (the shortcut cycle's redo) | TestPropertyShortcutHandler, TestSceneUndoredo | 2 and 6 |
| `portsize-redo-noop` | `ChangePortSizeCommand::redo` does nothing (the port-count cycle's redo) | TestPropertyShortcutHandler, TestSceneUndoredo | 5 and 10 |
| `cycle-no-macro` | a property cycle over several elements has no macro | TestPropertyShortcutHandler | 2 |
| `prev-morph-no-macro` | the previous morph of several elements has no macro | TestPropertyShortcutHandler | 1 |
| `wireless-undo-no-rebuild` | an undo of a wireless change does not rebuild the channels | TestCommands | 3 |
| `drop-not-obsolete` | a dropped undo command is not marked obsolete | TestSceneUndoredo | 1 |
| `push-skip-after-undo` | a new command after an undo is not pushed (in `Scene::pushCommand`) | TestSceneUndoredo | 2, the existing test among them |
| `reopen-pushes-step` | loading a file records a step | TestSceneUndoredo | 4 |
| `long-history-limit` | the stack keeps at most 100 steps | TestSceneUndoredo | 1 |
| `abandon-not-obsolete` (5B) | `CommandUtils::abandon` does not mark the command obsolete | TestCommands | 6 |
| `truthtable-bounds-unguarded` (5B) | the truth table's position bound in redo is removed | TestCommands | 1 |

**Verification** (2026-11-02, after the comment cleanup, on `79fab62ee` and `bcf4ca94b`, MSVC relwithdebinfo, every target
built first): TestICRegistry 47 passed, 2 skipped; TestICInline 172; TestCommands 107; TestSceneUndoredo 91;
TestPropertyShortcutHandler 22; TestWorkspace 38; TestICController 55, 1 skipped; TestBewavedDolphinGui 89; TestMainWindowGui
164. Zero failures. Before the restack, the whole suite ran class by class (236 processes) with 7358 passed, 0 failed,
154 skipped, and the expected failures listed in the sections above.

**Limits.** No product mutation checks U2, the discard of the redo, which is `QUndoStack`'s own behaviour. The morph's
redo mutation stops its class at the first unhandled exception, so only that probe per class is shown to fail. The five
abandon tests and the bounds test are skipped by design in a debug build, where `CommandUtils::abandon` asserts first; a
death test would be needed to check that assertion, and QTest has none. Not run: the debug preset, GCC and clang (MSVC
only). The single-process run of `test_wiredpanda` without a class name runs nothing and reports success, so only
per-class runs count. An expected failure that predates this work, `TestPortReaders::testValueFailsOnANonLevel`
(commit `37d893101`), is outside this section and not reviewed.

## Log

- 2026-10-21: the plan file and this ledger written on `gpu-canvas`, every row `todo`.
- 2026-10-22: P4.12 added with P4.D5, at the user's ruling: an IC's pin order saved in the format and every element
  snapped on load, together, after plan 2 makes setting a position exact and every edit snap (P2.42).
- 2026-11-02: branch `04-ics-and-format` created from `03-element-types` (`2fe7e07f9`).
- 2026-11-02: P4.1 done (`a7e568afe`). P4.8 blocked: the undo restores the input and the wire in the session; the defect is the save-and-reopen case (the file's new definition comes back), which P4.6 addresses. §1.2's first claim needs the user's review before P4.5.
- 2026-11-02: P4.1 edge probes (`89c7bc825`): on plan 1 all four fail; on the tip the shrinking undo and the two-instance undo pass, the growing undo and the second reload's undo fail, marked for P4.5. P4.8 done: plan 1 shows §1.2's one-input undo, and the tip fixed that case in plan 2.
- 2026-11-02: P4.13 and P4.14 done (`79fab62ee`, `bcf4ca94b`): the undo and redo probes, the seam and its rollback probe,
  each probe mutation-checked. The defects they found are listed in the section above and none is fixed here.
- 2026-11-02: P4.5 done (`d28c93d0e`): the undo and redo of a reload restore the definition their snapshot captured. The
  D5 probes, unmarked, failed before the fix and pass after it. The fix stays on plan 4 with the snapshot mechanism it
  uses on the exception-boundaries branch (EB.4, `422eeba70`), which is not moved: the plan's own rule gives its
  inventory-found fixes their own rows, and that fix depends on its other commits.
- 2026-11-02: plan 2's watch fix re-lined the exception inventory's ICRegistry rows (`f9d083cbd` on 02c). The restack onto
  it reverted five of those rows on `03-element-types` and on this branch through its conflict resolution; they are
  re-lined on each (`950910817`, and this branch's re-line lands in a later commit). The inventory check passes on this branch.
- 2026-11-02: P4.6 done (`490983679`): the document keeps a copy of each file-backed IC's definition with the hash of its file; the four cases on open are tested.
- 2026-11-02: P4.2a done (`33cc4ceaf`): D7, a change to a nested IC file, reloads the instances that nest it; the probe's expected-failure mark is removed.
