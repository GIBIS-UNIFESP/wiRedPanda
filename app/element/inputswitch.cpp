// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputswitch.h"

#include "common.h"
#include "qneport.h"

#include <QGraphicsSceneMouseEvent>

namespace
{
int id = qRegisterMetaType<InputSwitch>();
}

InputSwitch::InputSwitch(QGraphicsItem *parent)
    : GraphicElement(ElementType::InputSwitch, ElementGroup::Input, 0, 0, 1, 1, parent)
{
    m_defaultSkins = QStringList{
        ":/input/switchOff.png",
        ":/input/switchOn.png",
    };
    setPixmap(m_defaultSkins.first());

    m_locked = false;
    setCanChangeSkin(true);
    setRotatable(false);
    setHasLabel(true);
    setHasTrigger(true);
    setPortName("Switch");
    setToolTip(m_translatedName);
}

bool InputSwitch::on(const int port) const
{
    Q_UNUSED(port);
    return m_isOn;
}

void InputSwitch::setOff()
{
    setOn(!on());
}

void InputSwitch::setOn()
{
    setOn(!on());
}

void InputSwitch::setOn(const bool value, const int port)
{
    Q_UNUSED(port);
    m_isOn = value;
    if (!disabled()) {
        output()->setValue(static_cast<signed char>(m_isOn));
    }
    setPixmap(m_on ? m_defaultSkins.at(1) : m_defaultSkins.at(0));
}

void InputSwitch::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if ((!m_locked) && (event->button() == Qt::LeftButton)) {
        setOn(!m_isOn);
        event->accept();
    }
    QGraphicsItem::mousePressEvent(event);
}

void InputSwitch::save(QDataStream &stream) const
{
    GraphicElement::save(stream);
    stream << m_isOn;
    stream << m_locked;
}

void InputSwitch::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version)
{
    GraphicElement::load(stream, portMap, version);
    stream >> m_isOn;
    if (version >= 3.1) {
        stream >> m_locked;
    }
    setOn(m_isOn);
    output()->setValue(static_cast<signed char>(m_isOn));
}

void InputSwitch::setSkin(const bool defaultSkin, const QString &fileName)
{
    if (defaultSkin) {
        if (!m_on) {
            m_defaultSkins[0] = ":/input/switchOff.png";
            setPixmap(m_defaultSkins[0]);
        } else {
            m_defaultSkins[1] = ":/input/switchOn.png";
            setPixmap(m_defaultSkins[1]);
        }
    } else {
        if (!m_on) {
            m_defaultSkins[0] = fileName;
            setPixmap(m_defaultSkins[0]);
        } else {
            m_defaultSkins[1] = fileName;
            setPixmap(m_defaultSkins[1]);
        }
    }
}
