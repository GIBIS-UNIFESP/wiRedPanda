# Rendering baseline

Instructions under callgrind of `Scripts/profile_probe.sh baseline`, taken at the end of every plan (PLAN_01 §4.7,
P1.D8): each circuit opened by the probe, then 8 ticks of 501 ms of the probe's own time, so a 1 Hz clock flips at
every tick, each followed by a frame of the whole 1280×800 view, wire antialiasing held on; `run` at 1:1 at the
circuit's corner, `runfit` zoomed to fit. Only the phase is counted (`--toggle-collect`), not the load. Qt's hashes are
seeded with 0; two native runs of each phase print the same frames and outputs by status, so each count is taken once.
A relwithdebinfo build.

| Plan | Commit | Qt | Compiler | clocked_2000 run | clocked_2000 runfit | combinational_1000 run | combinational_1000 runfit | ic_heavy run | ic_heavy runfit |
|---|---|---|---|---:|---:|---:|---:|---:|---:|
| 1 | `61f552623` | 6.12.0 | GCC 15.2.0 | 20,661,867,938 | 3,744,965,284 | 187,668,977 | 620,535,775 | 52,476,187,621 | 10,637,974,430 |

## Plan 1

- `run` on `clocked_2000` and `ic_heavy` is over 91% `Connection::paint` → `QRasterPaintEngine::stroke`: the long,
  antialiased wires of each circuit's fan-out of 2,000, crossing the view at full scale.
- `combinational_1000` has no clock: its frames are of a circuit at rest. Zoomed to fit puts more of it in view, so
  `runfit` costs more than `run` there, where the clocked circuits' long wires make 1:1 the dearer.
- Whole native run (load included, 8 frames): clocked_2000 10.3 s / 5.5 s, combinational_1000 0.8 s / 1.1 s,
  ic_heavy 35 s / 22 s; all six under callgrind at once, 7 min 39 s.
