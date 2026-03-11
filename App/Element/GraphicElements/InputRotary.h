// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "App/Element/GraphicElementInput.h"

class InputRotary : public GraphicElementInput
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit InputRotary(QGraphicsItem *parent = nullptr);

    // --- State Queries ---

    bool isOn(const int port = 0) const override;
    int outputSize() const override;
    int outputValue() const override;

    // --- State Setters ---

    void setOff() override;
    void setOn() override;
    void setOn(const bool value, const int port = 0) override;

    // --- Visual ---

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void refresh() override;
    void setSkin(const bool defaultSkin, const QString &fileName) override;
    void updatePortsProperties() override;

    // --- Serialization ---

    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void save(QDataStream &stream) const override;

protected:
    // --- Event handlers ---

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    // --- Members ---

    QPixmap m_arrow;
    QPixmap m_rotary;
    int m_currentPort = 0;
};

Q_DECLARE_METATYPE(InputRotary)

