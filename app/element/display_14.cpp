#include "display_14.h"
#include "qneconnection.h"

#include <QDebug>
#include <QPainter>
#include <QPixmap>

int Display14::current_id_number = 0;

Display14::Display14(QGraphicsItem *parent)
    : GraphicElement(15, 15, 0, 0, parent)
{
    pixmapSkinName.append(":/output/counter/counter_14_off.png");
    pixmapSkinName.append(":/output/counter/counter_a.png");
    pixmapSkinName.append(":/output/counter/counter_b.png");
    pixmapSkinName.append(":/output/counter/counter_c.png");
    pixmapSkinName.append(":/output/counter/counter_d.png");
    pixmapSkinName.append(":/output/counter/counter_e.png");
    pixmapSkinName.append(":/output/counter/counter_f.png");
    pixmapSkinName.append(":/output/counter/counter_g1.png");
    pixmapSkinName.append(":/output/counter/counter_g2.png");
    pixmapSkinName.append(":/output/counter/counter_h.png");
    pixmapSkinName.append(":/output/counter/counter_j.png");
    pixmapSkinName.append(":/output/counter/counter_k.png");
    pixmapSkinName.append(":/output/counter/counter_l.png");
    pixmapSkinName.append(":/output/counter/counter_m.png");
    pixmapSkinName.append(":/output/counter/counter_n.png");
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
    g1 = QPixmap(pixmapSkinName[7]);
    g2 = QPixmap(pixmapSkinName[8]);
    h = QPixmap(pixmapSkinName[9]);
    j = QPixmap(pixmapSkinName[10]);
    k = QPixmap(pixmapSkinName[11]);
    l = QPixmap(pixmapSkinName[12]);
    m = QPixmap(pixmapSkinName[13]);
    n = QPixmap(pixmapSkinName[14]);
    dp = QPixmap(pixmapSkinName[15]);

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
    if (input(0)->value() == true) { /* G1 */
        painter->drawPixmap(QPoint(0, 0), g1);
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
    if (input(8)->value() == true) { /* G2 */
        painter->drawPixmap(QPoint(0, 0), g2);
    }
    if (input(9)->value() == true) { /* H */
        painter->drawPixmap(QPoint(0, 0), h);
    }
    if (input(10)->value() == true) { /* J */
        painter->drawPixmap(QPoint(0, 0), j);
    }
    if (input(11)->value() == true) { /* K */
        painter->drawPixmap(QPoint(0, 0), k);
    }
    if (input(12)->value() == true) { /* L */
        painter->drawPixmap(QPoint(0, 0), l);
    }
    if (input(13)->value() == true) { /* M */
        painter->drawPixmap(QPoint(0, 0), m);
    }
    if (input(14)->value() == true) { /* N */
        painter->drawPixmap(QPoint(0, 0), n);
    }
}

void Display14::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version)
{
    GraphicElement::load(ds, portMap, version);
}

void Display14::setSkin(bool defaultSkin, QString filename)
{
    if (defaultSkin)
        pixmapSkinName[0] = ":/output/counter/counter_14_off.png";
    else
        pixmapSkinName[0] = filename;
    setPixmap(pixmapSkinName[0]);
}
