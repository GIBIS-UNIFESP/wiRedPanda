// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
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
    setHasLabel(true);
    setHasTrigger(true);
    setRotatable(false);
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
    setPixmap(static_cast<int>(m_isOn));
    outputPort()->setStatus(static_cast<Status>(m_isOn));
}

void InputSwitch::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_locked && (event->button() == Qt::LeftButton)) {
        setOn(!m_isOn);
        event->accept();
    }

    QGraphicsItem::mousePressEvent(event);
}

void InputSwitch::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("isOn", m_isOn);
    map.insert("locked", m_locked);

    stream << map;
}

void InputSwitch::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    GraphicElement::load(stream, portMap, version);

    if (version < VERSION("4.1")) {
        stream >> m_isOn;

        if (version >= VERSION("3.1")) {
            stream >> m_locked;
        }
    }

    if (version >= VERSION("4.1")) {
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("isOn")) {
            m_isOn = map.value("isOn").toBool();
        }

        if (map.contains("locked")) {
            m_locked = map.value("locked").toBool();
        }
    }

    setOn(m_isOn);
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
