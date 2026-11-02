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
| 2 | `029057373` | 6.12.0 | GCC 15.2.0 | 21,970,251,552 | 3,463,272,333 | 225,322,558 | 620,623,122 | 52,511,646,927 | 10,642,411,072 |
| 02a | `a85c729ba` | 6.12.0 | GCC 15.2.0 | 21,975,138,126 | 3,463,103,827 | 225,285,134 | 620,569,055 | 52,505,558,738 | 10,635,928,101 |
| 02b | `787c5787f` | 6.12.0 | GCC 15.2.0 | 21,966,726,818 | 3,464,996,288 | 226,105,649 | 620,737,862 | 52,510,943,282 | 10,637,819,960 |
| 02c | `3505388d5` | 6.12.0 | GCC 15.2.0 | 21,972,066,145 | 3,463,907,129 | 225,283,300 | 627,932,558 | 52,515,928,273 | 10,636,806,372 |
| 02c | `0abd247c4` | 6.12.0 | GCC 15.2.0 | 21,972,066,145 | 3,463,907,129 | 225,283,300 | 627,932,558 | 52,515,928,273 | 10,636,806,372 |

## Plan 1

- `run` on `clocked_2000` and `ic_heavy` is over 91% `Connection::paint` → `QRasterPaintEngine::stroke`: the long,
  antialiased wires of each circuit's fan-out of 2,000, crossing the view at full scale.
- `combinational_1000` has no clock: its frames are of a circuit at rest. Zoomed to fit puts more of it in view, so
  `runfit` costs more than `run` there, where the clocked circuits' long wires make 1:1 the dearer.
- Whole native run (load included, 8 frames): clocked_2000 10.3 s / 5.5 s, combinational_1000 0.8 s / 1.1 s,
  ic_heavy 35 s / 22 s; all six under callgrind at once, 7 min 39 s.

## Plan 2

- The probe frames the circuit by `cachedCircuitBoundingRect()` from here on, where plan 1 used
  `cachedItemsBoundingRect()`: its 1:1 corner and its fit show a slightly different part of each circuit, so
  `clocked_2000` at 1:1 paints 7% more wire, zoomed to fit 7% less, and `combinational_1000` at 1:1 more wire (+18%) and
  more elements (+35%). The drawing per item did not change; every later plan frames alike.

## Plan 02a

- Within 0.4% of plan 2: the floor changes what the code may use, not the Qt it is built with here (6.12.0).

## Plan 02b

- Within 0.4% of the floor; every row is built with Qt 6.12.0, so the ceiling's move from 6.9.3 does not show here.

## Plan 02c

- Within 0.4% of the ceiling but `combinational_1000` zoomed to fit, 1.2% more.
