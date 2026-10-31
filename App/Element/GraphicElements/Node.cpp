// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Node.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<Node> {
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
        descriptor.pixmapPath = QStringLiteral(":/Components/Logic/node.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("Node", "NODE");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Node", "Node");
        descriptor.trContext = "Node";
        descriptor.defaultAppearances = QStringList({":/Components/Logic/node.svg"});
        // A mode out of range (a corrupted file) is None
        descriptor.parameters = {
            {
                .id = ParamId::Wireless,
                .defaultValue = static_cast<int>(WirelessMode::None),
                .key = "wirelessMode",
                .accept = [](const Element &, const ParamValue &value) -> std::optional<ParamValue> {
                    const int mode = std::get<int>(value);
                    return ((mode >= 0) && (mode <= 2)) ? value : ParamValue(static_cast<int>(WirelessMode::None));
                },
            },
            // The indicator's colour, which a wireless node follows; Unknown until the simulation first updates it
            {.id = ParamId::Carried, .defaultValue = static_cast<int>(Status::Unknown)},
        };
        // A node's map came with its mode, in 4.4 (VersionInfo::hasWirelessMode()); an older file holds nothing
        descriptor.mapSince = QVersionNumber(4, 4);
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Node(); });
        return true;
    }();
};

// Node is a wire junction / fan-out point: 1 input, 1 output (1:1 pass-through).
// Its sole logical role is to create a named connection split on the canvas.
Node::Node(QGraphicsItem *parent)
    : Element(ElementType::Node, parent)
{
    // A node's input is required (cannot be left unconnected) because its sole
    // purpose is to act as a visible junction/fan-out point on a wire.
    // In Rx mode parameterChanged() will relax this to optional.
    inputPort()->setRequired(true);

    updatePortsProperties();
}

std::optional<Element::PortAttributes> Node::inputAttributes(const int index) const
{
    // Ports sit at the left and right midpoints of the 32×32 node icon so they align with the horizontal wire grid
    // (y=16 is the vertical centre). The requirement is left alone: the wireless mode sets it.
    return attributesAt({{.position = QPointF(0, 16)}}, index);
}

std::optional<Element::PortAttributes> Node::outputAttributes(const int index) const
{
    return attributesAt({{.position = QPointF(32, 16)}}, index);
}

void Node::updateLogic()
{
    if (!simUpdateInputsAllowUnknown()) {
        carry(Status::Unknown);
        return;
    }
    const auto status = simInputs().at(0);
    setOutputValue(status);
    carry(status);
}

// --- Wireless mode ---

void Node::parameterChanged(const ParamId id, const ParamValue &previous)
{
    Element::parameterChanged(id, previous);
    if (id == ParamId::Carried) {
        pictureChanged();
        return;
    }

    const WirelessMode mode = wirelessMode();
    // Rx nodes receive their signal wirelessly; no physical wire required on the input port.
    // Tx nodes and plain nodes still need a physical wire driving them.
    // Guard against a redundant setRequired() call re-deriving the port status
    // and causing a visible status flash until the simulation runs. Skip if unchanged.
    const bool inputRequired = (mode != WirelessMode::Rx);
    if (inputPort()->isRequired() != inputRequired) {
        inputPort()->setRequired(inputRequired);
    }

    // Hide the port that is replaced by the wireless channel.
    inputPort()->setVisible(mode != WirelessMode::Rx);
    outputPort()->setVisible(mode != WirelessMode::Tx);
    // The ports and the indicator the body draws beyond them (NodePresenter) are measured again as one change
    changed(ElementChange::Ports);
}

// --- Wireless indicator ---

void Node::carry(const Status status)
{
    if (wirelessMode() != WirelessMode::None) {
        setParam(ParamId::Carried, static_cast<int>(status));
    }
}
