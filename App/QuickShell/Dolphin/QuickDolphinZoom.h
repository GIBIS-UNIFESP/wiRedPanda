// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickDolphinZoom: CanvasItem-side port of App/BeWavedDolphin/DolphinZoom.h.
 */

#pragma once

#include <QtGlobal>

/**
 * \class QuickDolphinZoom
 * \brief Quick-side port of DolphinZoom's zoom state and metrics math.
 *
 * \details Same two independent axes as DolphinZoom (a discrete column-zoom level widening
 * columns only, and a uniform Fit Screen scale affecting columns/rows/font together) and the
 * same formulas/constants (kDefaultColumnWidth/kDefaultRowHeight/kZoomStep/kMaxZoomLevel/
 * kMinFitScale/kMaxFitScale) -- only the "apply to a live view" half differs, since there is no
 * QTableView/QHeaderView here to call setDefaultSectionSize() on. Instead this class exposes
 * columnWidth()/rowHeight()/fontScale() as plain computed properties; SignalTable.qml's
 * TableView reads them from its rowHeightProvider/columnWidthProvider callbacks and calls
 * TableView::forceLayout() after any zoom change (see QuickDolphinController's zoom slots).
 *
 * fitScreen() is a simplification of DolphinZoom::fitScreen()'s iterative fixed-point
 * convergence: the original loop re-solves against the post-resize header size using
 * QHeaderView's real sizeHint(), available synchronously mid-computation. Quick's
 * HorizontalHeaderView/VerticalHeaderView resize through the normal property-binding/layout pass
 * instead (not synchronously queryable mid-call), so this version is single-pass: the caller
 * supplies the *current* (pre-update) header sizes as an estimate. This can under/overshoot
 * slightly on an extreme single zoom jump (e.g. resetZoom() immediately followed by fitScreen()
 * on a very differently-sized table).
 */
class QuickDolphinZoom
{
public:
    static constexpr int kDefaultColumnWidth = 38; ///< Per-column pixel width at zoom 1.0.
    static constexpr int kDefaultRowHeight = 30;    ///< Per-row pixel height at zoom 1.0.

    QuickDolphinZoom() = default;

    /// True if Zoom In is still possible (below the max level).
    [[nodiscard]] bool canZoomIn() const;
    /// True if Zoom Out is still possible (above the baseline).
    [[nodiscard]] bool canZoomOut() const;

    /// Increases the discrete column-zoom one step (capped).
    void zoomIn();
    /// Decreases the discrete column-zoom one step (floored at the baseline).
    void zoomOut();
    /// Resets both axes to the baseline (level 0, scale 1.0).
    void reset();
    /// Computes a uniform scale that fits \a columns x \a rows into a
    /// \a viewportWidth x \a viewportHeight viewport, minus \a headerWidth/\a headerHeight (the
    /// current row/column header sizes -- see this class's own doc comment on why this is an
    /// estimate, not a re-solved value). No-op on degenerate geometry (empty table, zero-sized
    /// viewport, or a non-positive resulting scale).
    void fitScreen(qreal viewportWidth, qreal viewportHeight, qreal headerWidth, qreal headerHeight,
                   int columns, int rows);

    /// Current discrete column-zoom level.
    [[nodiscard]] int zoomLevel() const { return m_zoomLevel; }
    /// Current uniform Fit Screen scale.
    [[nodiscard]] double fitScale() const { return m_fitScale; }

    /// Current column width in pixels (kDefaultColumnWidth scaled by both zoom axes).
    [[nodiscard]] qreal columnWidth() const;
    /// Current row height in pixels (kDefaultRowHeight scaled by the Fit Screen axis only).
    [[nodiscard]] qreal rowHeight() const;
    /// Current font scale (Fit Screen axis only -- column zoom leaves text untouched).
    [[nodiscard]] qreal fontScale() const { return m_fitScale; }

private:
    int m_zoomLevel = 0;
    double m_fitScale = 1.0;
};
