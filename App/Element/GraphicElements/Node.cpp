// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Node.h"

#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QSvgRenderer>

#include "App/Core/Common.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicNode.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Scene.h"
#include "App/Versions.h"

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
        .hasLabel = true,
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
        meta.logicCreator = [](GraphicElement *) { return std::make_shared<LogicNode>(); };
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
    // A node's input is required (cannot be left unconnected) because its sole
    // purpose is to act as a visible junction/fan-out point on a wire.
    // In Rx mode setWirelessMode() will relax this to optional.
    inputPort()->setRequired(true);

    // The node body is 32×32; move the label just below it (instead of the
    // default 64px offset which is designed for 64×64 elements).
    m_label->setPos(0, 32);

    Node::updatePortsProperties();
}

void Node::updatePortsProperties()
{
    // Position ports at the left and right midpoints of the 32×32 node icon so
    // they align with the horizontal wire grid (y=16 is the vertical centre).
    inputPort()->setPos(  0, 16);
    outputPort()->setPos(32, 16);
}

// --- Wireless mode ---

WirelessMode Node::wirelessMode() const
{
    return m_wirelessMode;
}

bool Node::hasWirelessMode() const
{
    return true;
}

void Node::setWirelessMode(WirelessMode mode)
{
    if (m_wirelessMode == mode) {
        return;
    }
    m_wirelessMode = mode;
    // Rx nodes receive their signal wirelessly; no physical wire required on the input port.
    // Tx nodes and plain nodes still need a physical wire driving them.
    // Guard: setRequired() calls setDefaultStatus() which immediately forces the port to
    // Invalid, causing a visible red flash until the simulation runs. Skip if unchanged.
    const bool inputRequired = (mode != WirelessMode::Rx);
    if (inputPort()->isRequired() != inputRequired) {
        inputPort()->setRequired(inputRequired);
    }

    // Hide the port that is replaced by the wireless channel.
    inputPort()->setVisible(mode != WirelessMode::Rx);
    outputPort()->setVisible(mode != WirelessMode::Tx);

    if (mode == WirelessMode::None) {
        if (m_wirelessIndicator) {
            m_wirelessIndicator->setVisible(false);
        }
        return;
    }

    // Create the indicator pixmap on first use.
    if (!m_wirelessIndicator) {
        QSvgRenderer renderer(QStringLiteral(":/Components/Logic/wireless_indicator.svg"));
        QPixmap pixmap(20, 20);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        renderer.render(&painter);
        painter.end();
        m_wirelessIndicator = new QGraphicsPixmapItem(pixmap, this);
        m_wirelessIndicator->setZValue(1);
    }

    m_wirelessIndicator->setVisible(true);

    if (mode == WirelessMode::Tx) {
        // Above the output port (x=32, y=16). The dot in the 20×20 pixmap is at
        // local (≈2, ≈18); setPos(30, -6) places the dot at node (32, 12) — just
        // above the port — with arcs fanning upper-right.
        m_wirelessIndicator->setPos(30, -6);
        m_wirelessIndicator->setTransform(QTransform());
    } else {
        // Above the input port (x=0, y=16), mirrored so arcs open upper-left.
        // With scale(-1,1): parent_x = setPos.x - local_x, so setPos.x=2 puts the
        // dot at node (0, 12) — just above the port.
        m_wirelessIndicator->setPos(2, -6);
        m_wirelessIndicator->setTransform(QTransform().scale(-1, 1));
    }
}

// --- Serialization ---

void Node::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("wirelessMode", static_cast<int>(m_wirelessMode));
    stream << map;
}

void Node::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElement::load(stream, context);

    if (context.version < Versions::V_4_4) {
        return;
    }

    QMap<QString, QVariant> map;
    stream >> map;

    // Clamp out-of-range values (e.g. corrupted file) to None.
    const int raw = map.value("wirelessMode", 0).toInt();
    setWirelessMode((raw >= 0 && raw <= 2) ? static_cast<WirelessMode>(raw) : WirelessMode::None);
}
