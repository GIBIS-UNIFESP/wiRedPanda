// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelementinput.h"

class InputRotary : public GraphicElementInput
{
    Q_OBJECT

public:
    explicit InputRotary(QGraphicsItem *parent = nullptr);

    bool isOn(const int port = 0) const override;
    int outputSize() const override;
    int outputValue() const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void refresh() override;
    void save(QDataStream &stream) const override;
    void setOff() override;
    void setOn() override;
    void setOn(const bool value, const int port = 0) override;
    void setSkin(const bool defaultSkin, const QString &fileName) override;
    void updatePortsProperties() override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QPixmap m_arrow;
    QPixmap m_rotary;
    int m_currentPort = 0;
};

Q_DECLARE_METATYPE(InputRotary)
