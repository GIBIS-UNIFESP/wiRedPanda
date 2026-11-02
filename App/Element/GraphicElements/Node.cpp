// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<ElementType::Node> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Node,
        .group = ElementGroup::Gate,
        .minInputSize = 1,
        .maxInputSize = 1,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeAppearance = true,
        .hasLabel = true,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.kernel = KernelKind::Node;
        descriptor.kernelParameters = {ParamId::Wireless};
        descriptor.kernelReadsLabel = true;
        descriptor.pixmapPath = QStringLiteral(":/Components/Logic/node.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("Node", "NODE");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Node", "Node");
        descriptor.trContext = "Node";
        descriptor.appearanceSlots = {{"body", ":/Components/Logic/node.svg"}};
        // A mode out of range (a corrupted file) is None. The ports follow the mode (the attributes below)
        descriptor.parameters = {
            {
                .id = ParamId::Wireless,
                .defaultValue = static_cast<int>(WirelessMode::None),
                .key = "wirelessMode",
                .range = {0.0, 2.0},
                .accept = [](const Element &, const ParamValue &value) -> std::optional<ParamValue> {
                    const int mode = std::get<int>(value);
                    return ((mode >= 0) && (mode <= 2)) ? value : ParamValue(static_cast<int>(WirelessMode::None));
                },
                .laysOutPorts = true,
            },
            // The indicator's colour, which a wireless node follows (KernelLogic); Unknown until the simulation first
            // updates it
            {.id = ParamId::Carried, .defaultValue = static_cast<int>(Status::Unknown), .repaints = true},
            // Whether the input needs a wire (the attributes below): an IC's proxy of a boundary input stands for the
            // IC's input, which needs one only for a clock (ICLoader)
            {.id = ParamId::Wired, .defaultValue = true, .laysOutPorts = true},
        };
        // The ports sit at the left and right midpoints of the 32x32 icon, on the horizontal wire grid (y=16 is the
        // vertical centre). An Rx node receives its signal wirelessly: its input needs no wire and is hidden, as a Tx
        // node's output is; a plain or a Tx node's input needs a wire, its sole purpose being a visible junction, unless
        // it is an IC's proxy that needs none (ParamId::Wired)
        descriptor.inputAttributes = [](const Element &element, const int index) -> std::optional<PortAttributes> {
            if (index != 0) {
                return std::nullopt;
            }
            const bool receives = (element.wirelessMode() == WirelessMode::Rx);
            return PortAttributes{.required = !receives && element.param<bool>(ParamId::Wired),
                                  .position = QPointF(0, 16),
                                  .visible = !receives};
        };
        descriptor.outputAttributes = [](const Element &element, const int index) -> std::optional<PortAttributes> {
            if (index != 0) {
                return std::nullopt;
            }
            return PortAttributes{.position = QPointF(32, 16), .visible = (element.wirelessMode() != WirelessMode::Tx)};
        };
        // A node's map came with its mode, in 4.4 (VersionInfo::hasWirelessMode()); an older file holds nothing
        descriptor.mapSince = QVersionNumber(4, 4);
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
