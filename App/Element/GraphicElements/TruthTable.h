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
 * cell in the editor dialog toggles the corresponding output bit.  The resulting
 * LogicTruthTable implements the simulation behaviour.
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
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    /// Forces a repaint of the element.
    void update();

    /// Resizes the truth-table key when input or output count changes.
    void updatePortsProperties() override;

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, SerializationContext &context) override;
    /// \reimp
    void save(QDataStream &stream) const override;

private:
    // --- Internal methods ---

    void generatePixmap();

    QBitArray m_key;
};

