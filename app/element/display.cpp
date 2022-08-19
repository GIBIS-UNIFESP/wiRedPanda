// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display.h"

#include "common.h"
#include "globalproperties.h"
#include "qneport.h"

#include <QPainter>
#include <QPixmap>

namespace
{
int id = qRegisterMetaType<Display>();
}

Display::Display(QGraphicsItem *parent)
    : GraphicElement(ElementType::Display, ElementGroup::Output, ":/output/counter/counter_on.svg", tr("7-SEGMENT DISPLAY"), tr("Display"), 8, 8, 0, 0, parent)
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
    a =  QVector<QPixmap>(5, m_defaultSkins.at(1));
    b =  QVector<QPixmap>(5, m_defaultSkins.at(2));
    c =  QVector<QPixmap>(5, m_defaultSkins.at(3));
    d =  QVector<QPixmap>(5, m_defaultSkins.at(4));
    e =  QVector<QPixmap>(5, m_defaultSkins.at(5));
    f =  QVector<QPixmap>(5, m_defaultSkins.at(6));
    g =  QVector<QPixmap>(5, m_defaultSkins.at(7));
    dp = QVector<QPixmap>(5, m_defaultSkins.at(8));

    qCDebug(three) << tr("Converting segments to other colors.");
    convertAllColors(a);
    convertAllColors(b);
    convertAllColors(c);
    convertAllColors(d);
    convertAllColors(e);
    convertAllColors(f);
    convertAllColors(g);
    convertAllColors(dp);

    setCanChangeSkin(true);
    setHasColors(true);
    setHasLabel(true);
    setPortName(m_translatedName);
    setRotatable(false);
    setToolTip(m_translatedName);

    Display::updatePortsProperties();
}

void Display::convertAllColors(QVector<QPixmap> &pixmaps)
{
    QImage tmp(pixmaps.at(0).toImage());
    pixmaps[0] = convertColor(tmp, true, true, true);
    pixmaps[1] = convertColor(tmp, true, false, false);
    pixmaps[2] = convertColor(tmp, false, true, false);
    pixmaps[3] = convertColor(tmp, false, false, true);
    pixmaps[4] = convertColor(tmp, true, false, true);
}

QPixmap Display::convertColor(const QImage &source, const bool red, const bool green, const bool blue)
{
    QImage target(source);

    for (int y = 0; y < target.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(target.scanLine(y));
        for (int x = 0; x < target.width(); ++x) {
            QRgb &rgb = line[x];
            const int value = qRed(rgb);
            rgb = qRgba(red   ? value : 0,
                        green ? value : 0,
                        blue  ? value : 0,
                        value);
        }
    }

    return QPixmap::fromImage(target);
}

void Display::refresh()
{
    update();
}

void Display::updatePortsProperties()
{
    inputPort(0)->setPos( 0,  8);    inputPort(0)->setName("G (" +  tr("middle")      + ")");
    inputPort(1)->setPos( 0, 24);    inputPort(1)->setName("F (" +  tr("upper left")  + ")");
    inputPort(2)->setPos( 0, 40);    inputPort(2)->setName("E (" +  tr("lower left")  + ")");
    inputPort(3)->setPos( 0, 56);    inputPort(3)->setName("D (" +  tr("bottom")      + ")");
    inputPort(4)->setPos(64,  8);    inputPort(4)->setName("A (" +  tr("top")         + ")");
    inputPort(5)->setPos(64, 24);    inputPort(5)->setName("B (" +  tr("upper right") + ")");
    inputPort(6)->setPos(64, 40);    inputPort(6)->setName("DP (" + tr("dot")         + ")");
    inputPort(7)->setPos(64, 56);    inputPort(7)->setName("C (" +  tr("lower right") + ")");

    for (auto *in : qAsConst(m_inputPorts)) {
        in->setRequired(false);
        in->setDefaultStatus(Status::Inactive);
    }
}

void Display::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicElement::paint(painter, option, widget);

    if (inputPort(0)->status() == Status::Active) { painter->drawPixmap(0, 0, g.at(m_colorNumber));  }
    if (inputPort(1)->status() == Status::Active) { painter->drawPixmap(0, 0, f.at(m_colorNumber));  }
    if (inputPort(2)->status() == Status::Active) { painter->drawPixmap(0, 0, e.at(m_colorNumber));  }
    if (inputPort(3)->status() == Status::Active) { painter->drawPixmap(0, 0, d.at(m_colorNumber));  }
    if (inputPort(4)->status() == Status::Active) { painter->drawPixmap(0, 0, a.at(m_colorNumber));  }
    if (inputPort(5)->status() == Status::Active) { painter->drawPixmap(0, 0, b.at(m_colorNumber));  }
    if (inputPort(6)->status() == Status::Active) { painter->drawPixmap(0, 0, dp.at(m_colorNumber)); }
    if (inputPort(7)->status() == Status::Active) { painter->drawPixmap(0, 0, c.at(m_colorNumber));  }
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
        QVector<int> order{2, 1, 4, 5, 0, 7, 3, 6};
        QVector<QNEInputPort *> aux = inputs();
        for (int i = 0; i < aux.size(); ++i) {
            aux[order[i]] = m_inputPorts.value(i);
        }
        setInputs(aux);
        updatePortsProperties();
    }
    if (version < 1.7) {
        qCDebug(zero) << tr("Remapping inputs.");
        QVector<int> order{2, 5, 4, 0, 7, 3, 6, 1};
        QVector<QNEInputPort *> aux = inputs();
        for (int i = 0; i < aux.size(); ++i) {
            aux[order[i]] = m_inputPorts.value(i);
        }
        setInputs(aux);
        updatePortsProperties();
    }
    if (version >= 3.1) {
        QString color_;
        stream >> color_;
        setColor(color_);
    }
}
