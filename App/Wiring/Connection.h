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

#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QRectF>

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
 * \details path()/pen()/isSelected() below are plain members and methods this class implements
 * itself. Drawing order (wires behind elements) and adaptive antialiasing are a rendering
 * host's concern, not something this class enforces on itself via setZValue()/scene() lookups.
 */
class Connection : public ItemWithId
{
public:
    /// File-format type tag; the literal value must never change (see
    /// GraphicElement::Type's identical note).
    static constexpr int Type = 65536 + 2;
    int type() const { return Type; }

    /// Constructs an unconnected wire.
    explicit Connection();
    ~Connection() override;

    // --- Port / Endpoint Access ---

    /// Returns the output port this connection originates from.
    OutputPort *startPort() const;
    /// Returns the input port this connection leads to.
    InputPort *endPort() const;
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

    /// Expands beyond the path's tight bounding box by 10 px on all sides so a thick
    /// selection outline / highlight halo is fully covered during repaints.
    QRectF boundingRect() const;
    /// Cached, flattened stroke of the wire path -- re-stroking the Bézier on every call
    /// (keeping its cubics) would make every shape-exact hit test (clicks, rubber-band
    /// selection) pay for stroking plus recursive curve subdivision. Invalidated when the
    /// path or pen width changes.
    QPainterPath shape() const;
    /// Returns the current angle of the bezier midpoint in radians.
    double angle();
    /// Recomputes the bezier control points from the current start/end positions.
    void updatePath();

    /// Moves the wire endpoints to match the current port positions.
    void updatePosFromPorts();

    /// Returns the wire's current Bézier path, in world/canvas coordinates (start/end
    /// positions already come from Port::scenePos()).
    QPainterPath path() const { return m_path; }

    /// Returns the pen paint() would use as the item's own real pen -- distinct from
    /// statusPen() -- kept for shape()'s stroke-tolerance calculation.
    QPen pen() const { return m_pen; }
    /// Sets the item's own real pen.
    void setPen(const QPen &pen) { m_pen = pen; }

    /// Returns \c true if this connection is currently selected.
    bool isSelected() const { return m_selected; }
    /// Sets the selection state and highlights/un-highlights both endpoint ports to match.
    void setSelected(bool selected);

    /// No-op placeholder for a rendering host's own repaint scheduling -- see
    /// GraphicElement::update()'s identical note.
    void update() {}

    // --- Visual rendering ---

    /// Draws the wire at its own world/canvas position (path() is already in that frame,
    /// unlike GraphicElement::paint()/Port::paint() which draw at local (0,0)).
    void paint(QPainter *painter) const;

    /// Refreshes wire colours from the current ThemeManager palette.
    void updateTheme();

    // --- Serialization ---

    /// Deserializes the connection from \a stream, resolving ports via \a context.
    void load(QDataStream &stream, SerializationContext &context);
    /// Serializes the connection endpoints to \a stream.
    void save(QDataStream &stream) const;

protected:
    /// Sets the item's own real path (see path()).
    void setPath(const QPainterPath &path) { m_path = path; }

private:
    friend class TestConnection;

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

    /// Pens for each status value, precomputed once per updateTheme() call rather than
    /// reconstructed (and its QBrush heap-allocated) on every single setStatus() --
    /// see applyStatusPen()'s own comment for why this matters on a large circuit.
    QPen m_unknownStatusPen;
    QPen m_inactiveStatusPen;
    QPen m_activeStatusPen;
    QPen m_errorStatusPen;

    // --- Members: Ports & positions ---

    OutputPort *m_startPort = nullptr;
    InputPort *m_endPort = nullptr;
    QPointF m_startPos;
    QPointF m_endPos;

    // --- Members: State ---

    Status m_status = Status::Unknown;
    bool m_highLight = false;
    bool m_selected = false;

    /// Pen paint() actually draws with, kept separate from the item's own real pen() -- see
    /// applyStatusPen() for why (avoids recomputing anything on every status colour change).
    QPen m_statusPen;

    QPainterPath m_path;
    QPen m_pen;

    /// Backing cache for shape(); mutable since the method is const.
    mutable QPainterPath m_cachedShape;
    mutable bool m_shapeDirty = true;
};
