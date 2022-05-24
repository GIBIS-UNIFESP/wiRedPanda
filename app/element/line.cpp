// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "line.h"

#include "common.h"

namespace
{
int id = qRegisterMetaType<Line>();
}

Line::Line(QGraphicsItem *parent)
    : GraphicElement(ElementType::Line, ElementGroup::Other, 0, 0, 0, 0, parent)
{
    m_defaultSkins = QStringList{":/line.png"};
    setPixmap(m_defaultSkins.first());

    setRotatable(true);
    setOutputsOnTop(true);
    setCanChangeSkin(true);
    setHasLabel(true);
    setPortName("LINE");
    setToolTip(m_translatedName);
}

void Line::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_defaultSkins[0] = defaultSkin ? ":/line.png" : fileName;
    setPixmap(m_defaultSkins[0]);
}
