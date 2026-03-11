// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display_7.h"

#include "common.h"
#include "globalproperties.h"
#include "qneport.h"

#include <QPainter>
#include <QPixmap>

Display7::Display7(QGraphicsItem *parent)
    : GraphicElement(ElementType::Display7, ElementGroup::Output, ":/components/output/counter/counter_on.svg", tr("7-SEGMENT DISPLAY"), tr("7-Segment Display"), 8, 8, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins = QStringList{
        ":/components/output/counter/counter_off.svg",
        ":/components/output/counter/counter_a.svg",
        ":/components/output/counter/counter_b.svg",
        ":/components/output/counter/counter_c.svg",
        ":/components/output/counter/counter_d.svg",
        ":/components/output/counter/counter_e.svg",
        ":/components/output/counter/counter_f.svg",
        ":/components/output/counter/counter_g.svg",
        ":/components/output/counter/counter_dp.svg",
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    qCDebug(three) << "Allocating pixmaps.";
    a =  QVector<QPixmap>(5, m_defaultSkins.at(1));
    b =  QVector<QPixmap>(5, m_defaultSkins.at(2));
    c =  QVector<QPixmap>(5, m_defaultSkins.at(3));
    d =  QVector<QPixmap>(5, m_defaultSkins.at(4));
    e =  QVector<QPixmap>(5, m_defaultSkins.at(5));
    f =  QVector<QPixmap>(5, m_defaultSkins.at(6));
    g =  QVector<QPixmap>(5, m_defaultSkins.at(7));
    dp = QVector<QPixmap>(5, m_defaultSkins.at(8));

    qCDebug(three) << "Converting segments to other colors.";
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
    setRotatable(false);

    Display7::updatePortsProperties();
}

void Display7::convertAllColors(QVector<QPixmap> &pixmaps)
{
    QImage tmp(pixmaps.at(0).toImage());
    pixmaps[0] = convertColor(tmp, true, true, true);
    pixmaps[1] = convertColor(tmp, true, false, false);
    pixmaps[2] = convertColor(tmp, false, true, false);
    pixmaps[3] = convertColor(tmp, false, false, true);
    pixmaps[4] = convertColor(tmp, true, false, true);
}

QPixmap Display7::convertColor(const QImage &source, const bool red, const bool green, const bool blue)
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

void Display7::refresh()
{
    update();
}

void Display7::updatePortsProperties()
{
    if (auto *port = inputPort(0)) { port->setPos( 0,  8); port->setName("G (" +  tr("middle")      + ")"); }
    if (auto *port = inputPort(1)) { port->setPos( 0, 24); port->setName("F (" +  tr("upper left")  + ")"); }
    if (auto *port = inputPort(2)) { port->setPos( 0, 40); port->setName("E (" +  tr("lower left")  + ")"); }
    if (auto *port = inputPort(3)) { port->setPos( 0, 56); port->setName("D (" +  tr("bottom")      + ")"); }
    if (auto *port = inputPort(4)) { port->setPos(64,  8); port->setName("A (" +  tr("top")         + ")"); }
    if (auto *port = inputPort(5)) { port->setPos(64, 24); port->setName("B (" +  tr("upper right") + ")"); }
    if (auto *port = inputPort(6)) { port->setPos(64, 40); port->setName("DP (" + tr("dot")         + ")"); }
    if (auto *port = inputPort(7)) { port->setPos(64, 56); port->setName("C (" +  tr("lower right") + ")"); }

    for (auto *in : std::as_const(m_inputPorts)) {
        in->setRequired(false);
        in->setDefaultStatus(Status::Inactive);
    }
}

void Display7::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicElement::paint(painter, option, widget);

    if (auto *port = inputPort(0); port && port->status() == Status::Active) { painter->drawPixmap(0, 0, g.at(m_colorNumber));  }
    if (auto *port = inputPort(1); port && port->status() == Status::Active) { painter->drawPixmap(0, 0, f.at(m_colorNumber));  }
    if (auto *port = inputPort(2); port && port->status() == Status::Active) { painter->drawPixmap(0, 0, e.at(m_colorNumber));  }
    if (auto *port = inputPort(3); port && port->status() == Status::Active) { painter->drawPixmap(0, 0, d.at(m_colorNumber));  }
    if (auto *port = inputPort(4); port && port->status() == Status::Active) { painter->drawPixmap(0, 0, a.at(m_colorNumber));  }
    if (auto *port = inputPort(5); port && port->status() == Status::Active) { painter->drawPixmap(0, 0, b.at(m_colorNumber));  }
    if (auto *port = inputPort(6); port && port->status() == Status::Active) { painter->drawPixmap(0, 0, dp.at(m_colorNumber)); }
    if (auto *port = inputPort(7); port && port->status() == Status::Active) { painter->drawPixmap(0, 0, c.at(m_colorNumber));  }
}

void Display7::setColor(const QString &color)
{
    m_color = color;

    if (color == "White")  { m_colorNumber = 0; }
    if (color == "Red")    { m_colorNumber = 1; }
    if (color == "Green")  { m_colorNumber = 2; }
    if (color == "Blue")   { m_colorNumber = 3; }
    if (color == "Purple") { m_colorNumber = 4; }
}

QString Display7::color() const
{
    return m_color;
}

void Display7::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("color", color());

    stream << map;
}

void Display7::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    GraphicElement::load(stream, portMap, version);
    /*
     * 0, 7, 2, 1, 3, 4, 5, 6
     * 7, 5, 4, 2, 1, 4, 6, 3, 0
     * 4, 5, 7, 3, 2, 1, 0, 6
     * 2, 1, 4, 5, 0, 7, 3, 6
     */

    if (version < VERSION("1.6")) {
        qCDebug(zero) << "Remapping inputs.";
        QVector<int> order{2, 1, 4, 5, 0, 7, 3, 6};
        QVector<QNEInputPort *> aux = inputs();

        for (int i = 0; i < aux.size(); ++i) {
            aux[order[i]] = m_inputPorts.value(i);
        }

        setInputs(aux);
        updatePortsProperties();
    }

    if (version < VERSION("1.7")) {
        qCDebug(zero) << "Remapping inputs.";
        QVector<int> order{2, 5, 4, 0, 7, 3, 6, 1};
        QVector<QNEInputPort *> aux = inputs();

        for (int i = 0; i < aux.size(); ++i) {
            aux[order[i]] = m_inputPorts.value(i);
        }

        setInputs(aux);
        updatePortsProperties();
    }

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
