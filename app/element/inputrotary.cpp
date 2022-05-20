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
    qCDebug(three) << "Creating rotary.";

    m_pixmapSkinName = QStringList{
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
    setPixmap(m_pixmapSkinName.first());

    m_rotary2  =    m_pixmapSkinName[0];
    m_rotary4  =    m_pixmapSkinName[1];
    m_rotary8  =    m_pixmapSkinName[2];
    m_rotary10 =    m_pixmapSkinName[3];
    m_rotary16 =    m_pixmapSkinName[4];

    m_pointer =     QVector<QPixmap>(16);

    m_pointer[0]  = m_pixmapSkinName[5];
    m_pointer[1]  = m_pixmapSkinName[6];
    m_pointer[2]  = m_pixmapSkinName[7];
    m_pointer[3]  = m_pixmapSkinName[8];
    m_pointer[4]  = m_pixmapSkinName[9];
    m_pointer[5]  = m_pixmapSkinName[10];
    m_pointer[6]  = m_pixmapSkinName[11];
    m_pointer[7]  = m_pixmapSkinName[12];
    m_pointer[8]  = m_pixmapSkinName[13];
    m_pointer[9]  = m_pixmapSkinName[14];
    m_pointer[10] = m_pixmapSkinName[15];
    m_pointer[11] = m_pixmapSkinName[16];
    m_pointer[12] = m_pixmapSkinName[17];
    m_pointer[13] = m_pixmapSkinName[18];
    m_pointer[14] = m_pixmapSkinName[19];
    m_pointer[15] = m_pixmapSkinName[20];

    m_locked = false;
    setRotatable(false);
    setOutputsOnTop(false);
    setCanChangeSkin(true);
    // updatePorts();
    setHasLabel(true);
    setHasTrigger(true);
    setPortName("Rotary");
    setToolTip(m_translatedName);
}

void InputRotary::refresh()
{
    switch (outputSize()) {
    case 2:  setPixmap(m_pixmapSkinName[0]); break;
    case 4:  setPixmap(m_pixmapSkinName[1]); break;
    case 8:  setPixmap(m_pixmapSkinName[2]); break;
    case 10: setPixmap(m_pixmapSkinName[3]); break;
 // case 16:
    default: setPixmap(m_pixmapSkinName[4]); break;
    }
    update();
}

void InputRotary::updatePorts()
{
    switch (outputSize()) {
    case 2:
        output(0)->setPos(20, bottomPosition());
        output(0)->setName("0");
        output(1)->setPos(43, bottomPosition());
        output(1)->setName("1");
        break;
    case 4:
        output(0)->setPos(10, bottomPosition());
        output(0)->setName("0");
        output(1)->setPos(25, bottomPosition());
        output(1)->setName("1");
        output(2)->setPos(39, bottomPosition());
        output(2)->setName("2");
        output(3)->setPos(54, bottomPosition());
        output(3)->setName("3");
        break;
    case 8:
        output(0)->setPos(10, bottomPosition());
        output(0)->setName("0");
        output(1)->setPos(25, bottomPosition());
        output(1)->setName("1");
        output(2)->setPos(39, bottomPosition());
        output(2)->setName("2");
        output(3)->setPos(54, bottomPosition());
        output(3)->setName("3");
        output(4)->setPos(10, topPosition());
        output(4)->setName("4");
        output(5)->setPos(25, topPosition());
        output(5)->setName("5");
        output(6)->setPos(39, topPosition());
        output(6)->setName("6");
        output(7)->setPos(54, topPosition());
        output(7)->setName("7");
        break;
    case 10:
        output(0)->setPos(2, bottomPosition());
        output(0)->setName("0");
        output(1)->setPos(17, bottomPosition());
        output(1)->setName("1");
        output(2)->setPos(32, bottomPosition());
        output(2)->setName("2");
        output(3)->setPos(47, bottomPosition());
        output(3)->setName("3");
        output(4)->setPos(61, bottomPosition());
        output(4)->setName("4");
        output(5)->setPos(2, topPosition());
        output(5)->setName("5");
        output(6)->setPos(17, topPosition());
        output(6)->setName("6");
        output(7)->setPos(32, topPosition());
        output(7)->setName("7");
        output(8)->setPos(47, topPosition());
        output(8)->setName("8");
        output(9)->setPos(61, topPosition());
        output(9)->setName("9");
        break;
    case 16:
        output(0)->setPos(11, bottomPosition());
        output(0)->setName("0");
        output(1)->setPos(25, bottomPosition());
        output(1)->setName("1");
        output(2)->setPos(39, bottomPosition());
        output(2)->setName("2");
        output(3)->setPos(53, bottomPosition());
        output(3)->setName("3");
        output(4)->setPos(bottomPosition(), 53);
        output(4)->setName("4");
        output(5)->setPos(bottomPosition(), 39);
        output(5)->setName("5");
        output(6)->setPos(bottomPosition(), 25);
        output(6)->setName("6");
        output(7)->setPos(bottomPosition(), 11);
        output(7)->setName("7");
        output(8)->setPos(53, topPosition());
        output(8)->setName("8");
        output(9)->setPos(39, topPosition());
        output(9)->setName("9");
        output(10)->setPos(25, topPosition());
        output(10)->setName("A");
        output(11)->setPos(11, topPosition());
        output(11)->setName("B");
        output(12)->setPos(topPosition(), 11);
        output(12)->setName("C");
        output(13)->setPos(topPosition(), 25);
        output(13)->setName("D");
        output(14)->setPos(topPosition(), 39);
        output(14)->setName("E");
        output(15)->setPos(topPosition(), 53);
        output(15)->setName("F");
        break;
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
            m_pixmapSkinName[0] = ":/input/rotary/rotary_2.png";
            setPixmap(m_pixmapSkinName[0]);
            break;
        case 4:
            m_pixmapSkinName[1] = ":/input/rotary/rotary_4.png";
            setPixmap(m_pixmapSkinName[1]);
            break;
        case 8:
            m_pixmapSkinName[2] = ":/input/rotary/rotary_8.png";
            setPixmap(m_pixmapSkinName[2]);
            break;
        case 10:
            m_pixmapSkinName[3] = ":/input/rotary/rotary_10.png";
            setPixmap(m_pixmapSkinName[3]);
            break;
        default: // case 16:
            m_pixmapSkinName[4] = ":/input/rotary/rotary_16.png";
            setPixmap(m_pixmapSkinName[4]);
            break;
        }
    } else {
        switch (outputSize()) {
        case 2:
            m_pixmapSkinName[0] = fileName;
            setPixmap(m_pixmapSkinName[0]);
            break;
        case 4:
            m_pixmapSkinName[1] = ":/input/rotary/rotary_4.png";
            setPixmap(m_pixmapSkinName[1]);
            break;
        case 8:
            m_pixmapSkinName[2] = ":/input/rotary/rotary_8.png";
            setPixmap(m_pixmapSkinName[2]);
            break;
        case 10:
            m_pixmapSkinName[3] = ":/input/rotary/rotary_10.png";
            setPixmap(m_pixmapSkinName[3]);
            break;
        default: // case 16:
            m_pixmapSkinName[4] = ":/input/rotary/rotary_16.png";
            setPixmap(m_pixmapSkinName[4]);
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
