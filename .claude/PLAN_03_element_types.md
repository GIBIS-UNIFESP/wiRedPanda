# Plan 3: element types as data

One element class, `Element`, with a descriptor per element type in place of 31 subclasses: each type's ports,
parameter schema, operations, strings, help text and serialization as data; typed parameters checked against the
schema; presenters per type that measure and paint a body, so painting leaves the element; audio in an application
service; appearance slots saved by key. Every saved file stays byte-identical except where a change names its cause.
It comes early because the simulation core, the exporters, MCP and the editor all read element types, so none of them
is written twice. It is data-driven design, not data-oriented: the data-oriented layout is plan 6's.

- **Depends on:** plans 1 and 2, the Qt floor's, the Qt ceiling's and the exception boundaries' plans, landed, since the
  plans run one at a time, in their order (the plans' rules, §1): plan 1's goldens; plan 2's fixes in this code (its §3,
  and its §2.19, a trigger loaded as the key sequence saved), which plan 3 keeps, each test green; the Qt 6.8 floor, on
  which every supported Qt renders the pictures' blur filters alike; and the exception boundaries' rule, so nothing Qt
  calls back into throws, and their fixes in this code.
- **Lands before:** plan 4, which follows it and builds on the one element class; plan 5, whose spec and reference
  read the descriptors; plan 6, whose kernels each descriptor names; and plan 9, which takes the one element class
  and its presenters.
- **Branch and ledger:** `03-element-types`, from `02c-exception-boundaries` once the exception boundaries' plan has
  landed, in slices (the plans' rules, §1 and §3). `LEDGER_03_element_types.md` has a row for every step, check and open
  question below.
- **Rules every plan shares** are the plans' rules (`PLAN_rules.md`) and `CLAUDE.md`'s "Building and testing"; this
  file states plan 3's own. Its decisions are numbered P3.D1 on.

## 1. Where `master` stands

Counted on `03-element-types`, which is `master` with plans 1 to 2c in (P3.1 corrected each figure taken on the branch
before): `GraphicElement.h` has 763 lines and 35 `virtual` declarations, the `.cpp` 853 lines; 31 type subclasses live
in `App/Element/GraphicElements/`, with the abstract `AudioOutputElement` (base of `Buzzer` and `AudioBox`), and `IC` in
`App/Element/`. One class carries identity, ports, parameters, placement, simulation (`updateLogic()` per subclass),
serialization, pictures, appearance, painting, highlight, theme and label geometry, help text and drop hooks, and the
Graphics View plumbing. The base declares accessors for parameters only some types have (audio, volume, colour,
frequency, delay, appearance, wireless mode), each guarded by a flag of the type's `ElementMetadata`, filled by an
`ElementInfo<T>` specialization in each element's source. Besides its metadata, its logic and its port layout (15
types override `updatePortsProperties()`), what each subclass holds:

| Subclasses | Besides that |
|---|---|
| And, Or, Nand, Nor, Xor, Xnor, Not, VCC, GND, Line | nothing |
| the four flip-flops | the simulation state they save, restore and reset (the last clock and inputs); their themed picture is the base's |
| the two latches | nothing; their themed picture is the base's (`updateTheme()` takes the metadata's themed path when the type has no default appearances) |
| Text | the empty-label hint, a child item, and its theme colour |
| Mux, Demux | a `paint()` and a `boundingRect()` override; the Demux also derives its input count and checks it on load |
| LED | a colour and its appearance states, with its own `setAppearance()` and generic properties |
| Display7, Display14, Display16 | a colour; a `paint()` override |
| Clock | frequency and delay, its appearance states, its generic properties; its output level and run-time phase, which are simulation state |
| Buzzer, AudioBox | frequency, audio file, volume, and playback (start, stop, volume, mute) |
| Node | the wireless mode, and the wireless indicator, a child item |
| InputSwitch, InputButton | the input state, its appearance states, and the click handling (press, and release for the button) |
| InputRotary | the input state, the click handling, the waveform hooks, a `paint()` override, its own `setAppearance()` and `refresh()` |
| TruthTable | the table; a `paint()` override; a double-click that asks for the table editor |
| IC | the definition reference, the embedded blob, its internal elements and their simulation (with its own settling and simulation state); a `paint()` override; a double-click that opens the sub-circuit, and the hover that drives its preview; its own retranslation |

The base's own double-click asks for the inline label editor, and every double-click reaches an element through Qt's
item dispatch once the scene's interaction code declines it. Once painting goes to presenters, parameters and
operations to descriptors, logic to kernels (plan 6) and playback to the audio service, no subclass holds anything a
descriptor cannot.

How tied the class is to Graphics View: switching its base to `QObject` in an experiment gave 481 errors on a first
pass, 234 of them `override` mismatches and the rest 22 distinct `QGraphicsItem` members (`pos`, `setPos`,
`sceneBoundingRect`, `setSelected`, `update`, `x` and `y`, `sceneTransform`, `prepareGeometryChange`, `isSelected`,
`setVisible` and `isVisible`, `scene`, `scenePos`, `moveBy`, `transform`, `setTransform`, `mapToScene`,
`childrenBoundingRect`, `setCacheMode`, `setFlags`, `setAcceptHoverEvents`, `ensureVisible`, `parentItem`), a lower
bound since a first pass stops at errors per file (the experiment ran on the branch). Element `qobject_cast`s: 42 in
`App` and `MCP`, 134 more in the tests; `qgraphicsitem_cast`s to element, port or wire types: 32 in `App` and `MCP`;
`QPointer`s to elements: 3 (the scene's interaction code, the IC preview popup, and the targets of
`Scene::recordOperation()`, plan 2's one command per pointer or key operation); `tr()` in 10 files of `App/Element`,
and `QT_TR_NOOP` or `translate()` in 15. `Port`'s item type
(`UserType + 1`) is never written to files; it serves the hit tests and a debug table in the serializer. Tests build
elements directly about 2,360 times in 165 files (`new` 1,622, on the stack 658, `make_unique` or `make_shared` 77),
plus 219 `new IC(`; on `master` the same count gives 2,111 in 158 files and 192 `new IC(` (the earlier 1,880 counted
fewer forms of `new`).

## 2. The type catalogue

What the schema reproduces byte for byte, taken from each type's metadata, `save()`, `load()`, setters,
`paint()`, `refresh()` and pointer handlers, the serializer, the property editor and MCP's set-properties. It was taken
on the branch after its first three phases, which held plan 2's fixes to this code, so it describes `master` with
those fixes in: Mux, Demux and the truth table offer no custom appearance, an open S, R or T reads Inactive, and the
loader refuses a port count above a type's maximum. Plan 3 runs after those fixes, so that is the state it keeps;
P3.1 checks the catalogue against the code again before anything moves.

### 2.1 Common serialization

The scene writes each item's `QGraphicsItem` type number as its tag (an element is `UserType + 3`, 65539; a wire
`UserType + 2`), so the format depends on Qt's item types; plan 9 gives the tags names of the same values. An
element's stream, after the tag and the `ElementType` number, holds four values and then the type's own map:

