/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INPUTROTARY_H
#define INPUTROTARY_H

#include "graphicelement.h"
#include "input.h"

class InputRotary : public GraphicElement, public Input
{
public:
    explicit InputRotary(QGraphicsItem *parent = nullptr);
    ~InputRotary() override = default;
    void refresh() override;
    void updatePorts() override;
    static int current_id_number; // Number used to create distinct labels for each instance of this element.

    int m_value;
    // Rotary pixmaps
    QPixmap m_rotary2, m_rotary4, m_rotary8, m_rotary10, m_rotary16;
    // Pointer maps
    QVector< QPixmap > m_pointer;
    /* QGraphicsItem interface */
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    /* GraphicElement interface */
public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void save(QDataStream &ds) const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    bool getOn(int port=0) const override;
    void setOn(bool value, int port=0) override;
    void setSkin(bool defaultSkin, const QString &filename) override;
    int outputSize() override;
};

#endif /* INPUTROTARY_H */
