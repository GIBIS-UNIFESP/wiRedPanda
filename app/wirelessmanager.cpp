// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wirelessmanager.h"

#include "common.h"
#include "node.h"
#include "qneconnection.h"
#include "scene.h"

#include <QDebug>

WirelessManager::WirelessManager(Scene *scene, QObject *parent)
    : QObject(parent), m_scene(scene)
{
    if (!m_scene) {
        qCWarning(zero) << "WirelessManager created with null scene!";
    }
}

void WirelessManager::onNodeLabelChanged(Node *node, const QString &oldLabel, const QString &newLabel)
{
    if (!node || !m_scene) {
        qCWarning(zero) << "WirelessManager::onNodeLabelChanged - invalid node or scene";
        return;
    }

    // Check if updates are blocked (e.g., during SplitCommand execution)
    if (m_updatesBlocked) {
        qDebug() << "[WIRELESS_DEBUG] Updates blocked, deferring wireless connection rebuild";
        return;
    }

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

void WirelessManager::onNodeDestroyed(Node *node)
{
    if (!node) {
        return;
    }

    qCDebug(zero) << "WirelessManager: Node" << node->id() << "destroyed";

    // Get the node's wireless label before destruction
    QString label = node->getWirelessLabel();
    if (!label.isEmpty()) {
        // Rebuild connections for this label after node removal
        rebuildConnectionsForLabel(label);
    }

    // Clean up any empty label groups
    cleanupEmptyGroups();
}

QStringList WirelessManager::getActiveLabels() const
{
    if (!m_scene) {
        return QStringList();
    }

    // Return all unique wireless labels that have at least one node
    QSet<QString> uniqueLabels;
    for (auto *node : getAllNodesInScene()) {
        QString label = node->getWirelessLabel();
        if (!label.isEmpty()) {
            uniqueLabels.insert(label);
        }
    }

    return uniqueLabels.values();
}

QList<QNEConnection *> WirelessManager::getWirelessConnectionsForLabel(const QString &label) const
{
    return m_wirelessConnections.value(label);
}

QSet<Node *> WirelessManager::getWirelessGroup(const QString &label) const
{
    QSet<Node *> group;

    if (!m_scene || label.isEmpty()) {
        return group;
    }

    // Find all nodes in the scene with this wireless label
    auto nodes = getNodesWithLabel(label);
    for (auto *node : nodes) {
        group.insert(node);
    }

    return group;
}

int WirelessManager::getGroupCount() const
{
    if (!m_scene) {
        return 0;
    }

    // Count unique wireless labels that have at least one node
    QSet<QString> uniqueLabels;
    for (auto *node : getAllNodesInScene()) {
        QString label = node->getWirelessLabel();
        if (!label.isEmpty()) {
            uniqueLabels.insert(label);
        }
    }

    return uniqueLabels.size();
}

QSet<Node *> WirelessManager::getConnectedNodes(Node *node) const
{
    QSet<Node *> connected;

    if (!node || node->getWirelessLabel().isEmpty()) {
        return connected;
    }

    // Get all nodes with the same wireless label
    const QString label = node->getWirelessLabel();
    auto nodes = getNodesWithLabel(label);
    for (auto *otherNode : nodes) {
        if (otherNode != node) {
            connected.insert(otherNode);
        }
    }

    return connected;
}

bool WirelessManager::hasWirelessConnections() const
{
    return !m_wirelessConnections.isEmpty();
}

void WirelessManager::serialize(QDataStream &stream) const
{
    if (!m_scene) {
        stream << QMap<int, QString>();
        return;
    }

    // Serialize node ID to wireless label mappings
    QMap<int, QString> nodeLabels;
    for (auto *node : getAllNodesInScene()) {
        QString label = node->getWirelessLabel();
        if (!label.isEmpty()) {
            nodeLabels[node->id()] = label;
        }
    }

    stream << nodeLabels;
}

void WirelessManager::rebuildConnectionsForLabel(const QString &label)
{
    if (label.isEmpty() || !m_scene) {
        return;
    }

    // Remove existing wireless connections for this label
    destroyWirelessConnectionsForLabel(label);

    // Find nodes with this label
    Node *source = findWirelessSource(label);
    QSet<Node *> sinks = findWirelessSinks(label);

    if (!source || sinks.isEmpty()) {
        return; // No complete wireless connection (need source AND sinks)
    }

    // Create WirelessConnection from source to each sink
    for (Node *sink : sinks) {
        createWirelessConnection(source, sink, label);
    }


    // Emit signal to notify that wireless connections have changed
    emit wirelessConnectionsChanged();
}

Node *WirelessManager::findWirelessSource(const QString &label) const
{
    if (!m_scene) {
        return nullptr;
    }


    // Find all nodes in the scene with this wireless label that have physical input connections
    auto nodes = getNodesWithLabel(label);
    for (auto *node : nodes) {
        // Check if node has physical input connection (making it a source)
        if (node->hasPhysicalInputConnection()) {
            return node; // This node is a wireless source
        }
    }

    return nullptr; // No wireless source found
}

QSet<Node *> WirelessManager::findWirelessSinks(const QString &label) const
{
    QSet<Node *> sinks;

    if (!m_scene) {
        return sinks;
    }


    // Find all nodes in the scene with this wireless label that have NO physical input connections
    auto nodes = getNodesWithLabel(label);
    for (auto *node : nodes) {
        // Check if node lacks physical input (making it a sink)
        if (!node->hasPhysicalInputConnection()) {
            sinks.insert(node); // This node is a wireless sink
        }
    }

    return sinks;
}

void WirelessManager::createWirelessConnection(Node *source, Node *sink, const QString &label)
{
    if (!source || !sink || !m_scene || label.isEmpty()) {
        return;
    }


    // Create the wireless connection
    auto *connection = new QNEConnection();
    connection->setWireless(true);  // Mark as wireless (invisible)

    connection->setStartPort(source->outputPort());
    connection->setEndPort(sink->inputPort());

    // Add to scene
    m_scene->addItem(connection);

    // Track the connection for management
    m_wirelessConnections[label].append(connection);

}

void WirelessManager::destroyWirelessConnectionsForLabel(const QString &label)
{
    if (label.isEmpty()) {
        return;
    }

    auto connections = m_wirelessConnections.take(label);

    for (auto *connection : connections) {
        if (connection && m_scene) {
            m_scene->removeItem(connection);
            delete connection;
        }
    }

}

void WirelessManager::cleanupEmptyGroups()
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

// Scene iteration helper methods to reduce code duplication
std::vector<Node *> WirelessManager::getAllNodesInScene() const
{
    std::vector<Node *> nodes;
    if (!m_scene) {
        return nodes;
    }

    for (auto *item : m_scene->items()) {
        if (auto *node = dynamic_cast<Node *>(item)) {
            nodes.push_back(node);
        }
    }
    return nodes;
}

std::vector<Node *> WirelessManager::getNodesWithLabel(const QString &label) const
{
    std::vector<Node *> nodes;
    if (label.isEmpty()) {
        return nodes;
    }

    for (auto *node : getAllNodesInScene()) {
        if (node->getWirelessLabel() == label) {
            nodes.push_back(node);
        }
    }
    return nodes;
}

void WirelessManager::setUpdatesBlocked(bool blocked)
{
    qDebug() << "[WIRELESS_DEBUG] WirelessManager updates" << (blocked ? "BLOCKED" : "UNBLOCKED");
    m_updatesBlocked = blocked;
}

bool WirelessManager::areUpdatesBlocked() const
{
    return m_updatesBlocked;
}
