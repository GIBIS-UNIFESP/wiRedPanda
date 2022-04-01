/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class JKLatch : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)

public:
    explicit JKLatch(QGraphicsItem *parent = nullptr);
    ~JKLatch() override = default;

    void updatePorts() override;

    const QString m_titleText = tr("<b>JK-LATCH</b>");
    const QString m_translatedName = tr("JK-Latch");
    const QString m_pixmap = ":/memory/light/JK-latch.png";
};

Q_DECLARE_METATYPE(JKLatch)
