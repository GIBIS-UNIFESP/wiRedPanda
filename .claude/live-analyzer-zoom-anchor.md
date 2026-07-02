# Live Analyzer: zoom drifted to t = 0 and read "everything low"

## Symptom (user report)

Fit shows waveforms correctly, but repeated zoom-in makes every trace render as a flat
LOW long before reaching the ns window where gate delays live. Pre-dates the Latest Edge
feature.

## Root cause (verified failing-first; messages below are the actual pre-fix output)

1. **Zoom was unanchored.** `AnalyzerCanvas::zoomIn()/zoomOut()` only rescaled
   `m_pixelsPerNs`; `QScrollArea` preserves the raw PIXEL scroll value across the canvas
   resize (`QAbstractSlider::setRange` clamps, never rescales). The sim-time at the
   viewport's left edge is `origin + value/ppn`, so every zoom-in halved every visible
   timestamp — the view slid exponentially toward t = 0 instead of magnifying what was on
   screen. Pre-fix test failure: *"zoom-in step 0 moved the viewport center from
   500000000 ns to 250000000 ns"*. At 1x (1e6 ns/tick → 1e9 ns per wall-second) a 30 s
   recording spans 3e10 ns: 4–5 clicks from Fit put the window inside [0, ~1 s]; the
   ns-resolving zoom is ~25 clicks away.
2. **The pre-record region was painted as definite LOW.** `SignalTrace::statusAt()`
   returns `Inactive` before the first recorded sample and `drawTrace()` drew that as a
   normal low line in the trace color. Watch All is normally clicked after the run
   started, so the drifted-to-t≈0 window sat entirely before every trace's first sample →
   all traces flat low, indistinguishable from real data.
3. **Fit fit to the wrong width.** `zoomFit()` used the canvas's own `width()`, not the
   scroll-area viewport — once the canvas had grown, Fit spanned the recording across the
   huge canvas (pre-fix failure: ppn stayed 0.01 after Fit on a 1e7 px canvas).
4. **The canvas never shrank.** The paint-time `setMinimumSize(hint)` grows a too-small
   canvas immediately (Qt resizes up to a raised minimum) but a smaller hint never shrank
   an already-grown one, so stale width kept feeding the scroll ranges.

## Fix

- `LiveAnalyzerPanel::applyAnchoredZoom(targetPpn, anchorViewportX)`: capture the instant
  under the anchor at the old scale → rescale → settle the paint-time canvas resize
  (`settleCanvasGeometry()`, the `grab(QRect(0,0,1,1))` trick shared with
  `zoomToLatestEdge()`) → re-derive the scroll position from the instant (applied ppn and
  sliding-window origin re-read after the rescale; signed delta clamps instead of wraps).
  - Toolbar +/− → panel `zoomIn()/zoomOut()` anchored at the viewport center.
  - Ctrl+wheel → canvas emits `zoomStepRequested(direction, canvasX)` (it no longer
    rescales itself); the panel anchors at the cursor.
- Panel `zoomFit()`: `ppn = (viewport − 20)/maxTime` (the sizeHint's +20 px tail pad),
  scroll home; the horizontal range collapses to 0, which re-engages sticky-tail follow.
  `AnalyzerCanvas::zoomFit()` removed.
- `paintEvent`: `setMinimumSize(hint)` + `resize(hint)` so the canvas also shrinks.
- Pre-record no-data rendering: `drawTrace()` leaves the region before a trace's first
  sample blank (dotted baseline only) instead of asserting a LOW that was never observed.

## Tests

- `TestBewavedDolphinGui::testLiveAnalyzerZoomKeepsAnchor` — viewport-center instant fixed
  across 5 zoom-ins and a zoom-out (pre-fix: halves each step).
- `TestBewavedDolphinGui::testLiveAnalyzerCtrlWheelZoomsAtCursor` — instant under the
  cursor fixed across a Ctrl+wheel zoom.
- `TestBewavedDolphinGui::testLiveAnalyzerFitSpansViewport` — after deep zoom, Fit spans
  the recording across the viewport, scrolls home, no horizontal range left.
- `TestTemporalSimulation::testCanvasWheelZoomRequiresCtrl` — updated to the
  request-signal contract (plain wheel ignored, no self-rescale).
- `TestTemporalSimulation::testCanvasPreRecordRegionBlank` — no trace pixels before the
  first recorded sample.

## Follow-up (user feedback on the first iteration)

1. **Wheel zoom required Ctrl, unlike the stimulus editor.** The grid page consumes ANY
   plain wheel as zoom (`BewavedDolphin::eventFilter`, "the original beWavedDolphin
   behavior"); the analyzer page ignored a bare wheel, so it appeared to have no wheel
   zoom. Now any wheel over the canvas emits the cursor-anchored zoom request — the wheel
   never scrolls the trace list (same convention as the grid: scrollbars pan).
2. **Button zoom centered even while following the tail.** With `m_followTail` engaged
   (the normal state after Watch All / Fit), a center anchor slid the newest data off the
   right edge and silently disengaged the follow. `buttonZoomStep()` is follow-aware:
   while following, rescale + re-pin at the scroll maximum (scope-style, zooming in and
   out alike); otherwise center-anchor as before.
3. **`zoomFit()` now sets `m_followTail = true` explicitly** — `setValue(0)` emits no
   `valueChanged` when the bar is already at 0, so the recompute-on-scroll hook alone
   could leave the follow stale.

Tests: `testLiveAnalyzerButtonZoomFollowsTail` (failing-first: "first zoom-in: view must
stay pinned at the tail (value 3121, max 3338)"), `testLiveAnalyzerWheelZoomsAtCursor`
(plain wheel), `testCanvasWheelZoomAlwaysRequests`, and a post-Fit pinned zoom-in step in
`testLiveAnalyzerFitSpansViewport`.
