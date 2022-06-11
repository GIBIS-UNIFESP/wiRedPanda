// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputswitch.h"

#include "qneport.h"

#include <QGraphicsSceneMouseEvent>

namespace
{
int id = qRegisterMetaType<InputSwitch>();
}

InputSwitch::InputSwitch(QGraphicsItem *parent)
    : GraphicElementInput(ElementType::InputSwitch, ElementGroup::Input, 0, 0, 1, 1, parent)
{
    if (GlobalProperties::skipInit) { return; }

    m_defaultSkins = QStringList{
        ":/input/switchOff.png",
        ":/input/switchOn.png",
    };
    m_alternativeSkins = m_defaultSkins;
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

    if (!isDisabled()) {
        output()->setValue(static_cast<Status>(m_isOn));
    }

    setPixmap(m_usingDefaultSkin ? m_defaultSkins.at(m_isOn) : m_alternativeSkins.at(m_isOn));
}

void InputSwitch::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->modifiers() & Qt::ControlModifier){
        return;
    }
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
    output()->setValue(static_cast<Status>(m_isOn));
}

void InputSwitch::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_usingDefaultSkin = defaultSkin;
    m_alternativeSkins[m_isOn] = fileName;
    setPixmap(defaultSkin ? m_defaultSkins.at(m_isOn) : m_alternativeSkins.at(m_isOn));
}