1. a common map (`QMap<QString, QVariant>`, which streams in key order): `pos` always; `id` in portable files only
   (snapshots keep ids through the ports' `serialId`); `rotation`, `label` and `trigger`, which a portable file writes
   only when not 0, empty and empty, and every other purpose always; `flippedX` and `flippedY` only when true, in every
   purpose;
2. the input port list, one map per port: `serialId` (`quint64`) and `name`; a portable file writes no `serialId`
   (the loader derives it from the element's `id`) and no `name` except an IC's, so other types' maps are empty; its
   length is the input count, the only place a variable-size type's port count is stored;
3. the output port list, the same shape;
4. the appearance list, one map per slot: `skinName`, written through `ExternalFilePath::forWriting`; a portable file
   writes resource (`:/`) slots as empty maps, and an empty list when no slot is custom.

Reading refuses a non-finite position or rotation; reads `trigger` as a key sequence, or as its text; takes three
port-map shapes (`serialId`, the older `ptr`, a portable file's none, derived from `id`); adds every listed port and
trims the constructed surplus never below the type's minimum (so a short list keeps the minimum); reads the
appearances, where a snapshot's resource path restores the slot's current default (the latches' and flip-flops'
theme) and an empty entry keeps the slot's default; recomputes whether the default appearance is in use, and
refreshes. Then it refuses a port count above the type's maximum (the IC exempt, its counts coming from its
definition; plan 2's fix), lays out the ports and applies the orientation. Before 4.1 the common part was a fixed
sequence: `pos`; `rotation`, with a ±90° compensation by group
except for the displays and Node; `label` (from 1.2); port sizes (from 1.3, read and discarded); `trigger` (from 1.9);
an unused priority (from 4.0.1); the port lists (`ptr`, `name`, `flags`); appearance names (from 2.7; an IC drains
them). The version predicates the per-type branches use: clock 1.1, labels 1.2, port sizes 1.3, Display7 colour 1.6
and extended colour 1.7, trigger 1.9, audio 2.4, appearance names 2.7, lock state 3.1, unused priority 4.0.1, map
format 4.1, truth-table data 4.2, delay fix 4.3, wireless mode 4.4, slim portable payload Rev101.

### 2.2 Common parameters

Position, rotation (any `qreal`; MCP takes any) and flips, where `rotatesGraphic` decides whether the picture or only
the ports turn; the label (`hasLabel`); the trigger key (`hasTrigger`); the port counts within the type's `[min, max]`
(`setPortSize` ignores any other); the appearance slots, one path per default slot (`canChangeAppearance`).

### 2.3 Types at a glance

Ports are inputs / outputs as min–max; the default count is the minimum. Flags: A `canChangeAppearance`, C
`hasColors`, F `hasFrequency`, D `hasDelay`, L `hasLabel`, T `hasTrigger`, V `hasVolume`, B `hasAudioBox`, TT
`hasTruthTable`, R̸ `rotatesGraphic` false. `hasAudio` is false on every type.

| Type (id) | Group | In | Out | Flags | Own map keys | Default slots | Paint |
|---|---|---|---|---|---|---|---|
| And (5), Or (6), Nand (7), Nor (8), Xor (10), Xnor (11) | Gate | 2–8 | 1 | A | none | 1 | picture |
| Not (4) | Gate | 1 | 1 | A | none | 1 | picture |
| Node (23) | Gate | 1 | 1 | A L | `wirelessMode` | 1 | picture + wireless indicator child |
| InputButton (1) | Input | 0 | 1 | A T L R̸ | `locked` | 2 (released, pressed) | picture by state |
| InputSwitch (2) | Input | 0 | 1 | A T L R̸ | `isOn`, `locked` | 2 (off, on) | picture by state |
| InputRotary (21) | Input | 0 | 2–16 | A T L | `currentPort`, `locked` | 2 (only 0 drawn; the arrow is a fixed resource) | picture + marks + arrow |
| Clock (9) | Input | 0 | 1 | A F D L R̸ | `frequency`, `delay`, `locked` | 2 (low, high) | picture by state |
| InputVcc (12), InputGnd (13) | StaticInput | 0 | 1 | A R̸ | none | 1 | picture |
| Led (3) | Output | 1–4 | 0 | A C L R̸ | `color` | 26 | picture by inputs and colour |
| Display7 (14) | Output | 8 | 0 | A C L R̸ | `color` | 9 | background + segment SVGs |
| Display14 (27) | Output | 15 | 0 | A C L R̸ | `color` | 16 | background + segment SVGs |
| Display16 (33) | Output | 17 | 0 | A C L R̸ | `color` | 18 | background + segment SVGs |
| Buzzer (26) | Output | 1 | 0 | A F L V R̸ | `frequency`, `volume` | 2 (off, on) | picture by playing |
| AudioBox (31) | Output | 1 | 0 | A B L V | `audiobox`, `volume` | 2 (off, on) | picture by playing |
| DLatch (15), SRLatch (32) | Memory | 2 | 2 | A | none | 1, themed | themed picture |
| DFlipFlop (17), TFlipFlop (20) | Memory | 4 | 2 | A | none | 1, themed | themed picture |
| JKFlipFlop (18), SRFlipFlop (19) | Memory | 5 | 2 | A | none | 1, themed | themed picture |
| Mux (24) | Mux | 3–11 | 1 | none | none | 1 (a transparent sizing picture) | drawn in code |
| Demux (25) | Mux | 2–4 (derived) | 2–8 | none | none (load checks inputs) | 1 (sizing picture) | drawn in code |
| TruthTable (30) | IC | 2–8 | 1–8 | L TT | `key` | 1 (sizing picture) | drawn in code |
| IC (22) | IC | from definition | from definition | L | `name` | none | drawn in code (`ICRenderer`) |
| Text (28) | Other | 0 | 0 | L | none | 2 (only 0 shown) | transparent picture + hint child |
| Line (29) | Other | 0 | 0 | A L | none | 1 | picture |
| JKLatch (16) | none | | | | | | no creator: a file naming it fails to load |
| Unknown (0) | none | | | | | | not a type |

### 2.4 Per-type parameters

A portable file omits a value equal to the default; every other purpose writes every key. "Older files" is the bare
value read before 4.1, in that order after the common sequence.

| Type | Parameter | Default | Range and rules | Key | Older files |
|---|---|---|---|---|---|
| Node | wireless mode (None 0, Tx 1, Rx 2) | None | 0–2; a file value outside loads as None; MCP rejects it | `wirelessMode` (int) | none before 4.4; the map is read only from 4.4 |
| InputButton | locked | false | | `locked` | 3.1–4.0: bare bool |
| InputButton | pressed | false | transient, not saved | | |
| InputSwitch | on | false | | `isOn` | before 4.1: bare bool |
| InputSwitch | locked | false | | `locked` | 3.1–4.0: bare bool after `isOn` |
| InputRotary | position | 0 | `[0, outputs)`; anything else becomes 0 | `currentPort` | before 4.1: bare int |
| InputRotary | locked | false | | `locked` | 3.1–4.0: bare bool after it |
| Clock | frequency (Hz, double) | 1.0 | setter: finite, with a half period of at least 1 µs (so positive), else ignored; editor 0.1–50 step 0.1; MCP > 0 | `frequency` | 1.1–4.0: bare float; none before 1.1 |
| Clock | delay (fraction of the period) | 0 | setter ignores a non-finite value and clamps to [−1, 1]; a file value that is no number reads as 0; editor slider −4…4 → ±0.5 in eighths; MCP rejects outside [−1, 1] | `delay` | read and discarded before 4.3 |
| Clock | locked | false | | `locked` | 3.1–4.0: bare bool after the frequency |
| Led | colour | `White` | `White`, `Red`, `Green`, `Blue`, `Purple`; any other string is kept, saved verbatim and drawn as White; offered only with one input | `color` | 1.1–4.0: bare string |
| Display7/14/16 | colour | `Red` | as Led | `color` | 3.1–4.0: bare string; none before 3.1 |
| Buzzer | frequency (Hz) | 1047 | setter: finite and > 0, else ignored; editor 20–20000 step 10; MCP > 0 | `frequency`, or old `note` | 2.4–4.0: bare note name; in a 4.1+ map, `note` when `frequency` is absent |
| Buzzer | volume (float) | 0.35 | no setter check; editor slider 0–100 → 0–1; MCP rejects outside [0, 1]; a file value that reads as a float is taken as is | `volume` (double) | none before 4.1 |
| AudioBox | audio file | `:/Components/Output/Audio/wiredpanda.wav` | a file that does not exist or cannot be read keeps its path while the built-in sound plays in its place, until the box next plays with the file back; opening a file reports it (plan 2c) | `audiobox` (a portable file omits a resource path) | 2.4–4.0: bare string |
| AudioBox | volume | 0.5 | as Buzzer's | `volume` | none before 4.1 |
| TruthTable | table | 2048 zero bits | resized to 2048 bits (2⁸ rows × 8 outputs, output 0's rows first) | `key` (`QBitArray`), written in portable files too | none before 4.2 |
| IC | definition | none (an empty name throws on load) | an exact blob name in the registry, then the file's base name as a blob, then a file against the context directory; a blob or file whose bytes this IC last built is not built again | `name`; also read `fileName`, `blobName`; a snapshot also writes `files`, the IC files the circuit read, which its restore loads in place of any deleted since (plan 2c) | 1.2–4.0: bare file path, reduced to its file name (backslashes normalized) |

Port counts: Led 1–4 inputs; InputRotary 2–16 outputs; TruthTable 2–8 inputs and 1–8 outputs; Mux 3–11 inputs (data
plus `ceil(log2(data))` select lines; the editor offers 2–8 data inputs, totals 3, 5, 6, 8, 9, 10, 11; a select value
past the last data input gives an Unknown output); Demux 2–8 outputs with derived inputs (1 + select lines;
`setInputSize` is a no-op and `load()` throws when the file's input count differs). Node: Rx hides its input port
and makes it not required, Tx hides its output port, and the indicator child shows for Tx and Rx at (30, −6), or
(2, −6) mirrored, coloured by the live signal. Switch and rotary loads end by setting the outputs and the picture from
the loaded state. Display7 before 1.6, and again before 1.7, permutes its eight inputs on load (`{2,1,4,5,0,7,3,6}`,
then `{2,5,4,0,7,3,6,1}`). Buzzer note names: C6 1047, D6 1175, E6 1319, F6 1397, G6 1568, A6 1760, B6 1976, C7 2093,
A7 3520, B7 3951; any other name gives 1047. The types without their own map (gates, Not, Vcc, Gnd, latches,
flip-flops, Mux, Demux, Text, Line) save the common part only.

The entry points disagree on ranges (the setter, the editor and MCP each have their own). Two exceptions to "a portable
file omits defaults": the truth table's `key` is always written, and every type with its own map writes the map even
when it is empty. The old `note` key and the IC's `fileName` and `blobName` stay as reading branches only.

### 2.5 Pictures, selection, pointer, audio and state

Every slot's path is the default's or the custom one's.

| Type | Picture | Selected by |
|---|---|---|
| Gates, Node, Line, Vcc, Gnd | slot 0 | nothing |
| InputButton, InputSwitch | slot 0 or 1 | on |
| Clock | slot 0 or 1 | the live clock level |
| Buzzer, AudioBox | slot 0 or 1 | playing, which is the input Active |
| Led | slot by colour and inputs | 1 input: 2 × colour + input; 2 inputs: 18 + code, both on 25; 3 inputs: 18 + code; 4 inputs: 10 + code |
| Display7/14/16 | slot 0 background, then each active segment's SVG recoloured (5 colours, cached per resource path) | input statuses, colour |
| InputRotary | slot 0, a mark per output, the arrow on the selected one | output count, position |
| Latches, flip-flops | `:/Components/Memory/<theme>/…svg` | the theme |
| Mux, Demux, TruthTable | drawn in code | port counts, and the table |
| IC | `ICRenderer` | the definition's ports and labels |
| Text | slot 0 (transparent); the empty-label hint is a child item | nothing |

The label is a child item of the element; the IC's label turns 90°, every other label 0.

**Selection.** One rule in two places: a rounded rectangle (radius 5, pen 0.5, the theme's selection brush and pen)
over the bounding rectangle, before the body; `ElementAppearance::render` draws it for every picture type, and Mux,
Demux, the truth table and the IC repeat the code in their own `paint()`.

**Pointer and keys.** InputButton: a left press while unlocked turns it on, the release turns it off. InputSwitch: a
left press while unlocked toggles it. InputRotary: a left press while unlocked steps to the next position. No element
pushes a command: the scene records what a press changes (`Scene::recordOperation()` around Qt's dispatch, over the
elements under the pointer) as one `UpdateCommand` of the elements whose snapshot changed, and nothing when none did;
a release is not recorded. TruthTable: a double-click
opens the table editor. IC: a double-click opens the sub-circuit and cancels the preview; hovering requests and moves
the preview, hidden over a port. Other labelled types: a double-click opens the inline label editor. Keyboard triggers
(`Scene::keyPress` and `keyRelease`): an unlocked input whose trigger matches is turned on at the press (a switch
toggles, a rotary steps, a button goes on; the clock has no trigger), recorded as one command as a press is, and only
a button is turned off at the release, unrecorded; Ctrl suppresses them, and auto-repeat is ignored.

**Audio.** Buzzer: a `QAudioSink` with a `ToneGenerator` (a sine, 44.1 kHz, 16-bit); AudioBox: a `QMediaPlayer` with a
`QAudioOutput`, looping. Both play while their input is Active; whether they are playing, muted and have a device is
session state, not saved. Without an output device the parameters still change and the picture still follows the
input.

**Simulation state beyond outputs.** D and T flip-flops: the last clock and last value; JK: the last clock, J and K; SR:
the last clock, S and R; the IC recurses into its internal elements; every other type: its outputs.

**The IC instance** holds its definition's reference (`m_file`, `m_blobName`); the blob bytes last loaded, which skip a
rebuild when unchanged; its own copies of the internal elements, wires and boundary ports, built per instance, with
their sorted order, boundary inputs and a feedback flag for its own simulation; a preview picture rendered at load; and
port counts set by the loader to the definition's. After plans 3, 4 and 6, every one of these is the reference,
derived from the shared definition, or simulation state in the core, so the IC needs no subclass then.

**Dead or unused:** `hasAudio` (MCP's audio branch, the `Audio` property type and the editor's tone box with its eight
note names have no user); Text's second default slot; Display7 null-checks its ports in `paint()` and the 14- and
16-segment displays do not, which is safe at their fixed counts.

## 3. What plan 3 builds

In build order; each step's pitfalls from the branch follow it.

### 3.1 Body goldens, first

Before any presenter replaces painting, goldens pin every type in all sixteen orientations and every picture state:
gates at 2 and 8 inputs, on and off, every LED colour and code, display colours and segment patterns, every rotary
count, Mux, Demux and truth-table counts, node modes and signal colours, audio playing, a custom PNG and SVG, an IC from
a file and embedded. A text golden holds the bounds, pivot, item transform, picture and drawing children; one PNG atlas
per type holds unselected and selected rows; memory types in both themes; about 4.1 MB in all. Rows plan 1's
port-layout golden already pins (gate counts 3 to 7) stay out of the atlases.

- The memory pictures' text is masked by its own glyph ink (each `<text>` drawn alone, upright at its transformed
  centre, dilated by 2 px), and a Text's bounds beyond its picture are masked; the masks are checked to hold every
  label.
- Pictures with a blur filter (`feGaussianBlur`: the clock, LED, switch, constant 1 and buzzer cases on the branch)
  render alike from Qt 6.8 on, so every pixel of them is compared. Before 6.8 they differ by up to 12 per channel, and
  the branch, whose CI ran 6.7.3, masked them there; the 6.8 floor leaves no Qt that needs the masks.
- Setting input statuses directly and then orienting re-lays the ports and resets unwired inputs to their defaults, so
  the displays were first pinned unlit; the goldens wire a VCC or GND to each input instead.
- Audio is pinned playing with its input wired, or `refresh()` stops it.

### 3.2 The rename

One compiler-checked commit renames `GraphicElement` to `Element`, `GraphicElementInput` to `ElementInput`,
`GraphicElementSerializer` to `ElementSerializer` and `Port::graphicElement()` to `element()`, with the names that
carry the old one (`m_graphicElement`, the theme's label colour field, the developer guide's anchor and tables), the
test classes included (on the branch, 289 files). It keeps the translation contexts `GraphicElement` and
`GraphicElementSerializer`, which the catalogues key on, and the `GraphicElements/` directory; include runs are
re-sorted case-insensitively, as the tree does, and the source, test and label lists re-sorted. Meta-object class names
reach neither files nor MCP output. The strings golden does not change.

### 3.3 Ports as data, with untranslated source names

- Each type gives `inputAttributes()` and `outputAttributes()` (a name, required, a default and an optional position)
  in place of the 15 `updatePortsProperties()` overrides; the base applies only the fields an element sets, in one
  helper for inputs and outputs. A test gives every port of every type its own value in each field its attributes
  leave unset and lists every field an update changes: the attribute design depends on unset fields being left alone,
  and a real path carries it (an edit reloads a live Rx node, `setWirelessMode(Rx)` returns early, and only the update
  leaving the requirement alone keeps the input optional through the edit, its undo and its redo). An input no
  attribute covers keeps its place; only a Demux file with surplus inputs reached that case, which plan 2's loader now
  refuses.
- A port takes a name source, evaluated on each request until it gets an explicit name; the displays use it. A derived
  name and its untranslated source are given together, from one `QT_TR_NOOP` literal per port (a helper builds the
  shown name through the element's `tr()` and keeps the source), so no derived port lacks its source and the 40
  segment names cannot drift. `sourceName()` gives the source while the name is derived and the explicit name
  otherwise, with no fallback to the translated text; an empty source fails a debug assertion. Tests: every port of
  every type at each count has a source name; every untranslated port name equals its source. The exporters take
  identifiers from these source names (plan 8), in place of the source names plan 2's ports hold beside their
  displayed ones.
- An IC's pins are plan 4's: it derives their names from the circuit, for the pins and the file header alike, which
  changes the format. Until then a pin standing for a display's port keeps the source name plan 2 gives it.
- Plan 1's port-layout golden pins every port's name, position, requirement and default for every type at every legal
  count and orientation, and is unchanged by this step.

### 3.4 Names and help text on demand

`translatedName()` and `helpText()` are looked up when asked, so the stored title, name, object name and tooltip go,
with `setPortName()` and the main window's per-element retranslation pass. Help follows Qt's rule in
`Scene::helpEvent()`: the topmost item with text, an element's text being its help text, a loaded IC having none (plan
2's IC tooltip fix, kept). Readers of `objectName()` move to `translatedName()`, the same string: serializer logs and
load errors, the Demux's load error, the morph's undo text, MCP's missing-port messages, test helpers. Tests: a name
and help text, a translator installed after construction, an IC from a file, a blob and unloaded in two languages, a
help event over an element, under a tooltip item and over empty space.

### 3.5 Descriptors and typed parameters

- `ElementMetadata` becomes `ElementTypeDescriptor` (compiler-checked), with a static `of()`, `add()` and `has()`,
  filled by each type's `ElementInfo<T>::describe()`. It holds the type's group, port rules, parameter schema,
  translation context and strings, help text, rotatable and label flags, operations and serialization, and names the
  simulation kernel with the parameters it reads, and an input type's source level, for plan 6. `master` has no
  propagation delay, so plan 3 adds none: plan 6 gives each logic type its delay and plan 7 makes it a parameter.
- A `ParamSpec` per parameter: an id, the default, the file key (none for a transient parameter), an accept rule (the
  old setter's), whether it is offered, and a range. A `Parameters` store with `param<T>()` and `setParam()`, which
  reports a saved parameter's change as one Parameter change and a transient one's as none; a type reacts in
  `parameterChanged()`. Moved into it: the input level and lock, the rotary position, the clock's frequency and delay,
  LED and display colours, the buzzer's frequency and volume, the audio box's file and volume, the node's mode and the
  truth table's bits. Accessors become non-virtual; the `has*` flags come from the schema (the LED offers its colour
  with one input only).
- One rule per parameter (P3.D7): the spec's accept rule is the setter's, as files read it, and states what a file
  outside it loads as; MCP takes the spec's range; the editor offers the spec's range or a subset its widget needs (the
  clock's 0.1 to 50 Hz), named in the spec. Each change to what an entry point accepts is listed in the commit and the
  release notes; on the branch MCP's volume became [0, 1] and its frequency's minimum of 0 went.
- The dead tone property goes everywhere: the `hasAudio` flag, the `Audio` property type, the editor's tone box with
  its eight note names, the morph's copy, MCP's branch; the buzzer's `setAudio(note)` becomes `setNote()`. The audio
  feature test then checks the audio capabilities that remain, or goes: on the branch it was left asserting nothing.
  The rotary's pixmap loaded in its constructor and never used goes with its subclass.
- A test checks every type's schema against its old `save()` and `load()` keys and defaults, each accept rule and what
  a change is (102 cases on the branch), and is shown to fail by mutations of the schema.

### 3.6 Serialization by schema

The types with their own maps lose `save()` and `load()`; the serializer writes the keyed parameters as the map
after the common part (a snapshot every key; a portable file omits a default unless the spec keeps it, as the truth
table's does; file references through `forWriting`, a resource omitted in a portable file) and reads each key through
`setParam()`. Older versions become data and functions of the spec: from which version a type has a map (Node from
4.4, the truth table from 4.2); from which version a value means something (the clock's delay before 4.3 is dropped);
older keys (the buzzer's `note`); the bare values per type before 4.1, Display7's two input permutations included; and
a rule applied on every load (the rotary's position drives the outputs a load adds). The volume takes the nearest
float, as `toFloat()` did. The old loaders' writes when a map lacks a key were unreachable and go. Byte identity is
plan 1's payload goldens, unchanged by this step. The IC keeps its own `save()` and `load()` until its internal
elements go; the Demux keeps its input-count check, a port rule. The round-trip fuzzer compares every keyed parameter
except file references. Mutations the suite had not caught before the branch's tests: the every-load rule, the
portable file keeping a default, the volume's float.

### 3.7 Appearance slots saved by key

Each slot of a type's descriptor has a key, each saved appearance names its slot, and an older file's positional list
is read through the descriptor, which keeps the old positions, a dropped slot's included (P3.D8). Today the loader
throws on an entry past a type's slots (`GraphicElementSerializer.cpp:431-433`), so no type can drop or reorder one.
Text's second default picture, which nothing shows, then goes. It extends the current format, as every plan does:
older files read as before, a file the application writes reads back and saves again byte for byte, and the extension
is tested by reading files written before it (the backward-compatibility files and plan 1's payload goldens); the
payload goldens of files with appearances change, each named by this cause.

### 3.8 The change funnel

Every setter of saved state ends in one `Element::changed(ElementChanges)` with the flags Placement, Parameter, Body,
Label and Ports: it re-measures the bounds, repaints unless only the placement changed (a move keeps the device
cache), and notifies. A change batch holds the reports until the outermost scope ends, so a rotation, a layout or a
load is one report, also when it throws. `boundingRect()` is final and returns the bounds Qt was last given (measured
on entering the scene), and `prepareGeometryChange()` is called while the cached value stands, as Qt's
before-the-change contract wants. A picture change repaints, and reports Body only when the bounds moved. A switch's
or rotary's level is a Parameter change; the truth table has a const `key()` and a `setKeyBit()`. A test runs every
change every type offers and asserts the flags, bounds equal to the measured ones, one report per change, Body when
the bounds move, nothing for a picture alone, and a report from a change that throws.

### 3.9 Presenters

- The model declares `BodyShape`, `BodyMeasure` and `BodyPainter`; presentation installs an `ElementPresenter` per type
  into a registry through one explicit call, `installPresenters()`, made by the application and the test runner as
  they start, before any element exists (P3.D10). A test that links the model alone installs a stub or nothing. The
  branch installed them from a static initializer, which a static library drops when nothing references its object
  file, and which held only because the same file defined the lookup every caller used. The element keeps its
  body, measured at every flush of changes and every picture change (which sets the pivot); measuring the bounds is
  not virtual, and `paint()` is final. A test compares the stored body with a fresh measure after every operation of
  every type.
- The model declares a `PictureLibrary`, which presentation implements: one entry per file version (path, modification
  time, size), SVG variants built from the source read at load, per picture. A presenter gives the picture slot shown
  (0 by default); an element reports a replaced picture on a state change; the LED's slot is the one at its last
  refresh.
- Picture presenters: picture, on/off, LED, audio, rotary (marks and arrow), one segment-display presenter with a
  per-input segment table for 7, 14 and 16 segments (the null check only Display7 made goes), and Text. Footprint
  presenters: Mux, Demux and the truth table (pivot at the centre), and the IC, which draws its DIP package coloured
  by whether it is embedded; `ICRenderer`'s body moves here, and its preview is plan 9's. The sizing pictures go. An
  unloaded IC turns about its footprint's centre, which keeps plan 2's fix for a flipped IC reopened from a file.
- The node's indicator and Text's hint become the body's decoration, drawn beyond the body and neither selected, hit
  nor aligned; no child items are left. The outline is the body with its ports, and the shape is the outline when
  decorated, which align, distribute, the clone drag, the label editor and zoom to fit use.
- **Themes.** Memory types name their Light picture, and the pictures map a Light-folder resource to its Dark sibling;
  one type-picture lookup (palette, memory tab, menus) replaces the factory's pixmap; themed pictures are measured on
  the Light picture (the two sizes are equal, which a test checks); a presenter draws in the theme it is handed, never
  the current one (picking the current theme was a defect the branch made and fixed), and the theme attributes record
  which theme they are. Plan 2's theme fix for these pictures stays green.
- **Labels.** The model measures the label (anchor, turn, box) before the body at each flush; a presenter's default
  anchor is (0, 64), the node's (0, 32), the audio types' (64, 34), the truth table's and the IC's below the
  footprint, and the IC's label turns 90°. A label change is told once at the batch's end, and a port re-layout
  reports Label only when the label moved. The box is, per line, the sum of the advances by
  `ceil(QFontMetricsF::height())`; text with a combining mark is measured laid out, since HarfBuzz inserts a dotted
  circle (U+25CC) for a broken syllable; the drawn area pads half a line height above and below and a quarter on each
  side; Text's bounds take the label's box. `QStaticText` is not used, since it would change pixel goldens. In Qt 6
  `QString::toUcs4()` returns `QList<uint>`.
- **The element painter** draws the selection, then the body, wholly in the theme it is handed: one place for the
  selection rectangle the five painters repeat today. An element's own `paint()` equals the painter's in the current
  theme, which a test checks.

Each family moves with its `measure()` and `paintBody` shown equal to the old code before the old code goes, and the
body goldens and plan 1's goldens unchanged.

### 3.10 The picture identity key

A struct compared exactly, not a hash, so no collision can hand out another element's picture: the picture variant
shown in the Light theme (one variant for every orientation of a picture without baked-in text); the type only where it
draws more than its picture; packed state (dial positions, lit segments and colour, node mode and status); an empty
Text's hint; a footprint's rectangle; and the orientation the truth table and the IC counter-orient their icon for. It
leaves out the selection and the position (P3.D4). Its test runs every body state by orientation by theme: bodies within
bounds, the same state the same key, selection and position not in it, one key one drawing, one drawing one key per
orientation (equivalent orientations may differ in key). On the branch it found the display defect plan 2 fixes and the
unlit display goldens. Plan 9's sprites are keyed by it.

### 3.11 Pictures that change or fail

- A custom picture that no longer loads shows its slot's default and the restore goes on, in a state change and in a
  snapshot's restore alike; opening a file still reports it (P3.D6). Plan 2 brings it to `master` (its §3.17), and
  plan 3 keeps it. On the branch a slot whose file failed showed the other state's default, nothing reported it, and
  undoing a morph whose picture was gone threw and leaked.
- A new or failed version of a picture file is the picture of every element showing that path, so each of them
  measures it again and its bounds follow the picture it draws; on the branch only the element that loaded it was
  measured. A slot with no default of its own shows the type's first default.
- A body can hold translated text (an empty Text's hint), so every element measures its body again on a language
  change, from the scene's retranslation. The branch first fixed the hint's place at one line high so the body would
  not follow the language, which let a longer translation draw past the body, then reversed that. The test installs a
  translator that tags every string (a wider hint), retranslates, checks the body grew to hold the tagged hint, then
  removes the translator and checks the body is back to its first bounds. Plan 2's fix for the hint's text stays
  green.

Each of the three was a separate fix on the branch, found after its picture library landed.

### 3.12 Commands and MCP on the schema

- Plan 2's property command moves onto the schema: a property is a parameter, the label or the trigger key, held as a
  typed value; the command holds each element's before and after and writes only those; `apply()` sets the values,
  undoing them all on a refusal with its reason, and returns the command, or none when nothing changed; commands of one
  edit session merge when they name the same elements and properties, and merging back to the start leaves no step.
  Every command of a session names the same elements, an unchanged one included. The snapshot command stays for
  appearances and operations. The editor's property descriptors and `editableProperties()` go: the editor shows fields
  from the schema's flags, and plan 1's property goldens list them unchanged.
- The duplicate Tx label rule (plan 2) is the schema's, for every writer.
- MCP reads and writes properties through one table of keyed properties, which serves `list_elements` and
  `set_element_properties`, validating every value before setting any (plan 2's rules, now the schema's); the label
  applies to every element (MCP and the IC generators label gates; the editor shows it only where `hasLabel()`);
  `list_elements` reports a node's wireless mode. Colours are a schema enum of the five names; a trigger follows the
  rule plan 2 gave MCP, the editor's (P2.D6).
- A test checks the MCP schema file against the descriptors (keys, kinds, ranges); on the branch, making it pass
  removed the dead audio key, let `update_element` take the wireless mode, bounded the volume to [0, 1] everywhere,
  removed the frequency's minimum of 0 and said that an empty appearance string restores the default. The Python
  client's models follow.

### 3.13 Operations

An `ElementOperation` per descriptor: a press operation and a release operation (the switch toggles, the button presses
and releases, the rotary steps). `Element::operate()` sets parameters and ignores the lock (the caller checks it); a
presenter's `activationAt()` decides which operation a click at a point triggers, the press operation anywhere by
default. The scene dispatches as Qt did: a left press to the unlocked pressed element (a double-click that Qt delivers
as a press too), a left release to the element holding the mouse, and trigger keys, each inside plan 2's operation
recorder. The scene records which element last held the mouse after each press: a double-click on another element is a
press that operates it, and on the element that held the mouse it edits the label. No input includes the scene. Scene
event tests written first pass on the old code too, which shows the move keeps behaviour; mutations only added tests
caught on the branch: the middle button on press and on release, and the lock on release and on trigger keys.

### 3.14 Audio in an application service

One service owns playback, driven by the simulation's change notifications, with the tone generator and the mute per
document as today (P3.D5). On `master` the simulation's visual push calls each output element's `refresh()`, and
`AudioOutputElement::refresh()` plays while its input is Active and stops otherwise; the mute is set through the
simulation's host (`Simulation::setUserMuted()`). The service takes play and stop from that push, as a notification
of the audio elements whose input changed, and plan 6's runner sends the same notification. It keeps plan 2's
teardown order (the sink before the generator). A manual pass plays every audio element: a buzzer at several
frequencies and volumes, an audio box with its file, mute per tab.

### 3.15 One element class

- The element's signals (the label edit, the IC's, the truth table's) are emitted by the scene's interaction code,
  which also tracks hovering over an IC's body (Qt's hover dispatch today).
- The remaining casts of elements to their subclasses become type checks or descriptor flags. Pointers to elements
  become ids in plan 9, which takes `QObject` away.
- The subclasses are deleted family by family, each with its tests moved onto the factory, parameters and operations;
  the base's parameter accessors go last. Shared helpers in `Tests/Common` move first, and every deletion is checked by
  the compiler, never a silent no-op.
- The IC keeps its subclass through this plan: its internal elements and its own serialization go with plans 4, 6 and
  8 (§8).

### 3.16 The model never includes presentation

Plan 9 makes every layer a build target with an include check. Presentation first exists in this plan, so from the
change that adds it an include check fails when a file the model owns includes a presentation header (P3.D3); plan 9
extends the check to all six layers. A split into targets alone rejects nothing, since the source root is a global
include directory and one precompiled header holds every module, so the check is what enforces the direction. It is
shown to fail by a model file given a presentation include.

## 4. Steps and slices

| Row | Step | Slice |
|---|---|---|
| P3.1 | The type catalogue (§2) checked against `master`'s code again, each difference corrected before anything moves | 1 |
| P3.2 | Body goldens (§3.1) | 1 |
| P3.3 | The rename (§3.2) | 1 |
| P3.4 | Port attributes in place of the overrides; unset fields kept (§3.3) | 2 |
| P3.5 | Untranslated source names for ports (§3.3) | 2 |
| P3.6 | Names and help text on demand (§3.4) | 2 |
| P3.7 | The include check for the model's direction (§3.16) | 3 |
| P3.8 | The descriptor (§3.5) | 3 |
| P3.9 | The parameter schema and store; one rule per parameter; the tone property removed (§3.5) | 3 |
| P3.10 | Serialization by schema (§3.6) | 3 |
| P3.11 | The change funnel (§3.8) | 4 |
| P3.12 | The presenter registry, the body's measure and shape, and the picture library (§3.9) | 4 |
| P3.13 | Picture presenters, footprint presenters, the IC's body, decorations and the outline (§3.9) | 4 |
| P3.14 | Themes and labels out of the element; the element painter (§3.9) | 5 |
| P3.15 | The picture identity key (§3.10) | 5 |
| P3.16 | Pictures that change or fail; bodies measured again on a language change (§3.11) | 5 |
| P3.17 | Appearance slots saved by key (§3.7) | 6 |
| P3.18 | The property command on the schema; the editor's property descriptors gone (§3.12) | 6 |
| P3.19 | MCP through the schema; its schema file checked against the descriptors; the Python client's models (§3.12) | 6 |
| P3.20 | Operations (§3.13) | 7 |
| P3.21 | The audio service (§3.14) | 7 |
| P3.22 | The element's signals to the scene's interaction code; casts to type checks (§3.15) | 7 |
| P3.23 | The subclasses deleted family by family, tests moved to the factory; the base's accessors last (§3.15) | 8 onward |
| P3.24 | The manual pass: the property editor on every type, and every audio element (§3.14) | last |
| P3.25 | The developer guide's element model and the release notes' list of changes (ranges, keyed slots) | last |

The hand-over to plan 8 in §9 is a row too (P3.26).

## 5. Tests and pitfalls

Beside each step's own:

- The first simulation update after a structural edit builds the simulation and resets every clock HIGH on `master`
  (plan 2 keeps the clocks), so a test of a fast path builds first and is shown to fail.
- The morph's undo holds owning pointers until its elements replace the old ones; casting `reserve()` sizes needs
  `std::size_t` (`-Werror=sign-conversion`).
- The editor's undo tests cover a multi-selection, since the label-overwrites-fields defect shows only there.
- Unity builds join anonymous namespaces of 8 sources: two helpers named `byId` collided on the branch; helpers go in
  named namespaces.
- A string that moves keeps its translation context (`GraphicElement`, `GraphicElementSerializer`, each subclass's
  own); the strings golden shows any that changes, and is updated in the same commit.
- A designated initializer that leaves out a field with no default fails `-Wmissing-field-initializers` under CI's
  warning set, as one did on the branch.
- `-Wnull-dereference` at `-O2` after `qgraphicsitem_cast` following a type test: cast in the condition.

## 6. Decisions

Each ends with its basis: measured or tested (a run backs it), read (the code, with nothing run), or a ruling (the
user's choice between workable options).

- **P3.D1. Element types as data.** One element class; a descriptor per type (ports, parameter schema, simulation
  kernel, strings, help text, flags, operations, serialization); typed parameters checked against the schema; an `IC`
  subclass only while its internal elements exist. Basis: tested on the branch (the payload goldens unchanged; the
  schema test's 102 cases); the user's ruling, 2026-10-03/04.
- **P3.D2. `GraphicElement` renamed `Element`, with its family, by compiler-checked mechanical renames, one commit
  each,** keeping the translation contexts and the `GraphicElements/` directory. Basis: tested by the compiler on the
  branch; the user's ruling, 2026-10-03/04.
- **P3.D3. The model declares what it needs from presentation and never links it** (measuring, shaping and painting a
  body; the picture library), and an include check enforces the direction from the change that adds presentation,
  which plan 9 extends to every layer. Basis: the user's ruling, 2026-10-03/04, and the principle that layering is
  enforced by build targets, not described; the check's timing is this file's, since presentation first exists here.
- **P3.D4. A picture's identity leaves out the selection; the renderers draw the selection.** One drawing serves a
  selected and an unselected element; plans 9 and 10 draw the selection in each backend. Basis: tested on the branch
  (one key per drawing and one drawing per key over every state, orientation and theme); the user's ruling,
  2026-10-03/04.
- **P3.D5. Audio in an application service,** driven by the simulation's change notifications, the tone generator and
  the mute per document as today. Basis: the user's ruling, 2026-10-03/04.
- **P3.D6. A custom picture that no longer loads shows its slot's default,** in a state change and a snapshot's
  restore alike, and the restore goes on; opening a file still reports it. Basis: tested on the branch; the user's
  ruling, 2026-10-17.
- **P3.D7. One rule per parameter, in its spec.** The accept rule is the old setter's, which files read (byte
  compatible), and says what a file outside it loads as; MCP takes the spec's range; the editor offers it or a named
  subset. Basis: read on `master` (the three entry points disagree, §2.4); accepted with the restart's write-up,
  2026-10-21.
- **P3.D8. Appearance slots saved by key,** an older file's positional list read through the descriptor, which keeps
  the old positions, a dropped slot's included; Text's unused second default picture goes. It extends the current
  format. Basis: read on `master` (the loader throws on an appearance past a type's slots); the user's ruling,
  2026-10-20.
- **P3.D9. The inventory of what constructs the element subclasses is taken where it is used,** on `master`, before
  the subclasses go: about 2,360 constructions in 165 test files, plus 219 `new IC(`; the tests move to the factory
  first, each deletion checked by the compiler. Basis: measured on `master` for the restart, and again on the branch at
  P3.1; the user's ruling, 2026-10-04.
- **P3.D10. The presenters are installed by one explicit call,** `installPresenters()`, which the application and the
  test runner make as they start, not by a static initializer: the dependency is visible where the program starts,
  holds under any linker and any initialization order, and a test that links the model alone can install a stub.
  Basis: read on the branch (its static initializer held only because the file that defined it also defined the
  presenter lookup every caller used); the user's approval, 2026-10-21.

## 7. Risks

| Risk | Mitigation |
|---|---|
| Schema-driven serialization changes a type's bytes or its reading of older versions | Field order from the catalogue (§2); byte-identical round trips in plan 1's payload goldens and the backward-compatibility files |
| Presenter extraction changes a picture or bounds (custom painters, rotation and flip, SVG counter-rotation, labels) | Measure and paint shown equal per family before the old code goes; the body goldens |
| A moved or renamed class takes its strings into a new translation context | Contexts kept on purpose; the strings golden |
| A pixel-changing change that does not notify | One change funnel; a test over every change of every type |
| Tests construct and drive the subclasses that go (about 2,360 constructions in 165 files) | Shared helpers in `Tests/Common` move first; every deletion compiler-checked, never a silent no-op |
| Audio changes when playback leaves the elements | The same sink and player code in the service; a manual pass over every audio element |
| An element is measured before the presenters are installed, or a program never installs them | One explicit installation call at the start of the application and the test runner (P3.D10); a test that every type has a presenter; a debug assertion when an element is measured with none installed |

## 8. Claims to check and open questions

None. The glows' redrawing without blur filters is plan 9's question, with its sprites; plan 3 keeps every picture
as it is.

## 9. What other plans take from plan 3

- **Plan 4:** the IC's footprint presenter and its turning centre; the descriptors and the schema its definitions read;
  the ports' source and derived names, from which it derives an IC's pin names; the IC's own serialization until its
  definition owns it.
- **Plan 5:** the descriptors' kernel names and parameters, which its spec and reference read.
- **Plan 6:** each descriptor's kernel and source level; it adds each logic type's propagation delay to the
  descriptor, and its runner's change notifications drive the audio service.
- **Plan 7:** the schema, where the delay becomes a parameter with its file key, editor and MCP field.
- **Plan 8:** untranslated source names for every port, held by the descriptors in place of plan 2's; and the IC's
  subclass, which outlives this plan, since its internal elements and its own serialization go later: plan 8's last
  step, which deletes the internal elements, folds it into the one element class (P8.15), closing P3.26.
- **Plan 9:** one element class, the presenters, the appearance key for its sprites, the element painter, the outline
  and decorations, the include check it extends to six layers; pointers to elements become ids there.
- **Plan 10:** the appearance key and the presenters, which its GPU sprites draw from.

## 10. The branch's work, as evidence

Each step was built on `gpu-canvas` (or on a branch before it), where its commits show the change and its tests; plan 3
writes its own on `master`. A hash is the branch's, kept read-only.

| Step | Commits on the branch |
|---|---|
| The type catalogue | `.claude/TYPES_gpu_canvas.md` on the branch |
| Body goldens | `c35e219b1` |
| The rename | `300df2b6e` |
| Port attributes | `b2a3f77f1`, `a1adb7eeb`, `d5c3cb6b0`, `e809bd4a5`, `605f54098`, `c02bd0be3`, `33dfd89fb`, `19a4307e2` |
| Source names | `06faed468`, `868758dc4`, `83f127181`, `e187dad16` |
| Names and help on demand | `1172ef8b2` |
| The descriptor | `2e107c117` |
| Parameters and the schema; the tone property | `eaafef3f4`, `df128e8e1` |
| Serialization by schema | `eea660c18` |
| Presenters, pictures, footprints, decorations | `4e6ce0c4f`, `63b108b56`, `7308ac519`, `473ea1afe`, `aee21af47`, `992b36c30` |
| Themes and labels; the element painter | `66cdeaa19`, `2bbf319cb`, `25029647d`, `360445c7f` |
| The picture identity key | `07358be36` (the display fix `871390d61` and the unlit goldens' `834051236` before it) |
| Pictures that change or fail; the hint's measure | `1ba794d7e`, `92ff60bc4`, `848e92ef8`, `eb5ddd9f2`, `0848a90c3` |
| Commands, the editor and MCP on the schema | `aabb6d806`, `dd0ed8fe0`, `90a651f46`, `22e6f99ce`, `0a5c5c1c7`, `18dec210c`, `afa255b54`, `4242fcacc` |
| Operations | `099425e53` |
| The layer targets and their include check | the branch's `CMakeLists.txt` (`add_wiredpanda_layer`) and `cmake/CheckIncludes.cmake` |

The branch never built the audio service, the move of the element's signals, the casts, the subclasses' deletion or
the keyed slots; the earlier `perf/load-hotspots` prototype holds the audio service's intent.
