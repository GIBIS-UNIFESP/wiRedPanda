// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "node.h"

#include "common.h"
#include "globalproperties.h"
#include "qneconnection.h"
#include "qneport.h"
#include "scene.h"
#include "wirelessconnectionmanager.h"

#include <QDataStream>
#include <QPainter>
#include "thememanager.h"

// WirelessNodeOutputPort implementation
WirelessNodeOutputPort::WirelessNodeOutputPort(QGraphicsItem *parent)
    : QNEOutputPort(parent)
{
}

void WirelessNodeOutputPort::setStatus(const Status status)
{
    // Call parent implementation first
    QNEOutputPort::setStatus(status);

    // Notify parent Node to update wireless indicators
    if (auto* node = dynamic_cast<Node*>(graphicElement())) {
        if (node->hasWirelessLabel()) {
            node->update();
        }
    }
}

Node::Node(QGraphicsItem *parent)
    : GraphicElement(ElementType::Node, ElementGroup::Gate, ":/components/logic/node.svg", tr("NODE"), tr("Node"), 1, 1, 1, 1, parent)
    , m_wirelessLabel()
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    m_label->setPos(0, 32);

    setCanChangeSkin(true);
    setHasLabel(true);
    inputPort()->setRequired(true);

    // Replace the default output port with our custom wireless-aware port
    delete m_outputPorts[0];
    m_outputPorts[0] = new WirelessNodeOutputPort(this);
    m_outputPorts[0]->setName("out");
    m_outputPorts[0]->setGraphicElement(this);
    m_outputPorts[0]->setIndex(0);

    Node::updatePortsProperties();
}

Node::~Node()
{
    // Notify wireless manager that this node is being destroyed
    if (!m_wirelessLabel.isEmpty() && scene()) {
        if (auto *s = qobject_cast<Scene *>(scene())) {
            if (auto *manager = s->wirelessManager()) {
                manager->onNodeDestroyed(this);
            }
        }
    }
}

void Node::updatePortsProperties()
{
    inputPort()->setPos(  0, 16);
    outputPort()->setPos(32, 16);
}

void Node::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("wirelessLabel", m_wirelessLabel);

    stream << map;
}

void Node::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    GraphicElement::load(stream, portMap, version);

    if (version >= VERSION("4.4")) {
        QMap<QString, QVariant> map;
        stream >> map;

        if (map.contains("wirelessLabel")) {
            m_wirelessLabel = map.value("wirelessLabel").toString();

            // Note: Wireless connection will be restored later when node is added to scene
            // The scene() is null during deserialization, so we defer restoration
        }
    }
}

void Node::setLabel(const QString &label)
{
    const QString trimmedLabel = label.trimmed();

    // Store the old wireless label for change notification
    const QString oldWirelessLabel = m_wirelessLabel;

    // Call parent implementation to set the visual label
    GraphicElement::setLabel(trimmedLabel);

    // Update wireless label if it changed
    if (m_wirelessLabel != trimmedLabel) {
        // Validate 1-N constraint: only one source per wireless label
        if (!trimmedLabel.isEmpty() && hasPhysicalInputConnection() && scene()) {
            if (auto *s = qobject_cast<Scene *>(scene())) {
                if (auto *manager = s->wirelessManager()) {
                    // Check if there's already another source with this label
                    // Access the internal method directly since this is the only legitimate use case
                    Node *existingSource = manager->findWirelessSource(trimmedLabel);
                    if (existingSource && existingSource != this) {
                        // Don't update the label - constraint violation
                        return;
                    }
                }
            }
        }

        m_wirelessLabel = trimmedLabel;

        // Notify the scene's wireless auto manager
        if (scene()) {
            if (auto *s = qobject_cast<Scene *>(scene())) {
                if (s->wirelessManager()) {
                    s->wirelessManager()->onNodeLabelChanged(this, oldWirelessLabel, trimmedLabel);
                }
            }
        }

        emit wirelessLabelChanged(oldWirelessLabel, trimmedLabel);

        // Trigger a repaint to show/hide wireless indicators
        update();
    }
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
    // Call parent implementation first
    QVariant result = GraphicElement::itemChange(change, value);

    // When node is added to a scene, notify auto manager to rebuild connections
    if (change == ItemSceneHasChanged && scene() && !m_wirelessLabel.isEmpty()) {
        if (auto *s = qobject_cast<Scene *>(scene())) {
            if (s->wirelessManager()) {
                s->wirelessManager()->onNodeLabelChanged(this, QString(), m_wirelessLabel);
            }
        }
    }

    return result;
}

QString Node::getWirelessLabel() const
{
    return m_wirelessLabel;
}

bool Node::hasWirelessLabel() const
{
    return !m_wirelessLabel.isEmpty();
}

bool Node::hasConnection(QNEPort *port, ConnectionType type) const
{
    // Cast away const to access ports (they don't modify object state for our purposes)
    if (!port) {
        return false;
    }

    const auto& connections = port->connections();
    if (connections.isEmpty()) {
        return false;
    }

    switch (type) {
    case ConnectionType::Any:
        return true; // Has connections, type doesn't matter

    case ConnectionType::Physical:
        // Check each connection to see if it's a physical (non-wireless) connection
        for (auto *conn : connections) {
            // Skip wireless connections - only count regular QNEConnection objects
            if (!conn->isWireless()) {
                return true; // Found a physical connection
            }
        }
        return false; // Only wireless connections

    case ConnectionType::Wireless:
        // Check each connection to see if it's a wireless connection
        for (auto *conn : connections) {
            if (conn->isWireless()) {
                return true; // Found a wireless connection
            }
        }
        return false; // Only physical connections
    }

    return false;
}

