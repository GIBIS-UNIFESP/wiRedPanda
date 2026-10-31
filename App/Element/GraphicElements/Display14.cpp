// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Display14.h"

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
struct ElementInfo<Display14> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Display14,
        .group = ElementGroup::Output,
        .minInputSize = 15,
        .maxInputSize = 15,
        .canChangeAppearance = true,
        .hasLabel = true,
        .rotatesGraphic = false,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = []{ return QStringLiteral(":/Components/Output/Counter/counter_14_on.svg"); };
        descriptor.titleText = QT_TRANSLATE_NOOP("Display14", "14-SEGMENT DISPLAY");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Display14", "14-Segment Display");
        descriptor.trContext = "Display14";
        descriptor.defaultAppearances = QStringList({
            ":/Components/Output/Counter/counter_14_off.svg",
            ":/Components/Output/Counter/counter_a.svg",
            ":/Components/Output/Counter/counter_b.svg",
            ":/Components/Output/Counter/counter_c.svg",
            ":/Components/Output/Counter/counter_d.svg",
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
        // Any name is kept; one not in the list is drawn as White
        descriptor.parameters = {{
            .id = ParamId::Colour,
            .defaultValue = QString(Display14::kDefaultColor),
            .key = "color",
        }};
        descriptor.readBeforeMap = [](Element &element, QDataStream &stream, const SerializationContext &context) {
            // From 3.1 to 4.0 the colour was a bare string
            if (VersionInfo::hasLockState(context.version)) {
                element.setParam(ParamId::Colour, Serialization::readBoundedString(stream));
            }
        };
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Display14(); });
        return true;
    }();
};

Display14::Display14(QGraphicsItem *parent)
    : Element(ElementType::Display14, parent)
{
    a  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(1));
    b  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(2));
    c  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(3));
    d  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(4));
    e  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(5));
    f  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(6));
    g1 = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(7));
    g2 = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(8));
    h  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(9));
    j  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(10));
    k  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(11));
    l  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(12));
    m  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(13));
    n  = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(14));
    dp = Display7::cachedSegmentRenderers(m_appearance.defaultAppearances().at(15));

    updatePortsProperties();
}

void Display14::refresh()
{
    update();
}

std::optional<Element::PortAttributes> Display14::inputAttributes(const int index) const
{
    // Left-side ports (x=0): G1, F, E, D (outer horizontal/vertical) + G2, H, J (diagonals/center)
    // Right-side ports (x=64): A, B, DP, C + K, L, M, N
    // This two-column layout mirrors the physical dual-inline-package pin arrangement.
    static constexpr Display7::SegmentPort segments[] = {
        {  0,   -8, "G1", QT_TR_NOOP("middle left horizontal")},
        {  0,    8, "F",  QT_TR_NOOP("upper left vertical")},
        {  0,   24, "E",  QT_TR_NOOP("lower left vertical")},
        {  0,   40, "D",  QT_TR_NOOP("bottom")},
        { 64,   -8, "A",  QT_TR_NOOP("top")},
        { 64,    8, "B",  QT_TR_NOOP("upper right vertical")},
        { 64,   24, "DP", QT_TR_NOOP("dot")},
        { 64,   40, "C",  QT_TR_NOOP("lower right vertical")},
        {  0,   56, "G2", QT_TR_NOOP("middle right horizontal")},
        {  0,   72, "H",  QT_TR_NOOP("upper left diagonal")},
        {  0,   88, "J",  QT_TR_NOOP("upper center vertical")},
        { 64,   56, "K",  QT_TR_NOOP("upper right diagonal")},
        { 64,   72, "L",  QT_TR_NOOP("lower right diagonal")},
        { 64,   88, "M",  QT_TR_NOOP("lower center vertical")},
        { 64,  104, "N",  QT_TR_NOOP("lower left diagonal")},
    };
    if ((index < 0) || (index >= static_cast<int>(std::size(segments)))) {
        return std::nullopt;
    }
    const Display7::SegmentPort &segment = segments[index];
    return Display7::segmentInput(describedPort(segment.letter, segment.description), segment);
}

QRectF Display14::boundingRect() const
{
    return Element::boundingRect().united(QRectF(0, 0, 64, 64));
}

void Display14::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Base class paints the off-state background; active segments are overlaid as vector
    // renders on top, crisp at any zoom.
    Element::paint(painter, option, widget);

    const QRectF body(0, 0, 64, 64);
    if (inputPort(0)->status() == Status::Active)  { g1.at(m_colorNumber)->render(painter, body); }
    if (inputPort(1)->status() == Status::Active)  { f.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(2)->status() == Status::Active)  { e.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(3)->status() == Status::Active)  { d.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(4)->status() == Status::Active)  { a.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(5)->status() == Status::Active)  { b.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(6)->status() == Status::Active)  { dp.at(m_colorNumber)->render(painter, body); }
    if (inputPort(7)->status() == Status::Active)  { c.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(8)->status() == Status::Active)  { g2.at(m_colorNumber)->render(painter, body); }
    if (inputPort(9)->status() == Status::Active)  { h.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(10)->status() == Status::Active) { j.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(11)->status() == Status::Active) { k.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(12)->status() == Status::Active) { l.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(13)->status() == Status::Active) { m.at(m_colorNumber)->render(painter, body);  }
    if (inputPort(14)->status() == Status::Active) { n.at(m_colorNumber)->render(painter, body);  }
}

void Display14::parameterChanged(const ParamId id, const ParamValue &previous)
{
    Element::parameterChanged(id, previous);
    if (id == ParamId::Colour) {
        m_colorNumber = colorNameToIndex(color());
    }
}
