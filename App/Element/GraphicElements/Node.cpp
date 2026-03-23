// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Node.h"

#include "App/Element/ElementInfo.h"
#include "App/Nodes/QNEPort.h"

template<>
struct ElementInfo<Node> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Node,
        .group = ElementGroup::Gate,
        .minInputSize = 1,
        .maxInputSize = 1,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeSkin = true,
        .hasLabel = false,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Logic/node.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Node", "NODE");
        meta.translatedName = QT_TRANSLATE_NOOP("Node", "Node");
        meta.trContext = "Node";
        meta.defaultSkins = QStringList({":/Components/Logic/node.svg"});
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Node(); });
        return true;
    }();
};

// Node is a wire junction / fan-out point: 1 input, 1 output (1:1 pass-through).
// Its sole logical role is to create a named connection split on the canvas.
Node::Node(QGraphicsItem *parent)
    : GraphicElement(ElementType::Node, parent)
{
    // The single input must always be connected; an undriven node would silently
    // propagate an undefined state to all downstream connections.
    inputPort()->setRequired(true);

    Node::updatePortsProperties();
}

void Node::updatePortsProperties()
{
    // Position ports at the left and right midpoints of the 32×32 node icon so
    // they align with the horizontal wire grid (y=16 is the vertical centre).
    inputPort()->setPos(  0, 16);
    outputPort()->setPos(32, 16);
}

void Node::updateLogic()
{
    if (!updateInputs()) {
        return;
    }
    setOutputValue(simInputs().at(0));
}