bool Node::hasInputConnection() const
{
    return hasConnection(inputPort(), ConnectionType::Any);
}

bool Node::hasOutputConnection() const
{
    return hasConnection(outputPort(), ConnectionType::Any);
}

bool Node::hasPhysicalInputConnection() const
{
    return hasConnection(inputPort(), ConnectionType::Physical);
}

bool Node::hasPhysicalOutputConnection() const
{
    return hasConnection(outputPort(), ConnectionType::Physical);
}

bool Node::isWirelessSource() const
{
    // Node can be wireless source if it has PHYSICAL input connection AND wireless label
    // (receives signal physically, transmits wirelessly)
    return hasPhysicalInputConnection() && hasWirelessLabel();
}

bool Node::isWirelessSink() const
{
    // Node is wireless sink if it has wireless label but NO PHYSICAL input connection
    // (only receives wireless signals)
    return !hasPhysicalInputConnection() && hasWirelessLabel();
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Call parent implementation to draw the normal node
    GraphicElement::paint(painter, option, widget);

    // Draw wireless signal indicators if this node has wireless functionality
    if (hasWirelessLabel()) {
        painter->save();

        // Use the same color as connections based on the node's current status
        const ThemeAttributes theme = ThemeManager::attributes();
        QColor wirelessColor;

        // Get the current signal status from the output port
        Status currentStatus = Status::Invalid;
        if (outputPort()) {
            currentStatus = outputPort()->status();
        }

        switch (currentStatus) {
        case Status::Active:
            wirelessColor = theme.m_connectionActive;
            break;
        case Status::Inactive:
            wirelessColor = theme.m_connectionInactive;
            break;
        default:
            wirelessColor = theme.m_connectionInvalid;
            break;
        }

        painter->setPen(QPen(wirelessColor, 2));
        painter->setBrush(QBrush(wirelessColor));

        if (isWirelessSource()) {
            // Draw WiFi bars to the right of the output port (pointing right - outgoing signal)
            drawWirelessBars(painter, QPointF(40, 16), true);
        } else if (isWirelessSink()) {
            // Draw WiFi bars to the left of the input port (pointing left - incoming signal)
            drawWirelessBars(painter, QPointF(-8, 16), false);
        }

        painter->restore();
    }
}

void Node::drawWirelessBars(QPainter *painter, const QPointF &position, bool pointingRight) const
{
    // Draw 3 curved WiFi signal arcs of increasing radius
    const int numArcs = 3;
    const int baseRadius = 12;  // Tripled from 4
    const int radiusIncrement = 9;  // Tripled from 3
    const int arcSpan = 60; // degrees

    for (int i = 0; i < numArcs; ++i) {
        int radius = baseRadius + (i * radiusIncrement); // Increasing radius: 12, 21, 30

        QRectF arcRect;
        int startAngle, spanAngle;

        if (pointingRight) {
            // Arcs pointing right (for source/transmitter) - opening to the right
            arcRect = QRectF(position.x() - radius, position.y() - radius, radius * 2, radius * 2);
            startAngle = (0 - arcSpan/2) * 16; // Point right (0°) with span
            spanAngle = arcSpan * 16; // Span angle in 1/16th degrees
        } else {
            // Arcs pointing left (for sink/receiver) - opening to the left
            arcRect = QRectF(position.x() - radius, position.y() - radius, radius * 2, radius * 2);
            startAngle = (180 - arcSpan/2) * 16; // Point left (180°) with span
            spanAngle = arcSpan * 16;
        }

        // Draw the arc with a thicker pen for visibility
        QPen arcPen = painter->pen();
        arcPen.setWidth(3);  // Made thicker
        painter->setPen(arcPen);
        painter->drawArc(arcRect, startAngle, spanAngle);
    }
}

QRectF Node::boundingRect() const
{
    // Get the base bounding rect from parent
    QRectF baseRect = GraphicElement::boundingRect();

    // Extend the bounding rect to include wireless indicators if present
    if (hasWirelessLabel()) {
        QRectF extendedRect = baseRect;

        if (isWirelessSource()) {
            // Extend to the right for source indicators (largest arc radius = 30, plus pen width)
            extendedRect.setRight(std::max(baseRect.right(), 75.0)); // 40 + 30 + 5 pixels for largest arc + pen
            // Also extend vertically for arc height
            extendedRect.setTop(std::min(baseRect.top(), -14.0)); // 16 - 30 pixels
            extendedRect.setBottom(std::max(baseRect.bottom(), 46.0)); // 16 + 30 pixels
        } else if (isWirelessSink()) {
            // Extend to the left for sink indicators (largest arc radius = 30, plus pen width)
            extendedRect.setLeft(std::min(baseRect.left(), -43.0)); // -8 - 30 - 5 pixels for largest arc + pen
            // Also extend vertically for arc height
            extendedRect.setTop(std::min(baseRect.top(), -14.0)); // 16 - 30 pixels
            extendedRect.setBottom(std::max(baseRect.bottom(), 46.0)); // 16 + 30 pixels
        }

        return extendedRect;
    }

    return baseRect;
}

void Node::refresh()
{
    // Call parent implementation
    GraphicElement::refresh();

    // Trigger repaint to update wireless indicator colors if this node has wireless functionality
    if (hasWirelessLabel()) {
        update();
    }
}
