// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>

class Node;
class QDataStream;
class Scene;

/**
 * @brief Manages wireless connections between nodes using labels
 * 
 * The WirelessConnectionManager provides a label-based system for connecting
 * nodes wirelessly. Nodes with the same wireless label are considered connected.
 */
class WirelessConnectionManager : public QObject
{
    Q_OBJECT

public:
    explicit WirelessConnectionManager(Scene* scene, QObject* parent = nullptr);

    // Node management
    void setNodeWirelessLabel(Node* node, const QString& label);
    void removeNode(Node* node);
    QString getNodeWirelessLabel(Node* node) const;
    
    // Connection queries
    QSet<Node*> getWirelessGroup(const QString& label) const;
    QSet<Node*> getConnectedNodes(Node* node) const;
    
    // Group management
    QStringList getActiveLabels() const;
    int getGroupCount() const;
    bool hasWirelessConnections() const;
    
    // Serialization
    void serialize(QDataStream& stream) const;
    void deserialize(QDataStream& stream);
    
    // Debug support
    QString getDebugInfo() const;

signals:
    void wirelessConnectionsChanged();
    void groupCreated(const QString& label);
    void groupDestroyed(const QString& label);

private slots:
    void onNodeDestroyed(QObject* obj);

private:
    void cleanupEmptyGroups();
    void removeNodeFromGroup(Node* node, const QString& label);
    void addNodeToGroup(Node* node, const QString& label);

    Scene* m_scene;
    QHash<Node*, QString> m_nodeLabels;           // Node -> wireless label
    QHash<QString, QSet<Node*>> m_labelGroups;   // Label -> set of nodes
};
