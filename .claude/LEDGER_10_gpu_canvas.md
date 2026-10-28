# Ledger: plan 10, GPU canvas

Tracks the items of `.claude/PLAN_10_gpu_canvas.md`. The plan says what and why; this file says where each item
stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`P10.7a`, `P10.7b`, ...) and a line in the log. Status: `todo`, `doing`, `done`
(committed), `blocked` (on what, in the notes) or `dropped` (why, with the user's decision). The Commits column lists
every commit of the item. Every status change gets a dated line in the log.

**Branch:** `10-gpu-canvas`, from `09a-model-cost` once the model-cost round has landed; not created yet.

**Open decisions:** none yet; P10.22 (what fast mode does on the GPU) is settled once P10.17 has measured the budgets,
and P10.25 needs the user before Automatic picks the GPU.

Each row's notes say how its check is shown to fail; "as in a spike" marks a break a spike ran and saw fail.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| P10.1 | Build plumbing: the option, the shaders, CI's modules, the dev container, packaging, the documents' lines; the shells in the GPU target (§2.1, §2.11, §2.12) | todo | | Shown to fail: CI, the release or the translation workflow configured without `qtshadertools` fails the job instead of building CPU-only |
| P10.2 | The render pass, pipelines and bindings; the offscreen and Null test harness (§2.13) | todo | | Shown to fail: a job with `WIREDPANDA_REQUIRE_RHI` set and no backend fails instead of skipping |
| P10.3 | The host as the view's viewport, its real-input check and the two tab-switch questions (§2.1) | todo | | Shown to fail: a viewport whose paint and resize the view keeps never renders (as in a spike); P10.20 answered here |
| P10.4 | The per-tab lifecycle (§2.9) | todo | | Shown to fail: a hidden tab that keeps its GPU resources |
| P10.5 | The startup probe, the setting with its override (Automatic still the CPU), the fallback (§2.10) | todo | | Shown to fail: a CPU adapter, or pipelines that do not build, taken as the GPU |
| P10.6 | The mirror: slots, the free list, dirty ranges, the listener's changes, the dirty-marking and from-scratch tests (§2.2) | todo | | Shown to fail: a change kind left out of the dirty marking, against the mirror built from scratch |
| P10.7 | Wires: the curve in the shader, the status gather, the style table, selection, the halo, the wire being routed, the long-wire threshold (§2.3, §2.4) | todo | | Parity renders against the CPU backend on llvmpipe and Metal |
| P10.8 | Ports (§2.5) | todo | | Parity renders |
| P10.9 | Elements: quads, sprite atlases per zoom level, the budgeted re-render, eviction, every picture-key class, memory (§2.6) | todo | | Parity renders; shown to fail: a sprite kept after a change its key misses |
| P10.10 | Labels: shaping, the glyph atlas, right-to-left and CJK (§2.7) | todo | | Parity renders |
| P10.11 | The background shader and the overlays in the shared draw order (§2.8) | todo | | Parity renders |
| P10.12 | Automatic picks the GPU, every layer passing its parity renders (§2.10) | todo | | After P10.25 |
| P10.13 | Device loss, pipeline and shader failure, the Sentry tags (§2.9, §2.10) | todo | | Shown to fail: after a fallback to the CPU backend, the minimap's view rectangle stops following a resized window |
| P10.14 | The GPU target's include rules enforced (§2.1) | todo | | Shown to fail: a `QRhi` header included outside the GPU target |
| P10.15 | The build documents and the developer guide (§2.15) | todo | | |
| P10.16 | The manual pass (§2.15) | todo | | |
| P10.17 | The budgets on real hardware, Linux and Windows (§6) | todo | | Pan, zoom and recolour under 3 ms of CPU; a drag frame under 6 ms; the 8,000-element circuit's first frame under 100 ms |
| P10.18 | The macOS speed (§6) | todo | | Unmeasured: no machine |
| P10.19 | Whether plan 9's listener is enough for the mirror (§6) | todo | | Settled by plan 9's P9.29 and, for an optimisation that changes the listener's events, the model-cost round's MC.11, both before this plan starts; P10.6's coverage test confirms it |
| P10.20 | The host across tab switches (§6) | todo | | Settled by P10.3 |
| P10.21 | The long-wire threshold (§6) | todo | | From the parity renders |
| P10.22 | What fast mode does on the GPU (§6) | todo | | After P10.17 |
| P10.23 | The atlases' memory and churn (§6) | todo | | The IC-heavy and 20,000-element circuits; zooming over many custom bodies |
| P10.24 | The GPU path on real drivers (§6) | todo | | A manual pass on Linux, Windows and a Mac where one is at hand |
| P10.25 | What the user sees new (§6) | todo | | Needs the user: multisampled wires, the setting and where it is offered |

## Log

- 2026-10-21: the plan file and this ledger written on `gpu-canvas`, every row `todo`.
- 2026-10-21: the Qt floor raised to 6.8 at the user's ruling ("go with 6.8"): P10.D8 says 6.8, CI's `QRhi` coverage
  is 6.8 and 6.9, and the pitfall of blur filters before 6.8 went, no supported Qt having it. No row changes.
- 2026-10-28: P10.D1's basis states the comparison with Qt Quick, which cost the same per frame and lost on its
  hosts, its software fallback and its runtime libraries; kept at the user's ruling ("lets keep it"). No row
  changes.
