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
 * \brief Port classes: Port (base), InputPort, and OutputPort.
 */

#pragma once

#include <QBrush>
#include <QList>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QTransform>

#include "App/Core/Enums.h"

class GraphicElement;
class Connection;
class Port;
class QPainter;

/**
 * \class Port
 * \brief Abstract base class for circuit element ports (connection endpoints).
 *
 * \details A port is the small glyph drawn at the edge of a graphic element where
 * wires attach — a circle for inputs, a right-pointing triangle for outputs.
 * It stores a reference to the owning GraphicElement, all attached
 * Connections, and its current logical status.
 *
 * \details pos()/transform() below are this port's own placement within its owning element's
 * local frame, analogous to a child QGraphicsItem's pos()+transform() relative to its parent --
 * see mapToOwnerLocal()/scenePos().
 *
 * Concrete subclasses InputPort and OutputPort differentiate direction.
 */
class Port
{
public:
    // --- Type ---

    /// File-format type tag; the literal value must never change (see
    /// GraphicElement::Type's identical note).
    static constexpr int Type = 65536 + 1;
    int type() const { return Type; }

    /// Hit-area shape: the full ±kRadius square regardless of the painted glyph.
    QPainterPath shape() const;

    /// Pads the glyph path's exact bound by a small margin so a rendering host's own device
    /// cache has room to antialias the stroke edge instead of clipping it (visible at high zoom).
    QRectF boundingRect() const;

    /// Draws the port glyph at local (0,0) with m_currentPen (see updateTheme()) and brush() --
    /// a no-op when !isVisible(). The caller is responsible for translating/transforming the
    /// painter to this port's own placement (mapToOwnerLocal()'s pos()+transform(), then the
    /// owner's own placement) first.
    void paint(QPainter *painter) const;

    /// Returns \c true if this port should currently be painted/hit-tested. Some element
    /// types (e.g. Node in wireless mode) hide a port that's logically replaced by another
    /// mechanism.
    bool isVisible() const { return m_visible; }
    /// Sets whether this port should currently be painted/hit-tested.
    void setVisible(bool visible) { m_visible = visible; }

    // --- Position & Transform ---

    /// Returns this port's position in its owning element's local frame.
    QPointF pos() const { return m_pos; }
    /// Sets this port's position and keeps attached wires in sync.
    void setPos(const QPointF &pos);
    /// \overload
    void setPos(qreal x, qreal y) { setPos(QPointF(x, y)); }

    /// Returns this port's own extra transform (rotation+flip about its own pivot), applied
    /// about its own local origin before pos(). Identity for a port on a rotatesGraphic()
    /// element (the owner itself rotates/flips instead — see GraphicElement::pointToScene());
    /// set by ElementOrientation::orientPort() for a port on a non-rotatable element.
    QTransform transform() const { return m_transform; }
    /// Sets this port's own extra transform.
    void setTransform(const QTransform &transform) { m_transform = transform; }

    /// Maps a point in this port's own local coordinates into the owner's local frame,
    /// applying transform() then pos() -- mirrors what a child QGraphicsItem's own
    /// pos()+transform() composition did relative to its parent.
    QPointF mapToOwnerLocal(const QPointF &localPoint) const { return m_transform.map(localPoint) + m_pos; }

    /// Inverse of mapToOwnerLocal(): maps a point in the owner's local frame into this
    /// port's own local coordinates. Used by hit-testing that needs to know whether a
    /// point the owner received actually lands on one of its ports.
    QPointF mapFromOwnerLocal(const QPointF &ownerLocalPoint) const { return m_transform.inverted().map(ownerLocalPoint - m_pos); }

    /// Returns \c true if \a ownerLocalPoint (in the owner's local frame) lands within this
    /// port's hit-area shape() -- mirrors QGraphicsItem::contains()'s default, shape()-based
    /// behavior.
    bool contains(const QPointF &ownerLocalPoint) const { return shape().contains(mapFromOwnerLocal(ownerLocalPoint)); }

