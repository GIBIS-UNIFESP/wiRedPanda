# Ledger: plan 3, element types as data

Tracks the items of `.claude/PLAN_03_element_types.md`. The plan says what and why; this file says where each item
stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`P3.13a`, `P3.13b`, ...) and a line in the log. Status: `todo`, `doing`, `done`
(committed), `blocked` (on what, in the notes) or `dropped` (why, with the user's decision). The Commits column lists
every commit of the item. Every status change gets a dated line in the log.

**Branch:** `03-element-types`, from `02c-exception-boundaries` at `dd502ee6e`, created 2026-10-30.

**Open decisions:** none.

Each row's notes say how its check is shown to fail; "as on the branch" marks a break `gpu-canvas` ran and saw fail.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| P3.1 | The type catalogue checked against `master`'s code with plan 2's fixes in, each difference corrected in the plan file (§2) | done | `43ab3fd9d` | The catalogue was taken on the branch after its first three phases. Checked on `03-element-types` against each type's metadata, `save()`, `load()`, setters, `paint()` and handlers, the serializer and the scene: §1's counts retaken; §1's table, §2.1's load order, §2.4's sound file, IC keys and Clock and volume rules, §2.5's pointer and key recording corrected; §2.3 matched |
| P3.2 | Body goldens of every type, orientation and picture state, every blurred pixel compared (§3.1) | done | `547f7119a` | Shown to fail: an altered pixel in the And atlas, and one in the clock's blurred glow (a pixel where its on and off cells differ by about 60 beside the lit square), each by more than the tolerance. The glyph masks went: plan 2 drew the memory pictures' letters as outlines (P2.92), compared like every pixel. The Text mask holds: with the hint's and the labels' font tripled and the hint painted, every golden made at the normal size passes; the first mask, beyond the picture's right and bottom only, failed that check (the selection outline moved on the other sides and its corners bent into the picture), so the mask is the cell but the picture's inside. The ports and the label, child items here, left out (port-layout and label goldens). Passes on Qt 6.12.0 (relwithdebinfo) and 6.9.3 (debug); 4.5 MB. Tip checked 2026-10-30: suite 228/228 on relwithdebinfo, the source compiled alone and at `-O3` with GCC 15 and 13, `fix_style.py` clean |
| P3.2a | The body goldens' Mux and Demux atlases differ on CI's Debian i386 job | done | `547f7119a` | Build 41077764147 at `24a2aeeed`: 128 pixels each by at most 8 per channel (Mux first at 361,349, Demux at 374,349); every other job green. Both are drawn in code. Reproduced in the i386 container (Debian Qt 6.8.2): exactly one pixel per cell, rows of Mux data 3 to 6 and the matching Demux rows, on the inner trapezoid's round-joined corner (integer coordinates), grey 207 for 199. Debian trixie amd64, the same Qt 6.8.2, passes; our code built on i386 with `-msse2 -mfpmath=sse` renders byte-identical atlases and still fails: the rounding is inside Debian's i386 Qt library, not our code. Fixed at the user's ruling ("go ahead" on giving that renderer its own baselines, not a skip or a looser comparison): `BodySnapshot/i386/` holds the Mux and Demux atlases as the i386 container draws them, read only on 32-bit x86 and compared as exactly. In the container: passes; the shared `Mux.png` put back as the i386 one fails. On amd64 and on 6.9.3 (debug): passes; an i386 copy equal to the shared atlas fails, and a stray file there fails. Regenerating on i386 also rewrote the shared `InputRotary.png` (drawn apart within the tolerance), so the test's comment says shared goldens are regenerated on a 64-bit platform |
| P3.3 | `GraphicElement` renamed `Element`, with its family, in one commit (§3.2) | done | `31955ece6` | Checked by the compiler; the strings golden unchanged. 293 files: the classes, their files, `Port::element()`/`setElement()`, `m_element`, `m_elementLabelColor`, the three test classes, the serializer's debug type name, the developer guide (its aligned columns realigned by hand), the fuzz harnesses and the exception inventory's rows. The element's one `tr()` now names its "GraphicElement" context; the serializer keeps `Q_DECLARE_TR_FUNCTIONS(GraphicElementSerializer)`. Shown to fail: `tr("Default")` put back moves the string to the `Element` context and fails the strings golden. No meta-object class name is read in `App` or the MCP server. Tip checked 2026-10-30: all relwithdebinfo targets build without a warning, suite 228/228, debug (Qt 6.9.3) builds, Doxygen 1.16.1 clean, the inventory check passes, `fix_style.py` clean; the eight changed fuzz harnesses compile with GCC against the renamed code (the pre-rename one fails), a rename needing no coverage round |
| P3.4 | Port attributes in place of the 15 overrides, in one helper; unset fields kept (§3.3) | todo | | Shown to fail: each guard applying a default instead, and a port with no attributes made required (as on the branch, 42 fields over 20 types); the helper's default dropped fails the port-layout golden |
| P3.5 | Untranslated source names for ports (§3.3); an IC's pins are plan 4's (P4.4) | todo | | Shown to fail: a hand-written source with a typo, a helper that leaves the description untranslated, the old fallback to the translated name (as on the branch) |
| P3.6 | Names and help text on demand; the stored names, the tooltip and the retranslation pass gone (§3.4) | todo | | Shown to fail by mutations of the lookups (six on the branch); plan 2's IC tooltip and Text hint tests stay green |
| P3.7 | The include check: no file the model owns includes a presentation header (§3.16) | todo | | Shown to fail: a model file given a presentation include. Plan 9 extends it to six layers |
| P3.8 | `ElementMetadata` becomes `ElementTypeDescriptor` (§3.5) | todo | | Compiler-checked |
| P3.9 | The parameter schema and store; one rule per parameter; the tone property and the rotary's unused pixmap removed (§3.5) | todo | | The schema test (102 cases on the branch), shown to fail by mutations; each change to what an entry point accepts listed for the release notes |
| P3.10 | Serialization by schema, older versions as functions (§3.6) | todo | | Plan 1's payload goldens unchanged. Shown to fail: the every-load rule, the portable default and the volume's float, each mutated (as on the branch) |
| P3.11 | The change funnel (§3.8) | todo | | The test over every change of every type, shown to fail by a setter that does not report |
| P3.12 | The presenter registry, installed by one explicit call at the start of the application and the test runner (P3.D10); the body's measure and shape; the picture library (§3.9) | todo | | Shown to fail: without the measure at each flush, the stored body differs from a fresh one (four tests on the branch); a type without a presenter fails; an element measured with no presenters installed fails the debug assertion |
| P3.13 | Picture and footprint presenters, the IC's body, decorations and the outline (§3.9) | todo | | Measure and paint equal to the old code per family before it goes; body goldens unchanged; the outline's users tested |
| P3.14 | Themes and labels out of the element; the element painter (§3.9) | todo | | Shown to fail: a presenter drawing the current theme, not the one handed; themed sizes unequal |
| P3.15 | The picture identity key (§3.10) | todo | | Its test over every state, orientation and theme, shown to fail by each key field dropped |
| P3.16 | A failed slot shows its own default; a new file version re-measures every element showing it; a language change re-measures bodies (§3.11) | todo | | Each regression test fails with its defect put back (as on the branch) |
| P3.17 | Appearance slots saved by key; Text's second default picture gone (§3.7) | todo | | Read through the backward-compatibility files; the payload goldens of files with appearances change, each named |
| P3.18 | The property command on the schema; the Tx rule in the schema; the editor's property descriptors gone (§3.12) | todo | | Plan 2's command and editor tests stay green; plan 1's property goldens unchanged |
| P3.19 | MCP through the schema; the schema file checked against the descriptors; the Python client's models (§3.12) | todo | | Shown to fail: a schema key, kind, bound, the table and a type's range, each mutated (as on the branch) |
| P3.20 | Operations from the descriptor, picked by the presenter; the scene's dispatch (§3.13) | todo | | Scene event tests pass on the old code first; the middle button and the lock on release and trigger keys each caught |
| P3.21 | The audio service, from the simulation's visual push (§3.14) | todo | | A test of play and stop from the push and of the mute per document |
| P3.22 | The element's signals to the scene's interaction code; casts to type checks or flags (§3.15) | todo | | |
| P3.23 | The subclasses deleted family by family, tests moved onto the factory, the base's accessors last (§3.15) | todo | | Sub-rows per family. About 2,360 test constructions in 165 files, plus 219 `new IC(`, counted on the branch at P3.1 |
| P3.24 | The manual pass: the property editor on every type, every audio element (§3.14) | todo | | |
| P3.25 | The developer guide's element model; the release notes' list of changes (§4) | todo | | |
| P3.26 | The IC's subclass, folded into the one element class by plan 8's last step, with the internal elements (§9) | todo | | Carried by plan 8 (P8.15); closed here when that step lands |

## Log

- 2026-10-21: the plan file and this ledger written on `gpu-canvas`, every row `todo`.
- 2026-10-21: the Qt floor raised to 6.8 at the user's ruling ("go with 6.8"): blur filters render alike on every Qt
  it allows, so the body goldens compare every blurred pixel and need no blur masks (P3.2), and the row for the body
  goldens on Qt 6.7.3 went; the IC subclass's hand-over to plan 8, P3.27, is now P3.26.
- 2026-10-30: the branch created from `02c-exception-boundaries` at `dd502ee6e`, the exception boundaries' plan done
  with CI green; P3.1 started.
- 2026-10-30: P3.1 done (`43ab3fd9d`). Corrected: §1's counts (the class grew with plans 2 to 2c; 42 element
  `qobject_cast`s, 134 in tests, 32 item casts, 3 `QPointer`s, 2,360 test constructions in 165 files, 219 `new IC(`);
  the latches and flip-flops have no theme override, the flip-flops save their simulation state; §2.1's order and
  the IC's port names in a portable file; the sound file that is gone keeps its path (plan 2c); the IC's snapshot
  writes `files`; the scene records a press or a trigger key as one command; `load()`, not `loadState()`; the
  appearance throw at `GraphicElementSerializer.cpp:431-433`. The same count's 1,880 on `master` was 2,111: P3.23's
  note follows it.
- 2026-10-30: P3.2 done (`547f7119a`), recreated from the branch's test on this code: no glyph masks (the letters are
  outlines since P2.92), no blur masks (the 6.8 floor), the ports and the label left out as child items, the item's
  whole transform recorded (here the rotation is not in `transform()`), and the Text's cell masked but for its
  picture's inside after a tripled font failed the first mask. §3.1 follows it.
- 2026-10-30: P3.2a found (Build 41077764147 at `24a2aeeed`, the i386 job) and done the same day: Debian's i386 Qt
  rasterises with x87 floating point, shown with the same Qt on amd64 and our code built with SSE2 math on i386; at the
  user's ruling the Mux and Demux atlases got i386 baselines, folded into the body goldens' commit (`547f7119a`).
  Sanitizers 41077764405 green at `24a2aeeed`.
- 2026-10-30: P3.3 done (`31955ece6`), the rename as the user approved it for this plan (D23 on `gpu-canvas`): one
  regular transform, gated by the build, the suite and the strings golden.
