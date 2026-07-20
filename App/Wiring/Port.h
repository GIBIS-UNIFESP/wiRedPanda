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
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QTransform>

#include "App/Core/Enums.h"

class GraphicElement;
class Connection;
class Port;

/**
 * \class Port
 * \brief Abstract base class for circuit element ports (connection endpoints).
 *
 * \details A port is the small glyph drawn at the edge of a graphic element where
 * wires attach — a circle for inputs, a right-pointing triangle for outputs.
 * It stores a reference to the owning GraphicElement, all attached
 * Connections, and its current logical status.
 *
 * \details No longer a `QGraphicsPathItem` -- `pos()`/`path()`/etc. below are plain
 * members/methods, not inherited Qt Graphics View machinery. `scenePos()` computes the
 * port's world/canvas position fresh from the owning element's current pos()/rotation()/
 * flip state every call (oracle-validated formula, qtquick-rewrite plan Phase 8a) rather
 * than caching a transform the way `QGraphicsItem::setTransform()` used to.
 *
 * Concrete subclasses InputPort and OutputPort differentiate direction.
 */
class Port
{
public:
    // --- Lifecycle ---

    /// Constructs the port.
    Port();
    virtual ~Port();

    // --- Element Access ---

    /// Returns the graphic element that owns this port.
    GraphicElement *graphicElement() { return m_graphicElement; }
    /// \overload
    const GraphicElement *graphicElement() const { return m_graphicElement; }

    // --- Geometry ---

    /// Returns the port's base position, relative to the owning element's local frame.
    QPointF pos() const { return m_pos; }
    /// Sets the port's base position.
    void setPos(const QPointF &pos) { m_pos = pos; }
    /// \overload
    void setPos(qreal x, qreal y) { m_pos = QPointF(x, y); }

    /// Returns the glyph shape (circle for InputPort, triangle for OutputPort), in the
    /// port's own local frame centred on (0,0).
    QPainterPath path() const { return m_path; }
    /// Sets the glyph shape. Called once by InputPort/OutputPort's own constructor.
    void setPath(const QPainterPath &path) { m_path = path; }

    /// Returns \c true if this port should be drawn/hit-tested (Node hides whichever port a
    /// wireless Tx/Rx mode replaces with a channel). Plain state -- mirrors what
    /// QGraphicsItem::setVisible()/isVisible() used to provide for free; CanvasItem's own
    /// rendering/hit-testing doesn't consult this yet (a real, separately-tracked gap).
    bool isVisible() const { return m_visible; }
    /// Sets the visibility flag.
    void setVisible(bool visible) { m_visible = visible; }

    /// Local-frame bounding rect, padded 1 unit beyond the pen-exact glyph bound so a
    /// device-coordinate-cached paint has room to antialias the stroke edge instead of
    /// clipping it (visible at high zoom).
    QRectF boundingRect() const { return m_path.boundingRect().adjusted(-1, -1, 1, 1); }

    /// Returns this port's position in world/canvas coordinates -- the owning element's
    /// pos()/rotation()/flip state composed with this port's own pos(), computed fresh each
    /// call. Oracle-validated (qtquick-rewrite plan, Phase 8a): a rotatable element applies
    /// no per-port transform (its own rotation, applied uniformly, is enough); a
    /// non-rotatable element bakes rotation+flip into each port individually about a
    /// per-port pivot (mirrors the original QGraphicsItem::setTransform()-based
    /// ElementOrientation::orientPort(), now computed lazily here instead of cached on the
    /// port).
    QPointF scenePos() const;

    /// Returns the per-port rotate+flip transform a caller composes with pos() to draw this
    /// port's glyph (path()) correctly oriented -- mirrors what QGraphicsItem::setTransform()
    /// used to hold cached on Port before it stopped being one. Identity for a rotatable
    /// element's port: the element's own rotation, applied once by the caller's outer transform,
    /// already reorients the whole element including its ports, so no separate per-port
    /// transform is needed there (see scenePos()'s identical rotatesGraphic() branch). For a
    /// non-rotatable element this is exactly the transform scenePos() maps (0,0) through, minus
    /// the outer pos() translation -- oracle-validated together with scenePos() (qtquick-rewrite
    /// plan, Phase 8a).
    QTransform orientationTransform() const;

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

