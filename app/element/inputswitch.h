/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"
#include "input.h"

class InputSwitch : public GraphicElement, public Input
{
    Q_OBJECT
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

public:
    explicit InputSwitch(QGraphicsItem *parent = nullptr);

    bool on(const int port = 0) const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version) override;
    void save(QDataStream &stream) const override;
    void setOff() override;
    void setOn() override;
    void setOn(const bool value, const int port = 0) override;
    void setSkin(const bool defaultSkin, const QString &fileName) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    const QString m_pixmap = ":/input/switchOn.png";
    const QString m_titleText = tr("<b>INPUT SWITCH</b>");
    const QString m_translatedName = tr("Input Switch");

    bool m_on = false;
};

Q_DECLARE_METATYPE(InputSwitch)
