// Copyright (c) 2012, STANISLAW ADASZEWSKI
// SPDX-License-Identifier: BSD-3-Clause
//
// Portions Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Originally derived from Stanislaw Adaszewski's Qt Node Editor (qneblock); since
// re-authored for wiRedPanda. The BSD attribution above is retained as a license
// obligation for the derived design.

/** \file
 * \brief Port classes: Port (abstract base), InputPort, and OutputPort.
 */

#pragma once

#include <QBrush>
#include <QGraphicsPathItem>

#include "App/Core/Enums.h"

class Connection;
class GraphicElement;

/**
 * \class Port
 * \brief Abstract base class for circuit element ports (connection endpoints).
 *
 * \details A port is the small diamond drawn at the edge of a graphic element where
 * wires attach. It stores a reference to the owning GraphicElement, all attached
 * Connections, and its current logical status. Concrete subclasses InputPort and
 * OutputPort differentiate direction.
 */
class Port : public QGraphicsPathItem
{
public:
    // --- Type ---

    enum { Type = QGraphicsItem::UserType + 1 };
    int type() const override { return Type; }

    // --- Lifecycle ---

    /// Constructs the port with optional \a parent item.
    explicit Port(QGraphicsItem *parent = nullptr);

    // --- Element access ---

    /// Returns the graphic element that owns this port.
    GraphicElement *graphicElement() const { return m_graphicElement; }

    // --- Identity & status ---

    /// Returns the port's label text.
    QString name() const;

    /// Returns the default status applied when the port is unconnected.
    Status defaultValue() const { return m_defaultStatus; }

    /// Returns the current logical status (Active/Inactive/Unknown/Error).
    Status status() const { return m_status; }

    /// Returns \c true if this port is connected to \a otherPort via any wire.
    bool isConnected(Port *otherPort);

    /// Returns \c true if a connection to this port is mandatory.
    bool isRequired() const;

    /// Returns the list of wires attached to this port.
    const QList<Connection *> &connections() const;

    /// Returns the port's visual/logical index within the element.
    int index() const;

    /// Returns \c true if this is an input port. Pure virtual.
    virtual bool isInput() const = 0;

    /// Returns \c true if this is an output port. Pure virtual.
    virtual bool isOutput() const = 0;

    /// Returns \c true if the port satisfies its connection requirement. Pure virtual.
    virtual bool isValid() const = 0;

    // --- Status management ---

    virtual void setStatus(Status status) = 0;

    // --- Connection management ---

    /// Registers \a conn as a connection attached to this port.
    void attachConnection(Connection *conn);

    /// Removes \a conn from this port's connection list.
    void detachConnection(Connection *conn);

    /// Applies hover-enter visual feedback.
    void hoverEnter();

    /// Reverts hover-enter visual feedback.
    void hoverLeave();

    /// Sets the brush used to fill the port shape.
    void setCurrentBrush(const QBrush &currentBrush);

    /// Sets the status applied when the port has no connection.
    void setDefaultStatus(const Status defaultStatus);

    /// Binds this port to \a graphicElement.
    void setGraphicElement(GraphicElement *graphicElement);

    /// Sets the port's visual index within the element.
    void setIndex(const int index);

    /**
     * \brief Sets the label text shown next to the port.
     * \param name Label string (e.g. "A", "CLK").
     */
    void setName(const QString &name);

    /**
     * \brief Marks whether a wire to this port is mandatory.
     * \param required \c true if a connection is required for valid simulation.
     * \note Does not affect the port's default status — call setDefaultStatus() separately.
     */
    void setRequired(const bool required);

    /// Triggers a path update on all attached connections.
    void updateConnections();

    void setSerialId(quint64 serialId);
    quint64 serialId() const;

protected:
    // --- Qt event handling ---

    /// \reimp
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    /// Refreshes port colours from the current ThemeManager palette. Pure virtual.
    virtual void updateTheme() = 0;

    /**
     * \brief Drains all attached connections, breaking back-references before deletion.
     * \param isInput True if this is an input port (sets endPort to nullptr); false for output.
     * \details Called from the InputPort and OutputPort destructors to avoid re-entrant
     * disconnect() calls into a partially destroyed port.
     */
    void drainConnections(bool isInput);

    /// Port hit-area / visual half-extent in pixels. A 5 px radius gives a 10×10 px
    /// clickable diamond — large enough to hit reliably without obscuring neighbours.
    static constexpr int kRadius = 5;

    // --- Members ---

    GraphicElement *m_graphicElement = nullptr;
    QBrush m_currentBrush;
    QList<Connection *> m_connections;
    QString m_name;
    Status m_defaultStatus = Status::Unknown;
    Status m_status = Status::Inactive;
    bool m_required = true;
    int m_index = 0;
    quint64 m_serialId = 0;

private:
    QBrush currentBrush() const;
};

/**
 * \class InputPort
 * \brief A port that receives a signal (the destination end of a wire).
 *
 * \details Input ports are valid when they have exactly one connected wire or when
 * they are optional (isRequired() == false).
 */
class InputPort : public Port
{
public:
    /// Constructs an input port attached to \a parent.
    explicit InputPort(QGraphicsItem *parent = nullptr);
    ~InputPort() override;

    // --- Type queries ---

    /// \reimp
    bool isInput() const override;
    /// \reimp
    bool isOutput() const override;
    /// \reimp
    bool isValid() const override;

    // --- Status management ---

    /// \reimp
    void setStatus(const Status status) override;

    // --- Theme ---

    /// \reimp Re-applies the pen/brush for the current status from the active palette.
    void updateTheme() override;

private:
    /// Applies the pen + fill brush matching m_status from the current theme.
    void applyStatusStyle();
};

/**
 * \class OutputPort
 * \brief A port that drives a signal (the source end of a wire).
 *
 * \details Output ports can fan out to multiple input ports and are always considered
 * valid regardless of connection count.
 */
class OutputPort : public Port
{
public:
    /// Constructs an output port attached to \a parent.
    explicit OutputPort(QGraphicsItem *parent = nullptr);
    ~OutputPort() override;

    // --- Type queries ---

    /// \reimp
    bool isInput() const override;
    /// \reimp
    bool isOutput() const override;
    /// \reimp
    bool isValid() const override;

    // --- Status management ---

    /// \reimp
    void setStatus(const Status status) override;

    // --- Theme ---

    /// \reimp
    void updateTheme() override;
};
