// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display_14.h"
#include "display.h"

#include "qneport.h"

#include <QPainter>
#include <QPixmap>

int Display14::current_id_number = 0;

Display14::Display14(QGraphicsItem *parent)
    : GraphicElement(ElementType::DISPLAY14, ElementGroup::OUTPUT, 15, 15, 0, 0, parent)
{
    m_pixmapSkinName = {
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

    setRotatable(false);
    setHasColors(true);
    setCanChangeSkin(true);
    setOutputsOnTop(true);
    Display14::updatePorts();
    setBottomPosition(58);
    setTopPosition(6);
    setHasLabel(true);
    m_color = "Red";
    m_color_number = 1;

    setPixmap(m_pixmapSkinName[0]);
    a = QVector<QPixmap>(5, m_pixmapSkinName[1]);
    b = QVector<QPixmap>(5, m_pixmapSkinName[2]);
    c = QVector<QPixmap>(5, m_pixmapSkinName[3]);
    d = QVector<QPixmap>(5, m_pixmapSkinName[4]);
    e = QVector<QPixmap>(5, m_pixmapSkinName[5]);
    f = QVector<QPixmap>(5, m_pixmapSkinName[6]);
    g1 = QVector<QPixmap>(5, m_pixmapSkinName[7]);
    g2 = QVector<QPixmap>(5, m_pixmapSkinName[8]);
    h = QVector<QPixmap>(5, m_pixmapSkinName[9]);
    j = QVector<QPixmap>(5, m_pixmapSkinName[10]);
    k = QVector<QPixmap>(5, m_pixmapSkinName[11]);
    l = QVector<QPixmap>(5, m_pixmapSkinName[12]);
    m = QVector<QPixmap>(5, m_pixmapSkinName[13]);
    n = QVector<QPixmap>(5, m_pixmapSkinName[14]);
    dp = QVector<QPixmap>(5, m_pixmapSkinName[15]);

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

    setPortName("Display14");
    for (QNEPort *in : qAsConst(m_inputs)) {
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
    input(0)->setPos(topPosition(), -4); /* G1 */
    input(1)->setPos(topPosition(), 8); /* F */
    input(2)->setPos(topPosition(), 20); /* E */
    input(3)->setPos(topPosition(), 32); /* D */
    input(4)->setPos(bottomPosition(), -10); /* A */
    input(5)->setPos(bottomPosition(), 2); /* B */
    input(6)->setPos(bottomPosition(), 14); /* DP */
    input(7)->setPos(bottomPosition(), 26); /* C */
    input(8)->setPos(topPosition(), 44); /* G2 */
    input(9)->setPos(topPosition(), 56); /* H */
    input(10)->setPos(topPosition(), 68); /* J */
    input(11)->setPos(bottomPosition(), 38); /* K */
    input(12)->setPos(bottomPosition(), 50); /* L */
    input(13)->setPos(bottomPosition(), 62); /* M */
    input(14)->setPos(bottomPosition(), 74); /* N */
    input(0)->setName("G1 (mid left)");
    input(1)->setName("F (upper left)");
    input(2)->setName("E (lower left)");
    input(3)->setName("D (bottom)");
    input(4)->setName("A (top)");
    input(5)->setName("B (upper right)");
    input(6)->setName("DP (dot)");
    input(7)->setName("C (lower right)");
    input(8)->setName("G2 (mid right)");
    input(9)->setName("H (mid upper left)");
    input(10)->setName("J (mid top)");
    input(11)->setName("K (mid upper right)");
    input(12)->setName("L (mid lower right)");
    input(13)->setName("M (mid bottom)");
    input(14)->setName("N (mid lower left)");
}

void Display14::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicElement::paint(painter, option, widget);
    if (input(0)->value() == 1) { /* G1 */
        painter->drawPixmap(QPoint(0, 0), g1[m_color_number]);
    }
    if (input(1)->value() == 1) { /* F */
        painter->drawPixmap(QPoint(0, 0), f[m_color_number]);
    }
    if (input(2)->value() == 1) { /* E */
        painter->drawPixmap(QPoint(0, 0), e[m_color_number]);
    }
    if (input(3)->value() == 1) { /* D */
        painter->drawPixmap(QPoint(0, 0), d[m_color_number]);
    }
    if (input(4)->value() == 1) { /* A */
        painter->drawPixmap(QPoint(0, 0), a[m_color_number]);
    }
    if (input(5)->value() == 1) { /* B */
        painter->drawPixmap(QPoint(0, 0), b[m_color_number]);
    }
    if (input(6)->value() == 1) { /* DP */
        painter->drawPixmap(QPoint(0, 0), dp[m_color_number]);
    }
    if (input(7)->value() == 1) { /* C */
        painter->drawPixmap(QPoint(0, 0), c[m_color_number]);
    }
    if (input(8)->value() == 1) { /* G2 */
        painter->drawPixmap(QPoint(0, 0), g2[m_color_number]);
    }
    if (input(9)->value() == 1) { /* H */
        painter->drawPixmap(QPoint(0, 0), h[m_color_number]);
    }
    if (input(10)->value() == 1) { /* J */
        painter->drawPixmap(QPoint(0, 0), j[m_color_number]);
    }
    if (input(11)->value() == 1) { /* K */
        painter->drawPixmap(QPoint(0, 0), k[m_color_number]);
    }
    if (input(12)->value() == 1) { /* L */
        painter->drawPixmap(QPoint(0, 0), l[m_color_number]);
    }
    if (input(13)->value() == 1) { /* M */
        painter->drawPixmap(QPoint(0, 0), m[m_color_number]);
    }
    if (input(14)->value() == 1) { /* N */
        painter->drawPixmap(QPoint(0, 0), n[m_color_number]);
    }
}

void Display14::setColor(const QString &color)
{
    m_color = color;
    if (color == "White") {
        m_color_number = 0;
    } else if (color == "Red") {
        m_color_number = 1;
    } else if (color == "Green") {
        m_color_number = 2;
    } else if (color == "Blue") {
        m_color_number = 3;
    } else if (color == "Purple") {
        m_color_number = 4;
    }
    refresh();
}

QString Display14::getColor() const
{
    return m_color;
}

void Display14::save(QDataStream &ds) const
{
    GraphicElement::save(ds);
    ds << getColor();
}

void Display14::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version)
{
    GraphicElement::load(ds, portMap, version);
    if (version >= 3.1) {
      QString clr;
      ds >> clr;
      setColor(clr);
    }
}

void Display14::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/output/counter/counter_14_off.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
