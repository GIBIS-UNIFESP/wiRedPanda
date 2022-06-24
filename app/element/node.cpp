// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "node.h"

#include "qneport.h"

namespace
{
int id = qRegisterMetaType<Node>();
}

Node::Node(QGraphicsItem *parent)
    : GraphicElement(ElementType::Node, ElementGroup::Gate, 1, 1, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    Node::updatePorts();
    setCanChangeSkin(true);
    setPortName(m_translatedName);
    setToolTip(m_translatedName);
    inputPort()->setRequired(true);
}

void Node::updatePorts()
{
    inputPort()->setPos(  0, 16);
    outputPort()->setPos(32, 16);
}
