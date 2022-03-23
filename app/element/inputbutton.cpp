// Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputbutton.h"

#include "qneport.h"

#include <QGraphicsSceneMouseEvent>

int InputButton::current_id_number = 0;

InputButton::InputButton(QGraphicsItem *parent)
    : GraphicElement(ElementType::BUTTON, ElementGroup::INPUT, 0, 0, 1, 1, parent)
{
    m_pixmapSkinName = {
        ":/input/buttonOff.png",
        ":/input/buttonOn.png",
    };
    locked = false;
    setOutputsOnTop(false);
    setCanChangeSkin(true);
    setPixmap(m_pixmapSkinName[0]);
    setRotatable(false);
    m_outputs.first()->setValue(0);
    InputButton::setOn(false);
    setHasLabel(true);
    setHasTrigger(true);
    setPortName("Button");
}

void InputButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if ((!locked)&&(event->button() == Qt::LeftButton)) {
        setOn(true);
        event->accept();
    }
    QGraphicsItem::mousePressEvent(event);
}

void InputButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if ((!locked)&&(event->button() == Qt::LeftButton)) {
        setOn(false);
        event->accept();
    }
    GraphicElement::mouseReleaseEvent(event);
}

void InputButton::save(QDataStream &ds) const
{
    GraphicElement::save(ds);
    ds << locked;
}

void InputButton::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version)
{
    GraphicElement::load(ds, portMap, version);
    if (version >= 3.1) {
        ds >> locked;
    }
}

bool InputButton::getOn(int port) const
{
    Q_UNUSED(port);
    return on;
}

void InputButton::setOn(const bool value, int port)
{
    Q_UNUSED(port);
    on = value;
    setPixmap(on ? m_pixmapSkinName[1] : m_pixmapSkinName[0]);
    if (!disabled()) {
        output()->setValue(on);
    }
}

void InputButton::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        if (!on) {
            m_pixmapSkinName[0] = ":/input/buttonOff.png";
            setPixmap(m_pixmapSkinName[0]);
        } else {
            m_pixmapSkinName[1] = ":/input/buttonOn.png";
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
