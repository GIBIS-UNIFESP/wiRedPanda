# Temporal Waveform canvas overflow — UI freeze + setMinimumSize warning spam

> **Relocation note:** the widgets described here were later restructured into the Live
> Analyzer page of BeWavedDolphin (`App/BeWavedDolphin/LiveAnalyzer.{h,cpp}`:
> `AnalyzerCanvas` / `AnalyzerTimeRuler` / `AnalyzerLabelColumn` / `LiveAnalyzerPanel`);
> the standalone Temporal Waveform dock no longer exists. The fixes below (canvas clamp,
> sliding `timeOrigin()`, exposed-region painting, sticky-tail follow) carried over intact.

## Symptom

Testing the binary with `ic.panda`: open the Temporal Waveform dock, Watch All, run in
temporal mode. The whole app becomes progressively slower and the terminal spams:

```
QWidget::setMinimumSize: (/TemporalWaveformWidget) The largest allowed size is (16777215,16777215)
```

## Root cause

`TemporalWaveformWidget` models scrolling by growing the canvas widget to the full
recorded timeline: `sizeHint()` width = `maxTime * m_pixelsPerNs`, clamped only to
`INT_MAX`. Qt hard-caps widget dimensions at `QWIDGETSIZE_MAX` (16'777'215 px). Sim time
grows 1e9 ns per wall-second at 1x speed, so at any meaningful zoom (wheel zoom goes up to
10 px/ns) the demanded width passes the cap within seconds.

Past the cap, three compounding failures:

1. `QWidget::setMinimumSize()` emits the warning **before** its same-value early-out, so
   every call with an over-limit hint warns — and `paintEvent()` re-called it every frame,
   because the clamped `size()` could never equal the over-limit hint.
2. Each `setMinimumSize()` re-invalidated layout mid-paint → relayout → repaint → a
   self-sustaining repaint loop pinning the GUI thread.
3. Each repaint painted the **entire timeline**, not the exposed viewport:
   `drawTimeRuler()` iterated one tick per ~100 px across the whole canvas (~167k
   `drawLine`+`drawText` per frame at the cap) and `drawTrace()` linearly scanned every
   recorded transition of every trace from t = 0.

## Fix (`App/UI/TemporalWaveformWidget.{h,cpp}`)

- `sizeHint()` clamps both dimensions to `QWIDGETSIZE_MAX` → the hint settles, no warning,
  no relayout churn.
- New `timeOrigin()`: once the timeline no longer fits within `QWIDGETSIZE_MAX` at the
  current zoom, the canvas freezes at the cap and the origin slides so the window always
  covers the **newest** data at the user's chosen zoom (older data reachable by zooming
  out). The right-edge auto-follow in the dock keeps showing live data.
- `paintEvent()` converts the exposed paint rect (± 120 px margin for boundary labels and
  2 px edges) to a sim-time window; the ruler ticks only that window and `drawTrace()`
  binary-searches (`std::lower_bound`) its first visible transition with entry level from
  `statusAt()`. Repaint cost is now O(viewport), independent of recording length. Rows
  fully outside the exposed rect are skipped.
- `timeToX()` computes its delta signed — `SimTime` is unsigned, and a ruler tick rounded
  down past the window start would otherwise wrap to a huge positive x.

## Regression test

`TestTemporalSimulation::testWidgetCanvasClampedAtQtLimit` — 6 s recording at 10 px/ns
(demands 6e10 px): asserts the hint clamps to `QWIDGETSIZE_MAX`, the origin slides to keep
the newest sample on-canvas, `QTest::failOnWarning` on the setMinimumSize message, and a
`grab()` of an exposed strip renders. Verified to fail pre-fix
(`hint.width() = 2147483647` vs 16777215).

## Follow-up: auto-follow disengaged under normal growth

User follow-up ("isn't the viewer supposed to auto-scroll/follow?") exposed a second,
pre-existing defect: the follow was a fixed-threshold check on the 100 ms refresh timer
(`hBar->value() >= hBar->maximum() - 4` → pin). The canvas only grows inside the *later*
paint, so each tick compared a value pinned at the previous maximum against a maximum
that had grown afterwards — at the auto-adapted zoom that's ~viewport/50 px per tick
(≈20 px ≫ 4 px slack), so the follow disengaged permanently the moment the recording
outgrew the viewport; the trace ran off the right edge with the view stuck at the start.

Fix (`MainWindow::toggleTemporalWaveformDock()`): sticky tail on the scroll bar's own
signals — `valueChanged` tracks whether the user is at the right edge
(`m_waveformFollowTail`, 4 px slack), `rangeChanged` pins to the new maximum while
engaged. Growth arrives *as* `rangeChanged`, so no step can be missed at any rate;
scrolling back releases the pin, scrolling to the edge re-engages it. The timer lambda
now only repaints.

Regression test: `TestMainWindowGui::testWaveformViewerFollowsNewestData` (pin on
growth / stay pinned / hold position after scroll-back / re-engage at the edge; drives
the recorder directly and forces synchronous paints via `grab(QRect)`). Verified to fail
pre-fix exactly like the report: value stuck at 0 vs maximum 999256.

## Residual (known, accepted)

- Transition history still grows unboundedly in memory (~16 bytes/transition); painting no
  longer cares, but multi-hour sessions accumulate. Bounding the history is a feature
  decision (drops scroll-back data), not part of this fix.
- Past the cap, a scrolled-back view slides with the window (content shifts as the origin
  advances). Only occurs where the old behavior froze the app entirely.
