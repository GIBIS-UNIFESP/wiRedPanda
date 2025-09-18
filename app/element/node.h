// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"
#include "qneport.h"

// Forward declarations
class Node;
class WirelessManager;

// Custom output port that notifies parent Node when status changes
class WirelessNodeOutputPort : public QNEOutputPort
{
public:
    explicit WirelessNodeOutputPort(QGraphicsItem *parent = nullptr);
    void setStatus(const Status status) override;
};

class Node : public GraphicElement
{
    Q_OBJECT

public:
    explicit Node(QGraphicsItem *parent = nullptr);
    ~Node() override;

    void updatePortsProperties() override;
    void save(QDataStream &stream) const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;

    // Override setLabel to handle wireless functionality automatically
    void setLabel(const QString &label) override;

    // Override itemChange to automatically register wireless labels when added to scene
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    // Override paint to add wireless signal indicators
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // Override boundingRect to include wireless signal indicators
    QRectF boundingRect() const override;

    // Override refresh to update wireless indicator colors during simulation
    void refresh() override;

    // Wireless functionality
    QString getWirelessLabel() const;
    bool hasWirelessLabel() const;

    // Connection validation methods
    bool hasInputConnection() const;   // Legacy method - checks for any input connection
    bool hasOutputConnection() const;  // Legacy method - checks for any output connection
    bool hasPhysicalInputConnection() const;
    bool hasPhysicalOutputConnection() const;

    // Wireless constraint methods
    bool isWirelessSource() const;  // Has physical input connection AND wireless label
    bool isWirelessSink() const;    // Has NO physical input connection AND wireless label

signals:
    void wirelessLabelChanged(const QString &oldLabel, const QString &newLabel);

private:
    // Helper methods
    bool hasPhysicalConnection(QNEPort *port) const;
    WirelessManager* getWirelessManager() const;
    void drawWirelessBars(QPainter *painter, const QPointF &position, bool pointingRight) const;
    void notifyWirelessManager(const QString &oldLabel, const QString &newLabel);
    bool validateWirelessConstraint(const QString &label) const;

    QString m_wirelessLabel;
};

Q_DECLARE_METATYPE(Node)
