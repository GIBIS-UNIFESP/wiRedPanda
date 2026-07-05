# Exercises

An Exercise is a step-by-step circuit-building challenge, validated against the live scene as
the user works. Each `.json` file in this folder is one exercise; `App/Exercise/ExerciseEngine`
loads and drives it, `App/Exercise/ExerciseBrowserDialog` lists all of them for the user to pick
from.

## Adding a new built-in exercise (for contributors)

Drop a new `.json` file in this folder with a globally unique `id` (unique across **both**
`App/Resources/Exercises/` and `App/Resources/Tours/` — the translation catalog described below
is keyed by `id` alone, with no folder namespacing). Reconfigure (`cmake --preset debug`) so the
new file is picked up — no C++ or `.qrc` edit needed, it's discovered automatically. Then run the
catalog generator and commit the result:

```bash
python3 Scripts/generate_exercise_tour_catalog.py
```

## Adding your own exercise without building from source

The built-in exercises above ship compiled into the app. Anyone running an already-installed
copy of wiRedPanda can add their own exercises too — no C++ toolchain needed — by dropping a
`.json` file (same schema as below) into one of two other real, on-disk locations, each aimed
at a different audience. `App/Core/ExerciseTourResources::discover()` merges all of them (plus
the built-in content above) into one list every time the Circuit Exercises dialog opens; if an
`id` collides across sources, the built-in file wins and the duplicate is silently skipped.

**For an individual user**: click **"Open My Exercises Folder"** in the Circuit Exercises
dialog. It opens (creating if needed) a simple, visible folder next to the installed app —
normally `<install dir>/Exercises`. If that location isn't writable (e.g. installed under
`Program Files` without admin rights), the button transparently falls back to a folder under
your Documents folder instead (`Documents/wiRedPanda/Exercises`) — either way, whatever it
opens is where you drop your `.json` file; reopen the dialog and it appears.

**For a teacher/IT admin provisioning a shared/managed install** (e.g. a school computer lab
image or a login script): place `.json` files in
`<AppData>/wiRedPanda/Exercises` — the platform-specific location
`QStandardPaths::AppDataLocation` resolves to, e.g.
`%APPDATA%\GIBIS-UNIFESP\wiRedPanda\Exercises` on Windows,
`~/.local/share/wiRedPanda/Exercises` on Linux, or
`~/Library/Application Support/wiRedPanda/Exercises` on macOS. This folder is auto-created the
first time the dialog opens, is always scanned, but is **never** opened or created by the "Open
Folder" button and has no other in-app affordance — it's deliberately not something an ordinary
end-user needs to know about or stumble onto while browsing.

## Schema

```json
{
  "id": "basic-and-gate",
  "title": "Building an AND Gate Circuit",
  "description": "Learn to place logic gates, add inputs and outputs, and wire them together.",
  "steps": [
    {
      "key": "place-and-gate",
      "instruction": "Place an AND gate on the canvas",
      "hint": "Click the Gates tab in the left panel, then drag the AND gate onto the canvas.",
      "requiredElements": [
        { "type": "And", "minCount": 1 }
      ],
      "requiredConnections": [],
      "click": ["gatesTab"]
    }
  ]
}
```

| Field | Type | Required | Notes |
|---|---|---|---|
| `id` | string | yes | Globally unique (see above). Used as the progress/completion key and as the translation-catalog namespace. |
| `title` | string | yes | Shown in the exercise browser dialog. |
| `description` | string | no | Shown below the title in the exercise browser dialog. |
| `steps` | array | yes | At least one step. |
| `steps[].key` | string | no | Stable slug (e.g. `"place-and-gate"`), unique within this file's `steps[]`. Used **only** to build translation-catalog keys — never read by engine logic. Omit it and the step's text is simply not translatable (English-only fallback, not an error). Give every step a `key` unless you're certain it'll never need translation. |
| `steps[].instruction` | string | yes | Main instruction text shown while this step is active. |
| `steps[].hint` | string | no | Extra help text. |
| `steps[].requiredElements` | array | no | `{ "type": "<ElementType>", "minCount": N }`. The step auto-advances once the scene contains at least `minCount` elements of each listed type. |
| `steps[].requiredConnections` | array | no | `{ "fromType": "<ElementType>", "toType": "<ElementType>", "minCount": N }`. Counts connections whose source element is `fromType` and destination element is `toType`. |
| `steps[].click` | array of strings | no | Widget/action IDs handled by `MainWindow` before the step is presented. See "Closed widget vocabulary" below. |
| `steps[].context` | string | no | `""` or `"circuit"` (default) overlays on the main canvas; `"bwd"` overlays on the BeWavedDolphin waveform window instead. |

A step whose `requiredElements` **and** `requiredConnections` are both empty/absent is an
"observe" step: the engine never auto-advances it — the user must click Next/Finish. The last
example step in `basic-and-gate.json` (toggle the switches and observe the LED) is one of these.

### Element type names

`requiredElements[].type` / `requiredConnections[].fromType` / `.toType` must exactly match one
of the enum keys in `Enums::ElementType` (`App/Core/Enums.h`), resolved at load time by
`ElementFactory::textToType()` via `QMetaEnum` — case-sensitive, exactly as spelled there. A
handful for orientation: `"And"`, `"InputSwitch"`, `"Led"`, `"Clock"`. Check `Enums.h` directly
for the full list; an unrecognized name resolves to `ElementType::Unknown` and that requirement
can never be satisfied.

### Closed widget/action vocabulary

`click` IDs are a **closed, hardcoded** set understood only by `MainWindow::clickTarget()`
(`App/UI/MainWindow.cpp`) — not data-driven. Currently valid: `ioTab`, `gatesTab`,
`combinational`, `memoryTab`, `actionPlay`, `actionWaveform`. Referencing an ID outside this
list is silently a no-op. If your exercise needs to trigger something not in this list, add a
case to `clickTarget()` first — there's no way to do it from JSON alone.

## Translations

Step text (`instruction`, `hint`) and the root `title`/`description` never pass through Qt's
`tr()` — this is JSON, and `lupdate` only scans `.cpp`/`.h`/`.ui`. Instead, a separate,
Weblate-managed catalog carries the translations: `App/Resources/Translations/ExerciseTour/en.json`
is generated from this folder's content by `Scripts/generate_exercise_tour_catalog.py`, keyed
`<id>.title`, `<id>.description`, `<id>.<step key>.instruction`, `<id>.<step key>.hint`.
Non-English `ExerciseTour/<lang>.json` files are submitted by translators through Weblate's
normal web UI (a *separate* "JSON file" component from the app's main `.ts` component) —
translators never touch this folder's `.json` files directly. If a translation is missing or a
step has no `key`, the app falls back to the raw English text here.
