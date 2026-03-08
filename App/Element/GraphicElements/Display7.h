// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the 7-segment LED display.
 */

#pragma once

#include "App/Element/GraphicElement.h"

class QNEPort;

/**
 * \class Display7
 * \brief 7-segment LED display output element (7 segments + decimal point = 8 inputs).
 *
 * \details Each input port controls one segment (a–g + dp).  Supports red, green,
 * and blue color variants.
 */
class Display7 : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Display7(QGraphicsItem *parent = nullptr);

    // --- Color utilities ---

    /// Returns a recolored copy of \a source using the given channel flags.
    static QPixmap convertColor(const QImage &source, const bool red, const bool green, const bool blue);

    /**
     * \brief Fills \a pixmaps with red, green, and blue variants derived from the first entry.
     * \param pixmaps Vector of source pixmaps; expanded in-place with color variants.
     */
    static void convertAllColors(QVector<QPixmap> &pixmaps);

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

    /// Updates port positions (fixed at 8 inputs).
    void updatePortsProperties() override;

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;

    /// Serializes the color selection to \a stream.
    void save(QDataStream &stream) const override;

private:
    // --- Members ---

    QString m_color = "Red";
    QVector<QPixmap> a, b, c, d, e, f, g, dp;
    int m_colorNumber = 1;
};

Q_DECLARE_METATYPE(Display7)

