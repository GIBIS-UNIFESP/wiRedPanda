# Port warnings redesign

## Status

Implemented the pure port-validity model and moved warning presentation to pull-based UI.

## Changes

- Port issues are derived from `InputPort::isValid()` at query time.
- `ElementPorts` clears port owner back-pointers before Qt destroys child ports.
- Connection and port changes repaint only the owning element.
- Rejected wires and audio/logic compatibility do not create element warnings.
- Missing IC warnings are no longer stored on elements.
- Element tooltips contain only translated element names.
- Invalid ports show translated `Not connected` or `Multiple drivers` hover chips.
- The warnings panel uses stable element IDs and is refreshed after circuit changes and file loads.
- The redundant `ElementWarningLabel` implementation was removed.

## Validation

- WSL debug library build passes with Unity enabled.
- Full application and test builds are in progress.
