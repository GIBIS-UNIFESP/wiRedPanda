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
    m_defaultSkins = QStringList{":/basic/node.png"};
    setPixmap(m_defaultSkins.first(), QRect(16, 16, 48, 48));

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

void Node::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_defaultSkins[0] = (defaultSkin) ? ":/basic/node.png" : fileName;
    setPixmap(m_defaultSkins[0]);
}
