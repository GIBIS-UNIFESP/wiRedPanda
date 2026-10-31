// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for a user-programmable truth table.
 */

#pragma once

#include "App/Element/Element.h"

/**
 * \class TruthTable
 * \brief Programmable truth-table element with configurable inputs and outputs.
 *
 * \details The truth table is stored as a flat QBitArray (the key).  Clicking a
 * cell in the editor dialog toggles the corresponding output bit.
 */
class TruthTable : public Element
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit TruthTable(QGraphicsItem *parent = nullptr);

    // --- Key Access ---

    /// Returns the truth-table output bit-array (ParamId::Table).
    const QBitArray &key() const;
    /// Sets the truth-table output bit-array to \a key, padded with zeros or truncated to the table's size.
    void setkey(const QBitArray &key);
    /// Sets bit \a index of the truth-table output bit-array to \a value; \a index must be within the key.
    void setKeyBit(int index, bool value);

    // --- Visual ---

    /// Draws the truth-table body (DIP rect, table icon, shadow) straight onto \a painter as
    /// vectors, so it stays crisp at any zoom instead of blitting a fixed-resolution pixmap.
    void drawBody(QPainter &painter) const override;

    /// \reimp — opens the truth table editor on double-click.
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    /// Emitted on double-click to request opening the truth table editor.
    void requestOpenTruthTableEditor();

public:
    /// Looks up the current input pattern in the truth table and drives outputs.
    void updateLogic() override;

    /// The inputs, named A, B, C, ...
    std::optional<PortAttributes> inputAttributes(int index) const override;

    /// The outputs, named S0, S1, ...
    std::optional<PortAttributes> outputAttributes(int index) const override;

protected:
    /// Lays the ports out, then places the label below the body and redraws it for the new port counts.
    void layoutPorts() override;

private:
    // --- Internal methods ---

    void generatePixmap();
};
