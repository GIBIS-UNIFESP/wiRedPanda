// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Display16.h"

#include <QPainter>

#include "App/Element/ElementInfo.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Nodes/QNEPort.h"
#include "App/Versions.h"

template<>
struct ElementInfo<Display16> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Display16,
        .group = ElementGroup::Output,
        .minInputSize = 17,
        .maxInputSize = 17,
        .minOutputSize = 0,
        .maxOutputSize = 0,
        .canChangeSkin = true,
        .hasColors = true,
        .hasAudio = false,
        .hasAudioBox = false,
        .hasTrigger = false,
        .hasFrequency = false,
        .hasDelay = false,
        .hasLabel = true,
        .hasTruthTable = false,
        .rotatable = false,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Output/Counter/counter_16_on.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Display16", "16-SEGMENT DISPLAY");
        meta.translatedName = QT_TRANSLATE_NOOP("Display16", "16-Segment Display");
        meta.trContext = "Display16";
        meta.defaultSkins = QStringList({
            ":/Components/Output/Counter/counter_16_off.svg",
            ":/Components/Output/Counter/counter_a1.svg",
            ":/Components/Output/Counter/counter_a2.svg",
            ":/Components/Output/Counter/counter_b.svg",
            ":/Components/Output/Counter/counter_c.svg",
            ":/Components/Output/Counter/counter_d1.svg",
            ":/Components/Output/Counter/counter_d2.svg",
            ":/Components/Output/Counter/counter_e.svg",
            ":/Components/Output/Counter/counter_f.svg",
            ":/Components/Output/Counter/counter_g1.svg",
            ":/Components/Output/Counter/counter_g2.svg",
            ":/Components/Output/Counter/counter_h.svg",
            ":/Components/Output/Counter/counter_j.svg",
            ":/Components/Output/Counter/counter_k.svg",
            ":/Components/Output/Counter/counter_l.svg",
            ":/Components/Output/Counter/counter_m.svg",
            ":/Components/Output/Counter/counter_n.svg",
            ":/Components/Output/Counter/counter_dp.svg",
        });
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Display16(); });
        return true;
    }();
};

Display16::Display16(QGraphicsItem *parent)
    : GraphicElement(ElementType::Display16, parent)
{
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

    Display16::updatePortsProperties();
}

void Display16::refresh()
{
    update();
}

void Display16::updatePortsProperties()
{
    // Ports on the left (x=0) drive left-side and center segments;
    // ports on the right (x=64) drive right-side segments and DP.
    // The port order follows the standard 16-segment naming convention
    // (A1/A2 = top, B/C = right verticals, D1/D2 = bottom, E/F = left verticals,
    //  G1/G2 = middle, H/J/K/L/M/N = diagonals and center verticals).
    inputPort( 0)->setPos( 0,  -8);    inputPort( 0)->setName("G1 (" + tr("middle left horizontal")  + ")");
    inputPort( 1)->setPos( 0,   8);    inputPort( 1)->setName("F ("  + tr("upper left vertical")     + ")");
    inputPort( 2)->setPos( 0,  24);    inputPort( 2)->setName("E ("  + tr("lower left vertical")     + ")");
    inputPort( 3)->setPos( 0,  40);    inputPort( 3)->setName("D1 (" + tr("bottom left horizontal")  + ")");
    inputPort( 4)->setPos( 0,  56);    inputPort( 4)->setName("D2 (" + tr("bottom right horizontal") + ")");
    inputPort( 5)->setPos(64,  -8);    inputPort( 5)->setName("A1 (" + tr("top left horizontal")     + ")");
    inputPort( 6)->setPos(64,   8);    inputPort( 6)->setName("A2 (" + tr("top right horizontal")    + ")");
    inputPort( 7)->setPos(64,  24);    inputPort( 7)->setName("B ("  + tr("upper right vertical")    + ")");
    inputPort( 8)->setPos(64,  40);    inputPort( 8)->setName("DP (" + tr("dot")                     + ")");
    inputPort( 9)->setPos(64,  56);    inputPort( 9)->setName("C ("  + tr("lower right vertical")    + ")");
    inputPort(10)->setPos( 0,  72);    inputPort(10)->setName("G2 (" + tr("middle right horizontal") + ")");
    inputPort(11)->setPos( 0,  88);    inputPort(11)->setName("H ("  + tr("upper left diagonal")     + ")");
    inputPort(12)->setPos( 0, 104);    inputPort(12)->setName("J ("  + tr("upper center vertical")   + ")");
    inputPort(13)->setPos(64,  72);    inputPort(13)->setName("K ("  + tr("upper right diagonal")    + ")");
    inputPort(14)->setPos(64,  88);    inputPort(14)->setName("L ("  + tr("lower right diagonal")    + ")");
    inputPort(15)->setPos(64, 104);    inputPort(15)->setName("M ("  + tr("lower center vertical")   + ")");
    inputPort(16)->setPos(64, 120);    inputPort(16)->setName("N ("  + tr("lower left vertical")     + ")");

    for (auto *port : std::as_const(m_inputPorts)) {
        port->setRequired(false);
        port->setDefaultStatus(Status::Inactive);
    }
}

void Display16::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Base class draws the off-state background; active segments are overlaid on top
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

void Display16::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElement::load(stream, context);

    if ((Versions::V_3_1 <= context.version) && (context.version < Versions::V_4_1)) {
        // v3.1–4.0 stored color as a bare QString
        QString color_; stream >> color_;
        setColor(color_);
    }

    if (context.version >= Versions::V_4_1) {
        // v4.1+ uses a key-value map
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("color")) {
            setColor(map.value("color").toString());
        }
    }
}

