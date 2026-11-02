// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/DisplaySegments.h"
#include "App/Element/Element.h"
#include "App/Element/ElementColour.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"

template<>
struct ElementInfo<ElementType::Display16> {
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
        descriptor.appearanceSlots = {
            {"off", ":/Components/Output/Counter/counter_16_off.svg"},
            {"a1", ":/Components/Output/Counter/counter_a1.svg"},
            {"a2", ":/Components/Output/Counter/counter_a2.svg"},
            {"b", ":/Components/Output/Counter/counter_b.svg"},
            {"c", ":/Components/Output/Counter/counter_c.svg"},
            {"d1", ":/Components/Output/Counter/counter_d1.svg"},
            {"d2", ":/Components/Output/Counter/counter_d2.svg"},
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
            .defaultValue = QStringLiteral("Red"),
            .key = "color",
            .choices = ElementColour::names(),
        }};
        descriptor.readBeforeMap = [](Element &element, QDataStream &stream, const SerializationContext &context) {
            // From 3.1 to 4.0 the colour was a bare string
            if (VersionInfo::hasLockState(context.version)) {
                element.setParam(ParamId::Colour, Serialization::readBoundedString(stream));
            }
        };
        descriptor.inputAttributes = [](const Element &element, const int index) {
            // Ports on the left (x=0) drive left-side and center segments;
            // ports on the right (x=64) drive right-side segments and DP.
            // The port order follows the standard 16-segment naming convention
            // (A1/A2 = top, B/C = right verticals, D1/D2 = bottom, E/F = left verticals,
            //  G1/G2 = middle, H/J/K/L/M/N = diagonals and center verticals).
            static constexpr DisplaySegment segments[] = {
                {  0,   -8, "G1", QT_TRANSLATE_NOOP("Display16", "middle left horizontal")},
                {  0,    8, "F",  QT_TRANSLATE_NOOP("Display16", "upper left vertical")},
                {  0,   24, "E",  QT_TRANSLATE_NOOP("Display16", "lower left vertical")},
                {  0,   40, "D1", QT_TRANSLATE_NOOP("Display16", "bottom left horizontal")},
                {  0,   56, "D2", QT_TRANSLATE_NOOP("Display16", "bottom right horizontal")},
                { 64,   -8, "A1", QT_TRANSLATE_NOOP("Display16", "top left horizontal")},
                { 64,    8, "A2", QT_TRANSLATE_NOOP("Display16", "top right horizontal")},
                { 64,   24, "B",  QT_TRANSLATE_NOOP("Display16", "upper right vertical")},
                { 64,   40, "DP", QT_TRANSLATE_NOOP("Display16", "dot")},
                { 64,   56, "C",  QT_TRANSLATE_NOOP("Display16", "lower right vertical")},
                {  0,   72, "G2", QT_TRANSLATE_NOOP("Display16", "middle right horizontal")},
                {  0,   88, "H",  QT_TRANSLATE_NOOP("Display16", "upper left diagonal")},
                {  0,  104, "J",  QT_TRANSLATE_NOOP("Display16", "upper center vertical")},
                { 64,   72, "K",  QT_TRANSLATE_NOOP("Display16", "upper right diagonal")},
                { 64,   88, "L",  QT_TRANSLATE_NOOP("Display16", "lower right diagonal")},
                { 64,  104, "M",  QT_TRANSLATE_NOOP("Display16", "lower center vertical")},
                { 64,  120, "N",  QT_TRANSLATE_NOOP("Display16", "lower left diagonal")},
            };
            return DisplaySegments::input(element, segments, index);
        };
        // The segments the inputs light are drawn as the inputs are now
        descriptor.pictureFollowsInputs = true;
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
