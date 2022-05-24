/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class Xnor : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

public:
    explicit Xnor(QGraphicsItem *parent = nullptr);

private:
    const QString m_pixmap = ":/basic/xnor.png";
    const QString m_titleText = tr("<b>XNOR</b>");
    const QString m_translatedName = tr("Xnor");
};

Q_DECLARE_METATYPE(Xnor)
