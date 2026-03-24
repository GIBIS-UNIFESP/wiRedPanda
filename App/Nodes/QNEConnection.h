// Copyright (c) 2012, STANISLAW ADASZEWSKI
// SPDX-License-Identifier: BSD-3-Clause
//
// Portions Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QNEConnection: a wire that connects an output port to an input port in the circuit scene.
 */

#pragma once

#include <QCoreApplication>
#include <QGraphicsPathItem>

#include "App/Core/Enums.h"
#include "App/Core/ItemWithId.h"

struct SerializationContext;
class QNEInputPort;
class QNEOutputPort;
class QNEPort;

/**
 * \class QNEConnection
 * \brief A bezier-curve wire connecting an output port to an input port in the scene.
 *
 * \details QNEConnection draws a smooth path between its start (output) and end (input)
 * ports.  It is coloured according to its logical status (active/inactive/unknown/selected)
 * and can be highlighted when the cursor hovers over it.  Connections are serializable
 * and participate in the undo/redo system.
 */
class QNEConnection : public QGraphicsPathItem, public ItemWithId
{
    Q_DECLARE_TR_FUNCTIONS(QNEConnection)

public:
    enum { Type = QGraphicsItem::UserType + 2 }; ///< Custom QGraphicsItem type discriminator.
    int type() const override { return Type; }

    /// Constructs an unconnected wire.
    explicit QNEConnection(QGraphicsItem *parent = nullptr);
    ~QNEConnection() override;
    QNEConnection(const QNEConnection &other) : QNEConnection(other.parentItem()) {}

    // --- Port / Endpoint Access ---

    /// Returns the output port this connection originates from.
    QNEOutputPort *startPort() const;
    /// Returns the input port this connection leads to.
    QNEInputPort *endPort() const;
    /// Returns the port at the other end of this connection from \a port.
    QNEPort *otherPort(const QNEPort *port) const;

    // --- Port Configuration ---

    /// Sets the output port this connection originates from.
    void setStartPort(QNEOutputPort *port);
    /// Sets the input port this connection leads to.
    void setEndPort(QNEInputPort *port);
    /// Sets the visual start position to \a point (used when dragging).
    void setStartPos(const QPointF point);
    /// Sets the visual end position to \a point (used when dragging).
    void setEndPos(const QPointF point);

    // --- Status & Visualization ---

    /// Returns the current four-state signal status (Active/Inactive/Unknown/Error).
    Status status() const;
    /// Sets the connection status and triggers a visual refresh.
    void setStatus(const Status status);
    /// Returns \c true if this connection is highlighted.
    bool highLight();
    /// Enables or disables connection highlighting.
    void setHighLight(const bool highLight);

    // --- Geometric properties ---

    /// \reimp
    QRectF boundingRect() const override;
    /// Returns the current angle of the bezier midpoint in radians.
    double angle();
    /// Recomputes the bezier control points from the current start/end positions.
    void updatePath();

    /// Moves the wire endpoints to match the current port positions.
    void updatePosFromPorts();

    // --- Visual rendering ---

    /// \reimp
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// Refreshes wire colours from the current ThemeManager palette.
    void updateTheme();

    // --- Serialization ---

    /// Deserializes the connection from \a stream, resolving ports via \a context.
    void load(QDataStream &stream, SerializationContext &context);
    /// Serializes the connection endpoints to \a stream.
    void save(QDataStream &stream) const;

protected:
    // --- Qt event overrides ---

    /// \reimp
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    /// \reimp
    bool sceneEvent(QEvent *event) override;

private:
    // --- Members: Theme colors ---

    QColor m_activeColor;
    QColor m_inactiveColor;
    QColor m_unknownColor;
    QColor m_errorColor;
    QColor m_selectedColor;

    // --- Members: Ports & positions ---

    QNEOutputPort *m_startPort = nullptr;
    QNEInputPort *m_endPort = nullptr;
    QPointF m_startPos;
    QPointF m_endPos;

    // --- Members: State ---

    Status m_status = Status::Unknown;
    bool m_highLight = false;
};

Q_DECLARE_METATYPE(QNEConnection)

QDataStream &operator<<(QDataStream &stream, const QNEConnection *conn);

