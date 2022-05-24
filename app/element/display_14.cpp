// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display_14.h"

#include "common.h"
#include "display.h"
#include "qneport.h"

#include <QPainter>
#include <QPixmap>

namespace
{
int id = qRegisterMetaType<Display14>();
}

Display14::Display14(QGraphicsItem *parent)
    : GraphicElement(ElementType::Display14, ElementGroup::Output, 15, 15, 0, 0, parent)
{
    m_defaultSkins = QStringList{
        ":/output/counter/counter_14_off.png",
        ":/output/counter/counter_a.png",
        ":/output/counter/counter_b.png",
        ":/output/counter/counter_c.png",
        ":/output/counter/counter_d.png",
        ":/output/counter/counter_e.png",
        ":/output/counter/counter_f.png",
        ":/output/counter/counter_g1.png",
        ":/output/counter/counter_g2.png",
        ":/output/counter/counter_h.png",
        ":/output/counter/counter_j.png",
        ":/output/counter/counter_k.png",
        ":/output/counter/counter_l.png",
        ":/output/counter/counter_m.png",
        ":/output/counter/counter_n.png",
        ":/output/counter/counter_dp.png",
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(m_defaultSkins.first());

    a = QVector<QPixmap>(5, m_defaultSkins[1]);
    b = QVector<QPixmap>(5, m_defaultSkins[2]);
    c = QVector<QPixmap>(5, m_defaultSkins[3]);
    d = QVector<QPixmap>(5, m_defaultSkins[4]);
    e = QVector<QPixmap>(5, m_defaultSkins[5]);
    f = QVector<QPixmap>(5, m_defaultSkins[6]);
    g1 = QVector<QPixmap>(5, m_defaultSkins[7]);
    g2 = QVector<QPixmap>(5, m_defaultSkins[8]);
    h = QVector<QPixmap>(5, m_defaultSkins[9]);
    j = QVector<QPixmap>(5, m_defaultSkins[10]);
    k = QVector<QPixmap>(5, m_defaultSkins[11]);
    l = QVector<QPixmap>(5, m_defaultSkins[12]);
    m = QVector<QPixmap>(5, m_defaultSkins[13]);
    n = QVector<QPixmap>(5, m_defaultSkins[14]);
    dp = QVector<QPixmap>(5, m_defaultSkins[15]);

    Display::convertAllColors(a);
    Display::convertAllColors(b);
    Display::convertAllColors(c);
    Display::convertAllColors(d);
    Display::convertAllColors(e);
    Display::convertAllColors(f);
    Display::convertAllColors(g1);
    Display::convertAllColors(g2);
    Display::convertAllColors(h);
    Display::convertAllColors(j);
    Display::convertAllColors(k);
    Display::convertAllColors(l);
    Display::convertAllColors(m);
    Display::convertAllColors(n);
    Display::convertAllColors(dp);

    setRotatable(false);
    setHasColors(true);
    setCanChangeSkin(true);
    Display14::updatePorts();
    setRightPosition(58);
    setLeftPosition(6);
    setHasLabel(true);
    setPortName("Display14");
    setToolTip(m_translatedName);

    for (auto *in : qAsConst(m_inputs)) {
        in->setRequired(false);
        in->setDefaultValue(0);
    }
}

void Display14::refresh()
{
    update();
}

void Display14::updatePorts()
{
    input(0)->setPos(leftPosition(), -4);   /* G1 */
    input(1)->setPos(leftPosition(), 8);    /* F  */
    input(2)->setPos(leftPosition(), 20);   /* E  */
    input(3)->setPos(leftPosition(), 32);   /* D  */
    input(4)->setPos(rightPosition(), -10); /* A  */
    input(5)->setPos(rightPosition(), 2);   /* B  */
    input(6)->setPos(rightPosition(), 14);  /* DP */
    input(7)->setPos(rightPosition(), 26);  /* C  */
    input(8)->setPos(leftPosition(), 44);   /* G2 */
    input(9)->setPos(leftPosition(), 56);   /* H  */
    input(10)->setPos(leftPosition(), 68);  /* J  */
    input(11)->setPos(rightPosition(), 38); /* K  */
    input(12)->setPos(rightPosition(), 50); /* L  */
    input(13)->setPos(rightPosition(), 62); /* M  */
    input(14)->setPos(rightPosition(), 74); /* N  */

    input(0)->setName("G1 (" + tr("middle left") + ")");
    input(1)->setName("F (" +  tr("upper left") + ")");
    input(2)->setName("E (" +  tr("lower left") + ")");
    input(3)->setName("D (" +  tr("bottom") + ")");
    input(4)->setName("A (" +  tr("top") + ")");
    input(5)->setName("B (" +  tr("upper right") + ")");
    input(6)->setName("DP (" + tr("dot") + ")");
    input(7)->setName("C (" +  tr("lower right") + ")");
    input(8)->setName("G2 (" + tr("middle right") + ")");
    input(9)->setName("H (" +  tr("middle upper left") + ")");
    input(10)->setName("J (" + tr("middle top") + ")");
    input(11)->setName("K (" + tr("middle upper right") + ")");
    input(12)->setName("L (" + tr("middle lower right") + ")");
    input(13)->setName("M (" + tr("middle bottom") + ")");
    input(14)->setName("N (" + tr("middle lower left") + ")");
}

void Display14::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicElement::paint(painter, option, widget);
    if (input(0)->value() == 1)  { painter->drawPixmap(0, 0, g1[m_colorNumber]); }
    if (input(1)->value() == 1)  { painter->drawPixmap(0, 0, f[m_colorNumber]);  }
    if (input(2)->value() == 1)  { painter->drawPixmap(0, 0, e[m_colorNumber]);  }
    if (input(3)->value() == 1)  { painter->drawPixmap(0, 0, d[m_colorNumber]);  }
    if (input(4)->value() == 1)  { painter->drawPixmap(0, 0, a[m_colorNumber]);  }
    if (input(5)->value() == 1)  { painter->drawPixmap(0, 0, b[m_colorNumber]);  }
    if (input(6)->value() == 1)  { painter->drawPixmap(0, 0, dp[m_colorNumber]); }
    if (input(7)->value() == 1)  { painter->drawPixmap(0, 0, c[m_colorNumber]);  }
    if (input(8)->value() == 1)  { painter->drawPixmap(0, 0, g2[m_colorNumber]); }
    if (input(9)->value() == 1)  { painter->drawPixmap(0, 0, h[m_colorNumber]);  }
    if (input(10)->value() == 1) { painter->drawPixmap(0, 0, j[m_colorNumber]);  }
    if (input(11)->value() == 1) { painter->drawPixmap(0, 0, k[m_colorNumber]);  }
    if (input(12)->value() == 1) { painter->drawPixmap(0, 0, l[m_colorNumber]);  }
    if (input(13)->value() == 1) { painter->drawPixmap(0, 0, m[m_colorNumber]);  }
    if (input(14)->value() == 1) { painter->drawPixmap(0, 0, n[m_colorNumber]);  }
}

void Display14::setColor(const QString &color)
{
    m_color = color;
    if (color == "White")  { m_colorNumber = 0; }
    if (color == "Red")    { m_colorNumber = 1; }
    if (color == "Green")  { m_colorNumber = 2; }
    if (color == "Blue")   { m_colorNumber = 3; }
    if (color == "Purple") { m_colorNumber = 4; }
    refresh();
}

QString Display14::color() const
{
    return m_color;
}

void Display14::save(QDataStream &stream) const
{
    GraphicElement::save(stream);
    stream << color();
}

void Display14::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version)
{
    GraphicElement::load(stream, portMap, version);
    if (version >= 3.1) {
        QString color;
        stream >> color;
        setColor(color);
    }
}
