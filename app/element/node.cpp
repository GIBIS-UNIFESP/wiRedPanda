// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "node.h"

#include "common.h"
#include "globalproperties.h"
#include "nodes/physicalconnection.h"  
#include "nodes/wirelessconnection.h"
#include "qneport.h"
#include "scene.h"
#include "wirelessconnectionautomanager.h"
#include "wirelessconnectionmanager.h"

#include <QDataStream>

Node::Node(QGraphicsItem *parent)
    : GraphicElement(ElementType::Node, ElementGroup::Gate, ":/basic/node.svg", tr("NODE"), tr("Node"), 1, 1, 1, 1, parent)
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

    Node::updatePortsProperties();
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

    if (version >= VERSION("4.3")) {
        QMap<QString, QVariant> map;
        stream >> map;

        if (map.contains("wirelessLabel")) {
            m_wirelessLabel = map.value("wirelessLabel").toString();
            qCDebug(zero) << "Node" << id() << "loaded wireless label:" << m_wirelessLabel;

            // Note: Wireless connection will be restored later when node is added to scene
            // The scene() is null during deserialization, so we defer restoration
        }
    }
}

void Node::setLabel(const QString &label)
{
    const QString trimmedLabel = label.trimmed();
    
    qDebug() << "Node::setLabel called on node" << id() << "with label" << trimmedLabel;

    // Store the old wireless label for change notification
    const QString oldWirelessLabel = m_wirelessLabel;
    qDebug() << "Old wireless label was:" << oldWirelessLabel;

    // Call parent implementation to set the visual label
    GraphicElement::setLabel(trimmedLabel);

    // Update wireless label if it changed
    if (m_wirelessLabel != trimmedLabel) {
        m_wirelessLabel = trimmedLabel;

        // Notify the scene's wireless auto manager
        if (scene()) {
            qDebug() << "Node has scene, checking for auto manager";
            if (auto *s = qobject_cast<Scene *>(scene())) {
                qDebug() << "Scene cast successful";
                if (s->wirelessAutoManager()) {
                    qDebug() << "Calling auto manager onNodeLabelChanged";
                    s->wirelessAutoManager()->onNodeLabelChanged(this, oldWirelessLabel, trimmedLabel);
                } else {
                    qDebug() << "No wireless auto manager found!";
                }
            } else {
                qDebug() << "Scene cast failed!";
            }
        } else {
            qDebug() << "Node has no scene!";
        }

        emit wirelessLabelChanged(oldWirelessLabel, trimmedLabel);
        qCDebug(zero) << "Node" << id() << "wireless label changed from" << oldWirelessLabel << "to" << trimmedLabel;
    }
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
    // Call parent implementation first
    QVariant result = GraphicElement::itemChange(change, value);

    // When node is added to a scene, notify auto manager to rebuild connections  
    if (change == ItemSceneHasChanged && scene() && !m_wirelessLabel.isEmpty()) {
        if (auto *s = qobject_cast<Scene *>(scene())) {
            if (s->wirelessAutoManager()) {
                qCDebug(zero) << "Auto-registering node" << id() << "with wireless label" << m_wirelessLabel;
                s->wirelessAutoManager()->onNodeLabelChanged(this, QString(), m_wirelessLabel);
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

bool Node::hasInputConnection() const
{
    // Cast away const to access ports (they don't modify object state for our purposes)
    auto *self = const_cast<Node*>(this);
    return self->inputPort() && !self->inputPort()->connections().isEmpty();
}

bool Node::hasOutputConnection() const
{
    // Cast away const to access ports (they don't modify object state for our purposes)
    auto *self = const_cast<Node*>(this);
    return self->outputPort() && !self->outputPort()->connections().isEmpty();
}

bool Node::hasPhysicalInputConnection() const
{
    // Cast away const to access ports 
    auto *self = const_cast<Node*>(this);
    if (!self->inputPort()) {
        return false;
    }
    
    // Check each connection to see if it's a PhysicalConnection or legacy QNEConnection
    for (auto* conn : self->inputPort()->connections()) {
        if (qgraphicsitem_cast<PhysicalConnection*>(conn)) {
            return true; // Found a physical connection
        }
        // Also support legacy QNEConnection objects (for backward compatibility and tests)
        if (qgraphicsitem_cast<QNEConnection*>(conn) && 
            !qgraphicsitem_cast<WirelessConnection*>(conn)) {
            return true; // Found a legacy physical connection (not wireless)
        }
    }
    
    return false; // Only wireless connections or no connections
}

bool Node::hasPhysicalOutputConnection() const
{
    // Cast away const to access ports
    auto *self = const_cast<Node*>(this);
    if (!self->outputPort()) {
        return false;
    }
    
    // Check each connection to see if it's a PhysicalConnection or legacy QNEConnection
    for (auto* conn : self->outputPort()->connections()) {
        if (qgraphicsitem_cast<PhysicalConnection*>(conn)) {
            return true; // Found a physical connection
        }
        // Also support legacy QNEConnection objects (for backward compatibility and tests)
        if (qgraphicsitem_cast<QNEConnection*>(conn) && 
            !qgraphicsitem_cast<WirelessConnection*>(conn)) {
            return true; // Found a legacy physical connection (not wireless)
        }
    }
    
    return false; // Only wireless connections or no connections
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