    /// Marks this port as mid-teardown without draining its connections yet -- see
    /// m_draining's own comment. Called by ElementPorts::~ElementPorts() on every port of an
    /// element (not just the one about to be deleted) before any of them start deleting, so a
    /// still-alive neighbor's own surviving fan-out (its updateConnections() reaching one of
    /// our other, not-yet-deleted ports via a shared connection) can detect the mid-destruction
    /// owner via scenePos() and bail out, instead of reaching into GraphicElement's already-
    /// destroyed m_appearance member (declared after m_ports, so destroyed first).
    void markDraining() { m_draining = true; }

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

    /// Returns the port's currently-painted fill brush -- the status colour, or the transient
    /// hover-highlight hoverEnter() applies on top of it (see setCurrentBrush()/hoverEnter()).
    QBrush brush() const { return m_ownBrush; }

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

    /// Triggers a path update on all attached connections.
    void updateConnections();

    /// No-op, kept for call-site compatibility with the many places that used to call this
    /// unconditionally when Port was a QGraphicsItem (a scene() guard already made it dead
    /// work in every one of those cases -- see project_quick_hotspot_fixes_2_landed.md).
    void show() {}

protected:
    /**
     * \brief Drains all attached connections, breaking back-references before deletion.
     * \param isInput True if this is an input port (sets endPort to nullptr); false for output.
     * \details Called from the InputPort and OutputPort destructors to avoid
     * re-entrant disconnect() calls into a partially destroyed port. Sets m_draining so
     * updateConnections() no-ops for the rest of teardown -- see that flag's own comment.
     */
    void drainConnections(bool isInput);

    /// Hit-area / glyph half-size in pixels. ±kRadius gives a 10×10 px clickable square,
    /// large enough to hit reliably without obscuring nearby elements.
    static constexpr int kRadius = 5;

    // --- Members ---

    GraphicElement *m_graphicElement = nullptr;
    QPointF m_pos;
    QPainterPath m_path;
    QBrush m_currentBrush;
    /// Pen paint() actually draws the outline with, kept separate from m_brush -- see
    /// updateTheme() for why (avoids recomputing a QPen on every status colour change; all
    /// four status pens share the same width).
    QPen m_currentPen;
    QList<Connection *> m_connections;
    QString m_name;
    Status m_defaultStatus = Status::Unknown;
    Status m_status = Status::Unknown;
    bool m_required = true;
    bool m_visible = true;
    int m_index = 0;
    /// Set once drainConnections() starts (this port is being destroyed). While true,
    /// updateConnections() no-ops instead of redrawing/re-validating: detachConnection()'s
    /// changePortAttachment() -> detachConnection() re-entry (setStartPort(nullptr)/
    /// setEndPort(nullptr) routes back through the general attach/detach path) would
    /// otherwise call updateConnections() for every connection still left on this port,
    /// each of which calls Connection::updatePosFromPorts() -> Port::scenePos() ->
    /// GraphicElement::boundingRect() on this port's own (mid-destruction) owner --
    /// real, AddressSanitizer-confirmed crashes on procedural-render-pixmap elements
    /// (IC/Mux/Demux/TruthTable) found via the qtquick-rewrite plan's CPU/Level test port
    /// (TestCPUAlu/TestCPUInstructionExecute/TestCPURegisterBank). None of this recompute
    /// is ever observed anyway -- the port and/or its owner are moments from being freed.
    bool m_draining = false;

private:
    /// Sets the currently-painted brush directly, bypassing the hover-preserving check
    /// setCurrentBrush() does. Used by setCurrentBrush() itself and hoverEnter()/hoverLeave().
    void setBrush(const QBrush &brush) { m_ownBrush = brush; }
    QBrush currentBrush() const;

    QBrush m_ownBrush; ///< The port's currently-painted fill brush; see brush().
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
    InputPort();
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
    OutputPort();
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
