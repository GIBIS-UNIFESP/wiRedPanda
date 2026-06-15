// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief DolphinZoom: zoom state and metrics for the beWavedDolphin waveform table.
 */

#pragma once

class QTableView;

/**
 * \class DolphinZoom
 * \brief Owns the waveform table's zoom state and the math that maps it to view metrics.
 *
 * \details Two independent axes: a discrete column-zoom level (Zoom In/Out/Reset) that
 * widens columns only — a horizontal "time stretch" — and a uniform Fit Screen scale that
 * scales columns, rows, and font together. Holds a non-owning pointer to the table view and
 * resizes its Fixed-mode header sections + font on apply(). The controller keeps the UI glue
 * (sentry breadcrumbs, guarded slots, action enable-state, the wheel event filter).
 */
class DolphinZoom
{
public:
    /// Constructs the zoom controller for \a view (not owned; must outlive this).
    explicit DolphinZoom(QTableView *view);

    /// Increases the discrete column-zoom one step (capped) and re-applies.
    void zoomIn();
    /// Decreases the discrete column-zoom one step (floored at the baseline) and re-applies.
    void zoomOut();
    /// Resets both axes to the baseline (level 0, scale 1.0) and re-applies.
    void reset();
    /// Computes a uniform scale that fits the content in the viewport and re-applies.
    /// On degenerate geometry (empty/hidden table) it leaves the zoom untouched.
    void fitScreen();
    /// Applies the current zoom to the view's row/column section sizes and font.
    void apply() const;

    /// Current discrete column-zoom level.
    int zoomLevel() const { return m_zoomLevel; }
    /// Current uniform Fit Screen scale.
    double fitScale() const { return m_fitScale; }
    /// True if Zoom In is still possible (below the max level).
    bool canZoomIn() const;
    /// True if Zoom Out is still possible (above the baseline).
    bool canZoomOut() const;

private:
    QTableView *m_view = nullptr; ///< The waveform table (not owned).
    int m_zoomLevel    = 0;       ///< Discrete column-width zoom step; column width only.
    double m_fitScale  = 1.0;     ///< Uniform Fit Screen scale; columns + rows + font.
};
