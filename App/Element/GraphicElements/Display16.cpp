// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Display16.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"

template<>
struct ElementInfo<Display16> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Display16,
        .group = ElementGroup::Output,
        .minInputSize = 17,
        .maxInputSize = 17,
        .canChangeAppearance = true,
        .hasLabel = true,
        .rotatesGraphic = false,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = QStringLiteral(":/Components/Output/Counter/counter_16_on.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("Display16", "16-SEGMENT DISPLAY");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Display16", "16-Segment Display");
        descriptor.trContext = "Display16";
        descriptor.defaultAppearances = QStringList({
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
        // Any name is kept; one not in the list is drawn as White
        descriptor.parameters = {{
            .id = ParamId::Colour,
            .defaultValue = QString(Display16::kDefaultColor),
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
        ElementFactory::registerCreator(constraints.type, [] { return new Display16(); });
        return true;
    }();
};

Display16::Display16(QGraphicsItem *parent)
    : Element(ElementType::Display16, parent)
{
    updatePortsProperties();
}

void Display16::refresh()
{
    pictureChanged();
}

std::optional<Element::PortAttributes> Display16::inputAttributes(const int index) const
{
    // Ports on the left (x=0) drive left-side and center segments;
    // ports on the right (x=64) drive right-side segments and DP.
    // The port order follows the standard 16-segment naming convention
    // (A1/A2 = top, B/C = right verticals, D1/D2 = bottom, E/F = left verticals,
    //  G1/G2 = middle, H/J/K/L/M/N = diagonals and center verticals).
    static constexpr Display7::SegmentPort segments[] = {
        {  0,   -8, "G1", QT_TR_NOOP("middle left horizontal")},
        {  0,    8, "F",  QT_TR_NOOP("upper left vertical")},
        {  0,   24, "E",  QT_TR_NOOP("lower left vertical")},
        {  0,   40, "D1", QT_TR_NOOP("bottom left horizontal")},
        {  0,   56, "D2", QT_TR_NOOP("bottom right horizontal")},
        { 64,   -8, "A1", QT_TR_NOOP("top left horizontal")},
        { 64,    8, "A2", QT_TR_NOOP("top right horizontal")},
        { 64,   24, "B",  QT_TR_NOOP("upper right vertical")},
        { 64,   40, "DP", QT_TR_NOOP("dot")},
        { 64,   56, "C",  QT_TR_NOOP("lower right vertical")},
        {  0,   72, "G2", QT_TR_NOOP("middle right horizontal")},
        {  0,   88, "H",  QT_TR_NOOP("upper left diagonal")},
        {  0,  104, "J",  QT_TR_NOOP("upper center vertical")},
        { 64,   72, "K",  QT_TR_NOOP("upper right diagonal")},
        { 64,   88, "L",  QT_TR_NOOP("lower right diagonal")},
        { 64,  104, "M",  QT_TR_NOOP("lower center vertical")},
        { 64,  120, "N",  QT_TR_NOOP("lower left diagonal")},
    };
    if ((index < 0) || (index >= static_cast<int>(std::size(segments)))) {
        return std::nullopt;
    }
    const Display7::SegmentPort &segment = segments[index];
    return Display7::segmentInput(describedPort(segment.letter, segment.description), segment);
}
