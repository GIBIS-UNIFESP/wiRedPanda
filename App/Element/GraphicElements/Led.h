// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the LED output indicator.
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class Led
 * \brief LED output element that lights up when its input is logic-1.
 *
 * \details Supports multiple colors (white, red, green, blue, purple).
 * The displayed pixmap switches between on/off variants when the logic state changes.
 */
class Led : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Led(QGraphicsItem *parent = nullptr);

    // --- Color State ---

    /// Returns the current LED color name.
    QString color() const override;

    /// Returns a string describing the current color for the element editor.
    QString genericProperties() override;
    /// Sets the LED color to \a color.
    void setColor(const QString &color) override;

    // --- Visual ---

    /// Refreshes the visual appearance based on current state.
    void refresh() override;
    /// \reimp
    void setSkin(const bool useDefaultSkin, const QString &fileName) override;

    /// Updates port positions when input count changes.
    void updatePortsProperties() override;

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    /// \reimp
    void save(QDataStream &stream) const override;

private:
    // --- Internal methods ---

    int colorIndex();

    // --- Members ---

    QString m_color = "White";
    int m_colorIndex = 0; /* white = 0, red = 2, green = 4, blue = 6, purple = 8 */
};

