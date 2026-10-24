# Plan 2: fixes to `master`

Every defect the branch found that exists on `master`, each fixed in a small commit of its own on plan 2's branch,
with a test that fails on `master`'s code first and every line of plan 1's goldens it changes named by this cause. A
plan that later rewrites the fixed code keeps the fix, pinned by its test, so the rewrite preserves behaviour. A
defect whose fix is itself another plan's decision is that plan's: a change to the document format (plan 4) or the new
simulation semantics (plan 6's switch); §8 lists them.

- **Depends on:** plan 1, landed. The plans run one at a time, in their order (the plans' rules, §1), so every fix
  has all of plan 1's goldens and helpers; §9 names the rows each rests on.
- **Lands before:** the Qt 6.8 floor's plan, then plan 3.
- **Branch and ledger:** `02-master-fixes`, from `01-characterization` once plan 1 has landed, each fix a slice of its
  own (the plans' rules, §1 and §3).
  `LEDGER_02_master_fixes.md` has a row for every fix, check and open question below, and a new row for every defect
  found on `master` later.
- **Rules every plan shares** are the plans' rules (`PLAN_rules.md`) and `CLAUDE.md`'s "Building and testing"; this
  file states plan 2's own. Its decisions are numbered P2.D1 on.

## 1. How a fix lands

- **The test first.** The fix's test is written and run on `master`, where it must fail; the failure goes in the
  commit message. Then the fix, in the same commit, so the commit is correct from its first line.
- **Golden lines named.** The golden lines a fix changes are regenerated in its commit, each named in the message by
  this cause. A golden that changes beyond what the fix explains stops the fix until the extra lines are explained.
- **Strings.** A fix that adds or changes a translatable string updates the translatable-strings golden in the same
  commit (plan 1's target, built before the suite); the `.ts` catalogs are refreshed at release, not here.
- **Kept by the plan that rewrites the code.** The fixes are grouped below by the plan that later rewrites their code;
  that plan keeps each fix, and its test stays green through the rewrite.
- **Reproduced first where unchecked.** Four defects were seen on the branch or read on `master` but not run there
  (§3.13 to §3.15, §2.7's chain); each is reproduced on `master` before its fix is written, and one that does not
  reproduce is closed in the ledger with what was run.
- **New defects.** A defect found on `master` later, by plan 1's goldens, its test readers or any plan's work, gets
  its row here when it is found, and is fixed by this file's rules.

## 2. In code no plan rewrites

### 2.1 Fuzz seeds never reach their targets

Seven harnesses (ICBlob, ICFile, ICRegistry, Clipboard, CodeGen, Undo, Structured) write the raw serialized seed behind
a file header that promises a compressed payload, so every seed is rejected at decompression and the fuzzers start
from nothing. Fix: serialize into a buffer and write it with `Serialization::writePayload()` behind the header;
Structured also needs its two IC blobs; the hand-built streams of v5.1 and older stay raw, as their versions require.

A seed check runs in each seeded harness's `LLVMFuzzerInitialize` (six harnesses; ICBlob builds its blob per input and
has no fixed seed): the header, the decompression and the metadata must decode, and the circuit must load into a scratch
workspace with at least one element, or the harness aborts naming itself and the reason. `readPandaHeader()` throws on
an unrecognized header instead of returning a null version, so the check's first header read sits inside a `try`: on the
branch it first did not, and an empty seed ended the harness through `std::terminate` without naming it. Each check is
shown to fail by a seed that fails it (no elements, an unknown header, an empty seed), and each harness runs once on its
seeds (`-runs=1`) as a start-up check. The harnesses run on the user's machine, overnight, through
`Tests/Fuzz/run-unattended.sh`, not in CI (P2.D10).

### 2.2 The fuzzers get no coverage feedback from the code under test

`master`'s `CMakeLists.txt` adds `-fsanitize=fuzzer-no-link` to each harness target alone, so libFuzzer sees the
harness's own edges: on the branch the core's harness saw 154 edges in three minutes, against 27,474 with the
libraries instrumented. Fix: `ENABLE_FUZZER` adds the flag project-wide. Shown by one harness's edge count before and
after. The harnesses run with `Tests/lsan_suppressions.txt`, which the scripts set; without it six harnesses report
leaks in `libfontconfig` and `libexpat`, with no application frames.

### 2.3 BeWavedDolphin's text export splits the rows by element count

`BewavedDolphin::saveToTxt()` passes `m_inputs.size()`, the number of input elements, to
`DolphinExporter::writeTruthTableText()` as the number of input rows, while the rows are per port (`m_inputPorts`), so
the split between inputs and outputs falls in the wrong row whenever an input element has more than one port. Fix: the
number of input ports. Test: a waveform of a circuit whose input element has several ports, exported as text, the
split after the last input port's row.

### 2.4 The Python MCP client rejects negative clock delays

`MCP/Client/domain/models.py` and `MCP/Client/protocol/commands.py` declare the clock delay `Field(ge=0)`, while the
server accepts [-1, 1]. Fix: the client takes [-1, 1] in both. Test: the client's own tests send a negative delay and
refuse one outside the range; ruff, pyright and mypy pass over the client, as CI checks them.

### 2.5 A save that fails part-way reports success on Qt 6.7.3

When a write fails (a full disk, a quota), 6.7.3's `QSaveFile::flush()` returns false and `error()` is `WriteError`,
yet `commit()` returns true and the truncated file replaces the target; 6.9.3's `commit()` returns false and writes
nothing. `master`'s commit-failure tests (in the workspace, Dolphin file, IC registry and IC tests), which cap the file
size at 16 bytes with `ScopedTinyFsizeLimit` (`RLIMIT_FSIZE`, `SIGXFSZ` ignored), fail on 6.7.3. Fix: one helper that
flushes and checks `error()` before `commit()` and otherwise leaves the file uncommitted, so its destructor discards
it, used by the five saves that call `commit()` directly: the workspace's save and autosave (`Workspace`),
`ICRegistry::extractToFile()`, the IC migration in `ICLoader` and `DolphinFile::save()`. A test commits over an
existing file and fails a commit over it, which keeps its contents; it fails on 6.7.3 with the check removed. `master`'s
CI has no 6.7.3 job, and the Qt floor that follows this plan (6.8) adds none, so this fix and the next are shown to fail
on a local 6.7.3 build. Both are right on every Qt (no commit after a failed write; a sink destroyed before its source)
and land as `master`'s fixes while it still declares Qt 6.2 (P2.D9).

### 2.6 A buzzer's destruction crashes on Qt 6.7.3

`~QObject` deletes children in creation order, the `ToneGenerator` before the `QAudioSink`; 6.7.3's sink, as it is
destroyed, disconnects from its source device, already gone, and crashes (`TestBuzzer::testPlayIdempotency`; gdb shows
`disconnectImpl` on the generator's address, then SIGSEGV in QtCore). 6.9.3 does not touch the source. Fix: a
destructor that deletes the sink first. `TestBuzzer` passes on 6.7.3 (five runs on the branch) and on 6.9.3. AudioBox,
a `QMediaPlayer`, is fine on both.

### 2.7 An MCP client that closes the connection leaves the application waiting on a question

When stdin reaches its end, `MCPProcessor` calls `QCoreApplication::quit()`, and `MainWindow::closeEvent()` then asks
"Are you sure?" whenever no tab has unsaved changes; nobody is left to answer, so a script that closed the application
waits until it is killed. Fix: the processor marks the main window as disconnected first, and a window with no unsaved
changes then closes without the question, while tabs with unsaved changes still get their save prompts. The defect is
known from reading the code: the branch's test checked only that a clean window asks the question on close and that
one marked disconnected closes with none. This test drives the whole chain, failing on `master` first: the processor's
input reaches its end, `quit()` runs, and a clean window closes with no question asked. The Windows stdin path goes
through the same function; the branch never compiled it on Windows, and CI's Windows jobs do.

### 2.8 `ItemWithId`'s destructor comment

The comment by its `LCOV_EXCL` marker gives the wrong reason: the deleting destructor never runs because no bare
`ItemWithId` exists, not because nothing deletes through a base pointer. A comment-only change, with no build or test.

### 2.9 The Python MCP client's models and the schema disagree with the server

Found while fixing §2.4, by comparing every command model of the client with `MCP/schema-mcp.json`, field by field
and bound by bound. The client has no model for seven commands the server takes (`export_arduino`,
`export_systemverilog`, `get_theme`, `set_theme`, `get_effective_theme`, `list_commands`, `describe_command`), so
it refuses to send them; `update_element`, a wrapper for `set_element_properties`, lacks seven of its fields, so the
client refuses them; `get_output_value` lacks the optional `port`; `create_waveform` has an `elements` field the
server does not take; `export_image`'s padding has no maximum, where the server clamps at 2000; a connection's ports
stop at 63, though an element has up to 256; positions are bounded at ±2³¹ where the server takes any finite number;
and `morph_element` takes any string as its type, where the schema lists the types. Each was checked against the
server, and the schema itself disagrees with the server in places: the two property commands' parameters differ
(frequency's minimum 0 where the server refuses 0, a volume unbounded in one and [0, 1] in the other, `update_element`
without `wireless_mode` or a required `element_id`), and `extract_ic`'s `blob_name` and `save_circuit`'s filename
may be empty where the server refuses an empty one. Fix: the schema follows the server, the two property commands
taking equal parameters; every client model follows the schema, `update_element` taking `set_element_properties`'
parameters, the commands' models in one map the client parses by. Test: every schema command has a model in that map,
with the same fields, required fields, numeric bounds, minimum lengths and enumerated values; shown to fail by a field,
a bound, a command and an enumeration removed in turn.

### 2.10 `get_output_value` on an input answers an error

The schema requires a `status` in `get_output_value`'s result, and the server checks every response against it before
sending; an input element's read set only its value, so reading any switch, button, rotary or clock answered an
internal validation error. It also ignored the port, reading a rotary's port 0 whatever was asked. Fix: an input reads
the state of the port asked for, range-checked, as high or low. Tests: plan 1's MCP responses golden also checks each
response against the schema (on `master` only the inputs' reads fail it); an input's read per port. Golden: the eight
reads of the four input types gain their status.

### 2.11 The MCP client suite passes with failed checks

A check helper records a failure in the suite's error list and returns false; a test that drops the result passes,
and the suite reports no failure: CI's run of the commit plan 1 landed on printed §2.10's error, then "Failed: 0".
The dropped results: four `assert_success` calls on a simulation's start in the sequential tests (which then return
`all_passed`, still true, when the start fails), four `verify_output_value` calls in the simulation and workflow
tests, and a connection's `assert_success` in the circuit builder. Fix: a test whose checks recorded a failure fails,
whatever it returns, which covers the builder's; and each test uses its checks' results. Shown to fail with §2.10's
fix removed, by the workflow test's result and, with that result dropped again, by its checks alone.

Two more forms of the same defect (P2.47a, P2.47b). A test that prints a failure ("❌") and goes on, with no result
and no recorded error: a scan of every such print found 45 candidates. Each real one fails its test. The performance
tests' wall-time and memory thresholds print one too, and enforcing them would put absolute wall-time bounds in
tests, so each becomes a recorded limitation. And the suite's limitations ("📋", monitored, not enforced) are never
recorded: `record_limitation()` is called nowhere, though 38 sites print one and pass, and at least one hides a test
that cannot pass, a clock-gating test that never toggles its master clock and so reads its flip-flop low. Each site is
a real limitation of the engine, recorded; a defect of the test, fixed; or a failure, counted.

## 3. In code plan 3 rewrites

### 3.1 SR and T flip-flops ignore every input while one input is open

`SRFlipFlop`'s S and R and `TFlipFlop`'s T default to Unknown, against their comments, which say Inactive. An Unknown
input fails the input check, so with S or R open an SR flip-flop ignores everything (S high on a clock edge with R open
leaves Q at 0, and with S open ~Preset does nothing), and with T open a T flip-flop ignores even ~Preset and ~Clear.
Across every type, port count and orientation they are the only three optional inputs whose default is Unknown. Fix:
Inactive, as the comments say and as the Arduino export already reads an open input (P2.D2). Tests: an SR flip-flop set
with R open and reset with S open; a T flip-flop preset, clocked (hold) and cleared with T open. Shown to fail by each
default put back to Unknown in turn. Goldens: the port layout's S, R and T default fields (48 lines on the branch);
the scene snapshot of `display-3bits-counter`, which records its three T flip-flops' default (their T is wired, so
only that field changes); any trace with S, R or T open.

### 3.2 The loader accepts port counts above a type's maximum

The loader adds every port a file lists and trims only down to the type's minimum. Measured on `master` by rewriting a
saved element's port lists: a Demux with 8 outputs and 2 inputs aborts a debug build when simulated (`QList::at` out of
range), a truth table with 9 inputs aborts on its key, and a release build reads out of bounds; a Demux with 2 outputs
and 4 inputs loads two unnamed required inputs. No release wrote such files: from 1.9.1 to 5.2.2 no type's maximum
shrank, and a Demux's inputs always followed its outputs. Fix: the serializer refuses a count above the type's maximum
(an IC is exempt, its counts coming from its definition, loaded after the ports), and the Demux's load refuses inputs
other than its data input and the select lines its outputs need. Tests: a truth table at its maximum loads and one
input or output more is refused; a Demux missing select lines, and one with surplus inputs, are refused. Shown to fail
by each check removed in turn. Every example and fixture still loads. The three new messages are translatable, so the
strings golden gains them in the same commit; on the branch they first did not, and a stale extraction hid it.

### 3.3 The AudioBox's audio file, chosen in the editor, is not undoable

`ElementEditor::audioBox()` calls `setAudio()` directly, so no command is pushed. Fix: snapshot the element and push
an update command, the pattern MCP's property setter uses. Test: the push, its undo and its redo; it fails with the
push removed. Golden: the editor-writes row of the AudioBox (`pushed=[]` on `master`).

### 3.4 A lowercase trigger key typed in the editor pushes two undo commands

`triggerChanged()` sets the uppercased text, which calls it again, and `apply()` pushes a command even when nothing
changed, so the second command does nothing. Fix: `triggerChanged()` returns after setting the uppercased text, so only
the call that setting it makes applies. Test: one command, and one undo restores the trigger; it fails on `master`
with two. Golden: the editor-writes golden's three Trigger rows. §3.9 then stops `apply()` pushing an empty command.

### 3.5 Mux, Demux and the truth table offer a custom appearance they never draw

Their bodies are drawn in code (Mux and Demux since they became variable-sized), so a custom appearance is stored and
pushed but never shown. Fix: stop offering it in the editor, the context menu and MCP, as the IC does, since a body
sized by its ports cannot take a fixed picture; loading a file that holds one still accepts it, since loading does not
read the flag (P2.D3). Test: the types whose bodies are drawn in code offer no appearance, and it fails naming Mux when
its flag is put back. Goldens: the three types' appearance entries in the property-kinds golden, their appearance rows
in the editor writes, and their appearance fields in MCP's property responses.

### 3.6 Most input operations are not undoable

Only `InputSwitch`'s mouse toggle pushes an update command, from the element itself, which reaches the scene and the
commands; the rotary's click and the switch's and rotary's trigger keys change saved state with no command, and since
the undo stack's clean state is the document's modified flag, the file is not even marked modified. Fix: the scene
records an operation: it snapshots the elements a press or a trigger key goes to, runs the operation and pushes one
update command for those whose saved state changed, restoring the selection the command's first `redo()` sets (a
trigger key selects nothing); a push button's press is transient and not recorded. A press's elements are those Qt may
give it to: the item holding the mouse and the items under the pointer, found as Qt's `itemsAtPosition()` finds them
(through the view's transform), each standing for its element. A double-click is not recorded: every input has a
label, so a double-click opens the label's editor and changes nothing saved; nor is a trigger key's release, which
only releases a button. `InputSwitch` then no longer includes the scene or the commands. Tests: a switch's and a
rotary's click; a rotary's click through a zoomed view; a left press going to the switch holding the mouse; a trigger
key over two switches and a rotary; a button not recorded.

### 3.7 Themed memory pictures go stale after a theme switch

`DLatch`, `SRLatch` and the four flip-flops seed their default picture slot with the theme's path at construction, and
their `updateTheme()` loads the picture path directly, so after a switch any refresh (every load, so every undo and
redo of an edit) draws the light picture again, and a switch draws the dark default over a custom picture while the
element still lists the custom file. Second cause: an undo snapshot restores a resource slot verbatim, so after a
switch an undo brings the old theme's path back as a custom appearance. Fix: the theme update points the default slot
of a type with no built-in appearances at its picture path (a custom slot stays), replacing the six types' own
`updateTheme()`; and a snapshot gives a resource slot the type's current built-in appearance, as a file already does.
Tests, each over all six types: a refresh after a switch; a custom appearance through a switch; an undo after a
switch. All three fail on `master`, the undo test without the snapshot part alone.

### 3.8 A morph's undo loses properties and wires

`MorphCommand::undo()` rebuilds the original types and copies back only what the morphed type shares: a clock morphed
into a switch loses its frequency, delay and lock; any element its picture, wireless mode and audio file; and inputs
the morphed type cannot have are lost with their wires (a 4-input And morphed through a Not comes back with 2). Fix:
redo saves the originals and undo loads them, then moves the wires. Tests: a clock through a switch and back; a 4-input
And through a Not and back, with its wires. Golden: the stacking order after a morph's undo (plan 1's §3.8), if the
restored element's place changes.

Pitfall from the branch: an undo whose saved element holds a custom picture that no longer loads threw, and then leaked
the elements it had rebuilt. The rebuilt elements are owned until they replace the old ones, so a throw leaks nothing;
what the restore shows for the missing picture is `master`'s rule for any snapshot restore, which §12 checks first and
plan 3 settles (a slot whose picture no longer loads shows its own default).

### 3.9 The property editor

Every field applies on each change signal and the update command has no `mergeWith()`, so there is one undo entry per
keystroke (a label, a frequency, a slider's every step); a label edit on a multi-selection writes the other fields too
(two clocks' differing delays set to 0, buzzers' volumes to the slider's); and undoing a lock or a value restores
nothing, since the snapshot is taken after the change. Fix: a property command on `master`'s own setters, which holds
each element's value of one property before and after, and merges with the session's next command on the same
elements and property, leaving no step when merged back to the start; each field sets its own property through it;
typed fields and sliders merge until `editingFinished`, `sliderReleased` or a refill of the fields; combos, the lock,
the value and the sound file are one step each; the appearance stays a snapshot step. The editor's destructor
disconnects its fields, since a focused field emits `editingFinished` while being destroyed. Plan 3 moves the command
onto the schema.

Pitfall from the branch: every command of an edit session names the same elements, an unchanged one included. When
the command dropped the elements whose value did not change, a slider dragged over two clocks with delays 0 and 1/8
made two undo steps, since the first command named one clock and the next both, so they did not merge. Tests: typing
into a field and dragging a slider over two clocks with different delays, and over two switches, each one step; a
label edit leaves the other fields; undoing a lock and a value restores them. Each fails on `master` first.

### 3.10 Duplicate wireless Tx labels

The inline label editor accepts any label, and two selected nodes edited into one Tx channel together pass. Fix: one
rule in the model for every writer (P2.D4): it takes the label and mode each edited node would have and compares every
edited node with the others as the edits leave them; a refused edit pushes nothing and says why in the status bar.
Duplicates that come from a file or a paste only warn and refuse no other edit. Tests: the rule's cases, refused and
allowed; the inline editor's refusal; the property editor's, a node selected with non-nodes included.

### 3.11 MCP's `set_element_properties`

It applies the label and colour before validating the frequency, clock delay or volume, so an error leaves them
applied with no undo entry; it skips silently a property the type cannot hold; it reports a clock frequency whose half
period is under 1 µs as set and ignores it; it throws on a picture that does not load after earlier changes are
applied; and it takes any colour and any trigger text. Fix: a request changes nothing unless every value passes (the
property exists on the type, has the right kind, is in range, is accepted by the type, an appearance index is in range
and comes with an appearance, a picture loads, no second Tx would share a label, a colour is one of the five the
editor offers, a trigger is one the editor accepts: P2.D6); then one update command, with the wires a change severs in
a macro, or none when nothing changed, and the old and new values read back. Tests: each refusal changes nothing and
pushes nothing; a valid request is one undo step. Golden: MCP's responses for the refused requests. Plan 3's schema
then holds these rules once.

### 3.12 The lock's tooltip misstates what a lock does

It says a lock prevents the element "from being toggled by clicking it during simulation" (`ElementEditorUI`). A lock
also blocks the element's trigger key (the scene's key press and release skip a locked input) and stops a locked clock
(`Clock::updateClock()` returns at once), and a clock has no click action, so on a clock the text hides what the lock
does. Fix: the tooltip says that during simulation neither a click nor the element's trigger key operates a locked
element, and that a locked clock stops (P2.D7). One string, so the strings golden changes in the same commit.

### 3.13 A display's small custom picture (reproduced first)

On the branch, a display whose custom picture was smaller than its segments' 64×64 box drew its lit segments outside
its bounds, which the device cache then cut off; the fix made the box part of the body's bounds. Very likely on
`master`, not checked there. Reproduced first: a 7-segment display with a smaller custom picture, lit, its lit segments
inside its bounds; then the fix, with that test.

### 3.14 A flipped IC reopened from a file (reproduced first)

On the branch a flipped IC reopened from a file landed 64 px off: an unloaded IC turns about a different centre from a
loaded one. Plan 1's port-layout golden shows the cause on `master` (orienting an IC and then loading it differs from
loading it and then orienting it); the reopen itself was not checked there. Reproduced first: an IC flipped, saved and
reopened keeps its ports' scene positions; then a fix that turns an unloaded IC about the centre a loaded one uses.

### 3.15 An empty Text's hint keeps the language it was made in (reproduced first)

Read on `master`: an empty Text sets its hint ("Double-click to add text") once, at construction, and nothing sets it
again; `GraphicElement::retranslate()`, which the main window runs on every element at a language change, is not
virtual. Reproduced first: a Text made in one language shows its hint in another after a switch. Fix: the Text
retranslates its hint, through a virtual `retranslate()`, which §4.1 needs too.

### 3.16 The trigger field shows its "many triggers" placeholder uppercased

Found with §3.4: the trigger field uppercases whatever text it gets, so when a selection's elements have different
triggers, the placeholder the editor fills in ("<Many triggers>", translated) shows as "<MANY TRIGGERS>". Fix: the
field uppercases a typed key, not its placeholder. Test: a selection of two switches with different triggers shows the
placeholder as the editor sets it, and pushes nothing; it fails on `master`.

## 4. In code plan 4 rewrites

### 4.1 An IC's tooltip comes back after a language change

Loading an IC clears its tooltip, since the hover preview names it (`ICLoader::loadFile()`, `loadFromBlob()`);
`GraphicElement::retranslate()` sets it back to the translated type name, so from then on the tooltip and the preview
both show. Fix: `retranslate()` is virtual and an IC's keeps a loaded IC's tooltip clear; the loader sites that cleared
it give way to the one place every load ends at, which clears it once and marks the IC loaded. An IC with nothing
loaded keeps its translated type name. Test: an IC loaded from a file and one from a blob keep no tooltip after a
retranslation; an unloaded one keeps its type name. The other three defects in this code are plan 4's (§8).

### 4.2 Embed and extract act on whichever element comes first

`ICController::selectedIC()` takes the first of the selected elements, in Qt's set order (§7.4), and answers nothing
unless that one is an IC. With an IC and a gate selected, Embed IC and Extract to file act in one run and do nothing in
another; with two ICs from different files, which one is embedded varies from run to run, and the other never is. Fix:
Embed acts on every distinct file among the selected file-backed ICs, Extract on every distinct blob among the
selected embedded ICs, each as one undo step (one save dialog per extracted blob); a selection without an IC does
nothing. Tests: two ICs from two files and a gate, embedded in one step and undone in one; the same for extraction;
a gate alone changes nothing.

## 5. In code plan 6 rewrites

### 5.1 Every rebuild of the simulation gives every flip-flop a rising edge

Clocks start HIGH at simulation start, and a flip-flop whose last clock was low sees an edge. A rebuild runs
`Simulation::initialize()`, which gives every clock a fresh HIGH start through `Clock::resetClock()`, as a restart
does, while the flip-flops keep their last clock sample. Fix: a rebuild keeps each existing clock's level and phase, as
a resume after a pause does; only a restart starts the clocks afresh. Test: a flip-flop clocked low, a structural edit,
and no edge. Pitfall: plan 1's fast-path test of a clock's phase (its §4.5) can then no longer tell the fast path from a
rebuild, since both keep the clock; it observes the rebuild itself instead, and is shown to fail again with the fast
path removed. Plan 1's traces freeze clocks without the timer and are checked unchanged.

### 5.2 An in-place reload leaves a flip-flop's wired ~Preset and ~Clear at their defaults

`Port::setDefaultStatus()` sets the status of a connected input too, and an element sets its ports' defaults each time
it lays them out; the simulation pulls an input from its driver only while running, so after an in-place reload (an
update command) a flip-flop's wired ~Preset and ~Clear read their defaults while the simulation is stopped, until a
wire changes. Fix: setting a port's default leaves a connected input on its wire, as its documentation says, and the
input takes its default when the wire goes. Tests: a port's default set while connected, then the wire removed; a
flip-flop's wired ~Preset through a reload.

### 5.3 The priority walk on a graph with cycles (measured first)

`legacyCalculatePriorities()` (`App/Core/Priorities.h`), which orders cyclic graphs, pops the wrong node when a node
has both a successor on the stack and an unvisited one: it pushes the unvisited successor, then assigns the current
node and pops the stack's top, which is that successor, so a producer can rank below its consumer, and the branch it
marks unreachable is reached. `master`'s comment on the walk asks that its order stay as it is unless the settling
semantics are redesigned, since gate-built latches settle from an undetermined start by it. Measured first: the fixed
walk against plan 1's traces of every example, fixture and IC component. With no trace changed, the fix lands with a
test of the walk; with traces changed, the user decides between the fix with each change named and leaving the walk
to plan 6's switch, which replaces it (P2.36).

## 6. In code plan 8 rewrites

### 6.1 SystemVerilog export of a truth table does not compile

`SystemVerilogCodeGen` names output 0 of every truth table `<type>_output`, the same in every module, and declares it
once per output of each table, while outputs 1 and up keep per-element names it never declares; iverilog reports
"already declared". Fix: a truth table's outputs keep the per-element names every element's outputs get, each declared
once. Test: two tables, one with two outputs, compiled under iverilog, their AND, OR and XOR rows checked in
simulation on all four input rows. No golden holds a truth table; the Arduino generator names each output per element
and is unaffected.

### 6.2 The exporters name signals and elements from translated names

Both generators read an element's `objectName()` (set through `setPortName()`) and its ports' displayed names, so
their output depends on the interface language. On `master` every port name is a literal except the segment displays'
(`Display7`: `"G (" + tr("middle") + ")"`, and the 14- and 16-segment displays alike), and an IC's pins are named
from the sub-circuit's ports they stand for (`buildPortLabels()` in `ICLoader`), so a pin standing for a display's
port carries its translated name too. Fix (P2.D8): element names from the factory's untranslated name
(`ElementFactory::typeToText()`); a port holds a name in the source language beside its displayed one, which the
three segment displays set from their untranslated text and which is the name itself for every other port; an IC's
pin takes a source name built as its displayed one is, from the source names of the ports it stands for; and the
generators name signals from source names, so the English output is unchanged. The file header's pin names stay as
they are, since changing them changes the format (plan 4's, §8). Test: one circuit, with each segment display and an
IC holding a 7-segment display, exported under two interface languages gives the same text from each exporter.
Golden: plan 1's generated-code goldens under the tagging translator become equal to the English output. Plan 3 moves
the source names into its port descriptors, and the displayed names follow the language there.

Pitfall from the branch: its first change named elements alone (`bea9b2e95`); the ports' source names came next
(`e187dad16`), and the IC's pins still took the sub-circuit port's translated name, so the code generated for an IC
followed the language it was loaded in, until `736d79a2e` carried the source name through to the pin.

### 6.3 The exporters and the simulation disagree on an Rx node that a wire also reaches

`master`'s simulation lets the wireless Tx override the wire (it repoints the Rx's predecessor;
`TestSimulation::testWirelessOverridesPhysicalWire` pins it), while both exporters resolve a wireless Rx only when its
input has no connection, so they export the wire's driver. Measured on the base of the earlier `temporal-engine`
branch, with the wireless source high and the wired one low: the simulation's LED 1, the SystemVerilog `assign
led5_1 = input_switch1_phys`, the sketch `aux_node_2 = input_switch1_phys_val`. Fix: both exporters resolve an Rx from
its Tx whether or not a wire also reaches it, as the simulation does (P2.D5). Test: an Rx reached both ways exports
its Tx's driver from each exporter. Under plan 5's rules the net has two drivers and reads Error, and plan 8 makes
both exporters refuse it.

## 7. In code plan 9 rewrites

### 7.1 The wheel zoom does not keep the point under the cursor

In `GraphicsView::wheelEvent`, `zoomIn()` calls `scale()`, which pivots about the view's centre (`AnchorUnderMouse`
applies only to Qt's own wheel handler, which `wheelEvent` replaces), and `centerOn()` then puts the post-zoom point
under the cursor at the viewport's centre instead of under the cursor. Measured: the scene point under the cursor moved
from (-192, -142) to (-307.2, -227.2), where the centre that keeps it is (32, 26); the scroll clamp at the scene rect
makes the error larger near the edges. Fix: centre the pre-zoom point back under the cursor at the new scale. Tests:
the point under the cursor stays under it; a wheel zoom from each viewport corner near the scene's edge keeps the view
inside the scene rect. Golden: the view golden's four wheel rows (two on the branch), no other. Keyboard zoom pivots
on the centre and stays so.

### 7.2 The hit test answers the bottom item

`Scene::portAt()`'s comment says topmost, but it walks Qt's `items()` in ascending stacking order and takes the port
drawn below; `itemAt()` does the same. With overlapping elements a press goes, through Qt, to the top one while the
scene's lookup answers the bottom one: a drag of the top element snapshots the bottom one and pushes no move command,
so the move cannot be undone (undo count 0). Fix: the hit test answers the topmost, an exact hit before a nearby one,
which the context menu and the morph's re-selection use too. Tests: a drag of the top one of two overlapping elements
is one undo step; where a point is within reach of two ports, the one drawn on top answers. Golden: the hit grid's
rows where items overlap.

### 7.3 MCP's image export cannot report an empty scene

Its "Scene is empty" error is unreachable: Qt's `itemsBoundingRect()` counts every item in the scene, the rubber band
among them, which is in the scene from the start, empty at the origin, and stays there between selections, hidden, with
its last rectangle. Fix:
`Scene::circuitBoundingRect()`, the union of the elements' and the wires' bounds with their children, which the export
reads. Test: a new, empty circuit, and the same after a rubber band dragged over it, each get the error and no file.

### 7.4 What acts on a selection follows the hash seed

`QGraphicsScene::selectedItems()` returns its items from a `QSet`, so their order follows the per-process hash seed and
the items' addresses. Found by plan 1's snapping-path golden (P1.12): the same copy and paste gives the copies other
ids in another run, each copy in the same place. The same order reaches every caller that walks the selection: copy,
cut and duplicate (`ClipboardManager` serializes in it, so the paste's ids follow it), the clone drag, the delete and
its undo, distribute (its `std::sort` is unstable, so ties fall in it), `ICController`'s first selected IC and the
element editor's first element. Fix: the scene's selection accessors return the elements in id order, which P2.43
makes the stacking order too, and every caller in the application uses them, never Qt's; distribute sorts with
`std::stable_sort`. Test: in one process, about fifty selected elements copied and pasted, deleted and undone, and
distributed with ties, each compared with the order by id; shown to fail with the accessors' sort removed, since fifty
items in a set are out of id order in every run. Golden: plan 1's paste row then prints the copies' real ids instead
of "new" (P1.12).

### 7.5 One rule for where an element goes

`GraphicElement::itemChange()` snaps a position to half the grid (8) only while the element is in a scene, so whether a
position snaps depends on whether it is set before or after `addItem()`. Load, the MCP's `create_element` and a morph's
transfer set it before and keep it exact; a paste, `instantiate_ic` and a rotate's constructor set it after and snap.
An element off the grid exists: releases before 4.0 saved arbitrary positions (279.333, 311.5), and 4,271 of the 4,504
elements of the 73 IC components are off it, written by the MCP generators. Undo restores through `setPos()`, so an
off-grid element moved, rotated or flipped comes back snapped, not where it was: the undo does not restore. Fix: one
snapping helper; setting a position is always exact, so a load, a deserialization, an undo and a redo put an element
where it was; `SceneInteraction` owns the element drag, and every edit snaps explicitly where it decides a place: Qt's
drag, the nudge (`Scene.cpp:1250`), align and distribute (`:937`), the rotate's constructor and redo
(`Commands.cpp:341`, `:387`), the flip (`:948`), the palette drop (`SceneDropHandler.cpp:85`), the clone drop (`:117`),
the add without a drag (`:165`), paste and duplicate (`ClipboardManager.cpp:323`), the split's node (`:648`, its step of
16 named), and the MCP's `create_element` (`ElementHandler.cpp:122`), `move_element` (`:273`) and `instantiate_ic`
(`ICHandler.cpp:213`). The move, rotate and flip commands restore exactly; the morph's is guarded by a test. The 73
components are regenerated on the grid by their generators, each component's pins in the same order. Tests: an off-grid
file from an older release through move, rotate, flip and morph with undo and redo, each restored exactly; `setPos()`
does not snap. Golden: plan 1's snapping-path rows of the undos from off-grid starts (P1.12), no other. Snapping on load
is plan 4's (§8), since it can reorder an IC's pins.

### 7.6 An undo puts a restored element on top

Elements stack in id order in all 27 loaded circuits; a load assigns ids in file order, which is the stacking order. An
element restored with its old id (a delete's or a cut's undo, a morph and its undo) is added last and so drawn on top
of every element, though its id places it below. Fix: one rule in `Scene::addItem()`'s branch for a preassigned id puts
the element back at its place in the stacking order, before the first element with a larger id. `stackBefore()` costs a
walk of the siblings (`qgraphicsitem.cpp:4675`), so the undo of a large delete on the 8,000-element circuit is measured
under callgrind, and restored in one batch if it grows quadratically. Tests: the undo of a delete, a cut and a morph,
each of the bottom of three overlapping elements, puts it back at the bottom; a delete of a thousand elements and its
undo give the order before. Golden: plan 1's order after the undo (P1.10).

### 7.7 The scene's bounds count the rubber band

Found with §7.3: every other reader of the scene's bounds read `itemsBoundingRect()` too. The scene's cached bounds
(the minimap's frame and zoom-to-fit), the scene rect `Workspace` sets after a load and a resize, and the image
`CircuitExporter` renders all reached the origin, where the band sits, empty, from the start, and grew to take in a
band dragged far from the circuit. Measured on `master`: a 199×193 image became 3329×3329, the scene rect -256,-256
3584×3584, and zoom-to-fit went from 3.81 to 0.134. Fix: each reads §7.3's circuit bounds, and the cache is named
after what it holds. An empty circuit has no bounds, and a null scene rect hands the rect back to Qt, which grows it
over every item, the band among them: a load of an empty circuit (from a file or a blob) and a resize of a scene with
no view keep the rect they have. Tests: a rubber band dragged far from a small circuit leaves the rendered image's
size, the scene rect after a resize and zoom-to-fit's scale as they were, each failing on `master`; an empty circuit's
loads and a viewless resize keep the scene rect, each failing with its null check removed; a port's hover label, which
the scene also holds, stays out of the circuit's bounds. Golden: the view golden's fit row and its four last wheel rows,
which no longer reach the origin.

## 8. Not plan 2's

Each of these exists on `master`, and its fix is another plan's decision:

- **Undoing an IC's reload after its file changed restores nothing** (the undo reloads the file), **IC pin names are
  saved in the interface language**, and **an IC's pin names and its file header's are built by two rules**: each fix
  changes the format, so each is plan 4's.
- **An element loaded off the grid stays off it.** Snapping on load is plan 4's, together with the order of an IC's
  pins saved in the format: today the pins are ordered by their inner elements' positions, and snapping collapses two
  close rows into one, which can swap two pins (measured: 0 of the 85 off-grid files reorder, while a control that
  collapses every row reorders 82 of 82). §7.5 keeps every loaded position exact until then.
- **A clock above 500 Hz runs slow**, and **the tick engine contradicts itself on invalid inputs**: the new semantics',
  with plan 6's switch; plan 5 lists both among its differences from `master`.
- **Two drivers on one net**: under plan 5's rules they read Error, and plan 8's exporters refuse such a net; until
  then §6.3's exporters follow the Tx.

## 9. The fixes and the goldens each touches

| ID | Fix | Plan 1's rows it rests on | Goldens it changes |
|---|---|---|---|
| P2.1 | Fuzz seeds (§2.1) | the helpers only | none |
| P2.2 | Fuzz coverage (§2.2) | the helpers only | none |
| P2.3 | Dolphin's text export (§2.3) | the helpers only | none |
| P2.4 | The Python client's delays (§2.4) | the helpers only | none |
| P2.5 | A failed save on Qt 6.7.3 (§2.5) | the helpers only | none |
| P2.6 | The buzzer's teardown on Qt 6.7.3 (§2.6) | the helpers only | none |
| P2.7 | An MCP client closing the connection (§2.7) | the helpers only | none |
| P2.8 | `ItemWithId`'s comment (§2.8) | the helpers only | none |
| P2.45 | The client's models and the schema (§2.9) | the helpers only | none |
| P2.46 | An input's `get_output_value` (§2.10) | P1.15 | the eight input reads of MCP's responses |
| P2.47 | The client suite's dropped checks (§2.11) | the helpers only | none |
| P2.47a | The client suite's printed failures (§2.11) | the helpers only | none |
| P2.47b | The client suite's limitations (§2.11) | the helpers only | none |
| P2.9 | SR and T defaults (§3.1) | P1.4, P1.6, P1.13 | port layout, one scene snapshot, any trace with S, R or T open |
| P2.10 | The loader's port counts (§3.2) | P1.8, P1.17 | the strings golden's three messages |
| P2.11 | The AudioBox's undo (§3.3) | P1.15 | one editor-writes row |
| P2.12 | The lowercase trigger (§3.4) | P1.15 | three editor-writes rows |
| P2.13 | No appearance where none is drawn (§3.5) | P1.15 | three types' property kinds, editor writes and MCP responses |
| P2.14 | Input operations undoable (§3.6) | P1.9 (the press path it wraps) | none expected |
| P2.15 | Themed memory pictures (§3.7) | P1.4 | none expected |
| P2.16 | A morph's undo (§3.8) | P1.10 | the order after a morph's undo, if it moves |
| P2.17 | The property editor (§3.9) | P1.15 | none expected (unchanged on the branch) |
| P2.18 | Duplicate Tx labels (§3.10) | P1.15 | none expected |
| P2.19 | MCP's property setter (§3.11) | P1.15, P1.17 | MCP's responses to refused requests; any new message |
| P2.20 | The lock's tooltip (§3.12) | P1.17 | one string |
| P2.21 | A display's small picture (§3.13) | P1.4 | as the reproduction shows |
| P2.22 | A flipped IC reopened (§3.14) | P1.6, P1.8 | as the reproduction shows |
| P2.23 | An empty Text's hint (§3.15) | the helpers only | none expected |
| P2.49 | The trigger placeholder's case (§3.16) | the helpers only | none expected |
| P2.24 | An IC's tooltip (§4.1) | the helpers only | none expected |
| P2.25 | The rebuild's edge (§5.1) | P1.13, P1.20 | plan 1's fast-path test, reworked |
| P2.26 | A connected input keeps its wire (§5.2) | P1.5 | none expected (unchanged on the branch) |
| P2.27 | The priority walk (§5.3) | P1.13 | measured first |
| P2.28 | SystemVerilog truth tables (§6.1) | the helpers only | none (no golden holds one) |
| P2.29 | Untranslated names in exports (§6.2) | P1.16 | the tagged lines become English |
| P2.30 | An Rx reached both ways (§6.3) | the helpers only | none expected |
| P2.31 | The wheel zoom (§7.1) | P1.12 | the wheel rows |
| P2.32 | The hit test's topmost item (§7.2) | P1.9 | the overlapping rows of the hit grid |
| P2.33 | MCP's empty-scene error (§7.3) | the helpers only | none (no golden exports an empty circuit) |
| P2.41 | The selection in id order (§7.4) | P1.12 | the paste row's ids |
| P2.42 | One rule for where an element goes (§7.5) | P1.10, P1.12, P1.14 | the undo rows from off-grid starts; the components' positions, pins unchanged |
| P2.43 | A restored element's stacking place (§7.6) | P1.10 | the order after a delete's and a morph's undo |
| P2.48 | The scene's bounds without the rubber band (§7.7) | P1.12 | the view golden's fit row and four wheel rows |
| P2.44 | Embed and extract on every selected IC (§4.2) | the helpers only | none expected |

"None expected" means the branch saw no golden change, or the fix touches nothing a golden prints; the fix's commit
says what changed.

## 10. Decisions

Each ends with its basis: measured or tested (a run backs it), read (the code, with nothing run), or a ruling (the
user's choice between workable options).

- **P2.D1. Every defect verified on `master` is plan 2's**, one small commit each, on all of plan 1's goldens,
  unless its fix is another plan's decision (§8); the plan that later rewrites the code keeps the fix. Basis: the
  user's rulings of 2026-10-21 (this rule, and the plans run one at a time, plan 2 starting once plan 1 has landed).
- **P2.D2. An open S, R or T reads Inactive**, as the code's comments say and as the Arduino export reads an open
  input. Basis: measured on the branch (each type ignored its inputs with one open); accepted by the user with the
  restart's write-up, 2026-10-21.
- **P2.D3. Mux, Demux and the truth table stop offering a custom appearance** rather than drawing one: a body sized by
  its ports cannot take a fixed picture. Loading still accepts one. Basis: tested on the branch; accepted with the
  write-up, 2026-10-21.
- **P2.D4. One rule against duplicate Tx labels, for every writer**: an edit that would give two Tx nodes one channel
  is refused wherever a label is set, saying why in the status bar; duplicates from a file or a paste only warn. Plan
  3's schema keeps the rule. Basis: a defect verified on `master`; the user's ruling of 2026-10-16.
- **P2.D5. The exporters follow the Tx**, as the simulation does, until plan 5's rules make two drivers read Error and
  plan 8 makes the exporters refuse them. Basis: measured on the base of `temporal-engine`; accepted with the
  write-up, 2026-10-21.
- **P2.D6. MCP takes only what the editor offers**: a colour among the five the editor lists (White, Red, Green, Blue,
  Purple) and a trigger the editor's field accepts (one letter, digit or space, stored uppercased). The branch's MCP
  fix took any single key without modifiers, wider than the editor; one rule for both writers is the editor's, until
  plan 3's schema holds it. Basis: read on `master` (`Led::setColor()` keeps any string; the editor's validator).
- **P2.D7. The lock's tooltip says what a lock does**: neither a click nor the trigger key operates a locked element
  during simulation, and a locked clock stops; plan 6's switch adds only the sentence on what an unlocked clock shows.
  Basis: read on `master`; the user's ruling of 2026-10-21.
- **P2.D8. Exported names do not depend on the interface language**: elements by the factory's untranslated name,
  ports and an IC's pins by a source name each port holds beside its displayed one, which plan 3's descriptors then
  take over; English output unchanged. Names by position were rejected: they would rename every port in every
  generated file, and plan 3 would rename them again. Basis: tested on the branch (`bea9b2e95`, `e187dad16`,
  `736d79a2e`); the user's approval of 2026-10-21.
- **P2.D9. The two Qt 6.7.3 fixes are plan 2's**, though the Qt 6.8 floor that follows this plan drops 6.7: the save
  loses data on `master` today, which declares Qt 6.2, and each fix is right on every Qt (no commit after a failed
  write; a sink destroyed before its source device). Whether 6.8 shows either defect is recorded for the release notes
  (P2.34). Basis: the user's rulings of 2026-10-21 (every defect verified on `master` is plan 2's; the floor at 6.8,
  right after plan 2).
- **P2.D10. The fuzz harnesses run on the user's machine, overnight, not in CI.** The fuzzer preset (libFuzzer with
  ASan and UBSan) is built there and the harnesses run through `Tests/Fuzz/run-unattended.sh`. A slice that adds or
  changes a harness (this plan's §2.1 and §2.2, plan 3's round trip, plan 4's IC files, plan 6's lockstep fuzzer) is
  finished after the user's overnight run of it; a crash becomes a row of the plan whose code it reaches, its input
  kept under `Tests/Fuzz/regressions/`. CI's workflows keep ignoring `Tests/Fuzz/`. Basis: the user's ruling of
  2026-10-21 ("i dont think its necessary/useful to run fuzzer in CI. i can run it overnight on my pc").
- **P2.D11. Everything that acts on a selection acts in id order**, through the scene's accessors, never Qt's set
  order (§7.4). Basis: measured on `01-characterization` (the paste's ids differ between runs); the user's ruling of
  2026-10-22 ("for pasted ids we need to make that deterministic").
- **P2.D12. Setting a position is exact; an edit snaps where it decides a place** (§7.5). An undo then restores, and
  every edit snaps, wherever it sets the position. Snapping on load waits for plan 4's saved pin order: making only
  files of the current version snap was rejected, since a file from an older release is saved in the current version
  on its first save and would snap on the next load. Basis: read on `master` and measured (the off-grid elements, the
  pin orders); the user's rulings of 2026-10-22 ("it makes no sense to have part of elements operations be
  grid-snapped and part not"; the recommendation adopted).
- **P2.D13. A restored element goes back to its stacking place** (§7.6), by one rule where an element with a
  preassigned id is added. Basis: measured on `01-characterization` (P1.10's golden puts it on top); the user's
  adoption of the recommendation, 2026-10-22.

## 11. Risks

| Risk | Mitigation |
|---|---|
| A fix lands before the golden that shows it, so the golden pins the fixed behaviour and its diff is never seen | §9's order; each fix after the goldens it changes |
| A fix changes behaviour users or files rely on: an open S, R or T, port counts a file holds, latch outcomes | Each change named by its cause in the golden diff and the release notes; no release wrote the refused port counts; the walk measured first |
| A fix in code a later plan rewrites is lost in the rewrite | Each fix's test stays; the rewriting plan lists the fixes it keeps (§13) |
| A test of a fix cannot fail | Each shown to fail on `master` first; a later fix that makes an older test vacuous (§5.1) reworks it |
| The fuzz fixes cannot be checked without running the fuzzers, which are clang builds | The user runs them overnight before the slice is finished (P2.D10) |

## 12. Claims to check and open questions

- **The two Qt 6.7.3 defects on Qt 6.8.** Both were seen on 6.7.3 and neither on 6.9.3; 6.8 was not checked. The
  fixes hold on every version; whether the floor's 6.8 shows the defects (`master`'s CI runs these tests on 6.8.3, and
  on Debian's 6.8.2 on i386) is recorded, for the release notes (P2.34).
- **What a snapshot restore does on `master` with a custom picture that no longer loads.** §3.8's morph undo restores
  through it, so its behaviour there is checked first, and the undo neither leaks nor crashes (P2.35). Checked
  2026-10-24 on an `InputSwitch`, through undo and redo: the restore loads the whole snapshot, so the element lists
  the missing file, then the picture's load draws the type's first default (read: whatever the slot) and throws out
  of the command's `undo()` or `redo()`, past its refresh of the runtime state. The undo stack's index does not move,
  so the element shows the restored state while the stack has not stepped: the next undo or redo applies the same
  command again, which then passes (a failed path is not loaded twice) and steps. In the application the throw
  reaches `Application::notify()`, which reports it. A restore whose picture is the one already drawn does not load
  it and does not throw. Plan 3 settles the rule (P3.D6: the slot shows its default and the restore goes on), which
  also steps the stack.
- **The priority walk's fix against plan 1's traces** (§5.3): measured first; with traces changed, the user decides
  (P2.36).
- **When the fuzz harnesses run** (P2.37): settled by the user, on the user's machine overnight, not in CI (P2.D10).

## 13. What other plans take from plan 2

- **Plan 3** keeps §3's fixes as it rebuilds the element types: the open-input defaults and the maximum port counts in
  each descriptor; the undoable AudioBox, trigger and input operations; no appearance where none is drawn; the themed
  pictures; the morph's undo; the property command, moved onto the schema; the Tx rule and MCP's validation, held once
  in the schema; the lock's tooltip; the display's bounds; the IC's turning centre; the Text's hint; the trigger
  placeholder's case. Its rule for a custom picture that no longer loads (P3.D6) also ends `master`'s undo that throws
  without stepping the stack (§12).
- **Plan 4** keeps the IC's tooltip and the embed and extract on every selected IC, fixes the three defects of §8
  that change the format, and adds the saved pin order with snapping on load.
- **Plan 5** lists `master`'s Rx override, the slow clock and the tick engine's contradiction among its differences
  from `master`.
- **Plan 6** keeps the clocks across a rebuild and a connected input on its wire; its condensation replaces the walk.
- **Plan 8** keeps the truth tables' regs, the untranslated names and the Tx resolution, then refuses two drivers.
- **Plan 9** keeps the wheel zoom, the topmost hit, the empty-scene error, the circuit's bounds, the selection in id
  order, the snapping rule with its exact undo, and the restored stacking place.
- **The Qt 6.8 floor's plan** follows this plan.

## 14. The branch's fixes, as evidence

Each fix was made on `gpu-canvas` (or designed there); reading its commit shows the change and its test, while plan 2
writes its own on `master`. A hash is the branch's, kept read-only.

| Fix | Commits on the branch | Test on the branch |
|---|---|---|
| Fuzz seeds and their check | `12cfd0e6`, `167cb01ba`, `941d37a77` | each seeded harness's `LLVMFuzzerInitialize` |
| Fuzz coverage | `d2b0c4ae2` | the core harness's edge count |
| The failed save | `a3ae1cb15` | `TestFileUtils` |
| The buzzer's teardown | `84d63d6ff` | `TestBuzzer` on 6.7.3 |
| `ItemWithId`'s comment | `0f66402ef` | none |
| SR and T defaults | `b96dd56fd`, `14584176a` | `TestSequentialLogic::testSRFlipFlopWithOneOfSAndROpen`, `testTFlipFlopWithTOpen` |
| The loader's port counts | `7f2f4dfb2` | `TestGraphicElementSerializer::testLoadRefusesPortCountsAboveTheTypeMaximum`, `testLoadRefusesDemuxInputsItsOutputsDoNotNeed` |
| The AudioBox's undo | `9287e8069` | `testAudioBoxChangeIsUndoable` |
| The lowercase trigger | `96ba1c28a` | `testLowercaseTriggerPushesOneCommand` |
| No appearance where none is drawn | `bca2a3a40` | `testBodiesDrawnInCodeOfferNoAppearance` |
| Input operations | `d7d6c4a33` | four tests in `TestInputElements` |
| Themed pictures | `ac95c2b60` | three tests in `TestSequentialLogic` |
| A morph's undo | `032a48b69` | two tests in `TestCommands`; `testSnapshotRestoresWithAPictureThatIsGone` |
| Duplicate Tx labels | `355c55fbb` | `testDuplicateTxLabel` (9 rows) and the inline refusal in `TestScene`; two in `TestElementEditor` |
| The property editor | `aabb6d806`, `dd0ed8fe0`, `90a651f46`, `22e6f99ce` | ten tests in `TestCommands`, nine in `TestElementEditor` |
| MCP's property setter | `18dec210c`, `afa255b54` | `TestElementHandler`; the MCP responses golden |
| The display's small picture | `871390d61` | `TestDisplay7::testLitSegmentsPastACustomPictureStayInTheBounds` |
| An IC's tooltip | `2a0c3db7c` | `TestICUnit::testLoadedIcKeepsNoToolTipAfterRetranslate` |
| SystemVerilog truth tables | `f9648d3f7` | `TestSystemVerilogExport::testTruthTablesExportAsTheirOwnRegs` |
| Untranslated names | `bea9b2e95`, `e187dad16`, `736d79a2e` | the generators' unit tests under the tagging translator; `TestPort`'s and `TestGraphicElement`'s source-name tests; `TestIC`'s display pins under a translator |
| MCP's empty-scene error | `fc780d437` (with plan 9's bounds work) | `TestFileHandler::testHandleExportImageRejectsEmptyCircuit` |
| The wheel zoom | `675f7bbe5`, `2966a6c33` | `testWheelKeepsCursorPoint`, `testWheelNearSceneEdgeKeepsViewInsideSceneRect` |
| A connected input keeps its wire; the topmost port | `990fb896f` (inside plan 9's port layer) | `TestPort::testSettingTheDefaultLeavesAConnectedInputOnItsWire`; the pointer golden |
| An MCP client closing the connection | `5cdf5cebb` | a clean window asks on close; one marked disconnected does not |
| An Rx reached both ways | `32b37643a`, `f9dbff955` on `temporal-engine` | each exporter's Rx test |

The branch's editor, MCP and morph fixes sit on its parameter schema (plan 3's work), so on `master` they are written
on `master`'s own setters, not ported. The Text's hint was never fixed: it was read on `master` for the restart.
