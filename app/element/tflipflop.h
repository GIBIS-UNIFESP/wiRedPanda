/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class TFlipFlop : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

public:
    explicit TFlipFlop(QGraphicsItem *parent = nullptr);

    void updatePorts() override;

private:
    const QString m_pixmap = ":/memory/light/T-flipflop.png";
    const QString m_titleText = tr("<b>T-FLIPFLOP</b>");
    const QString m_translatedName = tr("T-FlipFlop");
};

Q_DECLARE_METATYPE(TFlipFlop)
