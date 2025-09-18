// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QSet>

class Node;
class QNEConnection;
class Scene;

class WirelessManager : public QObject
{
    Q_OBJECT

public:
    explicit WirelessManager(Scene *scene, QObject *parent = nullptr);
    ~WirelessManager() override = default;

    // Called when any node's wireless label changes
    void onNodeLabelChanged(Node *node, const QString &oldLabel, const QString &newLabel);

    // Called when a node is destroyed to clean up connections
    void onNodeDestroyed(Node *node);

    // Get all active wireless labels
    QStringList getActiveLabels() const;

    // Get all wireless connections for a specific label (for debugging/visualization)
    QList<QNEConnection *> getWirelessConnectionsForLabel(const QString &label) const;

    // Blocking mechanism for critical operations (like SplitCommand)
    void setUpdatesBlocked(bool blocked);
    bool areUpdatesBlocked() const;

    // Label uniqueness methods for copy/paste operations
    bool wouldCreateSourceConflict(Node *node, const QString &label) const;
    QString generateUniqueLabel(const QString &baseLabel) const;

    // Find source node (has physical input connections) and sinks (no physical input connections)
    Node *findWirelessSource(const QString &label) const;
    QSet<Node *> findWirelessSinks(const QString &label) const;

    // Query methods for testing and UI
    QSet<Node *> getWirelessGroup(const QString &label) const;      // Gets all nodes with this label
    int getGroupCount() const;                                     // Gets number of active label groups
    QSet<Node *> getConnectedNodes(Node *node) const;               // Gets nodes connected to this node via wireless
    bool hasWirelessConnections() const;                          // Checks if any wireless connections exist
    void serialize(QDataStream &stream) const;                    // Serializes node ID to wireless label mappings

signals:
    void wirelessConnectionsChanged();

private:
    // Rebuild all connections for a specific label
    void rebuildConnectionsForLabel(const QString &label);

    // Create/destroy wireless connection objects
    void createWirelessConnection(Node *source, Node *sink, const QString &label);
    void destroyWirelessConnectionsForLabel(const QString &label);

    // Clean up empty label groups
    void cleanupEmptyGroups();

    // Scene iteration helper methods to reduce code duplication
    std::vector<Node *> getAllNodesInScene() const;
    std::vector<Node *> getNodesWithLabel(const QString &label) const;

    // Track wireless connections by label for management
    QMap<QString, QList<QNEConnection *>> m_wirelessConnections;

    // Scene pointer for adding/removing connections
    Scene *m_scene;

    // Blocking flag to prevent updates during critical operations
    bool m_updatesBlocked = false;
};
