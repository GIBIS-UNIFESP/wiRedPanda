// Copyright (c) 2012, STANISLAW ADASZEWSKI
// SPDX-License-Identifier: BSD-3-Clause
//
// Portions Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Enums.h"
#include "ItemWithId.h"

#include <QCoreApplication>
#include <QGraphicsPathItem>

class QNEInputPort;
class QNEOutputPort;
class QNEPort;

class QNEConnection : public QGraphicsPathItem, public ItemWithId
{
    Q_DECLARE_TR_FUNCTIONS(QNEConnection)

public:
    enum { Type = QGraphicsItem::UserType + 2 };
    int type() const override { return Type; }

    explicit QNEConnection(QGraphicsItem *parent = nullptr);
    ~QNEConnection() override;
    QNEConnection(const QNEConnection &other) : QNEConnection(other.parentItem()) {}

    QNEInputPort *endPort() const;
    QNEOutputPort *startPort() const;
    QNEPort *otherPort(const QNEPort *port) const;
    QRectF boundingRect() const override;
    Status status() const;
    bool highLight();
    double angle();
    void load(QDataStream &stream, const QMap<quint64, QNEPort *> &portMap = {});
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void save(QDataStream &stream) const;
    void setEndPort(QNEInputPort *port);
    void setEndPos(const QPointF point);
    void setHighLight(const bool highLight);
    void setStartPort(QNEOutputPort *port);
    void setStartPos(const QPointF point);
    void setStatus(const Status status);
    void updatePath();
    void updatePosFromPorts();
    void updateTheme();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    bool sceneEvent(QEvent *event) override;

private:
    QColor m_activeColor;
    QColor m_inactiveColor;
    QColor m_invalidColor;
    QColor m_selectedColor;
    QNEInputPort *m_endPort = nullptr;
    QNEOutputPort *m_startPort = nullptr;
    QPointF m_endPos;
    QPointF m_startPos;
    Status m_status = Status::Invalid;
    bool m_highLight = false;
};

Q_DECLARE_METATYPE(QNEConnection)

QDataStream &operator<<(QDataStream &stream, const QNEConnection *conn);

