// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputvcc.h"

#include "globalproperties.h"
#include "qneport.h"

namespace
{
    int id = qRegisterMetaType<InputVcc>();
}

InputVcc::InputVcc(QGraphicsItem *parent)
    : GraphicElement(ElementType::InputVcc, ElementGroup::StaticInput, ":/input/1.svg", tr("VCC"), tr("VCC"), 0, 0, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    setRotatable(false);

    m_outputPorts.constFirst()->setStatus(Status::Active);
}
