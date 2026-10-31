// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the 14-segment LED display.
 */

#pragma once

#include <memory>

#include "App/Element/Element.h"

class QSvgRenderer;

/**
 * \class Display14
 * \brief 14-segment LED display output element (14 segments + decimal point = 15 inputs).
 *
 * \details Supports alphanumeric characters.  Each input controls one segment.
 * Supports red, green, and blue color variants.
 */
class Display14 : public Element
{
    Q_OBJECT

public:
    /// Default segment color name. Single source of truth: used by the colour's
    /// spec and serialization's default-elision check.
    static constexpr const char *kDefaultColor = "Red";

    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Display14(QGraphicsItem *parent = nullptr);

    // --- Visual ---

    /// The base bounds plus the 64x64 box the lit segments are drawn in, which a custom picture need not cover.
    QRectF boundingRect() const override;

    /// \reimp
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// Refreshes the displayed segments based on current input states.
    void refresh() override;

    /// The 15 segment inputs, by their places and names.
    std::optional<PortAttributes> inputAttributes(int index) const override;

protected:
    /// Picks the segments' colour from a new colour.
    void parameterChanged(ParamId id, const ParamValue &previous) override;

private:
    // --- Members ---

    QVector<QPixmap> bkg;
    QVector<std::shared_ptr<QSvgRenderer>> a, b, c, d, e, f, g1, g2, h, j, k, l, m, n, dp;
    int m_colorNumber = 1;
};