    /// Returns this port's own anchor point (local (0,0)) mapped all the way to world/canvas
    /// coordinates, going through the owner's own local-to-scene mapping. Mirrors
    /// QGraphicsItem::scenePos(). Correct for both a rotatesGraphic() owner (transform() stays
    /// identity here, so the owner's own rotate+flip does all the work) and a non-rotatable
    /// owner (transform() carries this port's own rotate+flip, and the owner contributes only
    /// its pos()) -- see GraphicElement::pointToScene()'s own two branches.
    QPointF scenePos() const;

    // --- Lifecycle ---

    /// Constructs the port.
    explicit Port();
    // Port is abstract (isInput()/isOutput()/isValid()/setStatus() are pure virtual), so it can
    // never be the most-derived type of a real object -- its own vtable "deleting destructor"
    // (D0) slot is dead by construction. Every real deletion, even through a base Port*
    // pointer, resolves to InputPort's/OutputPort's own D0, which then invokes this D2
    // (base-object destructor).
    virtual ~Port() = default; // LCOV_EXCL_LINE

    // --- Element Access ---

    /// Returns the graphic element that owns this port.
    GraphicElement *graphicElement() { return m_graphicElement; }
    /// \overload
    const GraphicElement *graphicElement() const { return m_graphicElement; }

    // --- Identity & Status ---

    /// Returns the port's label text.
    QString name() const;

    /// Returns the default status applied when the port is unconnected.
    Status defaultValue() const { return m_defaultStatus; }

    /// Returns the current logical status (Active/Inactive/Unknown/Error).
    Status status() const { return m_status; }

    /**
     * \brief Returns \c true if this port is connected to \a otherPort via any wire.
     * \param otherPort Port to check for an existing connection.
     */
    bool isConnected(Port *otherPort);

    /// Returns \c true if a connection to this port is mandatory.
    bool isRequired() const;

    /// Returns the list of wires attached to this port.
    const QList<Connection *> &connections() const;

    /// Returns the port's visual/logical index within the element.
    int index() const;

    /// Returns this port's index within the element's combined input+output port
    /// sequence: index() for input ports, index() + graphicElement()->inputSize()
    /// for output ports. The single source of truth for the port-ordering half of
    /// the serial-ID formula.
    int globalIndex() const;

    /// Packs \a elementBase and \a globalIndex into a serial ID: (elementBase << 16)
    /// | (globalIndex & 0xFFFF). The single source of truth for the serial-ID format
    /// used to resolve connection endpoints across save/load.
    static quint64 makeSerialId(quint64 elementBase, int globalIndex);

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
    void attachConnection(Connection *conn);

    /**
     * \brief Removes \a conn from this port's connection list.
     * \param conn Wire to detach.
     */
    void detachConnection(Connection *conn);

    /// Applies hover-enter visual feedback.
    void hoverEnter();

    /// Reverts hover-enter visual feedback.
    void hoverLeave();

    /// Reapplies the status-based pen and brush from the current ThemeManager palette.
    void updateTheme();

    /**
     * \brief Sets the brush used to fill the port shape.
     * \param currentBrush New brush.
     */
    void setCurrentBrush(const QBrush &currentBrush);

    /// Returns the pen paint() draws the port outline with for its current status.
    QPen currentPen() const { return m_currentPen; }

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
     * \note Does not affect the port's default status — call setDefaultStatus() separately.
     */
    void setRequired(const bool required);

    /// Triggers a path update on all attached connections, then re-derives this port's own
    /// status/validity (see revalidateStatus()). Use this when the port itself actually moved
    /// (rotate/flip/drag) -- for anything that only changes which connections are attached
    /// (attach/detach/setRequired()), call revalidateStatus() directly instead, since those
    /// never move this port or any already-correctly-positioned sibling.
    void updateConnections();

    /// Just the status/validity tail of updateConnections(), without the sibling
    /// position-refresh loop: shows Error if isValid() fails, reverts an unconnected optional
    /// input to its default value, or re-syncs from a single remaining connection. Called by
    /// attachConnection()/detachConnection()/setRequired() -- none of which move this port or
    /// its siblings, so redoing every sibling's position on every call would be wasted work
    /// (a real O(fanout^2) cost during sequential file load on a high-fanout net).
    void revalidateStatus();

