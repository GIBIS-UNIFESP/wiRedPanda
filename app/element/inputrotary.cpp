// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputrotary.h"

#include "globalproperties.h"
#include "qneport.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

namespace
{
int id = qRegisterMetaType<InputRotary>();
}

InputRotary::InputRotary(QGraphicsItem *parent)
    : GraphicElementInput(ElementType::InputRotary, ElementGroup::Input, ":/input/rotary/rotary_icon.svg", tr("<b>ROTARY SWITCH</b>"), tr("Rotary Switch"), 0, 0, 2, 16, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins = QStringList{
        ":/input/rotary/rotary.svg",
        ":/input/rotary/rotary_arrow.svg"
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    m_rotary = m_defaultSkins[0];
    m_arrow  = m_defaultSkins[1];

    setCanChangeSkin(true);
    setHasLabel(true);
    setHasTrigger(true);
    setLocked(false);
    setPortName(m_translatedName);
    setToolTip(m_translatedName);

    InputRotary::updatePortsProperties();
}

void InputRotary::refresh()
{
    if (m_currentPort >= InputRotary::outputSize()) {
        m_currentPort = 0;
    }

    update();
}

void InputRotary::updatePortsProperties()
{
    switch (InputRotary::outputSize()) {
    case 2: {
        outputPort(0)->setPos(32,  0);    outputPort(0)->setName("0");
        outputPort(1)->setPos(32, 64);    outputPort(1)->setName("1");
        break;
    }
    case 3: {
        outputPort(0)->setPos(32,  0);   outputPort( 0)->setName("0");
        outputPort(1)->setPos(64, 48);   outputPort( 1)->setName("1");
        outputPort(2)->setPos(0,  48);   outputPort( 2)->setName("2");
        break;
    }
    case 4: {
        outputPort(0)->setPos(32,  0);    outputPort(0)->setName("0");
        outputPort(1)->setPos(64, 32);    outputPort(1)->setName("1");
        outputPort(2)->setPos(32, 64);    outputPort(2)->setName("2");
        outputPort(3)->setPos( 0, 32);    outputPort(3)->setName("3");
        break;
    }
    case 6: {
        outputPort(0)->setPos(32,  0);   outputPort( 0)->setName("0");
        outputPort(1)->setPos(64, 16);   outputPort( 1)->setName("1");
        outputPort(2)->setPos(64, 48);   outputPort( 2)->setName("2");
        outputPort(3)->setPos(32, 64);   outputPort( 3)->setName("3");
        outputPort(4)->setPos( 0, 48);   outputPort( 4)->setName("4");
        outputPort(5)->setPos( 0, 16);   outputPort( 5)->setName("5");
        break;
    }
    case 8: {
        outputPort(0)->setPos(32,  0);    outputPort(0)->setName("0");
        outputPort(1)->setPos(64,  0);    outputPort(1)->setName("1");
        outputPort(2)->setPos(64, 32);    outputPort(2)->setName("2");
        outputPort(3)->setPos(64, 64);    outputPort(3)->setName("3");
        outputPort(4)->setPos(32, 64);    outputPort(4)->setName("4");
        outputPort(5)->setPos( 0, 64);    outputPort(5)->setName("5");
        outputPort(6)->setPos( 0, 32);    outputPort(6)->setName("6");
        outputPort(7)->setPos( 0,  0);    outputPort(7)->setName("7");
        break;
    }
    case 10: {
        outputPort(0)->setPos(32,  0);    outputPort(0)->setName("0");
        outputPort(1)->setPos(56,  0);    outputPort(1)->setName("1");
        outputPort(2)->setPos(64, 24);    outputPort(2)->setName("2");
        outputPort(3)->setPos(64, 40);    outputPort(3)->setName("3");
        outputPort(4)->setPos(56, 64);    outputPort(4)->setName("4");
        outputPort(5)->setPos(32, 64);    outputPort(5)->setName("5");
        outputPort(6)->setPos( 8, 64);    outputPort(6)->setName("6");
        outputPort(7)->setPos( 0, 40);    outputPort(7)->setName("7");
        outputPort(8)->setPos( 0, 24);    outputPort(8)->setName("8");
        outputPort(9)->setPos( 8,  0);    outputPort(9)->setName("9");
        break;
    }
    case 12: {
        outputPort( 0)->setPos(32,  0);   outputPort( 0)->setName("0");
        outputPort( 1)->setPos(48,  0);   outputPort( 1)->setName("1");
        outputPort( 2)->setPos(64, 16);   outputPort( 2)->setName("2");
        outputPort( 3)->setPos(64, 32);   outputPort( 3)->setName("3");
        outputPort( 4)->setPos(64, 48);   outputPort( 4)->setName("4");
        outputPort( 5)->setPos(48, 64);   outputPort( 5)->setName("5");
        outputPort( 6)->setPos(32, 64);   outputPort( 6)->setName("6");
        outputPort( 7)->setPos(16, 64);   outputPort( 7)->setName("7");
        outputPort( 8)->setPos( 0, 48);   outputPort( 8)->setName("8");
        outputPort( 9)->setPos( 0, 32);   outputPort( 9)->setName("9");
        outputPort(10)->setPos( 0, 16);   outputPort(10)->setName("A");
        outputPort(11)->setPos(16,  0);   outputPort(11)->setName("B");
        break;
    }
    case 16: {
        outputPort( 0)->setPos(32,  0);   outputPort( 0)->setName("0");
        outputPort( 1)->setPos(48,  0);   outputPort( 1)->setName("1");
        outputPort( 2)->setPos(64,  0);   outputPort( 2)->setName("2");
        outputPort( 3)->setPos(64, 16);   outputPort( 3)->setName("3");
        outputPort( 4)->setPos(64, 32);   outputPort( 4)->setName("4");
        outputPort( 5)->setPos(64, 48);   outputPort( 5)->setName("5");
        outputPort( 6)->setPos(64, 64);   outputPort( 6)->setName("6");
        outputPort( 7)->setPos(48, 64);   outputPort( 7)->setName("7");
        outputPort( 8)->setPos(32, 64);   outputPort( 8)->setName("8");
        outputPort( 9)->setPos(16, 64);   outputPort( 9)->setName("9");
        outputPort(10)->setPos( 0, 64);   outputPort(10)->setName("A");
        outputPort(11)->setPos( 0, 48);   outputPort(11)->setName("B");
        outputPort(12)->setPos( 0, 32);   outputPort(12)->setName("C");
        outputPort(13)->setPos( 0, 16);   outputPort(13)->setName("D");
        outputPort(14)->setPos( 0,  0);   outputPort(14)->setName("E");
        outputPort(15)->setPos(16,  0);   outputPort(15)->setName("F");
        break;
    }
    }

    InputRotary::refresh();
}

void InputRotary::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicElement::paint(painter, option, widget);

    for (int port = 0; port < outputSize(); ++port) {
        painter->save();

        const double angle = 360. / outputSize() * port;
        const QPointF center = boundingRect().center();
        QRectF mark{30.2, 8.727, 3.6, 6.4};

        painter->setBrush(QBrush{QColor{255, 246, 213}});
        painter->translate(center.x(), center.y());
        painter->rotate(angle);
        painter->translate(-center.x(), -center.y());
        painter->drawRect(mark);

        painter->restore();

        if (m_currentPort != port) {
            continue;
        }

        painter->save();

        painter->translate(center.x(), center.y());
        painter->rotate(angle);
        painter->translate(-center.x(), -center.y());
        painter->drawPixmap(0, 0, m_arrow);

        painter->restore();
    }
}

