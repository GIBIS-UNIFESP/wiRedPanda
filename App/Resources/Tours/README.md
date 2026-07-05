# Tours

A Tour is a guided walkthrough of the UI: a sequence of spotlighted widgets with explanatory
callouts. Each `.json` file in this folder is one tour; `App/Tour/TourEngine` loads and drives
it, `App/Tour/TourBrowserDialog` lists all of them for the user to pick from.

For the general authoring workflow (adding a new file, running the translation-catalog
generator, the `key` field, how translation works, and the three discovery locations — built-in,
the install-relative/Documents-fallback pair opened by the **"Open My Tours Folder"** button,
and the AppData folder reserved for teacher/IT provisioning) see
[`../Exercises/README.md`](../Exercises/README.md) — it's identical for Tours, just with
"Tours" in place of "Exercises" throughout. This file covers only what's different: the
Tour-specific schema and the `target` vocabulary.

## Schema

```json
{
  "id": "ui-overview",
  "title": "wiRedPanda Interface Tour",
  "description": "A guided walkthrough of the main interface — palette, canvas, toolbar, and properties panel.",
  "steps": [
    {
      "key": "logic-gates",
      "title": "Logic Gates",
      "body": "The Gates tab contains AND, OR, NOT, NAND, NOR, XOR, and other fundamental logic gates.",
      "target": "gatesTab",
      "click": ["gatesTab"]
    }
  ]
}
```

| Field | Type | Required | Notes |
|---|---|---|---|
| `id` | string | yes | Globally unique across **both** `Exercises/` and `Tours/` (see Exercises README). |
| `title` | string | yes | Shown in the tour browser dialog. |
| `description` | string | no | Shown below the title in the tour browser dialog. |
| `steps` | array | yes | At least one step. |
| `steps[].key` | string | no | Same purpose as in Exercises — translation-catalog key only, unique within this file. |
| `steps[].title` | string | yes | Callout heading for this step. |
| `steps[].body` | string | no | Callout body text. |
| `steps[].target` | string | no | Which widget to spotlight — see "Closed `target` vocabulary" below. Empty string or `"none"` shows a centered callout with the whole window dimmed uniformly, no spotlight. |
| `steps[].click` | array of strings | no | Same as Exercises — widget/action IDs run before the step is presented. |

### Closed `target` vocabulary

Like `click` (shared with Exercises, see the Exercises README), `target` is a **closed,
hardcoded** set resolved only by `MainWindow::resolveTourTarget()` (`App/UI/MainWindow.cpp`) —
not data-driven. Currently valid: `toolbar`, `elementPalette`, `gatesTab`, `ioTab`, `canvasArea`,
`elementEditor`, `searchBar`, `bwd:tableView`, `bwd:toolbar`, `bwd:menuBar`, plus empty string /
`"none"` for the centered, un-spotlit callout. A tour step that needs to point at a widget
outside this list requires a `resolveTourTarget()` code change first.
