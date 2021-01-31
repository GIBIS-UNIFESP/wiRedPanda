// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "node.h"

Node::Node(QGraphicsItem *parent)
    : GraphicElement(ElementType::NODE, ElementGroup::GATE, 1, 1, 1, 1, parent)
{
    pixmapSkinName = {":/basic/node.png"};

    setPixmap(pixmapSkinName[0], QRect(QPoint(16, 16), QPoint(48, 48)));
    updatePorts();
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
        pixmapSkinName[0] = ":/basic/node.png";
    } else {
        pixmapSkinName[0] = filename;
    }
    setPixmap(pixmapSkinName[0]);
}