bool InputRotary::isOn(const int port) const
{
    return (m_currentPort == port);
}

void InputRotary::setOff()
{
}

void InputRotary::setOn()
{
    const int port = (outputValue() + 1) % outputSize();
    setOn(true, port);
}

void InputRotary::setOn(const bool value, const int port)
{
    Q_UNUSED(value)

    m_currentPort = port;

    if (m_currentPort >= outputSize()) {
        m_currentPort = 0;
    }

    for (int index = 0; index < outputSize(); ++index) {
        outputPort(index)->setStatus((m_currentPort == index) ? Status::Active : Status::Inactive);
    }

    refresh();
}

void InputRotary::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        return;
    }

    if (!m_locked && (event->button() == Qt::LeftButton)) {
        setOn(true, (m_currentPort + 1) % outputSize());
        event->accept();
    }

    QGraphicsItem::mousePressEvent(event);
}

void InputRotary::save(QDataStream &stream) const
{
    GraphicElement::save(stream);
    stream << m_currentPort;
    stream << m_locked;
}

void InputRotary::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version)
{
    GraphicElement::load(stream, portMap, version);
    stream >> m_currentPort;

    if (version >= 3.1) {
        stream >> m_locked;
    }

    setOn(true, m_currentPort);
}

void InputRotary::setSkin(const bool defaultSkin, const QString &fileName)
{
    if (defaultSkin) {
        m_alternativeSkins = m_defaultSkins;
    } else {
        m_alternativeSkins[0] = fileName;
    }

    m_usingDefaultSkin = defaultSkin;
    setPixmap(0);
}

int InputRotary::outputSize() const
{
    return outputs().size();
}

int InputRotary::outputValue() const
{
    return m_currentPort;
}
