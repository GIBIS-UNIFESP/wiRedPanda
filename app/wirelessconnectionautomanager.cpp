// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wirelessconnectionautomanager.h"

#include "common.h"
#include "element/node.h"
#include "nodes/wirelessconnection.h"
#include "scene.h"

#include <QDebug>

WirelessConnectionAutoManager::WirelessConnectionAutoManager(Scene* scene, QObject* parent)
    : QObject(parent), m_scene(scene)
{
    if (!m_scene) {
        qCWarning(zero) << "WirelessConnectionAutoManager created with null scene!";
    }
}

void WirelessConnectionAutoManager::onNodeLabelChanged(Node* node, const QString& oldLabel, const QString& newLabel)
{
    if (!node || !m_scene) {
        qCWarning(zero) << "WirelessConnectionAutoManager::onNodeLabelChanged - invalid node or scene";
        return;
    }
    
    qDebug() << "WirelessConnectionAutoManager: Node" << node->id() 
             << "label changed from" << oldLabel << "to" << newLabel;
    
    // Rebuild connections for both old and new labels
    if (!oldLabel.isEmpty()) {
        rebuildConnectionsForLabel(oldLabel);
    }
    
    if (!newLabel.isEmpty()) {
        rebuildConnectionsForLabel(newLabel);
    }
    
    // Clean up any empty label groups
    cleanupEmptyGroups();
}

void WirelessConnectionAutoManager::onNodeDestroyed(Node* node)
{
    if (!node) {
        return;
    }
    
    qCDebug(zero) << "WirelessConnectionAutoManager: Node" << node->id() << "destroyed";
    
    // Get the node's wireless label before destruction
    QString label = node->getWirelessLabel();
    if (!label.isEmpty()) {
        // Rebuild connections for this label after node removal
        rebuildConnectionsForLabel(label);
    }
    
    // Clean up any empty label groups
    cleanupEmptyGroups();
}

QStringList WirelessConnectionAutoManager::getActiveLabels() const
{
    return m_wirelessConnections.keys();
}

QList<WirelessConnection*> WirelessConnectionAutoManager::getWirelessConnectionsForLabel(const QString& label) const
{
    return m_wirelessConnections.value(label);
}

void WirelessConnectionAutoManager::rebuildConnectionsForLabel(const QString& label)
{
    if (label.isEmpty() || !m_scene) {
        qDebug() << "WirelessConnectionAutoManager::rebuildConnectionsForLabel - empty label or no scene";
        return;
    }
    
    qDebug() << "WirelessConnectionAutoManager: Rebuilding connections for label" << label;
    
    // Remove existing wireless connections for this label
    destroyWirelessConnectionsForLabel(label);
    
    // Find nodes with this label
    Node* source = findWirelessSource(label);
    QSet<Node*> sinks = findWirelessSinks(label);
    
    qDebug() << "Found source:" << source << "and" << sinks.size() << "sinks for label" << label;
    
    if (source) {
        qDebug() << "Source node" << source->id() << "has physical input connection:" << source->hasPhysicalInputConnection();
    }
    
    for (Node* sink : sinks) {
        qDebug() << "Sink node" << sink->id() << "has physical input connection:" << sink->hasPhysicalInputConnection();
    }
    
    if (!source || sinks.isEmpty()) {
        qDebug() << "Incomplete wireless connection for label" << label << "- skipping";
        return; // No complete wireless connection (need source AND sinks)
    }
    
    // Create WirelessConnection from source to each sink
    for (Node* sink : sinks) {
        createWirelessConnection(source, sink, label);
    }
    
    qDebug() << "Created" << m_wirelessConnections[label].size() << "wireless connections for label" << label;
}

