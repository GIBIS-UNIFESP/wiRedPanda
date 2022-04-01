/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class DFlipFlop : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)

public:
    explicit DFlipFlop(QGraphicsItem *parent = nullptr);
    ~DFlipFlop() override = default;

    void updatePorts() override;
    void setSkin(bool defaultSkin, const QString &filename) override;

    const QString m_titleText = tr("<b>D-FLIPFLOP</b>");
    const QString m_translatedName = tr("D-FlipFlop");
    const QString m_pixmap = ":/memory/light/D-flipflop.png";

private:
    bool lastClk;
    bool lastValue;
};

Q_DECLARE_METATYPE(DFlipFlop)
