/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"
#include "input.h"

class InputRotary : public GraphicElement, public Input
{
    Q_OBJECT
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

public:
    explicit InputRotary(QGraphicsItem *parent = nullptr);

    bool on(int port = 0) const override;
    int outputSize() const override;
    int outputValue() const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void refresh() override;
    void save(QDataStream &stream) const override;
    void setOff() override;
    void setOn() override;
    void setOn(bool value, int port = 0) override;
    void setSkin(const bool defaultSkin, const QString &fileName) override;
    void updatePorts() override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    const QString m_pixmap = ":/input/rotary/rotary_icon.png";
    const QString m_titleText = tr("<b>ROTARY SWITCH</b>");
    const QString m_translatedName = tr("Rotary Switch");

    QPixmap m_rotary10;
    QPixmap m_rotary16;
    QPixmap m_rotary2;
    QPixmap m_rotary4;
    QPixmap m_rotary8;
    QVector<QPixmap> m_pointer;
    int m_value = 0;
};

Q_DECLARE_METATYPE(InputRotary)
