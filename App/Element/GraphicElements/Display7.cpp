// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Display7.h"

#include <QPainter>
#include <QPixmap>

#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Nodes/QNEPort.h"

template<>
struct ElementInfo<Display7> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Display7,
        .group = ElementGroup::Output,
        .minInputSize = 8,
        .maxInputSize = 8,
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
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Output/Counter/counter_on.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Display7", "7-SEGMENT DISPLAY");
        meta.translatedName = QT_TRANSLATE_NOOP("Display7", "7-Segment Display");
        meta.trContext = "Display7";
        meta.defaultSkins = QStringList({
            ":/Components/Output/Counter/counter_off.svg",
            ":/Components/Output/Counter/counter_a.svg",
            ":/Components/Output/Counter/counter_b.svg",
            ":/Components/Output/Counter/counter_c.svg",
            ":/Components/Output/Counter/counter_d.svg",
            ":/Components/Output/Counter/counter_e.svg",
            ":/Components/Output/Counter/counter_f.svg",
            ":/Components/Output/Counter/counter_g.svg",
            ":/Components/Output/Counter/counter_dp.svg",
        });
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Display7(); });
        return true;
    }();
};

Display7::Display7(QGraphicsItem *parent)
    : GraphicElement(ElementType::Display7, parent)
{
    qCDebug(three) << "Allocating pixmaps.";
    // Each segment is stored as a vector of 5 color variants (White, Red, Green, Blue, Purple).
    // They all start as copies of the default (white) SVG; convertAllColors() then recolors
    // each copy in-place using pixel-level channel manipulation.
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

    Display7::updatePortsProperties();
}

void Display7::convertAllColors(QVector<QPixmap> &pixmaps)
{
    // All five slots start with the same source image; recolor each one by
    // selectively passing or zeroing the R/G/B channels:
    //   0 = White  (R+G+B), 1 = Red (R only), 2 = Green (G only),
    //   3 = Blue   (B only), 4 = Purple (R+B)
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
            // The source SVG uses grayscale: red channel encodes pixel brightness.
            // Reuse that brightness value for the enabled channels and set alpha
            // equal to brightness so transparent backgrounds remain transparent.
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
    // Ports on the left side (x=0) correspond to the left segments (G, F, E, D);
    // ports on the right side (x=64) correspond to the right segments and DP (A, B, DP, C).
    // This matches the physical pin layout used by common 7-segment display ICs.
    if (auto *port = inputPort(0)) { port->setPos( 0,  8); port->setName("G (" +  tr("middle")      + ")"); }
    if (auto *port = inputPort(1)) { port->setPos( 0, 24); port->setName("F (" +  tr("upper left")  + ")"); }
    if (auto *port = inputPort(2)) { port->setPos( 0, 40); port->setName("E (" +  tr("lower left")  + ")"); }
    if (auto *port = inputPort(3)) { port->setPos( 0, 56); port->setName("D (" +  tr("bottom")      + ")"); }
    if (auto *port = inputPort(4)) { port->setPos(64,  8); port->setName("A (" +  tr("top")         + ")"); }
    if (auto *port = inputPort(5)) { port->setPos(64, 24); port->setName("B (" +  tr("upper right") + ")"); }
    if (auto *port = inputPort(6)) { port->setPos(64, 40); port->setName("DP (" + tr("dot")         + ")"); }
    if (auto *port = inputPort(7)) { port->setPos(64, 56); port->setName("C (" +  tr("lower right") + ")"); }

    for (auto *in : std::as_const(m_inputPorts)) {
        // Segments are individually driven; leaving a pin unconnected turns that segment off
        in->setRequired(false);
        in->setDefaultStatus(Status::Inactive);
    }
}

void Display7::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // The base class draws the background (off-state SVG).
    // Active segments are painted on top by overlaying the pre-rendered colored pixmaps.
    // All segment pixmaps are the same dimensions as the background, so they composite correctly.
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

void Display7::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElement::load(stream, context);
    /*
     * 0, 7, 2, 1, 3, 4, 5, 6
     * 7, 5, 4, 2, 1, 4, 6, 3, 0
     * 4, 5, 7, 3, 2, 1, 0, 6
     * 2, 1, 4, 5, 0, 7, 3, 6
     */

    if (!VersionInfo::hasDisplay7Color(context.version)) {
        // The pin-to-segment mapping changed at v1.6. The permutation
        // order[i] = j means "the port that was at index i in the old file
        // should now live at index j in the canonical layout."
        // Permutation: old[0]→new[2], old[1]→new[1], old[2]→new[4], etc.
        qCDebug(zero) << "Remapping inputs.";
        QVector<int> order{2, 1, 4, 5, 0, 7, 3, 6};
        QVector<QNEInputPort *> aux = inputs();

        for (int i = 0; i < aux.size(); ++i) {
            aux[order[i]] = m_inputPorts.value(i);
        }

        setInputs(aux);
        updatePortsProperties();
    }

    if (!VersionInfo::hasDisplay7ExtColor(context.version)) {
        // A second pin-order change occurred at v1.7, requiring another permutation
        // applied on top of whatever v1.6 remapping already happened.
        qCDebug(zero) << "Remapping inputs.";
        QVector<int> order{2, 5, 4, 0, 7, 3, 6, 1};
        QVector<QNEInputPort *> aux = inputs();

        for (int i = 0; i < aux.size(); ++i) {
            aux[order[i]] = m_inputPorts.value(i);
        }

        setInputs(aux);
        updatePortsProperties();
    }

    if ((VersionInfo::hasLockState(context.version)) && (!VersionInfo::hasQMapFormat(context.version))) {
        // v3.1–4.0 stored color as a bare QString
        QString color_; stream >> color_;
        setColor(color_);
    }

    if (VersionInfo::hasQMapFormat(context.version)) {
        // v4.1+ uses a key-value map
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("color")) {
            setColor(map.value("color").toString());
        }
    }
}

