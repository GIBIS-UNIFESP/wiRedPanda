/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class Text : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)

public:
    explicit Text(QGraphicsItem *parent = nullptr);

private:
    const QString m_titleText = tr("<b>TEXT</b>");
    const QString m_translatedName = tr("Text");
    const QString m_pixmap = ":/text.png";
};

Q_DECLARE_METATYPE(Text)

