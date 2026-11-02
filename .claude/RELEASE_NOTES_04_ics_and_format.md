# Release notes: plan 4, ICs and the file format

Draft of what plan 4 changes that a user or a file can see, for the release notes of the version that ships it. Each
item names the ledger row it comes from. Changes that nothing outside can see are under "For contributors" at the end.

## Files

- **An IC's pins keep their order** (P4.12). A file saves each IC pin's place in the order the pins had, as the
  `inputOrder` and `outputOrder` metadata, so the pins of an IC keep their order when the file is opened again. A file
  that holds no order gives its pins the order their positions give, as before.
- **Elements snap to the grid when a file opens** (P4.12). An element a file saved off the half grid (8 px) opens on the
  nearest grid line, so the circuit's elements line up as a placement leaves them. Saving the file again writes the
  snapped positions.
- **A document keeps the IC files it uses** (P4.6, P4.6a). A document keeps a copy of each file-backed IC's definition,
  and of the files those ICs nest. Opened with a file gone, it keeps the circuit it had; opened with a file changed since
  the save, the new definition comes in as one step that undo reverts.
- **No format revision.** The pin-order keys are added metadata. Files saved by this version open in the previous
  release, and the backward-compatibility files read as before.

## IC pins

- **A pin takes its name in the current language** (P4.4). A pin of an IC shows its name in the interface language
  it is shown in, and changes when the language does. The saved file still holds the source-language name.
- **Clock and LED pins no longer carry a suffix** (P4.4). A pin named after a clock's frequency or an LED's colour now
  carries the plain name, as the element does.

## Reloading an IC's file

- **Undo of a reload restores the definition in memory** (P4.5, P4.8). Undoing the reload of an IC whose file changed
  brings back the definition the IC had before the reload.

## For contributors

- **A definition is parsed once and shared** (P4.2): every IC that loads the same `.panda` bytes holds one parsed
  `ICDefinition`, kept under a byte budget once no IC holds it.
- **A hover preview is rendered once per definition and directory** (P4.3): `ICPreviewCache`, LRU over 16 entries,
  dropped on a theme change. The origin check in the preview guard is defence-in-depth: no element reaches it, since a
  NaN origin also gives a NaN width, which the size check already catches.
- **Pin names are derived when asked** (P4.4): `Port::nameFunction()` returns the name as a function, and a proxy node
  keeps it, so a derived name stays derived across the IC boundary. `genericProperties()` is removed.
- **Undo and redo are probed item by item** (P4.13), and a failed IC save rolls back its earlier targets (P4.14), with
  the seam `WorkSpace::saveChildICBlob` for a test to fail a later one.
