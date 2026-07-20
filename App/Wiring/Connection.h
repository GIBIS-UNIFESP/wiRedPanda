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
 * \brief Connection: a wire that connects an output port to an input port in the circuit scene.
 */

#pragma once

#include <functional>

#include <QPainterPath>
#include <QPen>
#include <QPointF>

#include "App/Core/Enums.h"
#include "App/Core/ItemWithId.h"

struct SerializationContext;
class InputPort;
class OutputPort;
class Port;
class QPainter;

/**
 * \class Connection
 * \brief A bezier-curve wire connecting an output port to an input port in the scene.
 *
 * \details Connection draws a smooth path between its start (output) and end (input)
 * ports.  It is coloured according to its logical status (active/inactive/unknown/selected)
 * and can be highlighted when the cursor hovers over it.  Connections are serializable
 * and participate in the undo/redo system.
 *
 * \details No longer a `QGraphicsPathItem` -- `isSelected()`/`boundingRect()`/`shape()`/
 * `paint()` etc. below are plain members/methods, not inherited Qt Graphics View machinery.
 */
class Connection : public ItemWithId
{
public:
    /// File-format type tag Serialization::serialize()/deserialize() writes/reads to
    /// discriminate a Connection from a GraphicElement in the flat item stream. The literal
    /// value must stay 65536 + 2 -- existing .panda files on disk encode this exact integer,
    /// originally QGraphicsItem::UserType + 2 (UserType == 65536) back when this class
    /// overrode QGraphicsItem::type(); QGraphicsItem is no longer in scope, but the on-disk
    /// value can never change. See GraphicElement::Type's identical note.
    static constexpr int Type = 65536 + 2;

    /// Constructs an unconnected wire.
    Connection();
    ~Connection();

    /// Registers a callback invoked at the very start of ~Connection(), regardless of which
    /// code path triggers the deletion -- an explicit `owner->removeItem(this); delete this;`
    /// pair, or an out-of-band cascade (Port::drainConnections() bare-deleting this connection
    /// while tearing down one of its own ports' owning element). Lets an owning registry (e.g.
    /// CanvasItem) stay consistent even when it never got the chance to unregister this
    /// connection itself first -- see WIREDPANDA-HC. A plain std::function rather than a Qt
    /// signal: nothing could usefully connect to a signal on an object already mid-destruction,
    /// and Connection (unlike GraphicElement) isn't a QObject at all.
    void setDestroyedCallback(std::function<void(Connection *)> callback) { m_destroyedCallback = std::move(callback); }

    // --- Port / Endpoint Access ---

    /// Returns the output port this connection originates from.
    OutputPort *startPort() const { return m_startPort; }
    /// Returns the input port this connection leads to.
    InputPort *endPort() const { return m_endPort; }
    /// Returns the port at the other end of this connection from \a port.
    Port *otherPort(const Port *port) const;

    // --- Port Configuration ---

    /// Sets the output port this connection originates from.
    void setStartPort(OutputPort *port);
    /// Sets the input port this connection leads to.
    void setEndPort(InputPort *port);
    /// Sets the visual start position to \a point (used when dragging).
    void setStartPos(const QPointF point);
    /// Sets the visual end position to \a point (used when dragging).
    void setEndPos(const QPointF point);

    // --- Selection ---

    /// Returns \c true if this connection is currently selected.
    bool isSelected() const { return m_selected; }
    /// Sets the selection state. Plain flag, no propagation side effects (peer-port
    /// hover-highlight-on-select was itemChange()'s job, already confirmed dead for
    /// CanvasItem's own usage -- port-hover peer labels are a real, separately-tracked
    /// missing Quick feature, not something to reproduce here).
    void setSelected(bool selected) { m_selected = selected; }

    // --- Status & Visualization ---

    /// Returns the current four-state signal status (Active/Inactive/Unknown/Error).
    Status status() const;
    /// Sets the connection status and triggers a visual refresh.
    void setStatus(const Status status);
    /// Returns \c true if this connection is highlighted.
    bool highLight();
    /// Enables or disables connection highlighting.
    void setHighLight(const bool highLight);
    /// Returns the pen paint() draws the wire with for its current status.
    QPen statusPen() const { return m_statusPen; }

    // --- Geometric properties ---

    /// Bounding box in world/canvas coordinates -- pos() is always (0,0) for a Connection
    /// (its path is built directly in canvas coordinates by updatePath(), unlike
    /// GraphicElement), so this needs no separate "scene" variant.
    QRectF boundingRect() const;
    /// Cached, flattened stroke of the wire path -- re-stroking the Bézier (and keeping its
    /// cubics) on every shape-exact hit test would pay for stroking plus recursive curve
    /// subdivision each time. Invalidated when the path or the real pen width changes.
    QPainterPath shape() const;
    /// Returns the current angle of the bezier midpoint in radians.
    double angle();
    /// Recomputes the bezier control points from the current start/end positions.
    void updatePath();

    /// Moves the wire endpoints to match the current port positions.
    void updatePosFromPorts();

    // --- Visual rendering ---

    /// Draws the wire (and, for a selected/highlighted one, its halo/selection colour).
    void paint(QPainter *painter);

    /// Refreshes wire colours from the current ThemeManager palette.
    void updateTheme();

    // --- Serialization ---

    /// Deserializes the connection from \a stream, resolving ports via \a context.
    void load(QDataStream &stream, SerializationContext &context);
    /// Serializes the connection endpoints to \a stream.
    void save(QDataStream &stream) const;

private:
    Q_DISABLE_COPY_MOVE(Connection)

    /// Detaches \a oldPort and attaches \a newPort to this connection (base-pointer version).
    void changePortAttachment(Port *oldPort, Port *newPort);

    /// Sets the pen colour and width for the current status from the cached theme colours.
    void applyStatusPen();

    // --- Members: Theme colors ---

    QColor m_activeColor;
    QColor m_inactiveColor;
    QColor m_unknownColor;
    QColor m_errorColor;
    QColor m_selectedColor;

    // --- Members: Ports & positions ---

    OutputPort *m_startPort = nullptr;
    InputPort *m_endPort = nullptr;
    QPointF m_startPos;
    QPointF m_endPos;
    QPainterPath m_path;
    std::function<void(Connection *)> m_destroyedCallback;

    // --- Members: State ---

    Status m_status = Status::Unknown;
    bool m_highLight = false;
    bool m_selected = false;

    /// Pen paint() draws with, and shape()'s stroke-width source -- kept as one field (unlike
    /// the original QGraphicsPathItem-era split between this and the item's own real pen():
    /// that split existed only to avoid QGraphicsItem::setPen()'s expensive
    /// prepareGeometryChange()-triggering BSP-tree re-index on every status colour change,
    /// which is a no-op now (see GraphicElement.h's identical note).
    QPen m_statusPen;

    /// Backing cache for shape(); mutable since the override is const.
    mutable QPainterPath m_cachedShape;
    mutable bool m_shapeDirty = true;
};
