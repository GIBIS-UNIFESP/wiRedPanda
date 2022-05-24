// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputvcc.h"

#include "common.h"
#include "qneport.h"

namespace
{
int id = qRegisterMetaType<InputVcc>();
}

InputVcc::InputVcc(QGraphicsItem *parent)
    : GraphicElement(ElementType::InputVcc, ElementGroup::StaticInput, 0, 0, 1, 1, parent)
{
    m_defaultSkins = QStringList{":/input/1.png"};
    setPixmap(m_defaultSkins.first());

    setOutputsOnTop(false);
    setCanChangeSkin(true);
    setRotatable(false);
    setPortName("VCC");
    setToolTip(m_translatedName);
    m_outputs.first()->setValue(true);
}

void InputVcc::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_defaultSkins[0] = (defaultSkin) ? ":/input/1.png" : fileName;
    setPixmap(m_defaultSkins[0]);
}
