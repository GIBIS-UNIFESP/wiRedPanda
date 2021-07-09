// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"
#include "display.h"
#include "qneport.h"

#include <QPainter>
#include <QPixmap>

int Display::current_id_number = 0;

Display::Display(QGraphicsItem *parent)
    : GraphicElement(ElementType::DISPLAY, ElementGroup::OUTPUT, 8, 8, 0, 0, parent)
{
    m_pixmapSkinName = {
        ":/output/counter/counter_off.png",
        ":/output/counter/counter_a.png",
        ":/output/counter/counter_b.png",
        ":/output/counter/counter_c.png",
        ":/output/counter/counter_d.png",
        ":/output/counter/counter_e.png",
        ":/output/counter/counter_f.png",
        ":/output/counter/counter_g.png",
        ":/output/counter/counter_dp.png",
    };

    setRotatable(false);
    setHasColors(true);
    setCanChangeSkin(true);
    setOutputsOnTop(true);
    Display::updatePorts();
    setBottomPosition(58);
    setTopPosition(6);
    setHasLabel(true);
    m_color = "Red";
    m_color_number = 1;

    COMMENT("Allocating pixmaps.", 3);
    setPixmap(m_pixmapSkinName[0]);
    a = QVector<QPixmap>(5, m_pixmapSkinName[1]);
    b = QVector<QPixmap>(5, m_pixmapSkinName[2]);
    c = QVector<QPixmap>(5, m_pixmapSkinName[3]);
    d = QVector<QPixmap>(5, m_pixmapSkinName[4]);
    e = QVector<QPixmap>(5, m_pixmapSkinName[5]);
    f = QVector<QPixmap>(5, m_pixmapSkinName[6]);
    g = QVector<QPixmap>(5, m_pixmapSkinName[7]);
    dp = QVector<QPixmap>(5, m_pixmapSkinName[8]);

    COMMENT("Converting segments to other colors.", 3);
    convertAllColors(a);
    convertAllColors(b);
    convertAllColors(c);
    convertAllColors(d);
    convertAllColors(e);
    convertAllColors(f);
    convertAllColors(g);
    convertAllColors(dp);

    setPortName("Display");
    for (QNEPort *in : qAsConst(m_inputs)) {
        in->setRequired(false);
        in->setDefaultValue(0);
    }
}

void Display::convertAllColors(QVector<QPixmap> &maps)
{
  QImage tmp(maps[1].toImage());
  maps[0] = convertColor(tmp,true,true,true);
  maps[1] = convertColor(tmp,true,false,false);
  maps[2] = convertColor(tmp,false,true,false);
  maps[3] = convertColor(tmp,false,false,true);
  maps[4] = convertColor(tmp,true,false,true);
}

QPixmap Display::convertColor(const QImage &src, bool red, bool green, bool blue)
{
    QImage tgt(src);
    for(int y = 0; y < src.height(); y++) {
        const uchar *src_line = src.scanLine(y);
        uchar *tgt_line = tgt.scanLine(y);
        for(int x= 0; x < src.width(); x++) {
            src_line += 2;
            uchar src_red = *src_line;
            src_line += 2;
            *tgt_line++ = blue ? src_red : 0;
            *tgt_line++ = green ? src_red : 0;
            *tgt_line++ = red ? src_red : 0;
            tgt_line++;
        }
    }
    return(QPixmap::fromImage(tgt));
}

void Display::refresh()
{
    update();
}

void Display::updatePorts()
{
    input(0)->setPos(topPosition(), 10); /* G */
    input(1)->setPos(topPosition(), 25); /* F */
    input(2)->setPos(topPosition(), 39); /* E */
    input(3)->setPos(topPosition(), 54); /* D */
    input(4)->setPos(bottomPosition(), 10); /* A */
    input(5)->setPos(bottomPosition(), 25); /* B */
    input(6)->setPos(bottomPosition(), 39); /* DP */
    input(7)->setPos(bottomPosition(), 54); /* C */
    input(0)->setName("G (mid)");
    input(1)->setName("F (upper left)");
    input(2)->setName("E (lower left)");
    input(3)->setName("D (bottom)");
    input(4)->setName("A (top)");
    input(5)->setName("B (upper right)");
    input(6)->setName("DP (dot)");
    input(7)->setName("C (lower right)");
}

void Display::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicElement::paint(painter, option, widget);
    if (input(0)->value() == true) { /* G */
        painter->drawPixmap(QPoint(0, 0), g[m_color_number]);
    }
    if (input(1)->value() == true) { /* F */
        painter->drawPixmap(QPoint(0, 0), f[m_color_number]);
    }
    if (input(2)->value() == true) { /* E */
        painter->drawPixmap(QPoint(0, 0), e[m_color_number]);
    }
    if (input(3)->value() == true) { /* D */
        painter->drawPixmap(QPoint(0, 0), d[m_color_number]);
    }
    if (input(4)->value() == true) { /* A */
        painter->drawPixmap(QPoint(0, 0), a[m_color_number]);
    }
    if (input(5)->value() == true) { /* B */
        painter->drawPixmap(QPoint(0, 0), b[m_color_number]);
    }
    if (input(6)->value() == true) { /* DP */
        painter->drawPixmap(QPoint(0, 0), dp[m_color_number]);
    }
    if (input(7)->value() == true) { /* C */
        painter->drawPixmap(QPoint(0, 0), c[m_color_number]);
    }
}

void Display::setColor(const QString &color)
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

QString Display::getColor() const
{
    return m_color;
}

void Display::save(QDataStream &ds) const
{
    GraphicElement::save(ds);
    ds << getColor();
}

void Display::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version)
{
    GraphicElement::load(ds, portMap, version);
    //  qDebug( ) << "Version: " << version;
    /*
     * 0,7,2,1,3,4,5,6
     * 7,5,4,2,1,4,6,3,0
     * 4,5,7,3,2,1,0,6
     * 2,1,4,5,0,7,3,6
     */
    if (version < 1.6) {
        COMMENT("Remapping inputs", 0);
        QVector<int> order = {2, 1, 4, 5, 0, 7, 3, 6};
        QVector<QNEInputPort *> aux = inputs();
        for (int i = 0; i < aux.size(); ++i) {
            aux[order[i]] = m_inputs[i];
        }
        setInputs(aux);
        updatePorts();
    }
    if (version < 1.7) {
        COMMENT("Remapping inputs", 0);
        QVector<int> order = {2, 5, 4, 0, 7, 3, 6, 1};
        QVector<QNEInputPort *> aux = inputs();
        for (int i = 0; i < aux.size(); ++i) {
            aux[order[i]] = m_inputs[i];
        }
        setInputs(aux);
        updatePorts();
    }
    if (version >= 3.1) {
      QString clr;
      ds >> clr;
      setColor(clr);
    }
}

void Display::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/output/counter/counter_off.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
