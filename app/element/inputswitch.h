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
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)

public:
    explicit InputSwitch(QGraphicsItem *parent = nullptr);
    ~InputSwitch() override = default;

    static int current_id_number; // Number used to create distinct labels for each instance of this element.

    void save(QDataStream &ds) const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    bool getOn(int port = 0) const override;
    void setOn(bool value, int port = 0) override;
    void setSkin(bool defaultSkin, const QString &filename) override;

    const QString m_titleText = tr("<b>INPUT SWITCH</b>");
    const QString m_translatedName = tr("Input Switch");
    const QString m_pixmap = ":/input/switchOn.png";

    bool on;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
};

Q_DECLARE_METATYPE(InputSwitch)
