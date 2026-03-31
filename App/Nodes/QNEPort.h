// Copyright (c) 2012, STANISLAW ADASZEWSKI
// SPDX-License-Identifier: BSD-3-Clause
//
// Portions Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Port classes: QNEPort (base), QNEInputPort, and QNEOutputPort.
 */

#pragma once

#include <QBrush>
#include <QGraphicsPathItem>

#include "App/Core/Enums.h"

class GraphicElement;
class LogicElement;
class QNEConnection;
class QNEPort;

/**
 * \class QNEPort
 * \brief Abstract base class for circuit element ports (connection endpoints).
 *
 * \details A port is the small diamond/circle drawn at the edge of a graphic element
 * where wires attach.  It stores a reference to the owning GraphicElement, the
 * LogicElement slot it corresponds to, all attached QNEConnections, and its
 * current logical status.
 *
 * Concrete subclasses QNEInputPort and QNEOutputPort differentiate direction.
 */
class QNEPort : public QGraphicsPathItem
{
public:
    // --- Type ---

    enum { Type = QGraphicsItem::UserType + 1 };
    int type() const override { return Type; }

    // --- Lifecycle ---

    /// Constructs the port with optional \a parent item.
    explicit QNEPort(QGraphicsItem *parent = nullptr);

    // --- Element Access ---

    /// Returns the graphic element that owns this port.
    GraphicElement *graphicElement() const;

    /// Returns the LogicElement associated with this port.
    LogicElement *logic() const;

    // --- Identity & Status ---

    /// Returns the port's label text.
    QString name() const;

    /// Returns the default status applied when the port is unconnected.
    Status defaultValue() const;

    /// Returns the current logical status (Active/Inactive/Invalid).
    Status status() const;

    /**
     * \brief Returns \c true if this port is connected to \a otherPort via any wire.
     * \param otherPort Port to check for an existing connection.
     */
    bool isConnected(QNEPort *otherPort);

    /// Returns \c true if a connection to this port is mandatory.
    bool isRequired() const;

    /// Returns the list of wires attached to this port.
    const QList<QNEConnection *> &connections() const;

    /// Returns the port's visual/logical index within the element.
    int index() const;

    /// Returns \c true if this is an input port. Pure virtual.
    virtual bool isInput() const = 0;

    /// Returns \c true if this is an output port. Pure virtual.
    virtual bool isOutput() const = 0;

    /// Returns \c true if the port satisfies its connection requirement. Pure virtual.
    virtual bool isValid() const = 0;

    // --- Status Management ---

    virtual void setStatus(Status status) = 0;

    // --- Connection Management ---

    /// Registers \a conn as a connection attached to this port.
    void connect(QNEConnection *conn);

    /**
     * \brief Removes \a conn from this port's connection list.
     * \param conn Wire to detach.
     */
    void disconnect(QNEConnection *conn);

    /// Applies hover-enter visual feedback.
    void hoverEnter();

    /// Reverts hover-enter visual feedback.
    void hoverLeave();

    /**
     * \brief Sets the brush used to fill the port shape.
     * \param currentBrush New brush.
     */
    void setCurrentBrush(const QBrush &currentBrush);

    /**
     * \brief Sets the status applied when the port has no connection.
     * \param defaultStatus Default status value.
     */
    void setDefaultStatus(const Status defaultStatus);

    /**
     * \brief Binds this port to \a graphicElement.
     * \param graphicElement Owning graphic element.
     */
    void setGraphicElement(GraphicElement *graphicElement);

    /**
     * \brief Sets the port's visual index within the element.
     * \param index New index.
     */
    void setIndex(const int index);

    /**
     * \brief Sets the label text shown next to the port.
     * \param name Label string (e.g. "A", "CLK").
     */
    void setName(const QString &name);

    /**
     * \brief Marks whether a wire to this port is mandatory.
     * \param required \c true if a connection is required for valid simulation.
     */
    void setRequired(const bool required);

    /// Triggers a path update on all attached connections.
    void updateConnections();
    void setSerialId(quint64 serialId);
    quint64 serialId() const;

protected:
    // --- Qt Event Handling ---

    /// \reimp
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    /// Refreshes port colours from the current ThemeManager palette. Pure virtual.
    virtual void updateTheme() = 0;

    // --- Members ---

    GraphicElement *m_graphicElement = nullptr;
    QBrush m_currentBrush;
    QGraphicsTextItem *m_label = new QGraphicsTextItem(this); ///< Child text item displaying the port name label.
    QList<QNEConnection *> m_connections; // use smart pointers
    QString m_name;
    Status m_defaultStatus = Status::Invalid;
    Status m_status = Status::Inactive;
    bool m_required = true;
    int m_index = 0;
    int m_margin = 2;
    int m_radius = 5;
    quint64 m_serialId = 0;

private:
    QBrush currentBrush() const;
};

/**
 * \class QNEInputPort
 * \brief A port that receives a signal (the destination end of a wire).
 *
 * \details Input ports are valid when they have exactly one connected wire or
 * when they are optional (isRequired() == false).
 */
class QNEInputPort : public QNEPort
{
public:
    /// Constructs an input port attached to \a parent.
    explicit QNEInputPort(QGraphicsItem *parent = nullptr);
    ~QNEInputPort() override;
    QNEInputPort(const QNEInputPort &other) : QNEInputPort(other.parentItem()) {}

    // --- Type Queries ---

    /// \reimp
    bool isInput() const override;
    /// \reimp
    bool isOutput() const override;
    /// \reimp
    bool isValid() const override;

    // --- Status Management ---

    /// \reimp
    void setStatus(const Status status) override;

    // --- Theme ---

    /// \reimp
    void updateTheme() override;
};

Q_DECLARE_METATYPE(QNEInputPort)

/**
 * \class QNEOutputPort
 * \brief A port that drives a signal (the source end of a wire).
 *
 * \details Output ports can fan out to multiple input ports.
 * They are always considered valid regardless of connection count.
 */
class QNEOutputPort : public QNEPort
{
public:
    /// Constructs an output port attached to \a parent.
    explicit QNEOutputPort(QGraphicsItem *parent = nullptr);
    ~QNEOutputPort() override;
    QNEOutputPort(const QNEOutputPort &other) : QNEOutputPort(other.parentItem()) {}

    // --- Type Queries ---

    /// \reimp
    bool isInput() const override;
    /// \reimp
    bool isOutput() const override;
    /// \reimp
    bool isValid() const override;

    // --- Status Management ---

    /// \reimp
    void setStatus(const Status status) override;

    // --- Theme ---

    /// \reimp
    void updateTheme() override;
};

Q_DECLARE_METATYPE(QNEOutputPort)

