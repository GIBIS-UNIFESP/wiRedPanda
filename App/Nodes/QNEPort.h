// Copyright (c) 2012, STANISLAW ADASZEWSKI
// SPDX-License-Identifier: BSD-3-Clause
//
// Portions Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QBrush>
#include <QGraphicsPathItem>

#include "App/Core/Enums.h"

class GraphicElement;
class LogicElement;
class QNEConnection;
class QNEPort;

class QNEPort : public QGraphicsPathItem
{
public:
    // --- Type ---

    enum { Type = QGraphicsItem::UserType + 1 };
    int type() const override { return Type; }

    // --- Lifecycle ---

    explicit QNEPort(QGraphicsItem *parent = nullptr);

    // --- Element Access ---

    GraphicElement *graphicElement() const;
    LogicElement *logic() const;

    // --- Identity & Status ---

    QString name() const;
    Status defaultValue() const;
    Status status() const;
    bool isConnected(QNEPort *otherPort);
    bool isRequired() const;
    const QList<QNEConnection *> &connections() const;
    int index() const;

    // --- Type Queries ---

    virtual bool isInput() const = 0;
    virtual bool isOutput() const = 0;
    virtual bool isValid() const = 0;

    // --- Status Management ---

    virtual void setStatus(Status status) = 0;

    // --- Connection Management ---

    void connect(QNEConnection *conn);
    void disconnect(QNEConnection *conn);

    // --- Interaction ---

    void hoverEnter();
    void hoverLeave();

    // --- Property Setters ---

    void setCurrentBrush(const QBrush &currentBrush);
    void setDefaultStatus(const Status defaultStatus);
    void setGraphicElement(GraphicElement *graphicElement);
    void setIndex(const int index);
    void setName(const QString &name);
    void setRequired(const bool required);
    void updateConnections();

protected:
    // --- Qt Event Handling ---

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    virtual void updateTheme() = 0;

    // --- Members ---

    GraphicElement *m_graphicElement = nullptr;
    QBrush m_currentBrush;
    QGraphicsTextItem *m_label = new QGraphicsTextItem(this);
    QList<QNEConnection *> m_connections; // use smart pointers
    QString m_name;
    Status m_defaultStatus = Status::Invalid;
    Status m_status = Status::Inactive;
    bool m_required = true;
    int m_index = 0;
    int m_margin = 2;
    int m_radius = 5;

private:
    QBrush currentBrush() const;
};

class QNEInputPort : public QNEPort
{
public:
    // --- Lifecycle ---

    explicit QNEInputPort(QGraphicsItem *parent = nullptr);
    ~QNEInputPort() override;
    QNEInputPort(const QNEInputPort &other) : QNEInputPort(other.parentItem()) {}

    // --- Type Queries ---

    bool isInput() const override;
    bool isOutput() const override;
    bool isValid() const override;

    // --- Status Management ---

    void setStatus(const Status status) override;

    // --- Theme ---

    void updateTheme() override;
};

Q_DECLARE_METATYPE(QNEInputPort)

class QNEOutputPort : public QNEPort
{
public:
    // --- Lifecycle ---

    explicit QNEOutputPort(QGraphicsItem *parent = nullptr);
    ~QNEOutputPort() override;
    QNEOutputPort(const QNEOutputPort &other) : QNEOutputPort(other.parentItem()) {}

    // --- Type Queries ---

    bool isInput() const override;
    bool isOutput() const override;
    bool isValid() const override;

    // --- Status Management ---

    void setStatus(const Status status) override;

    // --- Theme ---

    void updateTheme() override;
};

Q_DECLARE_METATYPE(QNEOutputPort)

