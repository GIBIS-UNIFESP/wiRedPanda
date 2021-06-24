// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"
#include "inputrotary.h"
#include "qneport.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

int InputRotary::current_id_number = 0;

InputRotary::InputRotary(QGraphicsItem *parent)
    : GraphicElement(ElementType::ROTARY, ElementGroup::INPUT, 0, 0, 2, 16, parent)
{
    COMMENT("Creating rotaty.", 0);
    m_value = 0;
    m_pixmapSkinName = {
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

    setRotatable(false);
    setOutputsOnTop(false);
    setCanChangeSkin(true);
    //updatePorts();
    setHasLabel(true);
    setHasTrigger(true);
    setPortName("Rotary");

    setPixmap(m_pixmapSkinName[0]);
    m_rotary2 = QPixmap(m_pixmapSkinName[0]);
    m_rotary4 = QPixmap(m_pixmapSkinName[1]);
    m_rotary8 = QPixmap(m_pixmapSkinName[2]);
    m_rotary10 = QPixmap(m_pixmapSkinName[3]);
    m_rotary16 = QPixmap(m_pixmapSkinName[4]);
    m_pointer = QVector<QPixmap>(16);
    m_pointer[0] = QPixmap(m_pixmapSkinName[5]);
    m_pointer[1] = QPixmap(m_pixmapSkinName[6]);
    m_pointer[2] = QPixmap(m_pixmapSkinName[7]);
    m_pointer[3] = QPixmap(m_pixmapSkinName[8]);
    m_pointer[4] = QPixmap(m_pixmapSkinName[9]);
    m_pointer[5] = QPixmap(m_pixmapSkinName[10]);
    m_pointer[6] = QPixmap(m_pixmapSkinName[11]);
    m_pointer[7] = QPixmap(m_pixmapSkinName[12]);
    m_pointer[8] = QPixmap(m_pixmapSkinName[13]);
    m_pointer[9] = QPixmap(m_pixmapSkinName[14]);
    m_pointer[10] = QPixmap(m_pixmapSkinName[15]);
    m_pointer[11] = QPixmap(m_pixmapSkinName[16]);
    m_pointer[12] = QPixmap(m_pixmapSkinName[17]);
    m_pointer[13] = QPixmap(m_pixmapSkinName[18]);
    m_pointer[14] = QPixmap(m_pixmapSkinName[19]);
    m_pointer[15] = QPixmap(m_pixmapSkinName[20]);
}

void InputRotary::refresh()
{
    switch (outputSize()) {
    case 2:
        setPixmap(m_pixmapSkinName[0]);
        break;
    case 4:
        setPixmap(m_pixmapSkinName[1]);
        break;
    case 8:
        setPixmap(m_pixmapSkinName[2]);
        break;
    case 10:
        setPixmap(m_pixmapSkinName[3]);
        break;
    default: // case 16:
        setPixmap(m_pixmapSkinName[4]);
        break;
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
    for (int port=0; port < outputSize(); ++port) {
        if( m_value == port ) {
            output(port)->setValue(1);
            switch (outputSize()) {
            case 2:
                painter->drawPixmap(QPoint(0, 0), m_pointer[port*8]);
                break;
            case 4:
                painter->drawPixmap(QPoint(0, 0), m_pointer[port*4]);
                break;
            case 8:
                painter->drawPixmap(QPoint(0, 0), m_pointer[port*2]);
                break;
            default:
                painter->drawPixmap(QPoint(0, 0), m_pointer[port]);
                break;
            }
        }
        else {
            output(port)->setValue(0);
        }
    }
}

bool InputRotary::getOn(int port) const
{
    return(m_value == port);
}

void InputRotary::setOn(bool value, int port)
{
    if (value) {
        m_value = port;
        refresh();
    }
}

void InputRotary::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setOn(true, (m_value + 1)%outputSize());
        event->accept();
    }
    QGraphicsItem::mousePressEvent(event);
}

void InputRotary::save(QDataStream &ds) const
{
    GraphicElement::save(ds);
    ds << m_value;
}

void InputRotary::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version)
{
    GraphicElement::load(ds, portMap, version);
    ds >> m_value;
    setOn(true, m_value);
    //output(m_value)->setValue(true);
}

void InputRotary::setSkin(bool defaultSkin, const QString &filename)
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
        default: //case 16:
            m_pixmapSkinName[4] = ":/input/rotary/rotary_16.png";
            setPixmap(m_pixmapSkinName[4]);
            break;
        }
    } else {
        switch (outputSize()) {
        case 2:
            m_pixmapSkinName[0] = filename;
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
        default: //case 16:
            m_pixmapSkinName[4] = ":/input/rotary/rotary_16.png";
            setPixmap(m_pixmapSkinName[4]);
            break;
        }
    }
}

int InputRotary::outputSize()
{
    return(outputs().size());
}
