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
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)

public:
    explicit InputRotary(QGraphicsItem *parent = nullptr);

    static int current_id_number; // Number used to create distinct labels for each instance of this element.

    bool getOn(int port = 0) const override;
    int outputSize() const override;
    int outputValue() const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void refresh() override;
    void save(QDataStream &ds) const override;
    void setOn(bool value, int port = 0) override;
    void setSkin(bool defaultSkin, const QString &filename) override;
    void updatePorts() override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    const QString m_titleText = tr("<b>ROTARY SWITCH</b>");
    const QString m_translatedName = tr("Rotary Switch");
    const QString m_pixmap = ":/input/rotary/rotary_icon.png";

    QPixmap m_rotary2, m_rotary4, m_rotary8, m_rotary10, m_rotary16;
    QVector<QPixmap> m_pointer;
    int m_value;
};

Q_DECLARE_METATYPE(InputRotary)
