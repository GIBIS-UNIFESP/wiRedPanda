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
        .hasLabel = true,
        .rotatesGraphic = false,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = []{ return QStringLiteral(":/Components/Output/Counter/counter_on.svg"); };
        descriptor.titleText = QT_TRANSLATE_NOOP("Display7", "7-SEGMENT DISPLAY");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Display7", "7-Segment Display");
        descriptor.trContext = "Display7";
        descriptor.defaultAppearances = QStringList({
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
        // Any name is kept; one not in the list is drawn as White
        descriptor.parameters = {{
            .id = ParamId::Colour,
            .defaultValue = QString(Display7::kDefaultColor),
            .key = "color",
        }};
        descriptor.readBeforeMap = [](Element &element, QDataStream &stream, const SerializationContext &context) {
            // The inputs were in another order before 1.6 (VersionInfo::hasDisplay7Color()) and again before 1.7
            // (hasDisplay7ExtColor()): order[i] is where a file's input i is now
            const auto reorder = [&element](const QVector<int> &order) {
                QVector<InputPort *> ports = element.inputs();
                if (ports.size() == order.size()) {
                    for (int i = 0; i < ports.size(); ++i) {
                        ports[order[i]] = element.inputs().value(i);
                    }
                    element.setInputs(ports);
                    element.updatePortsProperties();
                }
            };
            if (!VersionInfo::hasDisplay7Color(context.version)) {
                reorder({2, 1, 4, 5, 0, 7, 3, 6});
            }
            if (!VersionInfo::hasDisplay7ExtColor(context.version)) {
                reorder({2, 5, 4, 0, 7, 3, 6, 1});
            }
            // From 3.1 to 4.0 the colour was a bare string
            if (VersionInfo::hasLockState(context.version)) {
                element.setParam(ParamId::Colour, Serialization::readBoundedString(stream));
            }
        };
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Display7(); });
        return true;
    }();
};

Display7::Display7(QGraphicsItem *parent)
    : Element(ElementType::Display7, parent)
{
    a  = cachedSegmentRenderers(m_appearance.defaultAppearances().at(1));
    b  = cachedSegmentRenderers(m_appearance.defaultAppearances().at(2));
    c  = cachedSegmentRenderers(m_appearance.defaultAppearances().at(3));
    d  = cachedSegmentRenderers(m_appearance.defaultAppearances().at(4));
    e  = cachedSegmentRenderers(m_appearance.defaultAppearances().at(5));
    f  = cachedSegmentRenderers(m_appearance.defaultAppearances().at(6));
    g  = cachedSegmentRenderers(m_appearance.defaultAppearances().at(7));
    dp = cachedSegmentRenderers(m_appearance.defaultAppearances().at(8));

    updatePortsProperties();
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
    pictureChanged();
}

std::optional<Element::PortAttributes> Display7::inputAttributes(const int index) const
{
    // Ports on the left side (x=0) correspond to the left segments (G, F, E, D);
    // ports on the right side (x=64) correspond to the right segments and DP (A, B, DP, C).
    // This matches the physical pin layout used by common 7-segment display ICs.
    static constexpr Display7::SegmentPort segments[] = {
        {  0,    8, "G",  QT_TR_NOOP("middle")},
        {  0,   24, "F",  QT_TR_NOOP("upper left")},
        {  0,   40, "E",  QT_TR_NOOP("lower left")},
        {  0,   56, "D",  QT_TR_NOOP("bottom")},
        { 64,    8, "A",  QT_TR_NOOP("top")},
        { 64,   24, "B",  QT_TR_NOOP("upper right")},
        { 64,   40, "DP", QT_TR_NOOP("dot")},
        { 64,   56, "C",  QT_TR_NOOP("lower right")},
    };
    if ((index < 0) || (index >= static_cast<int>(std::size(segments)))) {
        return std::nullopt;
    }
    const Display7::SegmentPort &segment = segments[index];
    return Display7::segmentInput(describedPort(segment.letter, segment.description), segment);
}

Element::PortAttributes Display7::segmentInput(PortAttributes described, const SegmentPort &segment)
{
    PortAttributes attributes = std::move(described);
    attributes.required = false;
    attributes.defaultStatus = Status::Inactive;
    attributes.position = QPointF(segment.x, segment.y);
    return attributes;
}

QRectF Display7::measureBoundingRect() const
{
    return Element::measureBoundingRect().united(QRectF(0, 0, 64, 64));
}

void Display7::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // The base class draws the background (off-state SVG).
    // Active segments are painted on top as vector renders, crisp at any zoom.
    // All segment renderers share the background's 64x64 box, so they composite correctly.
    Element::paint(painter, option, widget);

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

void Display7::parameterChanged(const ParamId id, const ParamValue &previous)
{
    Element::parameterChanged(id, previous);
    if (id == ParamId::Colour) {
        m_colorNumber = colorNameToIndex(color());
    }
}
