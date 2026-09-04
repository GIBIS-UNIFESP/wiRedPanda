// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Tests CanvasItem's zoom ladder (zoomIn()/zoomOut()/resetZoom()/zoomToFit()/canZoomIn()/
/// canZoomOut()), mirroring Tests/Unit/Scene/TestGraphicsView.cpp's scenarios against
/// GraphicsView's identical -9..7/1.25x/0.8x zoom-step design. TestCanvasItemInteraction already
/// exercises zoomIn/zoomOut/resetZoom/zoomToFit indirectly through their Ctrl+=/Ctrl+-/Ctrl+0/
/// Ctrl+Shift+F keyboard shortcuts; this file adds the min/max zoom-level limits (50 iterations
/// against the real -9..7 clamp) and a widescreen-circuit zoomToFit scenario.
///
/// Two of TestGraphicsView's seven tests have no CanvasItem equivalent:
/// - testFastMode: GraphicsView's QPainter render-hint toggle (Antialiasing/TextAntialiasing/
///   SmoothPixmapTransform) works around QGraphicsView's CPU-raster cost under load.
///   CanvasItem's GPU-composited QSGGeometryNode rendering has no equivalent per-hint toggle.
/// - testDragModeToggle: GraphicsView's QGraphicsView::ScrollHandDrag/NoDrag is a settable mode
///   swapped in/out around a drag gesture. CanvasItem's own pan (middle-button or space+drag) is
///   unconditional, gesture-driven state (m_panning/m_spacePanHeld), not a settable mode.
///
/// testAccessibleNameSet (GraphicsView::setAccessibleName()/setWhatsThis()) also has no
/// Quick-side equivalent -- App/QuickShell/ has no Accessible.* usage anywhere yet.
class TestQuickCanvasZoom : public QObject
{
    Q_OBJECT

private slots:
    void testZoomOutHitsMinimumLimit();
    void testZoomInHitsMaximumLimit();
    void testResetZoomRestoresBothDirections();
    void testZoomToFitFitsSpreadOutCircuitAndZoomsOut();
};
