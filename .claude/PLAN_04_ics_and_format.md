# Plan 4: ICs and the document format

One definition per IC file or embedded blob, parsed once and shared by its instances through an interface the model
declares; the IC preview rendered once per definition, on first hover; pin names derived from the circuit, in the
pins and the file header alike, never saved in the interface language; and the undo of a reload brought by a changed
IC file restoring the old definition, which the document keeps as a copy, so a save and a reopen keep it. The
additions to what a document holds extend the current format; nothing replaces it.

- **Depends on:** plans 1 to 3, the Qt floor's and the exception boundaries' plans, landed, since the plans run one at a
  time, in their order (the plans' rules, §1): plan 1's goldens, the payload goldens above all, where a format change
  shows first; plan 3's one element class, with the descriptors and the untranslated source names an IC's pins read;
  plan 2's fixes in the loader, registry, preview and IC controller, which this plan keeps, each test green (its §4.1
  and §4.2: a loaded IC keeps no tooltip after a language change, embed and extract act on every selected IC as one
  step; its §2.20 to §2.24: a preview scaled before it is rounded and rendered only when shown, an IC's load taking its
  ports' keys from the load's record, an embedded IC that fails to load leaving no undo macro open).
- **Lands before:** plan 5, whose reference simulator reads its definitions, then plan 6, whose netlist builder
  compiles each IC definition once.
- **Branch and ledger:** `04-ics-and-format`, from `03-element-types` once plan 3 has landed, in slices (the plans'
  rules, §1 and §3).
  `LEDGER_04_ics_and_format.md` has a row for every step, check and open question below.
- **Rules every plan shares** are the plans' rules (`PLAN_rules.md`) and `CLAUDE.md`'s "Building and testing"; the
  format rule is the plans' rules' §2: older files read as before, a file the application writes reads back and saves
  again byte for byte, and each extension is tested by reading files written before it. This file states plan 4's own
  rules. Its decisions are numbered P4.D1 on.

## 1. Where `master` stands

### 1.1 ICs

