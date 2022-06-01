// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputgnd.h"

#include "common.h"
#include "qneport.h"

namespace
{
int id = qRegisterMetaType<InputGnd>();
}

InputGnd::InputGnd(QGraphicsItem *parent)
    : GraphicElement(ElementType::InputGnd, ElementGroup::StaticInput, 0, 0, 1, 1, parent)
{
    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(m_defaultSkins.first());

    setCanChangeSkin(true);
    setRotatable(false);
    setPortName("GND");
    setToolTip(m_translatedName);
    m_outputs.first()->setValue(false);
}
