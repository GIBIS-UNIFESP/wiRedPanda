// Copyright (c) 2012, STANISLAW ADASZEWSKI
// SPDX-License-Identifier: BSD-3-Clause
//
// Portions Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QCoreApplication>
#include <QGraphicsPathItem>

#include "App/Core/Enums.h"
#include "App/Core/ItemWithId.h"

class QNEInputPort;
class QNEOutputPort;
class QNEPort;

class QNEConnection : public QGraphicsPathItem, public ItemWithId
{
    Q_DECLARE_TR_FUNCTIONS(QNEConnection)

public:
    enum { Type = QGraphicsItem::UserType + 2 };
    int type() const override { return Type; }

    // --- Lifecycle ---

    explicit QNEConnection(QGraphicsItem *parent = nullptr);
    ~QNEConnection() override;
    QNEConnection(const QNEConnection &other) : QNEConnection(other.parentItem()) {}

    // --- Port / Endpoint Access ---

    QNEOutputPort *startPort() const;
    QNEInputPort *endPort() const;
    QNEPort *otherPort(const QNEPort *port) const;

    // --- Port Configuration ---

    void setStartPort(QNEOutputPort *port);
    void setEndPort(QNEInputPort *port);
    void setStartPos(const QPointF point);
    void setEndPos(const QPointF point);

    // --- Status & Visualization ---

    Status status() const;
    void setStatus(const Status status);
    bool highLight();
    void setHighLight(const bool highLight);

    // --- Geometric properties ---

    QRectF boundingRect() const override;
    double angle();
    void updatePath();
    void updatePosFromPorts();

    // --- Visual rendering ---

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void updateTheme();

    // --- Serialization ---

    void load(QDataStream &stream, const QMap<quint64, QNEPort *> &portMap = {});
    void save(QDataStream &stream) const;

protected:
    // --- Qt event overrides ---

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    bool sceneEvent(QEvent *event) override;

private:
    // --- Members: Theme colors ---

    QColor m_activeColor;
    QColor m_inactiveColor;
    QColor m_invalidColor;
    QColor m_selectedColor;

    // --- Members: Ports & positions ---

    QNEOutputPort *m_startPort = nullptr;
    QNEInputPort *m_endPort = nullptr;
    QPointF m_startPos;
    QPointF m_endPos;

    // --- Members: State ---

    Status m_status = Status::Invalid;
    bool m_highLight = false;
};

Q_DECLARE_METATYPE(QNEConnection)

QDataStream &operator<<(QDataStream &stream, const QNEConnection *conn);
