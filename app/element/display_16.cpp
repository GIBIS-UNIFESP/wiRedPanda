// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display_16.h"

#include "display_7.h"
#include "globalproperties.h"
#include "qneport.h"

#include <QPainter>

Display16::Display16(QGraphicsItem *parent)
    : GraphicElement(ElementType::Display16, ElementGroup::Output, ":/output/counter/counter_16_on.svg", tr("16-SEGMENT DISPLAY"), tr("16-Segment Display"), 17, 17, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins = QStringList{
        ":/output/counter/counter_16_off.svg",
        ":/output/counter/counter_a1.svg",
        ":/output/counter/counter_a2.svg",
        ":/output/counter/counter_b.svg",
        ":/output/counter/counter_c.svg",
        ":/output/counter/counter_d1.svg",
        ":/output/counter/counter_d2.svg",
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

    a1 = QVector<QPixmap>(5, m_defaultSkins.at(1));
    a2 = QVector<QPixmap>(5, m_defaultSkins.at(2));
    b  = QVector<QPixmap>(5, m_defaultSkins.at(3));
    c  = QVector<QPixmap>(5, m_defaultSkins.at(4));
    d1 = QVector<QPixmap>(5, m_defaultSkins.at(5));
    d2 = QVector<QPixmap>(5, m_defaultSkins.at(6));
    e  = QVector<QPixmap>(5, m_defaultSkins.at(7));
    f  = QVector<QPixmap>(5, m_defaultSkins.at(8));
    g1 = QVector<QPixmap>(5, m_defaultSkins.at(9));
    g2 = QVector<QPixmap>(5, m_defaultSkins.at(10));
    h  = QVector<QPixmap>(5, m_defaultSkins.at(11));
    j  = QVector<QPixmap>(5, m_defaultSkins.at(12));
    k  = QVector<QPixmap>(5, m_defaultSkins.at(13));
    l  = QVector<QPixmap>(5, m_defaultSkins.at(14));
    m  = QVector<QPixmap>(5, m_defaultSkins.at(15));
    n  = QVector<QPixmap>(5, m_defaultSkins.at(16));
    dp = QVector<QPixmap>(5, m_defaultSkins.at(17));

    Display7::convertAllColors(a1);
    Display7::convertAllColors(a2);
    Display7::convertAllColors(b);
    Display7::convertAllColors(c);
    Display7::convertAllColors(d1);
    Display7::convertAllColors(d2);
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

    Display16::updatePortsProperties();
}

void Display16::refresh()
{
    update();
}

void Display16::updatePortsProperties()
{
    inputPort( 0)->setPos( 0,  -8);    inputPort( 0)->setName("G1 (" + tr("middle left")        + ")");
    inputPort( 1)->setPos( 0,   8);    inputPort( 1)->setName("F ("  + tr("upper left")         + ")");
    inputPort( 2)->setPos( 0,  24);    inputPort( 2)->setName("E ("  + tr("lower left")         + ")");
    inputPort( 3)->setPos( 0,  40);    inputPort( 3)->setName("D1 (" + tr("bottom left")        + ")");
    inputPort( 4)->setPos( 0,  56);    inputPort( 4)->setName("D2 (" + tr("bottom right")       + ")");
    inputPort( 5)->setPos(64,  -8);    inputPort( 5)->setName("A1 (" + tr("top left")           + ")");
    inputPort( 6)->setPos(64,   8);    inputPort( 6)->setName("A2 (" + tr("top right")          + ")");
    inputPort( 7)->setPos(64,  24);    inputPort( 7)->setName("B ("  + tr("upper right")        + ")");
    inputPort( 8)->setPos(64,  40);    inputPort( 8)->setName("DP (" + tr("dot")                + ")");
    inputPort( 9)->setPos(64,  56);    inputPort( 9)->setName("C ("  + tr("lower right")        + ")");
    inputPort(10)->setPos( 0,  72);    inputPort(10)->setName("G2 (" + tr("middle right")       + ")");
    inputPort(11)->setPos( 0,  88);    inputPort(11)->setName("H ("  + tr("middle upper left")  + ")");
    inputPort(12)->setPos( 0, 104);    inputPort(12)->setName("J ("  + tr("middle top")         + ")");
    inputPort(13)->setPos(64,  72);    inputPort(13)->setName("K ("  + tr("middle upper right") + ")");
    inputPort(14)->setPos(64,  88);    inputPort(14)->setName("L ("  + tr("middle lower right") + ")");
    inputPort(15)->setPos(64, 104);    inputPort(15)->setName("M ("  + tr("middle bottom")      + ")");
    inputPort(16)->setPos(64, 120);    inputPort(16)->setName("N ("  + tr("middle lower left")  + ")");

    for (auto *port : std::as_const(m_inputPorts)) {
        port->setRequired(false);
        port->setDefaultStatus(Status::Inactive);
    }
}

void Display16::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicElement::paint(painter, option, widget);

    if (inputPort( 0)->status() == Status::Active) { painter->drawPixmap(0, 0, g1.at(m_colorNumber)); }
    if (inputPort( 1)->status() == Status::Active) { painter->drawPixmap(0, 0, f.at(m_colorNumber));  }
    if (inputPort( 2)->status() == Status::Active) { painter->drawPixmap(0, 0, e.at(m_colorNumber));  }
    if (inputPort( 3)->status() == Status::Active) { painter->drawPixmap(0, 0, d1.at(m_colorNumber)); }
    if (inputPort( 4)->status() == Status::Active) { painter->drawPixmap(0, 0, d2.at(m_colorNumber)); }
    if (inputPort( 5)->status() == Status::Active) { painter->drawPixmap(0, 0, a1.at(m_colorNumber)); }
    if (inputPort( 6)->status() == Status::Active) { painter->drawPixmap(0, 0, a2.at(m_colorNumber)); }
    if (inputPort( 7)->status() == Status::Active) { painter->drawPixmap(0, 0, b.at(m_colorNumber));  }
    if (inputPort( 8)->status() == Status::Active) { painter->drawPixmap(0, 0, dp.at(m_colorNumber)); }
    if (inputPort( 9)->status() == Status::Active) { painter->drawPixmap(0, 0, c.at(m_colorNumber));  }
    if (inputPort(10)->status() == Status::Active) { painter->drawPixmap(0, 0, g2.at(m_colorNumber)); }
    if (inputPort(11)->status() == Status::Active) { painter->drawPixmap(0, 0, h.at(m_colorNumber));  }
    if (inputPort(12)->status() == Status::Active) { painter->drawPixmap(0, 0, j.at(m_colorNumber));  }
    if (inputPort(13)->status() == Status::Active) { painter->drawPixmap(0, 0, k.at(m_colorNumber));  }
    if (inputPort(14)->status() == Status::Active) { painter->drawPixmap(0, 0, l.at(m_colorNumber));  }
    if (inputPort(15)->status() == Status::Active) { painter->drawPixmap(0, 0, m.at(m_colorNumber));  }
    if (inputPort(16)->status() == Status::Active) { painter->drawPixmap(0, 0, n.at(m_colorNumber));  }
}

void Display16::setColor(const QString &color)
{
    m_color = color;

    if (color == "White")  { m_colorNumber = 0; }
    if (color == "Red")    { m_colorNumber = 1; }
    if (color == "Green")  { m_colorNumber = 2; }
    if (color == "Blue")   { m_colorNumber = 3; }
    if (color == "Purple") { m_colorNumber = 4; }
}

QString Display16::color() const
{
    return m_color;
}

void Display16::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("color", color());

    stream << map;
}

void Display16::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
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
