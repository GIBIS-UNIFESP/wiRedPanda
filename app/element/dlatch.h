/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class DLatch : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)

public:
    explicit DLatch(QGraphicsItem *parent = nullptr);
    ~DLatch() override = default;

    void updatePorts() override;
    void setSkin(bool defaultSkin, const QString &filename) override;

    const QString m_titleText = tr("<b>D-LATCH</b>");
    const QString m_translatedName = tr("D-Latch");
    const QString m_pixmap = ":/memory/light/D-latch.png";
};

Q_DECLARE_METATYPE(DLatch)
