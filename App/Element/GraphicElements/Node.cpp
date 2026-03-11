// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Node.h"

#include "App/GlobalProperties.h"
#include "App/Nodes/QNEPort.h"

Node::Node(QGraphicsItem *parent)
    : GraphicElement(ElementType::Node, ElementGroup::Gate, ":/Components/Logic/node.svg", tr("NODE"), tr("Node"), 1, 1, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    inputPort()->setRequired(true);

    Node::updatePortsProperties();
}

void Node::updatePortsProperties()
{
    inputPort()->setPos(  0, 16);
    outputPort()->setPos(32, 16);
}

