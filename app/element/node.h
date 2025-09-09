// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

class Node : public GraphicElement
{
    Q_OBJECT

public:
    enum { Type = QGraphicsItem::UserType + 4 };  // Unique type for Node, next in sequence after GraphicElement(+3)
    int type() const override { return Type; }
    
    explicit Node(QGraphicsItem *parent = nullptr);
    ~Node() override;

    void updatePortsProperties() override;
    void save(QDataStream &stream) const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;

    // Override setLabel to handle wireless functionality automatically
    void setLabel(const QString &label) override;

    // Override itemChange to automatically register wireless labels when added to scene
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    // Wireless functionality
    QString getWirelessLabel() const;
    bool hasWirelessLabel() const;

    // Connection validation for wireless UI constraints
    bool hasInputConnection() const;  // Legacy method - checks for any input connection
    bool hasOutputConnection() const; // Legacy method - checks for any output connection
    
    // Type-safe connection detection methods
    bool hasPhysicalInputConnection() const;   // Only counts PhysicalConnection objects
    bool hasPhysicalOutputConnection() const;  // Only counts PhysicalConnection objects

    // 1-N Wireless constraint methods (updated to use type-safe detection)
    bool isWirelessSource() const;  // Has physical input connection AND wireless label (transmits wirelessly)
    bool isWirelessSink() const;    // Has NO physical input connection AND wireless label (receives wirelessly)

signals:
    void wirelessLabelChanged(const QString &oldLabel, const QString &newLabel);

private:
    QString m_wirelessLabel;
};

Q_DECLARE_METATYPE(Node)
