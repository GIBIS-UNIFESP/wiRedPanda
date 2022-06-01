/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class Nor : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString pixmapPath MEMBER m_pixmapPath CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

public:
    explicit Nor(QGraphicsItem *parent = nullptr);

private:
    const QString m_pixmapPath = ":/basic/nor.png";
    const QString m_titleText = tr("<b>NOR</b>");
    const QString m_translatedName = tr("Nor");
};

Q_DECLARE_METATYPE(Nor)
