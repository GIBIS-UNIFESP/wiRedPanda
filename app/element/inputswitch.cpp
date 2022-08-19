// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputswitch.h"

#include "globalproperties.h"
#include "qneport.h"

#include <QGraphicsSceneMouseEvent>

namespace
{
int id = qRegisterMetaType<InputSwitch>();
}

InputSwitch::InputSwitch(QGraphicsItem *parent)
    : GraphicElementInput(ElementType::InputSwitch, ElementGroup::Input, ":/input/switchOn.svg", tr("INPUT SWITCH"), tr("Input Switch"), 0, 0, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins = QStringList{
        ":/input/switchOff.svg",
        ":/input/switchOn.svg",
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    m_locked = false;
    setCanChangeSkin(true);
    setRotatable(false);
    setHasLabel(true);
    setHasTrigger(true);
    setPortName(m_translatedName);
    setToolTip(m_translatedName);
}

bool InputSwitch::isOn(const int port) const
{
    Q_UNUSED(port)
    return m_isOn;
}

void InputSwitch::setOff()
{
    InputSwitch::setOn(!isOn());
}

void InputSwitch::setOn()
{
    InputSwitch::setOn(!isOn());
}

void InputSwitch::setOn(const bool value, const int port)
{
    Q_UNUSED(port)

    m_isOn = value;
    outputPort()->setStatus(static_cast<Status>(m_isOn));
    setPixmap(static_cast<int>(m_isOn));
}

void InputSwitch::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        return;
    }

    if (!m_locked && (event->button() == Qt::LeftButton)) {
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
    outputPort()->setStatus(static_cast<Status>(m_isOn));
}

void InputSwitch::setSkin(const bool defaultSkin, const QString &fileName)
{
    if (defaultSkin) {
        m_alternativeSkins = m_defaultSkins;
    } else {
        m_alternativeSkins[static_cast<int>(m_isOn)] = fileName;
    }

    m_usingDefaultSkin = defaultSkin;
    setPixmap(static_cast<int>(m_isOn));
}
