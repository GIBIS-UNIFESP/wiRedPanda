// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "demux.h"

#include "qneport.h"

namespace {
int id = qRegisterMetaType<Demux>();
}

Demux::Demux(QGraphicsItem *parent)
    : GraphicElement(ElementType::Demux, ElementGroup::Mux, 2, 2, 2, 2, parent)
{
    m_pixmapSkinName = {":/basic/demux.png"};
    setPixmap(m_pixmapSkinName[0]);
    setRotatable(true);
    setCanChangeSkin(true);
    Demux::updatePorts();
    setPortName("DEMUX");
    setToolTip(m_translatedName);
    setRotation(180.0);

    input(0)->setName("in");
    input(1)->setName("S");

    output(0)->setName("out0");
    output(1)->setName("out1");
}

void Demux::updatePorts()
{
    input(0)->setPos(32, 48); /* 0 */
    input(1)->setPos(58, 32); /* S */
    output(0)->setPos(32 - 12, 16); /* Out */
    output(1)->setPos(32 + 12, 16); /* Out */
}

void Demux::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/basic/demux.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
