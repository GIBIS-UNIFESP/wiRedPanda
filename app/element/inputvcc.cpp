// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputvcc.h"

#include "qneport.h"

namespace
{
int id = qRegisterMetaType<InputVcc>();
}

InputVcc::InputVcc(QGraphicsItem *parent)
    : GraphicElement(ElementType::InputVcc, ElementGroup::StaticInput, ":/input/1.svg", tr("<b>VCC</b>"), tr("VCC"), 0, 0, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    setRotatable(false);
    setPortName(m_translatedName);
    setToolTip(m_translatedName);
    m_outputPorts.first()->setValue(Status::Active);
}
