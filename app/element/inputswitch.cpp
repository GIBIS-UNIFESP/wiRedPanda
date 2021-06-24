// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputswitch.h"

#include "qneport.h"

#include <QGraphicsSceneMouseEvent>

int InputSwitch::current_id_number = 0;

InputSwitch::InputSwitch(QGraphicsItem *parent)
    : GraphicElement(ElementType::SWITCH, ElementGroup::INPUT, 0, 0, 1, 1, parent)
{
    m_pixmapSkinName = {
        ":/input/switchOff.png",
        ":/input/switchOn.png",
    };

    setOutputsOnTop(false);
    setCanChangeSkin(true);
    setRotatable(false);
    setPixmap(m_pixmapSkinName[0]);
    on = false;
    setHasLabel(true);
    setHasTrigger(true);
    setPortName("Switch");
}

bool InputSwitch::getOn(int port) const
{
    Q_UNUSED(port);
    return on;
}

void InputSwitch::setOn(bool value, int port)
{
    Q_UNUSED(port);
    on = value;
    if (!disabled()) {
        output()->setValue(on);
    }
    if (on) {
        setPixmap(m_pixmapSkinName[1]);
    } else {
        setPixmap(m_pixmapSkinName[0]);
    }
}

void InputSwitch::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setOn(!on);
        event->accept();
    }
    QGraphicsItem::mousePressEvent(event);
}

void InputSwitch::save(QDataStream &ds) const
{
    GraphicElement::save(ds);
    ds << on;
}

void InputSwitch::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version)
{
    GraphicElement::load(ds, portMap, version);
    ds >> on;
    setOn(on);
    output()->setValue(on);
}

void InputSwitch::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        if (!on) {
            m_pixmapSkinName[0] = ":/input/switchOff.png";
            setPixmap(m_pixmapSkinName[0]);
        } else {
            m_pixmapSkinName[1] = ":/input/switchOn.png";
            setPixmap(m_pixmapSkinName[1]);
        }
    } else {
        if (!on) {
            m_pixmapSkinName[0] = filename;
            setPixmap(m_pixmapSkinName[0]);
        } else {
            m_pixmapSkinName[1] = filename;
            setPixmap(m_pixmapSkinName[1]);
        }
    }
}

int InputSwitch::outputSize()
{
    return 1;
}
