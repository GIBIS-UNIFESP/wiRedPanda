// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Node.h"

#include "App/GlobalProperties.h"
#include "App/Nodes/QNEPort.h"

// Node is a wire junction / fan-out point: 1 input, 1 output (1:1 pass-through).
// Its sole logical role is to create a named connection split on the canvas.
Node::Node(QGraphicsItem *parent)
    : GraphicElement(ElementType::Node, ElementGroup::Gate, ":/Components/Logic/node.svg", tr("NODE"), tr("Node"), 1, 1, 1, 1, parent)
{
    // Skip full initialisation when building a property-probe instance (see ElementFactory).
    if (GlobalProperties::skipInit) {
        return;
    }

    // Seed skin lists from the constructor-supplied pixmap path (see And.cpp for details).
    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    setHasLabel(true);
    // The single input must always be connected; an undriven node would silently
    // propagate an undefined state to all downstream connections.
    inputPort()->setRequired(true);

    Node::updatePortsProperties();
}

void Node::updatePortsProperties()
{
    // Position ports at the left and right midpoints of the 32×32 node icon so
    // they align with the horizontal wire grid (y=16 is the vertical centre).
    inputPort()->setPos(  0, 16);
    outputPort()->setPos(32, 16);
}

