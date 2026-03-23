// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/WaveformView.h"

#include <QCursor>
#include <QWheelEvent>

WaveformView::WaveformView(QWidget *parent)
    : GraphicsView(parent)
{
}

int WaveformView::zoomLevel() const
{
    return m_zoomLevel;
}

bool WaveformView::canZoomOut() const
{
    // Level 0 is the baseline (reset) zoom; don't allow zooming below it
    return m_zoomLevel > 0;
}

bool WaveformView::canZoomIn() const
{
    // Cap at 6 discrete steps to prevent the waveform from becoming too large to render
    return m_zoomLevel < 6;
}

void WaveformView::zoomIn()
{
    ++m_zoomLevel;
    emit zoomChanged();
}

void WaveformView::zoomOut()
{
    --m_zoomLevel;
    emit zoomChanged();
}

void WaveformView::resetZoom()
{
    m_zoomLevel = 0;
    emit zoomChanged();
}

void WaveformView::wheelEvent(QWheelEvent *event)
{
    const int zoomDirection = event->angleDelta().y();

    // When redirect is enabled, the actual scaling is handled by BewavedDolphin
    // (which also resizes the table columns). Without redirect, the view scales itself.
    if (zoomDirection > 0 && canZoomIn()) {
        if (m_redirectZoom) {
            emit scaleIn();
        } else {
            zoomIn();
        }
    } else if (zoomDirection < 0 && canZoomOut()) {
        if (m_redirectZoom) {
            emit scaleOut();
        } else {
            zoomOut();
        }
    }

    // Keep the waveform centered under the cursor rather than jumping to the scene origin
    centerOn(QCursor::pos());

    event->accept();
}