Each IC instance loads its definition itself (`ICLoader`) and builds its own internal elements, wires and boundary
ports, with their sorted order, boundary inputs and a feedback flag for its own simulation (`ICSimulation`), and renders
its hover preview when the popup first shows it, from its definition read again (plan 2's P2.77). It saves its reference
only: a file name (a bare path before 4.1, reduced to its file name) or an embedded blob's name. Loading resolves an
exact blob name in the document's blob registry first, then the file's base name as a blob, then a file against the
context directory. Embedded definitions live in the file-level blob registry (from format 4.5; from 5.1 its stream has
an explicit version, since an unversioned stream's string encoding followed the Qt the program was built with, which
changed in Qt 6.6). `ICRegistry` keeps the bytes of a scene's IC files, watches them, and on a change reloads every
instance at once and pushes an `UpdateBlobCommand`.

The layering is crossed: the loader casts `ic.scene()` to `Scene` to reach the registry's bytes and reads
`Application::migrationEnabled`; the registry reaches the scene, the commands, the simulation (a blocker while it
reloads), and `Application::guardedSlot`; both code generators walk each instance's internal elements.

### 1.2 Three defects whose fixes change the format

Each exists on `master`, and plan 2 leaves it here because its fix changes what a document holds:

- **Undoing an IC's reload after its file changed restores nothing.** `ICRegistry`'s file watcher reloads every
  instance and pushes an `UpdateBlobCommand`; its `undo()` restores the blob and loads each instance's old data, whose
  old-format path in `IC::load` reads only the file name and calls `loadFile()` again, so the file's new definition
  stays, and a wire the reload cut stays cut. Probed on the branch on 2026-10-16: one input after the reload, still one
  after its undo, the cut wire still cut; read again on 2026-10-19, not rerun, and `master`'s undo takes the same path.
  Undo alone would leave a state the format cannot save: a file-backed IC saves only its file name, so a save, an
  autosave or a paste writes the name, and reopening brings the file's new definition back with no undo step.
- **IC pin names are saved in the interface language.** The serializer writes each port's displayed name, and on load
  `setName()` makes it an explicit name; an embedded IC whose blob is unchanged skips the reload (a label edit through
  an `UpdateCommand`), so its pins keep the saved text and stop following the language.
- **An IC's pin names and its file header's pin names are built by two rules.** The header puts a leading space for an
  empty label, the clock frequency in brackets, and lists pins in topological order; the pins use the boundary node's
  label or the type's name, and the port's name only for multi-port elements, and a node has no generic properties,
  so the frequency the pin's comment promises never reaches it. Nothing in the repository reads the header's names.

### 1.3 The IC component library

`Tests/Integration/IC/Components` holds 73 components: levels 1 to 9 (4, 13, 5, 13, 11, 8, 8, 4 and 6 of them; latches
and flip-flops, then decoders and multiplexers, up to a CPU's stages) and a wireless pass-through. Each is written by a
Python generator in `Tests/Integration/IC/Generators` (`create_level*.py`, with shared helpers and
`run_all_generators.py`, which runs them in dependency order) that drives a built `wiredpanda --mcp` through the MCP
client, headless; regenerating an unchanged generator gives byte-identical files. 180 files in
`Tests/Integration/IC/Tests` test them; the CPU tests build their circuits through a helper header (`CpuHelpers.h`),
not under `TestLevel*` names, and a test class's name may differ from its file's. Every component's SystemVerilog
export has a golden (`Tests/Integration/SystemVerilog/<component>.sv`, regenerated with
`GENERATE_EXPECTED_SYSTEMVERILOG=1`), and the export recurses into the components a component embeds. What the library
teaches a change to a component's ports or its reading:

- **Components embed each other by reference**, loaded at load time, so a change to a level-1 component reaches every
  level above. To find who embeds a component, read the generators, which name sub-components in plain text; a search
  of the `.panda` files finds nothing, since they store names in UTF-16.
- **An input added to a component breaks every embedder.** The embedder never drives the new pin, which reads 0 and,
  through an AND, zeroes the component's outputs; an added output is harmless. The fix is a cascade: drive the pin in
  every generator and C++ helper that embeds the component, regenerate the parents in dependency order (the bus
  multiplexer before the shift register, the stack pointer before the stack memory), bump the port counts the CPU
  helper asserts, and regenerate every golden (about 40 SystemVerilog files change, up to level 9).
- **An optional input whose default is its no-op state avoids the cascade:** an unconnected IC input reads that default
  in the simulation and in both exporters, so only the generator and the component's own test change; the embedders'
  files still gain the unconnected port, so their goldens regenerate. Plan 5's rule that an unconnected IC input reads
  the level of the input element it stands for keeps this working: the generator sets that element's level.
- **The level-1 JK component is a master-slave 7476 on purpose:** its master is transparent while the clock is high and
  the slave takes the value on the falling edge, so Q changes at the end of the pulse. The D component is a 7474
  (rising edge), and the native JK element is rising-edge too. `TestLevel1JKFlipFlop::testHoldDuringClockHigh` pins Q
  holding while the clock is high; the component exports structurally, from its gates.
- **The level-1 flip-flops' ~Preset and ~Clear are active low,** which the SystemVerilog flip-flop template hard-codes
  (`always @(posedge clk or negedge prst or negedge clr)`); an active-high variant was tried on an earlier branch and
  abandoned.

## 2. What plan 4 builds

### 2.1 The reload's undo, failing first

The first step reruns the branch's probe on `master` as a failing test: an IC from a file, wired; the file changed so
an input goes; the reload cuts the wire; the undo. On `master` the IC still has one input after the undo and the wire
stays cut. §2.5 and §2.6 make it pass.

### 2.2 A shared definition

`ICDefinition` is a file's or blob's container, opened: the format revision, the metadata, the blob registry and the
inflated element records, shared as `std::shared_ptr<const ICDefinition>`. Each instance builds its own elements from
the records through one parse path for files and bytes. A process-wide cache keeps 64 definitions and recycles the
least recently used, on the main thread only (asserted; nothing loads an IC off it). A file is keyed by its absolute
path and its bytes, a blob by its bytes, so a definition is reused only for the bytes it was parsed from (P4.D1). The
loader reads a scene's kept bytes through `ICFileCache`, an interface declared beside it and implemented by
`ICRegistry`; the scene gives it to each IC as the IC is added, and the IC drops it on any scene change, so the loader
no longer casts its scene, no element file includes the registry, and the loader no longer includes `Scene.h`. The
registry's own comments described a definition cache it never held, and are corrected. The definitions move into the
document's registry when the instances lose their internal elements (§2.7); until then, the process-wide cache.

- **Translations.** The file is read through a callback the loader passes, so "Error opening file: %1" stays in
  `ICLoader`'s translation context; throwing it from the definition would orphan its translations and change the
  translatable-strings golden.
- **Staleness.** None to state: a file that changes has other bytes, so a migrated file, a file the watcher reports
  changed, and a rewrite within the same millisecond at the same size each get their own definition, and a file made
  unreadable fails its read. Nothing forgets a definition; one no longer used ages out of the cache. The branch keyed
  a file by its path, modification time in milliseconds and size, which missed that rewrite and kept loading that
  file, and forgot a definition after a migration and on the watcher's report to make up for it.
- **Cost.** The loader reads the bytes either way, from the scene's kept copy inside a scene, so the key adds a compare
  of the bytes, and outside a scene (a circuit being opened) a read of the file per instance. It is measured on the
  IC-heavy fixture (2,000 instances of one file) before it lands (P4.10); should the load's profile show it, the key
  goes back to the file's path, modification time and size, with the two blind spots stated in the code.
- **Tests:** the shared definition across instances and files; parses counted; recency, not insertion order, decides
  eviction (in-memory empty circuits, one definition used again); a failed read keeps nothing; a migrated file, a file
  rewritten at the same size within the same millisecond, and a file made unreadable each read as they now are; a
  scene gives its ICs its registry and removal drops it; an IC loads through its file cache (with a file on disk it
  could not load). Elements in these tests are heap objects the scene owns: a stack element died inside its scope
  before the save.

### 2.3 The preview per definition

Plan 2 already renders a preview only when the popup first shows it, each instance its own, reading the IC's definition
again, and the IC fuzz harnesses ask for it (P2.77, P2.80); this plan makes it one per definition. The IC keeps the
definition it was built from and the directory that definition's references resolved against. A preview cache beside the
popup builds the designed circuit on the first hover (one nesting level deeper, as a load), renders it with a pure
function, and keeps the picture for every instance of the definition while the definition lives, at most 16, recycling
the least recently used, on the main thread only. The key is the shared definition plus the directory, since one blob's
IC references can resolve to two circuits in two directories; it reads no file on hover, cannot race the watcher, and
survives a renamed blob. A reload replaces the definition, so the old preview goes with it. A circuit that is empty, too
large, of unusable geometry or that no longer builds (an IC file it holds since gone or broken) keeps a null preview and
is not built again. A theme change drops every preview, since wires, ports and labels are drawn in the theme's colours
(the background is fixed). The preview's guard against non-finite bounds checks the origin too: once each element gives
one rect, a one-element circuit at a NaN position keeps a finite size and passed a guard that checked only the size. The
preview's renderer moves to the application in plan 9.

Tests: loading renders none (plan 2's `TestICUnit::testAPreviewRendersOnlyWhenShown`, kept); the first ask renders and
keeps; file and blob instances share; a reload asks for its own; one blob in two directories has two; no circuit, an
empty one and one that no longer builds have a null preview, the last two kept; the least recently used goes; a theme
change drops all. Plan 1's IC preview goldens stay unchanged.

### 2.4 Pin names from the circuit, in one rule

- A pin's name derives from the boundary element it stands for: its label, or its type's name when it has none, then
  the port's name only when the element stands for more than one pin; evaluated when asked, so the pin shows the
  current language, and with plan 3's source names the code generators get the untranslated text. This plan alone
  makes the pins' names derived: plan 2 gives a pin standing for a display's port its source name, so generated code
  is right before this plan, and plan 3 gives ports their source and derived names, not the pins. The clock frequency
  in the header goes, and `genericProperties()`, which fed nothing else, goes with its tests.
- The file header's pin names follow the same rule, in the order the file holds the elements, not the topological
  order and not the stacking order: on the branch a header built in stacking order flipped two pins after an undo
  reordered co-located inputs. Its test makes such a scene by an undo. Pins of a multi-port element keep port order.
- The loader gives a saved name only to a port it adds; portable streams hold no port names for any element, since the
  load names every port again; in-memory undo snapshots keep every field. Files from 5.2, which hold an IC's pin names,
  still load: the names reach only ports the sub-circuit load then names. A serializer test from `master` that expected
  an IC's existing pins renamed by an old-format entry pins the new rule instead: each pin keeps its port object and
  its name, with its saved id mapped to it.
- The traps, each a fix on the branch: a pin named after a segment display's port took the translated name as an
  explicit name, so the generated code and the tooltip followed the language the IC was loaded in (a derived name must
  stay derived through the boundary node to the pin); a property edit on an embedded IC must keep the derived names;
  and a saved file must hold no pin name in the interface language.
- Tests: an IC holding a display, loaded under a tagging translator: a pin's name, source name and tooltip, then the
  header, then the pin after the translator is removed; an embedded IC reloaded through the skipped path under the
  translator keeps its derived names, and a portable save holds no pin name; the header names the loaded pins, in
  order. Goldens: every saved payload holding an IC header or IC pin names changes, each named by this cause.

### 2.5 The reload's undo restores the old definition

Undoing a reload brought by a change to an IC's file restores, in memory, the definition captured before the reload,
never reading the file (P4.D2). The document can differ from the file until the file changes again.

### 2.6 The document keeps a copy

The document keeps a copy of each file-backed IC's definition, with a hash of the file as last read, in the store
embedded ICs use, and the IC keeps its link to its file (P4.D2). On open:

- a file that matches its hash leaves the copy in use, so a restored definition survives a save and a reopen;
- a file changed since the save comes in as the same undoable reload as during a session;
- a document with no copies reads its files, as now;
- a missing file leaves the copy in use.

It extends the current format: documents written before it read as before, and a document written with copies reads
back and saves again byte for byte. Tests: each of the four cases on open; the undo of a reload, saved and reopened,
keeps the restored definition and its wire; the backward-compatibility files and plan 1's payload goldens read as
before, with the payload goldens of documents holding file-backed ICs changed by this cause.

### 2.7 When the instances lose their internal elements

The instances' internal elements go when nothing reads them: plan 6's switch removes the simulation's use, and the
exporters walk them until plan 8 moves the exporters to plan 6's netlist builder, so deleting them is plan 8's last
step, plan 8 landing after plan 6. With them go the IC's own `save()` and `loadState()`: the definition reference
becomes a parameter read through the registry, and the definitions move into the document's registry. Tests that edit
an instance's internals (a label or blob reload inside an inline IC, moving an internal element) are rewritten then,
and IC tests read the designed circuit from the definition. Plan 4 leaves the reference and the IC's serialization as
they are and states this hand-over in its ledger (P4.11).

That paste, the IC reload, `UpdateBlobCommand`'s undo and redo and a load from a blob are each one bulk change is
plan 9's test (its P9.10): `master`'s scene has no bulk notifications, and the document listener that reports them is
plan 9's, which lands after this plan.

### 2.8 How an IC's boundary reads

Plan 5 decides it: an unconnected IC input reads the level of the input element it stands for, and a clock pin, which
is required, reads Unknown. On the branch that removed the loader's copies of the live status and changed ten scene
goldens (a connected clock pin's default from Error to Unknown). Plan 4's definitions keep the input elements' levels
that rule reads.

### 2.9 An IC's pin order saved, and every element snapped on load

On `master` an IC's pins are ordered by the positions of the input and output elements inside its circuit
(`ICLoader.cpp:27`, `comparePorts`: by y, then x, stably), and a load keeps every position exact, on the grid or not.
Releases before 4.0 saved arbitrary positions, and 4,271 of the 4,504 elements of the 73 IC components are off the
grid. Plan 2 makes setting a position exact and every edit snap (its P2.42), and leaves snapping on load here, since
snapping two close rows of pins to one row can swap them: measured, snapping on load reorders the pins of 0 of the 85
off-grid files among the 410 loaded, while a control that collapses every row reorders 82 of 82, so the order is
safe today only by the files' luck.

Every file saves the order of its pins, inputs and outputs, as a list of the boundary elements' ids, an extension of
the current format (P4.D4); a file without it gets the order from its positions as loaded, before any snap, exactly as
`master` computes it. Then every element is snapped on load, in every file. The two land together, so no load ever
snaps an element whose position still decides an order. Adding a pin puts it in the order by its position among the
others, as `master` would; the order is otherwise changed only by an edit that moves a pin, which reorders it by
position as today. Snapping only files of the current version was rejected: a file from an older release is saved in
the current version on its first save and would snap on the next load, so it postpones the same change by one save.

Tests: a file of an older release whose two pins differ by less than half the grid in y keeps its pin order, and its
elements land on the grid; a file saved by this plan keeps a pin order its positions contradict; the 73 components'
pin orders (plan 1's P1.14 golden) are unchanged. Shown to fail with the saved order ignored (the close pins swap) and
with the migration taken after the snap.

## 3. Steps and slices

| Row | Step | Slice |
|---|---|---|
| P4.1 | The reload's undo as a failing test on `master` (§2.1) | 1 |
| P4.2 | The shared definition, its cache and the loader's file-cache interface (§2.2) | 1 |
| P4.3 | The preview per definition, and its guard (§2.3) | 2 |
| P4.4 | Pin names from the circuit; the header in file order; no pin name saved in the interface language (§2.4) | 3 |
| P4.5 | The reload's undo restores the definition in memory (§2.5) | 4 |
| P4.6 | The document's copies of file-backed definitions, with their hash, and the four cases on open (§2.6) | 4 |
| P4.12 | The pin order saved in every file, migrated from positions; every element snapped on load (§2.9) | 3 |
| P4.7 | The developer guide's IC and format sections; the release notes' list of format changes | 4 |

The checks and the hand-over of §2.7 and §5 are rows too (P4.8 to P4.11).

## 4. Decisions

Each ends with its basis: measured or tested (a run backs it), read (the code, with nothing run), or a ruling (the
user's choice between workable options).

- **P4.D1. One definition per IC file or blob, from the prototype's intent:** parsed once, shared by its instances,
  read through an interface the model declares, a file's keyed by its path and bytes as a blob's by its bytes.
  Basis: tested on the branch (the shared definition, its reads counted, its eviction by recency, a failed read
  keeping nothing); read on `master` (each instance loads its own; the loader casts its scene); the user's rulings of
  2026-10-04 (the shared definition) and 2026-10-21 (the key by bytes, over the branch's path, time and size, whose
  blind spots it removes; its cost measured first, P4.10).
- **P4.D2. The undo of a file change's reload restores the old definition, and the document keeps a copy,** with the
  file's hash, in the store embedded ICs use, the IC keeping its link; the four cases on open of §2.6. It extends the
  current format. Basis: probed once on the branch and read on `master`, not run there, so P4.1 reproduces it on
  `master` first; the user's rulings of 2026-10-19 (the undo) and 2026-10-20 (the copy).
- **P4.D3. Pin names in one rule, the pins' form**, for the pins and the file header, in file order; no pin name saved
  in the interface language. Basis: tested on the branch; the user chose the pins' form there, and accepted the fix as
  this plan's with the restart's write-up, 2026-10-21.
- **P4.D4. ICs and the document format are a plan of their own, landing before plan 6**, every addition an extension
  of the current format, never a new one (the earlier prototype's new format is not adopted). Basis: the user's
  rulings of 2026-10-20 (each addition to a document an extension) and 2026-10-21 (this plan, and the format rule
  stated once for every plan).
- **P4.D5. An IC's pin order is saved, and every element is snapped on load, landing together** (§2.9). Basis:
  measured on `01-characterization` (the off-grid elements; the pin orders under a snap and its control); the user's
  rulings of 2026-10-22 (every operation on the grid; the recommendation over snapping only files of the current
  version).

## 5. Claims to check and open questions

- **The reload's undo on `master`** (P4.8): probed once on the branch, read on `master`; P4.1 runs it there, and the
  plan's design rests on what it shows.
- **Whether an IC's first reload in a scene always brings another definition** (P4.9), as a test comment on the
  branch assumed; that test's assertions did not rest on it. Checked where plans 4 and 6 meet: here by counting
  definitions across a first reload, and in plan 6 for its node ranges.
- **The cost of keying a file's definition by its bytes** (P4.10): a compare per instance inside a scene, and a read
  per instance outside one, measured on the IC-heavy fixture's load before the key lands (§2.2).

## 6. Risks

| Risk | Mitigation |
|---|---|
| A format extension breaks reading an older file, or a written file does not read back byte for byte | The backward-compatibility files and plan 1's payload goldens; each extension tested by reading files written before it |
| A pin-name change flips or renames an embedder's pins, so every embedder breaks | Header and pins from one rule in file order; the 73 components' tests and SystemVerilog goldens run on every change |
| A cached definition goes stale | A file's definition keyed by its bytes, so a changed file gets its own (§2.2) |
| Keying by bytes slows a load of many instances | Measured on the IC-heavy fixture first (P4.10); the metadata key, with its blind spots stated, if the profile shows it |
| A restored definition is lost by a save | The document's copy (§2.6), tested through a save and a reopen |

## 7. What other plans take from plan 4

- **Plan 5:** the definitions its reference simulator reads; the boundary rule of §2.8 is its own.
- **Plan 6:** the shared definitions, which its netlist builder compiles once each into node ranges, so no IC reaches
  the scene's simulation; and the check of P4.9.
- **Plan 8:** the definitions its exporters read through the netlist builder; the hand-over of §2.7, its last step.
- **Plan 9:** the preview's renderer, which moves to the application; the registry's reach into the scene and the
  simulation, untangled there; and the bulk-change test of paste, the IC reload and the blob loads (§2.7).

## 8. The branch's work, as evidence

| Step | Commits on the branch |
|---|---|
| The shared definition | `5c4b23cea` |
| The preview per definition | `9cd65594f`; the guard in `fc780d437` |
| Pin names and the header | `22c522bcc`, `76e62b064`, `736d79a2e`, `11f91225e` |
| The reload's undo and the document's copy | never built; the probe was a scratch test of 2026-10-16 |

The earlier `perf/load-hotspots` prototype holds the shared definition's first form (`ICNetlist` on its tick engine is
plan 6's intent, not this plan's).
