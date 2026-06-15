// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/DolphinZoom.h"

#include <algorithm>
#include <cmath>

#include <QAbstractItemModel>
#include <QApplication>
#include <QHeaderView>
#include <QScrollBar>
#include <QTableView>

static constexpr int    kDefaultColumnWidth = 38;    ///<  Per-column pixel width at zoom 1.0 (matches the pre-refactor on-screen size).
static constexpr int    kDefaultRowHeight   = 30;    ///<  Per-row pixel height at zoom 1.0 (matches the pre-refactor on-screen size).
static constexpr double kZoomStep           = 1.25;  ///<  Multiplicative factor per column-zoom step.
static constexpr int    kMaxZoomLevel       = 6;     ///<  Maximum discrete column-zoom step (baseline = 0).
static constexpr double kMinFitScale        = 0.05;  ///<  Smallest allowed Fit Screen scale.
static constexpr double kMaxFitScale        = 20.0;  ///<  Largest allowed Fit Screen scale.
static constexpr int    kFitIterations      = 6;     ///<  Max passes to converge Fit Screen's fixed point.
static constexpr double kFitEpsilon         = 1e-3;  ///<  Fit Screen scale convergence threshold.

DolphinZoom::DolphinZoom(QTableView *view)
    : m_view(view)
{
}

bool DolphinZoom::canZoomIn() const
{
    return m_zoomLevel < kMaxZoomLevel;
}

bool DolphinZoom::canZoomOut() const
{
    return m_zoomLevel > 0;
}

void DolphinZoom::zoomIn()
{
    // Column-width only; capped at kMaxZoomLevel discrete steps.
    if (m_zoomLevel < kMaxZoomLevel) {
        ++m_zoomLevel;
    }
    apply();
}

void DolphinZoom::zoomOut()
{
    // Column-width only; floored at the baseline (no shrink below the default width).
    if (m_zoomLevel > 0) {
        --m_zoomLevel;
    }
    apply();
}

void DolphinZoom::reset()
{
    m_zoomLevel = 0;
    m_fitScale  = 1.0;
    apply();
}

void DolphinZoom::fitScreen()
{
    // Fit Screen scales everything (columns, rows, font) uniformly to fit, and resets
    // the discrete column zoom.
    const int cols = m_view->model() ? m_view->model()->columnCount() : 0;
    const int rows = m_view->model() ? m_view->model()->rowCount() : 0;
    // Degenerate geometry (empty/hidden table): nothing to fit, leave the zoom untouched.
    if (cols <= 0 || rows <= 0) {
        return;
    }

    auto *vHeader = m_view->verticalHeader();
    auto *hHeader = m_view->horizontalHeader();
    auto *vbar    = m_view->verticalScrollBar();
    auto *hbar    = m_view->horizontalScrollBar();

    // viewport() already excludes the header gutter/strip and shrinks under any visible
    // scroll bar. Add both back to get the window interior available to the grid + its
    // chrome — a quantity independent of the current zoom (so the fit cannot feed back on
    // its own starting geometry, which is what forced a second Fit press to converge).
    const double interiorW = m_view->viewport()->width()
        + vHeader->width() + (vbar->isVisible() ? vbar->width()  : 0);
    const double interiorH = m_view->viewport()->height()
        + hHeader->height() + (hbar->isVisible() ? hbar->height() : 0);

    // The gutter/strip scale with the font, which scales with the fit factor, so the
    // available cell area depends on the result. Solve the fixed point: recompute against
    // the chrome size the new font yields (sizeHint() reflects the new font synchronously),
    // re-applying until the scale stabilizes — converging in one Fit press.
    for (int i = 0; i < kFitIterations; ++i) {
        const double availW = interiorW - vHeader->sizeHint().width();
        const double availH = interiorH - hHeader->sizeHint().height();
        const double sW = availW / (kDefaultColumnWidth * cols);
        const double sH = availH / (kDefaultRowHeight   * rows);
        // A hidden or too-small viewport yields a non-positive scale; leave the zoom untouched.
        if (sW <= 0 || sH <= 0) {
            return;
        }
        const double next = std::clamp((std::min)(sW, sH), kMinFitScale, kMaxFitScale);
        m_zoomLevel = 0;
        if (std::abs(next - m_fitScale) < kFitEpsilon) {
            break;
        }
        m_fitScale = next;
        apply();
    }
}

void DolphinZoom::apply() const
{
    // Sections use Fixed resize mode, so setting the header default section size resizes
    // every row/column uniformly without a per-cell loop. The discrete column-zoom widens
    // columns only; Fit Screen scales columns, rows, and font together.
    const double colScale = m_fitScale * std::pow(kZoomStep, m_zoomLevel);
    m_view->horizontalHeader()->setDefaultSectionSize(
        static_cast<int>(std::lround(kDefaultColumnWidth * colScale)));
    m_view->verticalHeader()->setDefaultSectionSize(
        static_cast<int>(std::lround(kDefaultRowHeight * m_fitScale)));

    // Scale the cell/header font with the Fit Screen factor only (column zoom leaves text
    // untouched), from the application's base point size.
    QFont font = QApplication::font();
    const double basePointSize = font.pointSizeF() > 0 ? font.pointSizeF() : 10.0;
    font.setPointSizeF(basePointSize * m_fitScale);
    m_view->setFont(font);
    m_view->horizontalHeader()->setFont(font);
    m_view->verticalHeader()->setFont(font);
}
