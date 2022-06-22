// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputrotary.h"

#include "qneport.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

namespace
{
int id = qRegisterMetaType<InputRotary>();
}

InputRotary::InputRotary(QGraphicsItem *parent)
    : GraphicElementInput(ElementType::InputRotary, ElementGroup::Input, 0, 0, 2, 16, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

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
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

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
    setPortName(m_translatedName);
    setToolTip(m_translatedName);
}

void InputRotary::refresh()
{
    switch (outputSize()) {
    case 2:  setPixmap(0); break;
    case 4:  setPixmap(1); break;
    case 8:  setPixmap(2); break;
    case 10: setPixmap(3); break;
    case 16: setPixmap(4); break;
    }
    update();
}

void InputRotary::updatePorts()
{
    switch (InputRotary::outputSize()) {
    case 2: {
        outputPort(0)->setPos(20, 64);    outputPort(0)->setName("0");
        outputPort(1)->setPos(43, 64);    outputPort(1)->setName("1");
        break;
    }
    case 4: {
        outputPort(0)->setPos(10, 64);    outputPort(0)->setName("0");
        outputPort(1)->setPos(25, 64);    outputPort(1)->setName("1");
        outputPort(2)->setPos(39, 64);    outputPort(2)->setName("2");
        outputPort(3)->setPos(54, 64);    outputPort(3)->setName("3");
        break;
    }
    case 8: {
        outputPort(0)->setPos(10, 64);    outputPort(0)->setName("0");
        outputPort(1)->setPos(25, 64);    outputPort(1)->setName("1");
        outputPort(2)->setPos(39, 64);    outputPort(2)->setName("2");
        outputPort(3)->setPos(54, 64);    outputPort(3)->setName("3");
        outputPort(4)->setPos(10, 0);     outputPort(4)->setName("4");
        outputPort(5)->setPos(25, 0);     outputPort(5)->setName("5");
        outputPort(6)->setPos(39, 0);     outputPort(6)->setName("6");
        outputPort(7)->setPos(54, 0);     outputPort(7)->setName("7");
        break;
    }
    case 10: {
        outputPort(0)->setPos(2,  64);    outputPort(0)->setName("0");
        outputPort(1)->setPos(17, 64);    outputPort(1)->setName("1");
        outputPort(2)->setPos(32, 64);    outputPort(2)->setName("2");
        outputPort(3)->setPos(47, 64);    outputPort(3)->setName("3");
        outputPort(4)->setPos(61, 64);    outputPort(4)->setName("4");
        outputPort(5)->setPos(2,  0);     outputPort(5)->setName("5");
        outputPort(6)->setPos(17, 0);     outputPort(6)->setName("6");
        outputPort(7)->setPos(32, 0);     outputPort(7)->setName("7");
        outputPort(8)->setPos(47, 0);     outputPort(8)->setName("8");
        outputPort(9)->setPos(61, 0);     outputPort(9)->setName("9");
        break;
    }
    case 16: {
        outputPort(0)->setPos(11, 64);    outputPort(0)->setName("0");
        outputPort(1)->setPos(25, 64);    outputPort(1)->setName("1");
        outputPort(2)->setPos(39, 64);    outputPort(2)->setName("2");
        outputPort(3)->setPos(53, 64);    outputPort(3)->setName("3");
        outputPort(4)->setPos(64, 53);    outputPort(4)->setName("4");
        outputPort(5)->setPos(64, 39);    outputPort(5)->setName("5");
        outputPort(6)->setPos(64, 25);    outputPort(6)->setName("6");
        outputPort(7)->setPos(64, 11);    outputPort(7)->setName("7");
        outputPort(8)->setPos(53, 0);     outputPort(8)->setName("8");
        outputPort(9)->setPos(39, 0);     outputPort(9)->setName("9");
        outputPort(10)->setPos(25, 0);    outputPort(10)->setName("A");
        outputPort(11)->setPos(11, 0);    outputPort(11)->setName("B");
        outputPort(12)->setPos(0, 11);    outputPort(12)->setName("C");
        outputPort(13)->setPos(0, 25);    outputPort(13)->setName("D");
        outputPort(14)->setPos(0, 39);    outputPort(14)->setName("E");
        outputPort(15)->setPos(0, 53);    outputPort(15)->setName("F");
        break;
    }
    }
}

void InputRotary::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicElement::paint(painter, option, widget);
    if (m_value >= outputSize()) {
        m_value = 0;
        outputPort(0)->setValue(Status::Active);
    }
    for (int port = 0; port < outputSize(); ++port) {
        if (m_value == port) {
            outputPort(port)->setValue(Status::Active);

            const int size = outputSize();
            int index = port;

            if (size == 2) { index *= 8; }
            if (size == 4) { index *= 4; }
            if (size == 8) { index *= 2; }

            painter->drawPixmap(0, 0, m_pointer[index]);
        } else {
            outputPort(port)->setValue(Status::Inactive);
        }
    }
}

bool InputRotary::isOn(const int port) const
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
    if (event->modifiers() & Qt::ControlModifier) {
        return;
    }

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
    m_usingDefaultSkin = defaultSkin;
    m_alternativeSkins[outputSize()] = fileName;

    switch (outputSize()) {
    case 2: {
        m_alternativeSkins[0] = fileName;
        setPixmap(0);
        break;
    }
    case 4: {
        m_alternativeSkins[1] = fileName;
        setPixmap(1);
        break;
    }
    case 8: {
        m_alternativeSkins[2] = fileName;
        setPixmap(2);
        break;
    }
    case 10: {
        m_alternativeSkins[3] = fileName;
        setPixmap(3);
        break;
    }
    case 16: {
        m_alternativeSkins[4] = fileName;
        setPixmap(4);
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
