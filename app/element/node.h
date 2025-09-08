// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

class Node : public GraphicElement
{
    Q_OBJECT

public:
    explicit Node(QGraphicsItem *parent = nullptr);

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
    bool hasInputConnection() const;  // Can set wireless labels (receives signals)
    bool hasOutputConnection() const; // Can select wireless labels (sends signals)

    // 1-N Wireless constraint methods
    bool isWirelessSource() const;  // Has input connection AND wireless label (transmits wirelessly)
    bool isWirelessSink() const;    // Has NO input connection AND wireless label (receives wirelessly)

signals:
    void wirelessLabelChanged(const QString &oldLabel, const QString &newLabel);

private:
    QString m_wirelessLabel;
};

Q_DECLARE_METATYPE(Node)
