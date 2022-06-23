// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display.h"

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
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins = QStringList{
        ":/output/counter/counter_off.svg",
        ":/output/counter/counter_a.svg",
        ":/output/counter/counter_b.svg",
        ":/output/counter/counter_c.svg",
        ":/output/counter/counter_d.svg",
        ":/output/counter/counter_e.svg",
        ":/output/counter/counter_f.svg",
        ":/output/counter/counter_g.svg",
        ":/output/counter/counter_dp.svg",
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    qCDebug(three) << tr("Allocating pixmaps.");
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
    setHasLabel(true);

    qCDebug(three) << tr("Converting segments to other colors.");
    convertAllColors(a);
    convertAllColors(b);
    convertAllColors(c);
    convertAllColors(d);
    convertAllColors(e);
    convertAllColors(f);
    convertAllColors(g);
    convertAllColors(dp);

    setPortName(m_translatedName);
    setToolTip(m_translatedName);

    for (auto *in : qAsConst(m_inputPorts)) {
        in->setRequired(false);
        in->setDefaultValue(Status::Inactive);
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
    inputPort(0)->setPos(6, 10);  /* G  */
    inputPort(1)->setPos(6, 25);  /* F  */
    inputPort(2)->setPos(6, 39);  /* E  */
    inputPort(3)->setPos(6, 54);  /* D  */
    inputPort(4)->setPos(58, 10); /* A  */
    inputPort(5)->setPos(58, 25); /* B  */
    inputPort(6)->setPos(58, 39); /* DP */
    inputPort(7)->setPos(58, 54); /* C  */

    inputPort(0)->setName("G (" +  tr("middle")      + ")");
    inputPort(1)->setName("F (" +  tr("upper left")  + ")");
    inputPort(2)->setName("E (" +  tr("lower left")  + ")");
    inputPort(3)->setName("D (" +  tr("bottom")      + ")");
    inputPort(4)->setName("A (" +  tr("top")         + ")");
    inputPort(5)->setName("B (" +  tr("upper right") + ")");
    inputPort(6)->setName("DP (" + tr("dot")         + ")");
    inputPort(7)->setName("C (" +  tr("lower right") + ")");
}

void Display::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicElement::paint(painter, option, widget);
    if (inputPort(0)->value() == Status::Active) { painter->drawPixmap(0, 0, g[m_colorNumber]);  }
    if (inputPort(1)->value() == Status::Active) { painter->drawPixmap(0, 0, f[m_colorNumber]);  }
    if (inputPort(2)->value() == Status::Active) { painter->drawPixmap(0, 0, e[m_colorNumber]);  }
    if (inputPort(3)->value() == Status::Active) { painter->drawPixmap(0, 0, d[m_colorNumber]);  }
    if (inputPort(4)->value() == Status::Active) { painter->drawPixmap(0, 0, a[m_colorNumber]);  }
    if (inputPort(5)->value() == Status::Active) { painter->drawPixmap(0, 0, b[m_colorNumber]);  }
    if (inputPort(6)->value() == Status::Active) { painter->drawPixmap(0, 0, dp[m_colorNumber]); }
    if (inputPort(7)->value() == Status::Active) { painter->drawPixmap(0, 0, c[m_colorNumber]);  }
}

void Display::setColor(const QString &color)
{
    m_color = color;
    if (color == "White")  { m_colorNumber = 0; }
    if (color == "Red")    { m_colorNumber = 1; }
    if (color == "Green")  { m_colorNumber = 2; }
    if (color == "Blue")   { m_colorNumber = 3; }
    if (color == "Purple") { m_colorNumber = 4; }
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
    /*
     * 0, 7, 2, 1, 3, 4, 5, 6
     * 7, 5, 4, 2, 1, 4, 6, 3, 0
     * 4, 5, 7, 3, 2, 1, 0, 6
     * 2, 1, 4, 5, 0, 7, 3, 6
     */
    if (version < 1.6) {
        qCDebug(zero) << tr("Remapping inputs.");
        QVector<int> order = {2, 1, 4, 5, 0, 7, 3, 6};
        QVector<QNEInputPort *> aux = inputs();
        for (int i = 0; i < aux.size(); ++i) {
            aux[order[i]] = m_inputPorts[i];
        }
        setInputs(aux);
        updatePorts();
    }
    if (version < 1.7) {
        qCDebug(zero) << tr("Remapping inputs.");
        QVector<int> order = {2, 5, 4, 0, 7, 3, 6, 1};
        QVector<QNEInputPort *> aux = inputs();
        for (int i = 0; i < aux.size(); ++i) {
            aux[order[i]] = m_inputPorts[i];
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
