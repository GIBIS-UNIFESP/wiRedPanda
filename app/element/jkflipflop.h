/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class JKFlipFlop : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)

public:
    explicit JKFlipFlop(QGraphicsItem *parent = nullptr);

    void setSkin(bool defaultSkin, const QString &filename) override;
    void updatePorts() override;

private:
    const QString m_titleText = tr("<b>JK-FLIPFLOP</b>");
    const QString m_translatedName = tr("JK-FlipFlop");
    const QString m_pixmap = ":/memory/light/JK-flipflop.png";

    bool lastClk;
};

Q_DECLARE_METATYPE(JKFlipFlop)
