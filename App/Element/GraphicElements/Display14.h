// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the 14-segment LED display.
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class Display14
 * \brief 14-segment LED display output element (14 segments + decimal point = 15 inputs).
 *
 * \details Supports alphanumeric characters.  Each input controls one segment.
 * Supports red, green, and blue color variants.
 */
class Display14 : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Display14(QGraphicsItem *parent = nullptr);

    // --- Color State ---

    /// Returns the current display color name.
    QString color() const override;

    /**
     * \brief Sets the display color.
     * \param color Color name ("Red", "Green", or "Blue").
     */
    void setColor(const QString &color) override;

    // --- Visual ---

    /// \reimp
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// Refreshes the displayed segments based on current input states.
    void refresh() override;

    /// Updates port positions (fixed at 15 inputs).
    void updatePortsProperties() override;

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, SerializationContext &context) override;

    /// Serializes the color selection to \a stream.
    void save(QDataStream &stream) const override;

private:
    // --- Members ---

    QString m_color = "Red";
    QVector<QPixmap> bkg, a, b, c, d, e, f, g1, g2, h, j, k, l, m, n, dp;
    int m_colorNumber = 1;
};

