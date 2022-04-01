/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class Nand : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)

public:
    explicit Nand(QGraphicsItem *parent = nullptr);
    ~Nand() override = default;

    void setSkin(bool defaultSkin, const QString &filename) override;

    const QString m_titleText = tr("<b>NAND</b>");
    const QString m_translatedName = tr("Nand");
    const QString m_pixmap = ":/basic/nand.png";
};

Q_DECLARE_METATYPE(Nand)
