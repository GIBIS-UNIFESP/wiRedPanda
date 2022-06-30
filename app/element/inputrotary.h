/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelementinput.h"

class InputRotary : public GraphicElementInput
{
    Q_OBJECT
    Q_PROPERTY(QString pixmapPath MEMBER m_pixmapPath CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

public:
    explicit InputRotary(QGraphicsItem *parent = nullptr);

    bool isOn(const int port = 0) const override;
    int outputSize() const override;
    int outputValue() const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void refresh() override;
    void save(QDataStream &stream) const override;
    void setOff() override;
    void setOn() override;
    void setOn(const bool value, const int port = 0) override;
    void setSkin(const bool defaultSkin, const QString &fileName) override;
    void updatePorts() override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    const QString m_pixmapPath = ":/input/rotary/rotary_icon.svg";
    const QString m_titleText = tr("<b>ROTARY SWITCH</b>");
    const QString m_translatedName = tr("Rotary Switch");

    QPixmap m_arrow;
    QPixmap m_rotary;
    int m_currentPort = 0;
};

Q_DECLARE_METATYPE(InputRotary)
