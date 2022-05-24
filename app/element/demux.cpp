// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "demux.h"

#include "common.h"
#include "qneport.h"

namespace
{
int id = qRegisterMetaType<Demux>();
}

Demux::Demux(QGraphicsItem *parent)
    : GraphicElement(ElementType::Demux, ElementGroup::Mux, 2, 2, 2, 2, parent)
{
    m_defaultSkins = QStringList{":/basic/demux.png"};
    setPixmap(m_defaultSkins.first());

    setCanChangeSkin(true);
    Demux::updatePorts();
    setPortName("DEMUX");
    setToolTip(m_translatedName);

    input(0)->setName("in");
    input(1)->setName("S");

    output(0)->setName("out0");
    output(1)->setName("out1");
}

void Demux::updatePorts()
{
    input(0)->setPos(16, 32);       /* 0   */
    input(1)->setPos(32, 58);       /* S   */

void Demux::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_defaultSkins[0] = (defaultSkin) ? ":/basic/demux.png" : fileName;
    setPixmap(m_defaultSkins[0]);
    output(0)->setPos(48, 32 - 12); /* Out */
    output(1)->setPos(48, 32 + 12); /* Out */
}
