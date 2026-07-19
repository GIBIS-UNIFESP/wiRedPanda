// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Phase 7f of the qtquick-rewrite plan: real coverage for CanvasItem's zoom ladder
/// (zoomIn()/zoomOut()/resetZoom()/zoomToFit()/canZoomIn()/canZoomOut()), mirroring
/// Tests/Unit/Scene/TestGraphicsView.cpp's own scenarios against GraphicsView's identical
/// -9..7/1.25x/0.8x zoom-step design (Phase 4's pan/zoom follow-up already claimed exact
/// parity with that ladder -- this is the permanent test that claim never had).
/// TestCanvasItemInteraction (Phase 7e-4) already exercises zoomIn/zoomOut/resetZoom/zoomToFit
/// indirectly through their Ctrl+=/Ctrl+-/Ctrl+0/Ctrl+Shift+F keyboard shortcuts; this file adds
/// the genuinely new coverage those tests don't reach: the min/max zoom-level *limits* (50
/// iterations against the real -9..7 clamp) and a real widescreen-circuit zoomToFit scenario.
///
/// Two of TestGraphicsView's seven tests have no CanvasItem equivalent to port, by design, not
/// oversight -- confirmed by reading CanvasItem.h/.cpp in full, not assumed:
/// - testFastMode: GraphicsView's QPainter render-hint toggle (Antialiasing/TextAntialiasing/
///   SmoothPixmapTransform) exists specifically to work around QGraphicsView's CPU-raster cost
///   under load. CanvasItem's GPU-composited QSGGeometryNode rendering has no equivalent
///   per-hint toggle -- the whole point of the GPU migration was not needing this hack. Same
///   disposition as Scene.h's wire-antialiasing-degradation system (already a documented
///   deletion, not a migration, in this plan's Context section).
/// - testDragModeToggle: GraphicsView's QGraphicsView::ScrollHandDrag/NoDrag is a settable mode
///   swapped in/out around a drag gesture. CanvasItem's own pan (middle-button or space+drag) is
///   unconditional, gesture-driven state (m_panning/m_spacePanHeld), not a settable mode -- no
///   equivalent concept exists to test.
///
/// One real, confirmed gap, deliberately NOT silently dropped: testAccessibleNameSet
/// (GraphicsView::setAccessibleName()/setWhatsThis(), a real "#14 accessibility sweep" feature)
/// has no Quick-side equivalent at all -- grepping the whole App/QuickShell/ tree found zero
/// Accessible.* usage anywhere. This is a genuine, currently-unported accessibility feature, not
/// a testing gap this pass can close (there is nothing yet to test) -- tracked as a real
/// follow-up, out of scope for a test-authoring sub-step.
class TestQuickCanvasZoom : public QObject
{
    Q_OBJECT

private slots:
    void testZoomOutHitsMinimumLimit();
    void testZoomInHitsMaximumLimit();
    void testResetZoomRestoresBothDirections();
    void testZoomToFitFitsSpreadOutCircuitAndZoomsOut();
};
