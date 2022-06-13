/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelementinput.h"

class InputButton : public GraphicElementInput
{
    Q_OBJECT
    Q_PROPERTY(QString pixmapPath MEMBER m_pixmapPath CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

public:
    explicit InputButton(QGraphicsItem *parent = nullptr);

    bool isOn(const int port = 0) const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version) override;
    void save(QDataStream &stream) const override;
    void setOff() override;
    void setOn() override;
    void setOn(const bool value, const int port = 0) override;
    void setSkin(const bool defaultSkin, const QString &fileName) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    const QString m_pixmapPath = ":/input/buttonOff.png";
    const QString m_titleText = tr("<b>PUSH BUTTON</b>");
    const QString m_translatedName = tr("Button");

    bool m_isOn = false;
};

Q_DECLARE_METATYPE(InputButton)
