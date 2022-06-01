// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "node.h"

#include "common.h"
#include "qneport.h"

namespace
{
int id = qRegisterMetaType<Node>();
}

Node::Node(QGraphicsItem *parent)
    : GraphicElement(ElementType::Node, ElementGroup::Gate, 1, 1, 1, 1, parent)
{
    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(m_defaultSkins.first(), QSize(32, 32));

    Node::updatePorts();
    setCanChangeSkin(true);
    setPortName("NODE");
    setToolTip(m_translatedName);
    input()->setRequired(true);
}

void Node::updatePorts()
{
    input()->setPos(0, 16);
    output()->setPos(32, 16);
}
