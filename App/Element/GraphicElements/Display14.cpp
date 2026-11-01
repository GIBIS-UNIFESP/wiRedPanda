// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Display14.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"

template<>
struct ElementInfo<ElementType::Display14> {
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
        descriptor.pixmapPath = QStringLiteral(":/Components/Output/Counter/counter_14_on.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("Display14", "14-SEGMENT DISPLAY");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Display14", "14-Segment Display");
        descriptor.trContext = "Display14";
        descriptor.appearanceSlots = {
            {"off", ":/Components/Output/Counter/counter_14_off.svg"},
            {"a", ":/Components/Output/Counter/counter_a.svg"},
            {"b", ":/Components/Output/Counter/counter_b.svg"},
            {"c", ":/Components/Output/Counter/counter_c.svg"},
            {"d", ":/Components/Output/Counter/counter_d.svg"},
            {"e", ":/Components/Output/Counter/counter_e.svg"},
            {"f", ":/Components/Output/Counter/counter_f.svg"},
            {"g1", ":/Components/Output/Counter/counter_g1.svg"},
            {"g2", ":/Components/Output/Counter/counter_g2.svg"},
            {"h", ":/Components/Output/Counter/counter_h.svg"},
            {"j", ":/Components/Output/Counter/counter_j.svg"},
            {"k", ":/Components/Output/Counter/counter_k.svg"},
            {"l", ":/Components/Output/Counter/counter_l.svg"},
            {"m", ":/Components/Output/Counter/counter_m.svg"},
            {"n", ":/Components/Output/Counter/counter_n.svg"},
            {"dp", ":/Components/Output/Counter/counter_dp.svg"},
        };
        // A file keeps any name, one not among the choices drawn as White; MCP sets the choices alone
        descriptor.parameters = {{
            .id = ParamId::Colour,
            .defaultValue = QString(Display14::kDefaultColor),
            .key = "color",
            .choices = Element::colorNames(),
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
    updatePortsProperties();
}

void Display14::refresh()
{
    pictureChanged();
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
