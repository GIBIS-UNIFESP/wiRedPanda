// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display_14.h"

#include "display_7.h"
#include "globalproperties.h"
#include "qneport.h"

#include <QPainter>

Display14::Display14(QGraphicsItem *parent)
    : GraphicElement(ElementType::Display14, ElementGroup::Output, ":/output/counter/counter_14_on.svg", tr("14-SEGMENT DISPLAY"), tr("14-Segment Display"), 15, 15, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins = QStringList{
        ":/output/counter/counter_14_off.svg",
        ":/output/counter/counter_a.svg",
        ":/output/counter/counter_b.svg",
        ":/output/counter/counter_c.svg",
        ":/output/counter/counter_d.svg",
        ":/output/counter/counter_e.svg",
        ":/output/counter/counter_f.svg",
        ":/output/counter/counter_g1.svg",
        ":/output/counter/counter_g2.svg",
        ":/output/counter/counter_h.svg",
        ":/output/counter/counter_j.svg",
        ":/output/counter/counter_k.svg",
        ":/output/counter/counter_l.svg",
        ":/output/counter/counter_m.svg",
        ":/output/counter/counter_n.svg",
        ":/output/counter/counter_dp.svg",
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    a  = QVector<QPixmap>(5, m_defaultSkins.at(1));
    b  = QVector<QPixmap>(5, m_defaultSkins.at(2));
    c  = QVector<QPixmap>(5, m_defaultSkins.at(3));
    d  = QVector<QPixmap>(5, m_defaultSkins.at(4));
    e  = QVector<QPixmap>(5, m_defaultSkins.at(5));
    f  = QVector<QPixmap>(5, m_defaultSkins.at(6));
    g1 = QVector<QPixmap>(5, m_defaultSkins.at(7));
    g2 = QVector<QPixmap>(5, m_defaultSkins.at(8));
    h  = QVector<QPixmap>(5, m_defaultSkins.at(9));
    j  = QVector<QPixmap>(5, m_defaultSkins.at(10));
    k  = QVector<QPixmap>(5, m_defaultSkins.at(11));
    l  = QVector<QPixmap>(5, m_defaultSkins.at(12));
    m  = QVector<QPixmap>(5, m_defaultSkins.at(13));
    n  = QVector<QPixmap>(5, m_defaultSkins.at(14));
    dp = QVector<QPixmap>(5, m_defaultSkins.at(15));

    Display7::convertAllColors(a);
    Display7::convertAllColors(b);
    Display7::convertAllColors(c);
    Display7::convertAllColors(d);
    Display7::convertAllColors(e);
    Display7::convertAllColors(f);
    Display7::convertAllColors(g1);
    Display7::convertAllColors(g2);
    Display7::convertAllColors(h);
    Display7::convertAllColors(j);
    Display7::convertAllColors(k);
    Display7::convertAllColors(l);
    Display7::convertAllColors(m);
    Display7::convertAllColors(n);
    Display7::convertAllColors(dp);

    setCanChangeSkin(true);
    setHasColors(true);
    setHasLabel(true);
    setRotatable(false);

    Display14::updatePortsProperties();
}

void Display14::refresh()
{
    update();
}

void Display14::updatePortsProperties()
{
    inputPort( 0)->setPos( 0,  -8);    inputPort(0)->setName("G1 (" + tr("middle left horizontal")  + ")");
    inputPort( 1)->setPos( 0,   8);    inputPort(1)->setName("F (" +  tr("upper left vertical")     + ")");
    inputPort( 2)->setPos( 0,  24);    inputPort(2)->setName("E (" +  tr("lower left vertical")     + ")");
    inputPort( 3)->setPos( 0,  40);    inputPort(3)->setName("D (" +  tr("bottom")                  + ")");
    inputPort( 4)->setPos(64,  -8);    inputPort(4)->setName("A (" +  tr("top")                     + ")");
    inputPort( 5)->setPos(64,   8);    inputPort(5)->setName("B (" +  tr("upper right vertical")    + ")");
    inputPort( 6)->setPos(64,  24);    inputPort(6)->setName("DP (" + tr("dot")                     + ")");
    inputPort( 7)->setPos(64,  40);    inputPort(7)->setName("C (" +  tr("lower right vertical")    + ")");
    inputPort( 8)->setPos( 0,  56);    inputPort(8)->setName("G2 (" + tr("middle right horizontal") + ")");
    inputPort( 9)->setPos( 0,  72);    inputPort(9)->setName("H (" +  tr("upper left diagonal")     + ")");
    inputPort(10)->setPos( 0,  88);    inputPort(10)->setName("J (" + tr("upper center vertical")   + ")");
    inputPort(11)->setPos(64,  56);    inputPort(11)->setName("K (" + tr("upper right diagonal")    + ")");
    inputPort(12)->setPos(64,  72);    inputPort(12)->setName("L (" + tr("lower right diagonal")    + ")");
    inputPort(13)->setPos(64,  88);    inputPort(13)->setName("M (" + tr("lower center vertical")   + ")");
    inputPort(14)->setPos(64, 104);    inputPort(14)->setName("N (" + tr("lower left diagonal")    + ")");

    for (auto *port : std::as_const(m_inputPorts)) {
        port->setRequired(false);
        port->setDefaultStatus(Status::Inactive);
    }
}

void Display14::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicElement::paint(painter, option, widget);

    if (inputPort(0)->status() == Status::Active)  { painter->drawPixmap(0, 0, g1.at(m_colorNumber)); }
    if (inputPort(1)->status() == Status::Active)  { painter->drawPixmap(0, 0, f.at(m_colorNumber));  }
    if (inputPort(2)->status() == Status::Active)  { painter->drawPixmap(0, 0, e.at(m_colorNumber));  }
    if (inputPort(3)->status() == Status::Active)  { painter->drawPixmap(0, 0, d.at(m_colorNumber));  }
    if (inputPort(4)->status() == Status::Active)  { painter->drawPixmap(0, 0, a.at(m_colorNumber));  }
    if (inputPort(5)->status() == Status::Active)  { painter->drawPixmap(0, 0, b.at(m_colorNumber));  }
    if (inputPort(6)->status() == Status::Active)  { painter->drawPixmap(0, 0, dp.at(m_colorNumber)); }
    if (inputPort(7)->status() == Status::Active)  { painter->drawPixmap(0, 0, c.at(m_colorNumber));  }
    if (inputPort(8)->status() == Status::Active)  { painter->drawPixmap(0, 0, g2.at(m_colorNumber)); }
    if (inputPort(9)->status() == Status::Active)  { painter->drawPixmap(0, 0, h.at(m_colorNumber));  }
    if (inputPort(10)->status() == Status::Active) { painter->drawPixmap(0, 0, j.at(m_colorNumber));  }
    if (inputPort(11)->status() == Status::Active) { painter->drawPixmap(0, 0, k.at(m_colorNumber));  }
    if (inputPort(12)->status() == Status::Active) { painter->drawPixmap(0, 0, l.at(m_colorNumber));  }
    if (inputPort(13)->status() == Status::Active) { painter->drawPixmap(0, 0, m.at(m_colorNumber));  }
    if (inputPort(14)->status() == Status::Active) { painter->drawPixmap(0, 0, n.at(m_colorNumber));  }
}

void Display14::setColor(const QString &color)
{
    m_color = color;

    if (color == "White")  { m_colorNumber = 0; }
    if (color == "Red")    { m_colorNumber = 1; }
    if (color == "Green")  { m_colorNumber = 2; }
    if (color == "Blue")   { m_colorNumber = 3; }
    if (color == "Purple") { m_colorNumber = 4; }
}

QString Display14::color() const
{
    return m_color;
}

void Display14::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("color", color());

    stream << map;
}

void Display14::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    GraphicElement::load(stream, portMap, version);

    if ((VERSION("3.1") <= version) && (version < VERSION("4.1"))) {
        QString color_; stream >> color_;
        setColor(color_);
    }

    if (version >= VERSION("4.1")) {
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("color")) {
            setColor(map.value("color").toString());
        }
    }
}
