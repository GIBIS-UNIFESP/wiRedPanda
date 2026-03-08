// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for a wire junction node.
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class Node
 * \brief Visible wire junction (T-junction) element with 1 input and 1 output.
 *
 * \details Nodes allow a signal to branch by adding a visible dot on the wire.
 * They can be rotated to orient the output in any of the four cardinal directions.
 */
class Node : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Node(QGraphicsItem *parent = nullptr);

    // --- Visual ---

    /// Recalculates port positions for the current port count.
    void updatePortsProperties() override;
};

Q_DECLARE_METATYPE(Node)

