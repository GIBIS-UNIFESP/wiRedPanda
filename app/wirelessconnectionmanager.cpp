// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wirelessconnectionmanager.h"

#include "common.h"
#include "node.h"
#include "scene.h"

#include <QDataStream>
#include <QDebug>

WirelessConnectionManager::WirelessConnectionManager(Scene *scene, QObject *parent)
    : QObject(parent)
    , m_scene(scene)
{
    if (!scene) {
        qCCritical(zero) << "WirelessConnectionManager created with null scene";
        return;
    }
}

void WirelessConnectionManager::setNodeWirelessLabel(Node *node, const QString &label)
{
    if (!node) {
        qCWarning(zero) << "Attempting to set wireless label on null node";
        return;
    }

    const QString trimmedLabel = label.trimmed();
    const QString currentLabel = m_nodeLabels.value(node, QString());

    // No change needed
    if (currentLabel == trimmedLabel) {
        return;
    }


    // Remove from old group if it had a label
    if (!currentLabel.isEmpty()) {
        removeNodeFromGroup(node, currentLabel);
    }

    // Add to new group if label is not empty
    if (!trimmedLabel.isEmpty()) {
        addNodeToGroup(node, trimmedLabel);
        m_nodeLabels[node] = trimmedLabel;
    } else {
        m_nodeLabels.remove(node);
    }

    // Connect to node's destroyed signal to clean up
    connect(node, &QObject::destroyed, this, &WirelessConnectionManager::onNodeDestroyed, Qt::UniqueConnection);

    cleanupEmptyGroups();
    emit wirelessConnectionsChanged();
}

void WirelessConnectionManager::removeNode(Node *node)
{
    if (!node) {
        return;
    }

    const QString currentLabel = m_nodeLabels.value(node, QString());
    if (!currentLabel.isEmpty()) {

        removeNodeFromGroup(node, currentLabel);
        m_nodeLabels.remove(node);

        cleanupEmptyGroups();
        emit wirelessConnectionsChanged();
    }

    // Disconnect signals
    disconnect(node, &QObject::destroyed, this, &WirelessConnectionManager::onNodeDestroyed);
}

QString WirelessConnectionManager::getNodeWirelessLabel(Node *node) const
{
    return m_nodeLabels.value(node, QString());
}

QSet<Node *> WirelessConnectionManager::getWirelessGroup(const QString &label) const
{
    return m_labelGroups.value(label, QSet<Node *>());
}

QSet<Node *> WirelessConnectionManager::getConnectedNodes(Node *node) const
{
    const QString label = getNodeWirelessLabel(node);
    if (label.isEmpty()) {
        return QSet<Node *>();
    }

    return getWirelessGroup(label);
}

QStringList WirelessConnectionManager::getActiveLabels() const
{
    QStringList labels;
    for (auto it = m_labelGroups.cbegin(); it != m_labelGroups.cend(); ++it) {
        if (!it.value().isEmpty()) {
            labels.append(it.key());
        }
    }
    labels.sort();
    return labels;
}

int WirelessConnectionManager::getGroupCount() const
{
    int count = 0;
    for (auto it = m_labelGroups.cbegin(); it != m_labelGroups.cend(); ++it) {
        if (!it.value().isEmpty()) {
            count++;
        }
    }
    return count;
}

bool WirelessConnectionManager::hasWirelessConnections() const
{
    return !m_nodeLabels.isEmpty();
}

void WirelessConnectionManager::serialize(QDataStream &stream) const
{
    // Save format: node_id -> wireless_label
    QMap<int, QString> nodeWirelessLabels;

    for (auto it = m_nodeLabels.cbegin(); it != m_nodeLabels.cend(); ++it) {
        auto *node = it.key();
        const QString &label = it.value();

        if (node && !label.isEmpty()) {
            nodeWirelessLabels[node->id()] = label;
        }
    }

    stream << nodeWirelessLabels;
}

void WirelessConnectionManager::deserialize(QDataStream &stream)
{
    QMap<int, QString> nodeWirelessLabels;
    stream >> nodeWirelessLabels;

    // Clear existing data
    m_labelGroups.clear();
    m_nodeLabels.clear();

    // Rebuild wireless connections after all nodes are loaded
    for (auto it = nodeWirelessLabels.cbegin(); it != nodeWirelessLabels.cend(); ++it) {
        const int nodeId = it.key();
        const QString &label = it.value();

        if (auto *node = m_scene->findNode(nodeId)) {
            setNodeWirelessLabel(node, label);
        } else {
            qCWarning(zero) << "Node" << nodeId << "not found for wireless label" << label;
        }
    }

    emit wirelessConnectionsChanged();
}

QString WirelessConnectionManager::getDebugInfo() const
{
    QStringList info;
    info << QString("Groups: %1").arg(getGroupCount());
    info << QString("Total nodes: %1").arg(m_nodeLabels.size());

    for (auto it = m_labelGroups.cbegin(); it != m_labelGroups.cend(); ++it) {
        const QString &label = it.key();
        const auto &nodes = it.value();
        if (!nodes.isEmpty()) {
            info << QString("  %1: %2 nodes").arg(label).arg(nodes.size());
        }
    }

    return info.join('\n');
}

void WirelessConnectionManager::onNodeDestroyed(QObject *obj)
{
    // Clean up when a node is destroyed
    auto *node = static_cast<Node *>(obj);

    const QString currentLabel = m_nodeLabels.value(node, QString());
    if (!currentLabel.isEmpty()) {

        // Remove from group and node mapping
        if (m_labelGroups.contains(currentLabel)) {
            m_labelGroups[currentLabel].remove(node);
        }
        m_nodeLabels.remove(node);

        cleanupEmptyGroups();
        emit wirelessConnectionsChanged();
    }
}

void WirelessConnectionManager::cleanupEmptyGroups()
{
    QStringList emptyLabels;

    for (auto it = m_labelGroups.begin(); it != m_labelGroups.end(); ++it) {
        if (it.value().isEmpty()) {
            emptyLabels.append(it.key());
        }
    }

    for (const QString &label : emptyLabels) {
        m_labelGroups.remove(label);
    }
}

void WirelessConnectionManager::removeNodeFromGroup(Node *node, const QString &label)
{
    if (m_labelGroups.contains(label)) {
        m_labelGroups[label].remove(node);
    }
}

void WirelessConnectionManager::addNodeToGroup(Node *node, const QString &label)
{
    m_labelGroups[label].insert(node);
}
