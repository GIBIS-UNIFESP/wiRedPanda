/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class Xor : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString pixmapPath MEMBER m_pixmapPath CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

public:
    explicit Xor(QGraphicsItem *parent = nullptr);

private:
    const QString m_pixmapPath = ":/basic/xor.png";
    const QString m_titleText = tr("<b>XOR</b>");
    const QString m_translatedName = tr("Xor");
};

Q_DECLARE_METATYPE(Xor)
