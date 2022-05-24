// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputrotary.h"

#include "common.h"
#include "qneport.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

namespace
{
int id = qRegisterMetaType<InputRotary>();
}

InputRotary::InputRotary(QGraphicsItem *parent)
    : GraphicElement(ElementType::InputRotary, ElementGroup::Input, 0, 0, 2, 16, parent)
{
    m_defaultSkins = QStringList{
        ":/input/rotary/rotary_2.png",
        ":/input/rotary/rotary_4.png",
        ":/input/rotary/rotary_8.png",
        ":/input/rotary/rotary_10.png",
        ":/input/rotary/rotary_16.png",
        ":/input/rotary/rotary_arrow_0.png",
        ":/input/rotary/rotary_arrow_1.png",
        ":/input/rotary/rotary_arrow_2.png",
        ":/input/rotary/rotary_arrow_3.png",
        ":/input/rotary/rotary_arrow_4.png",
        ":/input/rotary/rotary_arrow_5.png",
        ":/input/rotary/rotary_arrow_6.png",
        ":/input/rotary/rotary_arrow_7.png",
        ":/input/rotary/rotary_arrow_8.png",
        ":/input/rotary/rotary_arrow_9.png",
        ":/input/rotary/rotary_arrow_10.png",
        ":/input/rotary/rotary_arrow_11.png",
        ":/input/rotary/rotary_arrow_12.png",
        ":/input/rotary/rotary_arrow_13.png",
        ":/input/rotary/rotary_arrow_14.png",
        ":/input/rotary/rotary_arrow_15.png",
    };
    setPixmap(m_defaultSkins.first());

    m_rotary2  =    m_defaultSkins[0];
    m_rotary4  =    m_defaultSkins[1];
    m_rotary8  =    m_defaultSkins[2];
    m_rotary10 =    m_defaultSkins[3];
    m_rotary16 =    m_defaultSkins[4];

    m_pointer =     QVector<QPixmap>(16);

    m_pointer[0]  = m_defaultSkins[5];
    m_pointer[1]  = m_defaultSkins[6];
    m_pointer[2]  = m_defaultSkins[7];
    m_pointer[3]  = m_defaultSkins[8];
    m_pointer[4]  = m_defaultSkins[9];
    m_pointer[5]  = m_defaultSkins[10];
    m_pointer[6]  = m_defaultSkins[11];
    m_pointer[7]  = m_defaultSkins[12];
    m_pointer[8]  = m_defaultSkins[13];
    m_pointer[9]  = m_defaultSkins[14];
    m_pointer[10] = m_defaultSkins[15];
    m_pointer[11] = m_defaultSkins[16];
    m_pointer[12] = m_defaultSkins[17];
    m_pointer[13] = m_defaultSkins[18];
    m_pointer[14] = m_defaultSkins[19];
    m_pointer[15] = m_defaultSkins[20];

    m_locked = false;
    setRotatable(false);
    setCanChangeSkin(true);
    InputRotary::updatePorts();
    setHasLabel(true);
    setHasTrigger(true);
    setPortName("Rotary");
    setToolTip(m_translatedName);
}

void InputRotary::refresh()
{
    switch (outputSize()) {
    case 2:  setPixmap(m_defaultSkins[0]); break;
    case 4:  setPixmap(m_defaultSkins[1]); break;
    case 8:  setPixmap(m_defaultSkins[2]); break;
    case 10: setPixmap(m_defaultSkins[3]); break;
 // case 16:
    default: setPixmap(m_defaultSkins[4]); break;
    }
    update();
}

