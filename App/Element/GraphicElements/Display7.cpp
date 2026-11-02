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
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<ElementType::Display7> {
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
        descriptor.pixmapPath = QStringLiteral(":/Components/Output/Counter/counter_on.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("Display7", "7-SEGMENT DISPLAY");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Display7", "7-Segment Display");
        descriptor.trContext = "Display7";
        descriptor.appearanceSlots = {
            {"off", ":/Components/Output/Counter/counter_off.svg"},
            {"a", ":/Components/Output/Counter/counter_a.svg"},
            {"b", ":/Components/Output/Counter/counter_b.svg"},
            {"c", ":/Components/Output/Counter/counter_c.svg"},
            {"d", ":/Components/Output/Counter/counter_d.svg"},
            {"e", ":/Components/Output/Counter/counter_e.svg"},
            {"f", ":/Components/Output/Counter/counter_f.svg"},
            {"g", ":/Components/Output/Counter/counter_g.svg"},
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
        descriptor.inputAttributes = [](const Element &element, const int index) {
            // Ports on the left side (x=0) correspond to the left segments (G, F, E, D);
            // ports on the right side (x=64) correspond to the right segments and DP (A, B, DP, C).
            // This matches the physical pin layout used by common 7-segment display ICs.
            static constexpr DisplaySegment segments[] = {
                {  0,    8, "G",  QT_TRANSLATE_NOOP("Display7", "middle")},
                {  0,   24, "F",  QT_TRANSLATE_NOOP("Display7", "upper left")},
                {  0,   40, "E",  QT_TRANSLATE_NOOP("Display7", "lower left")},
                {  0,   56, "D",  QT_TRANSLATE_NOOP("Display7", "bottom")},
                { 64,    8, "A",  QT_TRANSLATE_NOOP("Display7", "top")},
                { 64,   24, "B",  QT_TRANSLATE_NOOP("Display7", "upper right")},
                { 64,   40, "DP", QT_TRANSLATE_NOOP("Display7", "dot")},
                { 64,   56, "C",  QT_TRANSLATE_NOOP("Display7", "lower right")},
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
