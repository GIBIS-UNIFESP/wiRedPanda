// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display.h"

#include <QDebug>
#include <QPainter>
#include <QPixmap>

int Display::current_id_number = 0;

Display::Display(QGraphicsItem *parent)
    : GraphicElement(ElementType::DISPLAY, ElementGroup::OUTPUT, 8, 8, 0, 0, parent)
{
    pixmapSkinName.append(":/output/counter/counter_off.png");
    pixmapSkinName.append(":/output/counter/counter_a.png");
    pixmapSkinName.append(":/output/counter/counter_b.png");
    pixmapSkinName.append(":/output/counter/counter_c.png");
    pixmapSkinName.append(":/output/counter/counter_d.png");
    pixmapSkinName.append(":/output/counter/counter_e.png");
    pixmapSkinName.append(":/output/counter/counter_f.png");
    pixmapSkinName.append(":/output/counter/counter_g.png");
    pixmapSkinName.append(":/output/counter/counter_dp.png");
    setRotatable(false);
    setCanChangeSkin(true);
    setOutputsOnTop(true);
    updatePorts();
    setBottomPosition(58);
    setTopPosition(6);
    setHasLabel(true);

    setPixmap(pixmapSkinName[0]);
    a = QPixmap(pixmapSkinName[1]);
    b = QPixmap(pixmapSkinName[2]);
    c = QPixmap(pixmapSkinName[3]);
    d = QPixmap(pixmapSkinName[4]);
    e = QPixmap(pixmapSkinName[5]);
    f = QPixmap(pixmapSkinName[6]);
    g = QPixmap(pixmapSkinName[7]);
    dp = QPixmap(pixmapSkinName[8]);

    setPortName("Display");
    for (QNEPort *in : qAsConst(m_inputs)) {
        in->setRequired(false);
        in->setDefaultValue(0);
    }
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
        painter->drawPixmap(QPoint(0, 0), g);
    }
    if (input(1)->value() == true) { /* F */
        painter->drawPixmap(QPoint(0, 0), f);
    }
    if (input(2)->value() == true) { /* E */
        painter->drawPixmap(QPoint(0, 0), e);
    }
    if (input(3)->value() == true) { /* D */
        painter->drawPixmap(QPoint(0, 0), d);
    }
    if (input(4)->value() == true) { /* A */
        painter->drawPixmap(QPoint(0, 0), a);
    }
    if (input(5)->value() == true) { /* B */
        painter->drawPixmap(QPoint(0, 0), b);
    }
    if (input(6)->value() == true) { /* DP */
        painter->drawPixmap(QPoint(0, 0), dp);
    }
    if (input(7)->value() == true) { /* C */
        painter->drawPixmap(QPoint(0, 0), c);
    }
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
}

void Display::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        pixmapSkinName[0] = ":/output/counter/counter_off.png";
    } else {
        pixmapSkinName[0] = filename;
    }
    setPixmap(pixmapSkinName[0]);
}
