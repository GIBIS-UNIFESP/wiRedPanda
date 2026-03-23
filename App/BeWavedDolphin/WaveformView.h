// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief WaveformView: zoom-limited GraphicsView for the beWavedDolphin waveform editor.
 */

#pragma once

#include "App/Scene/GraphicsView.h"

/**
 * \class WaveformView
 * \brief GraphicsView subclass with waveform-editor zoom behaviour.
 *
 * \details Overrides the wheel event to zoom the entire scene rather than scroll,
 * and provides a discrete zoom-level range (0–6) suitable for waveform display.
 * Level 0 is the baseline (reset) state.
 */
class WaveformView : public GraphicsView
{
    Q_OBJECT

public:
    /// Constructs the waveform view.
    explicit WaveformView(QWidget *parent = nullptr);

    /// Returns the current discrete zoom level (0 = reset, max = 6).
    int zoomLevel() const;

    /// Returns true if further zoom-in is possible.
    bool canZoomIn() const;

    /// Returns true if further zoom-out is possible.
    bool canZoomOut() const;

    /// Resets zoom to the default level.
    void resetZoom();

    /// Increases zoom by one step.
    void zoomIn();

    /// Decreases zoom by one step.
    void zoomOut();

protected:
    void wheelEvent(QWheelEvent *event) override;
};

