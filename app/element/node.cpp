// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "node.h"

#include "qneport.h"

Node::Node(QGraphicsItem *parent)
    : GraphicElement(ElementType::NODE, ElementGroup::GATE, 1, 1, 1, 1, parent)
{
    m_pixmapSkinName = {":/basic/node.png"};

    setPixmap(m_pixmapSkinName[0], QRect(QPoint(16, 16), QPoint(48, 48)));
    Node::updatePorts();
    setCanChangeSkin(true);
    setPortName("NODE");
    input()->setRequired(true);
}

void Node::updatePorts()
{
    input()->setPos(0, 16);
    output()->setPos(32, 16);
}

void Node::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/basic/node.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
