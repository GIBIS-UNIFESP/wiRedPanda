// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Display16.h"

#include <QPainter>
#include <QSvgRenderer>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<Display16> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Display16,
        .group = ElementGroup::Output,
        .minInputSize = 17,
        .maxInputSize = 17,
        .canChangeAppearance = true,
        .hasColors = true,
        .hasLabel = true,
        .rotatesGraphic = false,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Output/Counter/counter_16_on.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Display16", "16-SEGMENT DISPLAY");
        meta.translatedName = QT_TRANSLATE_NOOP("Display16", "16-Segment Display");
        meta.trContext = "Display16";
        meta.defaultAppearances = QStringList({
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
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementMetadata's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Display16(); });
        return true;
    }();
};

Display16::Display16(QGraphicsItem *parent)
    : Element(ElementType::Display16, parent)
{
    a1 = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(1));
    a2 = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(2));
    b  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(3));
    c  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(4));
    d1 = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(5));
    d2 = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(6));
    e  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(7));
    f  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(8));
    g1 = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(9));
    g2 = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(10));
    h  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(11));
    j  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(12));
    k  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(13));
    l  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(14));
    m  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(15));
    n  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(16));
    dp = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(17));

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
    inputPort( 0)->setPos( 0,  -8);    setDescribedPortName(inputPort( 0), "G1", QT_TR_NOOP("middle left horizontal"));
    inputPort( 1)->setPos( 0,   8);    setDescribedPortName(inputPort( 1), "F",  QT_TR_NOOP("upper left vertical"));
    inputPort( 2)->setPos( 0,  24);    setDescribedPortName(inputPort( 2), "E",  QT_TR_NOOP("lower left vertical"));
    inputPort( 3)->setPos( 0,  40);    setDescribedPortName(inputPort( 3), "D1", QT_TR_NOOP("bottom left horizontal"));
    inputPort( 4)->setPos( 0,  56);    setDescribedPortName(inputPort( 4), "D2", QT_TR_NOOP("bottom right horizontal"));
    inputPort( 5)->setPos(64,  -8);    setDescribedPortName(inputPort( 5), "A1", QT_TR_NOOP("top left horizontal"));
    inputPort( 6)->setPos(64,   8);    setDescribedPortName(inputPort( 6), "A2", QT_TR_NOOP("top right horizontal"));
    inputPort( 7)->setPos(64,  24);    setDescribedPortName(inputPort( 7), "B",  QT_TR_NOOP("upper right vertical"));
    inputPort( 8)->setPos(64,  40);    setDescribedPortName(inputPort( 8), "DP", QT_TR_NOOP("dot"));
    inputPort( 9)->setPos(64,  56);    setDescribedPortName(inputPort( 9), "C",  QT_TR_NOOP("lower right vertical"));
    inputPort(10)->setPos( 0,  72);    setDescribedPortName(inputPort(10), "G2", QT_TR_NOOP("middle right horizontal"));
    inputPort(11)->setPos( 0,  88);    setDescribedPortName(inputPort(11), "H",  QT_TR_NOOP("upper left diagonal"));
    inputPort(12)->setPos( 0, 104);    setDescribedPortName(inputPort(12), "J",  QT_TR_NOOP("upper center vertical"));
    inputPort(13)->setPos(64,  72);    setDescribedPortName(inputPort(13), "K",  QT_TR_NOOP("upper right diagonal"));
    inputPort(14)->setPos(64,  88);    setDescribedPortName(inputPort(14), "L",  QT_TR_NOOP("lower right diagonal"));
    inputPort(15)->setPos(64, 104);    setDescribedPortName(inputPort(15), "M",  QT_TR_NOOP("lower center vertical"));
    inputPort(16)->setPos(64, 120);    setDescribedPortName(inputPort(16), "N",  QT_TR_NOOP("lower left diagonal"));

    for (auto *port : inputs()) {
        port->setRequired(false);
        port->setDefaultStatus(Status::Inactive);
    }

    // This override doesn't chain to the base updatePortsProperties(), so re-apply the current
    // rotation/flip orientation to the freshly-positioned ports (keeps flip working on displays).
    rotatePorts();
}

QRectF Display16::boundingRect() const
{
    return Element::boundingRect().united(QRectF(0, 0, 64, 64));
}

void Display16::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Base class draws the off-state background; active segments are overlaid as vector
    // renders on top, crisp at any zoom.
    Element::paint(painter, option, widget);

    const QRectF body(0, 0, 64, 64);
    if (inputPort( 0)->status() == Status::Active) { g1.at(m_colorNumber)->render(painter, body); }
    if (inputPort( 1)->status() == Status::Active) { f.at(m_colorNumber)->render(painter, body);  }
    if (inputPort( 2)->status() == Status::Active) { e.at(m_colorNumber)->render(painter, body);  }
    if (inputPort( 3)->status() == Status::Active) { d1.at(m_colorNumber)->render(painter, body); }
    if (inputPort( 4)->status() == Status::Active) { d2.at(m_colorNumber)->render(painter, body); }
    if (inputPort( 5)->status() == Status::Active) { a1.at(m_colorNumber)->render(painter, body); }
    if (inputPort( 6)->status() == Status::Active) { a2.at(m_colorNumber)->render(painter, body); }
    if (inputPort( 7)->status() == Status::Active) { b.at(m_colorNumber)->render(painter, body);  }
    if (inputPort( 8)->status() == Status::Active) { dp.at(m_colorNumber)->render(painter, body); }
    if (inputPort( 9)->status() == Status::Active) { c.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(10)->status() == Status::Active) { g2.at(m_colorNumber)->render(painter, body); }
    if (inputPort(11)->status() == Status::Active) { h.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(12)->status() == Status::Active) { j.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(13)->status() == Status::Active) { k.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(14)->status() == Status::Active) { l.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(15)->status() == Status::Active) { m.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(16)->status() == Status::Active) { n.at(m_colorNumber)->render(painter, body);  }
}

void Display16::setColor(const QString &color)
{
    m_color = color;
    m_colorNumber = colorNameToIndex(color);
}

QString Display16::color() const
{
    return m_color;
}

void Display16::save(QDataStream &stream, SerializationOptions options) const
{
    Element::save(stream, options);

    QMap<QString, QVariant> map;
    // PortableFile streams omit the default; fresh-loaded elements start there
    // anyway. Snapshots write unconditionally (reloaded into live elements).
    if (options.purpose != SerializationPurpose::PortableFile || color() != kDefaultColor) {
        map.insert("color", color());
    }

    stream << map;
}

void Display16::load(QDataStream &stream, SerializationContext &context)
{
    Element::load(stream, context);

    if ((VersionInfo::hasLockState(context.version)) && (!VersionInfo::hasQMapFormat(context.version))) {
        // v3.1–4.0 stored color as a bare QString
        const QString color_ = Serialization::readBoundedString(stream);
        setColor(color_);
    }

    if (VersionInfo::hasQMapFormat(context.version)) {
        // v4.1+ uses a key-value map
        QMap<QString, QVariant> map = Serialization::readBoundedMetadata(stream);

        if (map.contains("color")) {
            setColor(map.value("color").toString());
        }
    }
}
