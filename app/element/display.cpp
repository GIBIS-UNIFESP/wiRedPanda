// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display.h"

#include "common.h"
#include "qneport.h"

#include <QPainter>
#include <QPixmap>

namespace
{
int id = qRegisterMetaType<Display>();
}

Display::Display(QGraphicsItem *parent)
    : GraphicElement(ElementType::Display, ElementGroup::Output, 8, 8, 0, 0, parent)
{
    m_defaultSkins = QStringList{
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
    m_alternativeSkins = m_defaultSkins;
    setPixmap(m_defaultSkins.first());

    qCDebug(three) << "Allocating pixmaps.";
    a =  QVector<QPixmap>(5, m_defaultSkins[1]);
    b =  QVector<QPixmap>(5, m_defaultSkins[2]);
    c =  QVector<QPixmap>(5, m_defaultSkins[3]);
    d =  QVector<QPixmap>(5, m_defaultSkins[4]);
    e =  QVector<QPixmap>(5, m_defaultSkins[5]);
    f =  QVector<QPixmap>(5, m_defaultSkins[6]);
    g =  QVector<QPixmap>(5, m_defaultSkins[7]);
    dp = QVector<QPixmap>(5, m_defaultSkins[8]);

    setRotatable(false);
    setHasColors(true);
    setCanChangeSkin(true);
    Display::updatePorts();
    setRightPosition(58);
    setLeftPosition(6);
    setHasLabel(true);

    qCDebug(three) << "Converting segments to other colors.";
    convertAllColors(a);
    convertAllColors(b);
    convertAllColors(c);
    convertAllColors(d);
    convertAllColors(e);
    convertAllColors(f);
    convertAllColors(g);
    convertAllColors(dp);

    setPortName("Display");
    setToolTip(m_translatedName);

    for (auto *in : qAsConst(m_inputs)) {
        in->setRequired(false);
        in->setDefaultValue(0);
    }
}

void Display::convertAllColors(QVector<QPixmap> &maps)
{
    QImage tmp(maps[1].toImage());
    maps[0] = convertColor(tmp, true, true, true);
    maps[1] = convertColor(tmp, true, false, false);
    maps[2] = convertColor(tmp, false, true, false);
    maps[3] = convertColor(tmp, false, false, true);
    maps[4] = convertColor(tmp, true, false, true);
}

QPixmap Display::convertColor(const QImage &src, const bool red, const bool green, const bool blue)
{
    QImage tgt(src);

    for (int y = 0; y < tgt.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(tgt.scanLine(y));
        for (int x = 0; x < tgt.width(); ++x) {
            QRgb &rgb = line[x];
            const int value = qRed(rgb);
            rgb = qRgba(red   ? value : 0,
                        green ? value : 0,
                        blue  ? value : 0,
                        value);
        }
    }

    return (QPixmap::fromImage(tgt));
}

void Display::refresh()
{
    update();
}

void Display::updatePorts()
{
    input(0)->setPos(leftPosition(), 10);  /* G  */
    input(1)->setPos(leftPosition(), 25);  /* F  */
    input(2)->setPos(leftPosition(), 39);  /* E  */
    input(3)->setPos(leftPosition(), 54);  /* D  */
    input(4)->setPos(rightPosition(), 10); /* A  */
    input(5)->setPos(rightPosition(), 25); /* B  */
    input(6)->setPos(rightPosition(), 39); /* DP */
    input(7)->setPos(rightPosition(), 54); /* C  */

    input(0)->setName("G (" +  tr("middle") + ")");
    input(1)->setName("F (" +  tr("upper left") + ")");
    input(2)->setName("E (" +  tr("lower left") + ")");
    input(3)->setName("D (" +  tr("bottom") + ")");
    input(4)->setName("A (" +  tr("top") + ")");
    input(5)->setName("B (" +  tr("upper right") + ")");
    input(6)->setName("DP (" + tr("dot") + ")");
    input(7)->setName("C (" +  tr("lower right") + ")");
}

void Display::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicElement::paint(painter, option, widget);
    if (input(0)->value() == 1) { painter->drawPixmap(0, 0, g[m_colorNumber]);  }
    if (input(1)->value() == 1) { painter->drawPixmap(0, 0, f[m_colorNumber]);  }
    if (input(2)->value() == 1) { painter->drawPixmap(0, 0, e[m_colorNumber]);  }
    if (input(3)->value() == 1) { painter->drawPixmap(0, 0, d[m_colorNumber]);  }
    if (input(4)->value() == 1) { painter->drawPixmap(0, 0, a[m_colorNumber]);  }
    if (input(5)->value() == 1) { painter->drawPixmap(0, 0, b[m_colorNumber]);  }
    if (input(6)->value() == 1) { painter->drawPixmap(0, 0, dp[m_colorNumber]); }
    if (input(7)->value() == 1) { painter->drawPixmap(0, 0, c[m_colorNumber]);  }
}

void Display::setColor(const QString &color)
{
    m_color = color;
    if (color == "White")  { m_colorNumber = 0; }
    if (color == "Red")    { m_colorNumber = 1; }
    if (color == "Green")  { m_colorNumber = 2; }
    if (color == "Blue")   { m_colorNumber = 3; }
    if (color == "Purple") { m_colorNumber = 4; }
    refresh();
}

QString Display::color() const
{
    return m_color;
}

void Display::save(QDataStream &stream) const
{
    GraphicElement::save(stream);
    stream << color();
}

void Display::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version)
{
    GraphicElement::load(stream, portMap, version);
    // qCDebug(zero) << "Version:" << version;
    /*
     * 0, 7, 2, 1, 3, 4, 5, 6
     * 7, 5, 4, 2, 1, 4, 6, 3, 0
     * 4, 5, 7, 3, 2, 1, 0, 6
     * 2, 1, 4, 5, 0, 7, 3, 6
     */
    if (version < 1.6) {
        qCDebug(zero) << "Remapping inputs.";
        QVector<int> order = {2, 1, 4, 5, 0, 7, 3, 6};
        QVector<QNEInputPort *> aux = inputs();
        for (int i = 0; i < aux.size(); ++i) {
            aux[order[i]] = m_inputs[i];
        }
        setInputs(aux);
        updatePorts();
    }
    if (version < 1.7) {
        qCDebug(zero) << "Remapping inputs.";
        QVector<int> order = {2, 5, 4, 0, 7, 3, 6, 1};
        QVector<QNEInputPort *> aux = inputs();
        for (int i = 0; i < aux.size(); ++i) {
            aux[order[i]] = m_inputs[i];
        }
        setInputs(aux);
        updatePorts();
    }
    if (version >= 3.1) {
        QString color;
        stream >> color;
        setColor(color);
    }
}
