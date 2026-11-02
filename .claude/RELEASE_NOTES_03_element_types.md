# Release notes: plan 3, element types as data

Draft of what plan 3 changes that a user, a file or an MCP client can see, for the release notes of the version that
ships it (P3.25). Each item names the ledger row it comes from. Changes inside the code that nothing outside can see
(the element subclasses gone, the parameter store, the presenters) are under "For contributors" at the end.

## Files

- **Appearances are saved by slot name** (P3.17). A custom picture is saved under its slot's key (`body`, `on`,
  `off`...) instead of only its position in a list. Each entry still sits at its slot's old position, so a released
  version opening a newer file reads every slot it knew as before. A file saves only the slots showing a picture of
  the user's. Older files read as before.
- **A slot added by a later version is refused by older ones.** A released version refuses an entry past its type's
  slots, so a file customising a slot that a later version adds will not open in an older version. The version adding
  such a slot says so in its own notes.
- **Slots no element showed are gone**: the rotary switch's arrow slot and the text element's second default picture
  (P3.17, P3.D8a). An older file naming either still opens; the entry is skipped.
- **Volumes are saved as the decimal they mean** (P3.29). A volume set by the slider was saved as the nearest float
  (0.3499999940395355 for 0.35); it is now saved as 0.35, and an older file's volume is read as that decimal. A volume
  in a file that is not a finite number (infinity, NaN) loads as the element's default instead of being kept.
- No format revision: files saved by this version open in the previous release, and the backward-compatibility files
  read as before.

## Property editor

- **The Tone box is gone** (P3.9a). It offered eight note names but applied to no element type; the buzzer's
  frequency is its tone.
- **A refused wireless label undoes the whole edit** (P3.18b). Giving a second transmitter the label another
  transmitter has is refused as before, and now every change of that edit (label, mode) is undone with it.
- **Relabelling an IC no longer reads its file** (P3.18b): the inline label editor sets the label alone, so a
  file-backed IC whose file is broken can still be relabelled.

## Pictures

- **A picture file changed on disk reaches every element showing it** (P3.16b): each element using that file takes the
  new version, and if the file stops loading, each shows its slot's default and the failure is reported.

## Audio

- **Playback moved from the elements to one audio service** (P3.21). Buzzers and audio boxes play and stop as before
  (an input Active plays), mute is per tab, and a pause works as before. **Not yet confirmed by ear**:
  the manual pass (P3.24) still has to hear every audio element, including a frequency change while playing.

## MCP

- **Values are checked against each property's range** (P3.19a, P3.19c): a volume within [0, 1], a delay within
  [-1, 1], a wireless mode within [0, 2], an LED's or display's colour among the five names (White, Red, Green, Blue,
  Purple), and a frequency by the element's own rule (a clock's must give a half-period of at least a microsecond, a
  buzzer's must be finite and positive). Before, the volume was not bounded and the frequency had a minimum of 0
  instead of its rule.
- **What an element holds is reported as it is**: `list_elements` reports a file's colour or volume outside what MCP
  may set without changing it, and reports a node's `wireless_mode`.
- **A request's property writes are one undo step and all or nothing** (P3.19b): a refused second transmitter label
  undoes every property the request set; a request with a rotation or an appearance is one step too.
- **The buzzer's default volume reads 0.35** (P3.29), not 0.3499999940395355.

## For contributors

- `GraphicElement` is `Element` (P3.3), and every element type but the IC is an `Element` of its type, described by
  its `ElementTypeDescriptor`: its kernel (logic in `KernelLogic`), its parameters (`ParamSpec`), its ports, its
  operations and its appearance (P3.23). There are no per-type classes to subclass or include.
- A type's settings are parameters: `offersParam()`, `param<T>()` and `setParam()` replace the per-setting accessors
  (`setFrequency()`, `color()`, `hasColors()` and the rest), with `ElementColour` for the colour cycle and
  `WirelessNode` for a node's mode (P3.23h). Setting a parameter the type lacks, or a value of another kind, asserts in
  a debug build.
- Drawing is the presentation layer's (`App/Presentation/`): a presenter per type draws its body, picture and label
  (P3.12 to P3.14).
- The developer guide describes the model as it is now (P3.25).
