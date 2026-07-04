// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Display7.h"

#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QPainter>
#include <QRegularExpression>
#include <QSvgRenderer>
#include <QThread>

#include "App/Core/Common.h"
#include "App/Core/I18n.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Wiring/Port.h"

namespace {

/// Substitutes every solid fill color in \a svgBytes with \a color, leaving each shape's own
/// fill-opacity untouched -- the segment SVGs are solid-filled single shapes, so this reproduces
/// the White/Red/Green/Blue/Purple variants the old per-pixel channel remap produced.
QByteArray recoloredSegmentSvg(const QByteArray &svgBytes, const QColor &color)
{
    static const QRegularExpression fillHex(QStringLiteral("fill:#[0-9a-fA-F]{6}"));
    QString svg = QString::fromUtf8(svgBytes);
    svg.replace(fillHex, QStringLiteral("fill:%1").arg(color.name()));
    return svg.toUtf8();
}

} // namespace

template<>
struct ElementInfo<Display7> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Display7,
        .group = ElementGroup::Output,
        .minInputSize = 8,
        .maxInputSize = 8,
        .canChangeAppearance = true,
        .hasColors = true,
        .hasLabel = true,
        .rotatesGraphic = false,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Output/Counter/counter_on.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Display7", "7-SEGMENT DISPLAY");
        meta.translatedName = QT_TRANSLATE_NOOP("Display7", "7-Segment Display");
        meta.trContext = "Display7";
        meta.defaultAppearances = QStringList({
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
    a  = cachedSegmentRenderers(m_appearance.defaultAppearances().at(1));
    b  = cachedSegmentRenderers(m_appearance.defaultAppearances().at(2));
    c  = cachedSegmentRenderers(m_appearance.defaultAppearances().at(3));
    d  = cachedSegmentRenderers(m_appearance.defaultAppearances().at(4));
    e  = cachedSegmentRenderers(m_appearance.defaultAppearances().at(5));
    f  = cachedSegmentRenderers(m_appearance.defaultAppearances().at(6));
    g  = cachedSegmentRenderers(m_appearance.defaultAppearances().at(7));
    dp = cachedSegmentRenderers(m_appearance.defaultAppearances().at(8));

    Display7::updatePortsProperties();
}

QVector<std::shared_ptr<QSvgRenderer>> Display7::cachedSegmentRenderers(const QString &resourcePath)
{
    Q_ASSERT(QCoreApplication::instance()->thread() == QThread::currentThread());

    static QHash<QString, QVector<std::shared_ptr<QSvgRenderer>>> cache;

    auto it = cache.find(resourcePath);
    if (it != cache.end()) {
        return *it;
    }

    QFile file(resourcePath);
    const QByteArray svgBytes = file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();

    // Index order matches colorNameToIndex(): 0=White, 1=Red, 2=Green, 3=Blue, 4=Purple
    static const QColor colors[] = {
        QColor(255, 255, 255), QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255), QColor(255, 0, 255)
    };

    QVector<std::shared_ptr<QSvgRenderer>> renderers;
    renderers.reserve(5);
    for (const QColor &color : colors) {
        renderers.append(std::make_shared<QSvgRenderer>(recoloredSegmentSvg(svgBytes, color)));
    }

    cache.insert(resourcePath, renderers);
    return renderers;
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
    if (auto *port = inputPort(0)) { port->setPos( 0,  8); port->setName("G (" +  i18n("middle")      + ")"); }
    if (auto *port = inputPort(1)) { port->setPos( 0, 24); port->setName("F (" +  i18n("upper left")  + ")"); }
    if (auto *port = inputPort(2)) { port->setPos( 0, 40); port->setName("E (" +  i18n("lower left")  + ")"); }
    if (auto *port = inputPort(3)) { port->setPos( 0, 56); port->setName("D (" +  i18n("bottom")      + ")"); }
    if (auto *port = inputPort(4)) { port->setPos(64,  8); port->setName("A (" +  i18n("top")         + ")"); }
    if (auto *port = inputPort(5)) { port->setPos(64, 24); port->setName("B (" +  i18n("upper right") + ")"); }
    if (auto *port = inputPort(6)) { port->setPos(64, 40); port->setName("DP (" + i18n("dot")         + ")"); }
    if (auto *port = inputPort(7)) { port->setPos(64, 56); port->setName("C (" +  i18n("lower right") + ")"); }

    for (auto *in : inputs()) {
        // Segments are individually driven; leaving a pin unconnected turns that segment off
        in->setRequired(false);
        in->setDefaultStatus(Status::Inactive);
    }

    // This override doesn't chain to the base updatePortsProperties(), so re-apply the current
    // rotation/flip orientation to the freshly-positioned ports (keeps flip working on displays).
    rotatePorts();
}

void Display7::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // The base class draws the background (off-state SVG).
    // Active segments are painted on top as vector renders, crisp at any zoom.
    // All segment renderers share the background's 64x64 box, so they composite correctly.
    GraphicElement::paint(painter, option, widget);

    const QRectF body(0, 0, 64, 64);
    if (auto *port = inputPort(0); port && port->status() == Status::Active) { g.at(m_colorNumber)->render(painter, body);  }
    if (auto *port = inputPort(1); port && port->status() == Status::Active) { f.at(m_colorNumber)->render(painter, body);  }
    if (auto *port = inputPort(2); port && port->status() == Status::Active) { e.at(m_colorNumber)->render(painter, body);  }
    if (auto *port = inputPort(3); port && port->status() == Status::Active) { d.at(m_colorNumber)->render(painter, body);  }
    if (auto *port = inputPort(4); port && port->status() == Status::Active) { a.at(m_colorNumber)->render(painter, body);  }
    if (auto *port = inputPort(5); port && port->status() == Status::Active) { b.at(m_colorNumber)->render(painter, body);  }
    if (auto *port = inputPort(6); port && port->status() == Status::Active) { dp.at(m_colorNumber)->render(painter, body); }
    if (auto *port = inputPort(7); port && port->status() == Status::Active) { c.at(m_colorNumber)->render(painter, body);  }
}

void Display7::setColor(const QString &color)
{
    m_color = color;
    m_colorNumber = colorNameToIndex(color);
}

QString Display7::color() const
{
    return m_color;
}

void Display7::save(QDataStream &stream, SerializationOptions options) const
{
    GraphicElement::save(stream, options);

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
        QVector<InputPort *> aux = inputs();

        if (aux.size() == order.size()) {
            for (int i = 0; i < aux.size(); ++i) {
                aux[order[i]] = inputs().value(i);
            }
            setInputs(aux);
            updatePortsProperties();
        }
    }

    if (!VersionInfo::hasDisplay7ExtColor(context.version)) {
        // A second pin-order change occurred at v1.7, requiring another permutation
        // applied on top of whatever v1.6 remapping already happened.
        qCDebug(zero) << "Remapping inputs.";
        QVector<int> order{2, 5, 4, 0, 7, 3, 6, 1};
        QVector<InputPort *> aux = inputs();

        if (aux.size() == order.size()) {
            for (int i = 0; i < aux.size(); ++i) {
                aux[order[i]] = inputs().value(i);
            }
            setInputs(aux);
            updatePortsProperties();
        }
    }

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
