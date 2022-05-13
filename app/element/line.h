/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class Line : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)

public:
    explicit Line(QGraphicsItem *parent = nullptr);

    void setSkin(bool defaultSkin, const QString &filename) override;

private:
    const QString m_titleText = tr("<b>LINE</b>");
    const QString m_translatedName = tr("Line");
    const QString m_pixmap = ":/line.png";
};

Q_DECLARE_METATYPE(Line)
