// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "node.h"

#include "common.h"
#include "globalproperties.h"
#include "qneconnection.h"
#include "qneport.h"
#include "scene.h"
#include "wirelessmanager.h"

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
    if (!m_wirelessLabel.isEmpty()) {
        if (auto *manager = getWirelessManager()) {
            manager->onNodeDestroyed(this);
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
    const QString oldWirelessLabel = m_wirelessLabel;

    // Call parent implementation to set the visual label
    GraphicElement::setLabel(trimmedLabel);

    // Update wireless label if it changed
    if (m_wirelessLabel != trimmedLabel) {
        // Validate 1-N constraint: only one source per wireless label
        if (!validateWirelessConstraint(trimmedLabel)) {
            return;
        }

        m_wirelessLabel = trimmedLabel;
        notifyWirelessManager(oldWirelessLabel, trimmedLabel);
        emit wirelessLabelChanged(oldWirelessLabel, trimmedLabel);
        updatePortVisibility(); // Update port visibility when wireless label changes
        update();
    }
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
    // Call parent implementation first
    QVariant result = GraphicElement::itemChange(change, value);

    // When node is added to a scene, notify wireless manager to rebuild connections
    if (change == ItemSceneHasChanged && scene() && !m_wirelessLabel.isEmpty()) {
        notifyWirelessManager(QString(), m_wirelessLabel);
        updatePortVisibility(); // Update port visibility when added to scene
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

bool Node::hasPhysicalConnection(QNEPort *port) const
{
    if (!port) {
        return false;
    }

    for (auto *conn : port->connections()) {
        if (!conn->isWireless()) {
            return true;
        }
    }
    return false;
}

bool Node::hasInputConnection() const
{
    return inputPort() && !inputPort()->connections().isEmpty();
}

bool Node::hasOutputConnection() const
{
    return outputPort() && !outputPort()->connections().isEmpty();
}

bool Node::hasPhysicalInputConnection() const
{
    return hasPhysicalConnection(inputPort());
}

bool Node::hasPhysicalOutputConnection() const
{
    return hasPhysicalConnection(outputPort());
}

bool Node::isWirelessSource() const
{
    return hasPhysicalInputConnection() && hasWirelessLabel();
}

bool Node::isWirelessSink() const
{
    return !hasPhysicalInputConnection() && hasWirelessLabel();
}

WirelessManager* Node::getWirelessManager() const
{
    if (auto *s = qobject_cast<Scene *>(scene())) {
        return s->wirelessManager();
    }
    return nullptr;
}

void Node::notifyWirelessManager(const QString &oldLabel, const QString &newLabel)
{
    if (auto *manager = getWirelessManager()) {
        manager->onNodeLabelChanged(this, oldLabel, newLabel);
    }
}

bool Node::validateWirelessConstraint(const QString &label) const
{
    if (label.isEmpty() || !hasPhysicalInputConnection() || !scene()) {
        return true;
    }

    auto *manager = getWirelessManager();
    if (!manager) {
        return true;
    }

    Node *existingSource = manager->findWirelessSource(label);
    return !existingSource || existingSource == this;
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
            // Draw WiFi bars close to the right edge of the node (output port is invisible)
            drawWirelessBars(painter, QPointF(20, 16), true);
        } else if (isWirelessSink()) {
            // Draw WiFi bars close to the left edge of the node (input port is invisible)
            drawWirelessBars(painter, QPointF(12, 16), false);
        }

        painter->restore();
    }
}

void Node::drawWirelessBars(QPainter *painter, const QPointF &position, bool pointingRight) const
{
    // Get the wireless indicator path and draw it
    QPainterPath path = getWirelessIndicatorPath(position, pointingRight);

    // Draw with a thicker pen for visibility
    QPen arcPen = painter->pen();
    arcPen.setWidth(3);
    painter->setPen(arcPen);

    painter->drawPath(path);
}

QRectF Node::boundingRect() const
{
    // Get the base bounding rect from parent
    QRectF baseRect = GraphicElement::boundingRect();

    // Extend the bounding rect to include wireless indicators if present
    if (hasWirelessLabel()) {
        QPainterPath wirelessPath;

        if (isWirelessSource()) {
            // Get path for source indicators (pointing right)
            wirelessPath = getWirelessIndicatorPath(QPointF(20, 16), true);
        } else if (isWirelessSink()) {
            // Get path for sink indicators (pointing left)
            wirelessPath = getWirelessIndicatorPath(QPointF(12, 16), false);
        }

        // Account for pen width in the bounding rect
        QPainterPathStroker stroker;
        stroker.setWidth(3); // Same as the pen width used in drawWirelessBars
        QPainterPath strokedPath = stroker.createStroke(wirelessPath);

        // Unite the base rect with the wireless indicator bounds
        return baseRect.united(strokedPath.boundingRect());
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

void Node::updatePortVisibility()
{
    // Update port visibility based on wireless status
    if (hasWirelessLabel()) {
        if (isWirelessSource()) {
            // Source nodes: hide output port (they transmit wirelessly)
            // but keep input port visible (they receive physical signals)
            inputPort()->setVisible(true);
            outputPort()->setVisible(false);
        } else if (isWirelessSink()) {
            // Sink nodes: hide input port (they receive wirelessly)
            // but keep output port visible (they can forward signals physically)
            inputPort()->setVisible(false);
            outputPort()->setVisible(true);
        }
    } else {
        // Non-wireless nodes: show both ports
        inputPort()->setVisible(true);
        outputPort()->setVisible(true);
    }
}

QPainterPath Node::getWirelessIndicatorPath(const QPointF &position, bool pointingRight) const
{
    QPainterPath path;
    const int numArcs = 3;
    const int baseRadius = 12;
    const int radiusIncrement = 9;
    const int arcSpan = 60; // degrees

    for (int i = 0; i < numArcs; ++i) {
        int radius = baseRadius + (i * radiusIncrement);

        // Calculate the rectangle that defines the arc
        QRectF arcRect(
            position.x() - radius, position.y() - radius,
            2 * radius, 2 * radius
        );

        // Calculate start angle based on direction
        int startAngle = pointingRight ? -30 * 16 : 150 * 16; // Qt uses 16ths of a degree
        int spanAngle = arcSpan * 16;

        // Add the arc to the path
        path.arcMoveTo(arcRect, startAngle / 16.0);
        path.arcTo(arcRect, startAngle / 16.0, spanAngle / 16.0);
    }

    return path;
}
