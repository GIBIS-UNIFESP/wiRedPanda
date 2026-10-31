// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the 16-segment LED display.
 */

#pragma once

#include <memory>

#include "App/Element/Element.h"

class QSvgRenderer;

/**
 * \class Display16
 * \brief 16-segment LED display output element (16 segments + decimal point = 17 inputs).
 *
 * \details Full alphanumeric coverage including diagonals.
 * Supports red, green, and blue color variants.
 */
class Display16 : public Element
{
    Q_OBJECT

public:
    /// Default segment color name. Single source of truth: used by the colour's
    /// spec and serialization's default-elision check.
    static constexpr const char *kDefaultColor = "Red";

    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Display16(QGraphicsItem *parent = nullptr);

    // --- Visual ---

    /// The base bounds plus the 64x64 box the lit segments are drawn in, which a custom picture need not cover.
    QRectF measureBoundingRect() const override;

    /// \reimp
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// Refreshes the displayed segments based on current input states.
    void refresh() override;

    /// The 17 segment inputs, by their places and names.
    std::optional<PortAttributes> inputAttributes(int index) const override;

protected:
    /// Picks the segments' colour from a new colour.
    void parameterChanged(ParamId id, const ParamValue &previous) override;

private:
    // --- Members ---

    QVector<QPixmap> bkg;
    QVector<std::shared_ptr<QSvgRenderer>> a1, a2, b, c, d1, d2, e, f, g1, g2, h, j, k, l, m, n, dp;
    int m_colorNumber = 1;
};