    /// Returns the port's glyph outline (circle/triangle), in its own local coordinates.
    QPainterPath path() const { return m_path; }
    /// Sets the port's glyph outline. Called once by InputPort's/OutputPort's constructor.
    void setPath(const QPainterPath &path) { m_path = path; }

    /// Returns the brush paint() actually fills the glyph with -- may be the transient
    /// hover-yellow set by hoverEnter(), not necessarily m_currentBrush (see setCurrentBrush()).
    QBrush brush() const { return m_brush; }
    /// Sets the brush paint() fills the glyph with.
    void setBrush(const QBrush &brush) { m_brush = brush; }

    /// Returns the tooltip text (the port's name).
    QString toolTip() const { return m_toolTip; }
    /// Sets the tooltip text.
    void setToolTip(const QString &toolTip) { m_toolTip = toolTip; }

    /// No-op placeholder for a rendering host's own repaint scheduling -- see
    /// GraphicElement::update()'s identical note.
    void update() {}

protected:
    /**
     * \brief Drains all attached connections, breaking back-references before deletion.
     * \param isInput True if this is an input port (sets endPort to nullptr); false for output.
     * \details Called from the InputPort and OutputPort destructors to avoid
     * re-entrant disconnect() calls into a partially destroyed port.
     */
    void drainConnections(bool isInput);

    /// Applies the precomputed pen/brush for the current status (see updateTheme()'s own
    /// comment for why these are precomputed rather than rebuilt here). Called by
    /// InputPort::setStatus()/OutputPort::setStatus() on every status change -- unlike
    /// updateTheme() itself, this never touches ThemeManager, so it's the cheap path.
    void applyStatusStyle();

    /// Hit-area / glyph half-size in pixels. ±kRadius gives a 10×10 px clickable square,
    /// large enough to hit reliably without obscuring nearby elements.
    static constexpr int kRadius = 5;

    // --- Members ---

    GraphicElement *m_graphicElement = nullptr;
    QBrush m_currentBrush;
    /// Pen paint() actually draws the outline with, kept separate from the "real" brush()/pen
    /// state -- see updateTheme() for why (avoids recomputing anything on every status colour
    /// change; all four status pens share the same width, so unlike Connection this never needs
    /// to fall back to any other pen).
    QPen m_currentPen;

    /// Pens/brushes for each status value, precomputed once per updateTheme() theme-refresh
    /// rather than rebuilt on every status change -- ThemeManager stores the status colours as
    /// bare QColor, and assigning a QColor directly to m_currentPen/m_currentBrush on every
    /// status change would go through QPen::operator=(QColor)/the implicit QColor->QBrush
    /// conversion, both of which unconditionally detach and reconstruct, on every single port
    /// status change across the whole circuit.
    QPen m_unknownStatusPen;
    QPen m_inactiveStatusPen;
    QPen m_activeStatusPen;
    QPen m_errorStatusPen;
    QBrush m_unknownStatusBrush;
    QBrush m_inactiveStatusBrush;
    QBrush m_activeStatusBrush;
    QBrush m_errorStatusBrush;
    QList<Connection *> m_connections;
    QPainterPath m_path;
    QPointF m_pos;
    QTransform m_transform;
    QBrush m_brush;
    QString m_toolTip;
    bool m_visible = true;
    QString m_name;
    Status m_defaultStatus = Status::Unknown;
    Status m_status = Status::Unknown;
    bool m_required = true;
    int m_index = 0;

private:
    QBrush currentBrush() const;
};

/**
 * \class InputPort
 * \brief A port that receives a signal (the destination end of a wire).
 *
 * \details Input ports are valid when they have exactly one connected wire or
 * when they are optional (isRequired() == false).
 */
class InputPort : public Port
{
public:
    /// Constructs an input port.
    explicit InputPort();
    ~InputPort() override;

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

private:
    Q_DISABLE_COPY_MOVE(InputPort)
};

/**
 * \class OutputPort
 * \brief A port that drives a signal (the source end of a wire).
 *
 * \details Output ports can fan out to multiple input ports.
 * They are always considered valid regardless of connection count.
 */
class OutputPort : public Port
{
public:
    /// Constructs an output port.
    explicit OutputPort();
    ~OutputPort() override;

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

private:
    Q_DISABLE_COPY_MOVE(OutputPort)
};
