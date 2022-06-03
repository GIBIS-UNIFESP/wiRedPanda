// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputbutton.h"

#include "qneport.h"

#include <QGraphicsSceneMouseEvent>

namespace
{
int id = qRegisterMetaType<InputButton>();
}

InputButton::InputButton(QGraphicsItem *parent)
    : GraphicElement(ElementType::InputButton, ElementGroup::Input, 0, 0, 1, 1, parent)
{
    m_defaultSkins = QStringList{
        ":/input/buttonOff.png",
        ":/input/buttonOn.png",
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(m_defaultSkins.first());

    m_locked = false;
    setCanChangeSkin(true);
    setRotatable(false);
    m_outputs.first()->setValue(0);
    InputButton::setOn(false);
    setHasLabel(true);
    setHasTrigger(true);
    setPortName("Button");
    setToolTip(m_translatedName);
}

void InputButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->modifiers() & Qt::ControlModifier){
        return;
    }
    if ((!m_locked) && (event->button() == Qt::LeftButton)) {
        setOn(true);
        event->accept();
    }
    QGraphicsItem::mousePressEvent(event);
}

void InputButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if ((!m_locked) && (event->button() == Qt::LeftButton)) {
        setOn(false);
        event->accept();
    }
    GraphicElement::mouseReleaseEvent(event);
}

void InputButton::save(QDataStream &stream) const
{
    GraphicElement::save(stream);
    stream << m_locked;
}

void InputButton::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version)
{
    GraphicElement::load(stream, portMap, version);
    if (version >= 3.1) {
        stream >> m_locked;
    }
}

bool InputButton::on(const int port) const
{
    Q_UNUSED(port);
    return m_isOn;
}

void InputButton::setOff()
{
    setOn(false);
}

void InputButton::setOn()
{
    setOn(true);
}

void InputButton::setOn(const bool value, const int port)
{
    Q_UNUSED(port);

    m_isOn = value;
    setPixmap(m_usingDefaultSkin ? m_defaultSkins.at(m_isOn) : m_alternativeSkins.at(m_isOn));

    if (!isDisabled()) {
        output()->setValue(static_cast<signed char>(m_isOn));
    }
}

void InputButton::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_usingDefaultSkin = defaultSkin;
    m_alternativeSkins[m_isOn] = fileName;
    setPixmap(defaultSkin ? m_defaultSkins.at(m_isOn) : m_alternativeSkins.at(m_isOn));
}
