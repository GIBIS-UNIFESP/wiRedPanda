// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for a user-programmable truth table.
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class TruthTable
 * \brief Programmable truth-table element with configurable inputs and outputs.
 *
 * \details The truth table is stored as a flat QBitArray (the key).  Clicking a
 * cell in the editor dialog toggles the corresponding output bit.
 */
class TruthTable : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit TruthTable(QGraphicsItem *parent = nullptr);

    // --- Key Access ---

    /// Returns a reference to the truth-table output bit-array.
    QBitArray &key();
    /// Sets the truth-table output bit-array to \a key.
    void setkey(const QBitArray &key);

    // --- Visual ---

    /// \reimp
    QRectF boundingRect() const override;
    /// \reimp
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    /// Forces a repaint of the element.
    void update();

    /// \reimp — opens the truth table editor on double-click.
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    /// Emitted on double-click to request opening the truth table editor.
    void requestOpenTruthTableEditor();

public:
    /// Looks up the current input pattern in the truth table and drives outputs.
    void updateLogic() override;

    /// Resizes the truth-table key when input or output count changes.
    void updatePortsProperties() override;

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, SerializationContext &context) override;
    /// \reimp
    void save(QDataStream &stream, SerializationOptions options) const override;

private:
    // --- Internal methods ---

    void generatePixmap();

    /// Draws the truth-table body (DIP rect, table icon, shadow) straight onto \a painter as
    /// vectors, so it stays crisp at any zoom instead of blitting a fixed-resolution pixmap.
    void drawBody(QPainter *painter);

    QBitArray m_key;
};
