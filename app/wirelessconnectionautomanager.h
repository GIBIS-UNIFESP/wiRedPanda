// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QMap>
#include <QSet>
#include <QList>

class Node;
class WirelessConnection;
class Scene;

class WirelessConnectionAutoManager : public QObject
{
    Q_OBJECT

public:
    explicit WirelessConnectionAutoManager(Scene* scene, QObject* parent = nullptr);
    ~WirelessConnectionAutoManager() override = default;

    // Called when any node's wireless label changes
    void onNodeLabelChanged(Node* node, const QString& oldLabel, const QString& newLabel);
    
    // Called when a node is destroyed to clean up connections
    void onNodeDestroyed(Node* node);
    
    // Get all active wireless labels
    QStringList getActiveLabels() const;
    
    // Get all wireless connections for a specific label (for debugging/visualization)
    QList<WirelessConnection*> getWirelessConnectionsForLabel(const QString& label) const;

signals:
    void wirelessConnectionsChanged();

private:
    // Rebuild all connections for a specific label
    void rebuildConnectionsForLabel(const QString& label);
    
    // Find source node (has physical input connections) and sinks (no physical input connections)
    Node* findWirelessSource(const QString& label);
    QSet<Node*> findWirelessSinks(const QString& label);
    
    // Create/destroy wireless connection objects
    void createWirelessConnection(Node* source, Node* sink, const QString& label);
    void destroyWirelessConnectionsForLabel(const QString& label);
    
    // Clean up empty label groups
    void cleanupEmptyGroups();
    
    // Track wireless connections by label for management
    QMap<QString, QList<WirelessConnection*>> m_wirelessConnections;
    
    // Scene pointer for adding/removing connections
    Scene* m_scene;
};