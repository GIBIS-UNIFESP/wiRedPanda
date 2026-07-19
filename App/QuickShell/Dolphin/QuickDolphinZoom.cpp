// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Dolphin/QuickDolphinZoom.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr double kZoomStep = 1.25;      ///< Multiplicative factor per column-zoom step.
constexpr int kMaxZoomLevel = 6;        ///< Maximum discrete column-zoom step (baseline = 0).
constexpr double kMinFitScale = 0.05;   ///< Smallest allowed Fit Screen scale.
constexpr double kMaxFitScale = 20.0;   ///< Largest allowed Fit Screen scale.
} // namespace

bool QuickDolphinZoom::canZoomIn() const
{
    return m_zoomLevel < kMaxZoomLevel;
}

bool QuickDolphinZoom::canZoomOut() const
{
    return m_zoomLevel > 0;
}

void QuickDolphinZoom::zoomIn()
{
    if (m_zoomLevel < kMaxZoomLevel) {
        ++m_zoomLevel;
    }
}

void QuickDolphinZoom::zoomOut()
{
    if (m_zoomLevel > 0) {
        --m_zoomLevel;
    }
}

void QuickDolphinZoom::reset()
{
    m_zoomLevel = 0;
    m_fitScale = 1.0;
}

void QuickDolphinZoom::fitScreen(qreal viewportWidth, qreal viewportHeight, qreal headerWidth, qreal headerHeight,
                                 int columns, int rows)
{
    if (columns <= 0 || rows <= 0) {
        return;
    }

    const double availW = viewportWidth - headerWidth;
    const double availH = viewportHeight - headerHeight;
    const double sW = availW / (kDefaultColumnWidth * columns);
    const double sH = availH / (kDefaultRowHeight * rows);
    if (sW <= 0 || sH <= 0) {
        return;
    }

    m_zoomLevel = 0;
    m_fitScale = std::clamp((std::min)(sW, sH), kMinFitScale, kMaxFitScale);
}

qreal QuickDolphinZoom::columnWidth() const
{
    return kDefaultColumnWidth * m_fitScale * std::pow(kZoomStep, m_zoomLevel);
}

qreal QuickDolphinZoom::rowHeight() const
{
    return kDefaultRowHeight * m_fitScale;
}