void InputRotary::updatePorts()
{
    switch (outputSize()) {
    case 2: {
        output(0)->setPos(20, rightPosition()); output(0)->setName("0");
        output(1)->setPos(43, rightPosition()); output(1)->setName("1");
        break;
    }
    case 4: {
        output(0)->setPos(10, rightPosition()); output(0)->setName("0");
        output(1)->setPos(25, rightPosition()); output(1)->setName("1");
        output(2)->setPos(39, rightPosition()); output(2)->setName("2");
        output(3)->setPos(54, rightPosition()); output(3)->setName("3");
        break;
    }
    case 8: {
        output(0)->setPos(10, rightPosition()); output(0)->setName("0");
        output(1)->setPos(25, rightPosition()); output(1)->setName("1");
        output(2)->setPos(39, rightPosition()); output(2)->setName("2");
        output(3)->setPos(54, rightPosition()); output(3)->setName("3");
        output(4)->setPos(10, leftPosition());  output(4)->setName("4");
        output(5)->setPos(25, leftPosition());  output(5)->setName("5");
        output(6)->setPos(39, leftPosition());  output(6)->setName("6");
        output(7)->setPos(54, leftPosition());  output(7)->setName("7");
        break;
    }
    case 10: {
        output(0)->setPos(2,  rightPosition()); output(0)->setName("0");
        output(1)->setPos(17, rightPosition()); output(1)->setName("1");
        output(2)->setPos(32, rightPosition()); output(2)->setName("2");
        output(3)->setPos(47, rightPosition()); output(3)->setName("3");
        output(4)->setPos(61, rightPosition()); output(4)->setName("4");
        output(5)->setPos(2,  leftPosition());  output(5)->setName("5");
        output(6)->setPos(17, leftPosition());  output(6)->setName("6");
        output(7)->setPos(32, leftPosition());  output(7)->setName("7");
        output(8)->setPos(47, leftPosition());  output(8)->setName("8");
        output(9)->setPos(61, leftPosition());  output(9)->setName("9");
        break;
    }
    case 16: {
        output(0)->setPos(11, rightPosition()); output(0)->setName("0");
        output(1)->setPos(25, rightPosition()); output(1)->setName("1");
        output(2)->setPos(39, rightPosition()); output(2)->setName("2");
        output(3)->setPos(53, rightPosition()); output(3)->setName("3");
        output(4)->setPos(rightPosition(), 53); output(4)->setName("4");
        output(5)->setPos(rightPosition(), 39); output(5)->setName("5");
        output(6)->setPos(rightPosition(), 25); output(6)->setName("6");
        output(7)->setPos(rightPosition(), 11); output(7)->setName("7");
        output(8)->setPos(53, leftPosition());  output(8)->setName("8");
        output(9)->setPos(39, leftPosition());  output(9)->setName("9");
        output(10)->setPos(25, leftPosition()); output(10)->setName("A");
        output(11)->setPos(11, leftPosition()); output(11)->setName("B");
        output(12)->setPos(leftPosition(), 11); output(12)->setName("C");
        output(13)->setPos(leftPosition(), 25); output(13)->setName("D");
        output(14)->setPos(leftPosition(), 39); output(14)->setName("E");
        output(15)->setPos(leftPosition(), 53); output(15)->setName("F");
        break;
    }
    }
}

void InputRotary::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicElement::paint(painter, option, widget);
    if (m_value >= outputSize()) {
        m_value = 0;
        output(0)->setValue(1);
    }
    for (int port = 0; port < outputSize(); ++port) {
        if (m_value == port) {
            output(port)->setValue(1);

            const int size = outputSize();
            int index = port;

            if (size == 2) { index *= 8; }
            if (size == 4) { index *= 4; }
            if (size == 8) { index *= 2; }

            painter->drawPixmap(0, 0, m_pointer[index]);
        } else {
            output(port)->setValue(0);
        }
    }
}

bool InputRotary::on(const int port) const
{
    return (m_value == port);
}

void InputRotary::setOff()
{
    const int port = (outputValue() + 1) % outputSize();
    setOn(false, port);
}

void InputRotary::setOn()
{
    const int port = (outputValue() + 1) % outputSize();
    setOn(true, port);
}

void InputRotary::setOn(const bool value, const int port)
{
    if (value) {
        m_value = port;
        refresh();
    }
}

void InputRotary::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if ((!m_locked) && (event->button() == Qt::LeftButton)) {
        setOn(true, (m_value + 1) % outputSize());
        event->accept();
    }
    QGraphicsItem::mousePressEvent(event);
}

void InputRotary::save(QDataStream &stream) const
{
    GraphicElement::save(stream);
    stream << m_value;
    stream << m_locked;
}

void InputRotary::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version)
{
    GraphicElement::load(stream, portMap, version);
    stream >> m_value;
    if (version >= 3.1) {
        stream >> m_locked;
    }
    setOn(true, m_value);
}

void InputRotary::setSkin(const bool defaultSkin, const QString &fileName)
{
    if (defaultSkin) {
        switch (outputSize()) {
        case 2:
            m_defaultSkins[0] = ":/input/rotary/rotary_2.png";
            setPixmap(m_defaultSkins[0]);
            break;
        case 4:
            m_defaultSkins[1] = ":/input/rotary/rotary_4.png";
            setPixmap(m_defaultSkins[1]);
            break;
        case 8:
            m_defaultSkins[2] = ":/input/rotary/rotary_8.png";
            setPixmap(m_defaultSkins[2]);
            break;
        case 10:
            m_defaultSkins[3] = ":/input/rotary/rotary_10.png";
            setPixmap(m_defaultSkins[3]);
            break;
        default: // case 16:
            m_defaultSkins[4] = ":/input/rotary/rotary_16.png";
            setPixmap(m_defaultSkins[4]);
            break;
        }
    } else {
        switch (outputSize()) {
        case 2:
            m_defaultSkins[0] = fileName;
            setPixmap(m_defaultSkins[0]);
            break;
        case 4:
            m_defaultSkins[1] = ":/input/rotary/rotary_4.png";
            setPixmap(m_defaultSkins[1]);
            break;
        case 8:
            m_defaultSkins[2] = ":/input/rotary/rotary_8.png";
            setPixmap(m_defaultSkins[2]);
            break;
        case 10:
            m_defaultSkins[3] = ":/input/rotary/rotary_10.png";
            setPixmap(m_defaultSkins[3]);
            break;
        default: // case 16:
            m_defaultSkins[4] = ":/input/rotary/rotary_16.png";
            setPixmap(m_defaultSkins[4]);
            break;
        }
    }
}

int InputRotary::outputSize() const
{
    return (outputs().size());
}

int InputRotary::outputValue() const
{
    return m_value;
}