Node* WirelessConnectionAutoManager::findWirelessSource(const QString& label)
{
    if (!m_scene) {
        qDebug() << "WirelessConnectionAutoManager::findWirelessSource - no scene";
        return nullptr;
    }
    
    qDebug() << "WirelessConnectionAutoManager: Finding wireless source for label" << label;
    
    // Find all nodes in the scene with this wireless label that have physical input connections
    for (auto* item : m_scene->items()) {
        if (auto* node = qgraphicsitem_cast<Node*>(item)) {
            qDebug() << "Checking node" << node->id() << "with label" << node->getWirelessLabel() 
                     << "has physical input:" << node->hasPhysicalInputConnection();
            if (node->getWirelessLabel() == label && node->hasPhysicalInputConnection()) {
                qDebug() << "Found wireless source:" << node->id();
                return node; // This node is a wireless source
            }
        }
    }
    
    qDebug() << "No wireless source found for label" << label;
    return nullptr; // No wireless source found
}

QSet<Node*> WirelessConnectionAutoManager::findWirelessSinks(const QString& label)
{
    QSet<Node*> sinks;
    
    if (!m_scene) {
        qDebug() << "WirelessConnectionAutoManager::findWirelessSinks - no scene";
        return sinks;
    }
    
    qDebug() << "WirelessConnectionAutoManager: Finding wireless sinks for label" << label;
    
    // Find all nodes in the scene with this wireless label that have NO physical input connections
    for (auto* item : m_scene->items()) {
        if (auto* node = qgraphicsitem_cast<Node*>(item)) {
            qDebug() << "Checking sink node" << node->id() << "with label" << node->getWirelessLabel() 
                     << "has physical input:" << node->hasPhysicalInputConnection();
            if (node->getWirelessLabel() == label && !node->hasPhysicalInputConnection()) {
                qDebug() << "Found wireless sink:" << node->id();
                sinks.insert(node); // This node is a wireless sink
            }
        }
    }
    
    qDebug() << "Found" << sinks.size() << "wireless sinks for label" << label;
    return sinks;
}

void WirelessConnectionAutoManager::createWirelessConnection(Node* source, Node* sink, const QString& label)
{
    if (!source || !sink || !m_scene || label.isEmpty()) {
        qDebug() << "WirelessConnectionAutoManager::createWirelessConnection - invalid parameters";
        return;
    }
    
    qDebug() << "Creating wireless connection from" << source->id() << "to" << sink->id() << "for label" << label;
    
    // Create the wireless connection
    auto* connection = new WirelessConnection(label);
    qDebug() << "Created WirelessConnection object:" << connection;
    
    connection->setStartPort(source->outputPort());
    qDebug() << "Set start port:" << source->outputPort();
    
    connection->setEndPort(sink->inputPort());
    qDebug() << "Set end port:" << sink->inputPort();
    
    // Add to scene
    m_scene->addItem(connection);
    qDebug() << "Added wireless connection to scene";
    
    // Track the connection for management
    m_wirelessConnections[label].append(connection);
    
    qDebug() << "Successfully created wireless connection" << connection << "for label" << label;
    qDebug() << "Total wireless connections for label" << label << ":" << m_wirelessConnections[label].size();
}

void WirelessConnectionAutoManager::destroyWirelessConnectionsForLabel(const QString& label)
{
    if (label.isEmpty()) {
        qDebug() << "WirelessConnectionAutoManager::destroyWirelessConnectionsForLabel - empty label";
        return;
    }
    
    auto connections = m_wirelessConnections.take(label);
    qDebug() << "Destroying" << connections.size() << "wireless connections for label" << label;
    
    for (auto* connection : connections) {
        if (connection && m_scene) {
            qDebug() << "Removing wireless connection" << connection << "from scene";
            m_scene->removeItem(connection);
            delete connection;
        }
    }
    
    qDebug() << "Finished destroying wireless connections for label" << label;
}

void WirelessConnectionAutoManager::cleanupEmptyGroups()
{
    QStringList emptyLabels;
    
    // Find labels with no connections
    for (auto it = m_wirelessConnections.begin(); it != m_wirelessConnections.end(); ++it) {
        if (it.value().isEmpty()) {
            emptyLabels.append(it.key());
        }
    }
    
    // Remove empty labels
    for (const QString& label : emptyLabels) {
        qCDebug(zero) << "Cleaning up empty wireless label group:" << label;
        m_wirelessConnections.remove(label);
    }
}