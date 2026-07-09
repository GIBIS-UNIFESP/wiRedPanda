// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ConnectionManager: manages wire creation, deletion, validation and hover feedback.
 */

#pragma once

#include <QCoreApplication>
#include <QList>
#include <QPair>
#include <QPointer>
#include <QPointF>

class GraphicElement;
class PortHoverLabel;
class Connection;
class InputPort;
class OutputPort;
class Port;
class Scene;

/**
 * \class ConnectionManager
 * \brief Manages interactive wire creation, deletion, validation and hover feedback.
 *
 * Extracted from Scene to decouple connection-editing concerns from the main
 * circuit scene.  Owns the in-progress connection state and hover-port tracking;
 * delegates item addition/removal and undo commands back to the owning Scene.
 */
class ConnectionManager
{
    Q_DECLARE_TR_FUNCTIONS(ConnectionManager)
    friend class TestConnectionManager;

public:
    explicit ConnectionManager(Scene *scene);

    // --- Wire creation workflow ---

    /// Begins a new in-progress wire anchored at \a startPort (output port).
    void startFromOutput(OutputPort *startPort);

    /// Begins a new in-progress wire anchored at \a endPort (input port).
    void startFromInput(InputPort *endPort);

    /**
     * \brief Attempts to complete the in-progress wire at the port under \a scenePos.
     *
     * If the port is compatible the wire is committed via an AddItemsCommand;
     * otherwise the in-progress wire is deleted.
     */
    void tryComplete(const QPointF &scenePos);

    /// Cancels and deletes the in-progress wire, if any.
    void cancel();

    /**
     * \brief Detaches the existing wire on \a endPort and starts a new one from the same output.
     *
     * Lets the user "grab" an existing wire and re-route it to a different input.
     */
    void detach(InputPort *endPort);

    // --- In-progress wire state ---

    /// Returns true if a wire is currently being drawn.
    [[nodiscard]] bool hasEditedConnection() const;

    /// Returns the in-progress wire, or nullptr.
    [[nodiscard]] Connection *editedConnection() const;

    /// Updates the free end of the in-progress wire to follow \a scenePos.
    void updateEditedEnd(const QPointF &scenePos);

    // --- Hover feedback ---

    /// Updates the hover state for the port under \a scenePos.
    void updateHover(const QPointF &scenePos);

    /// Clears the current hover state (unhighlights the port, resets cursor).
    void clearHover();

    /**
     * \brief Shows in-situ label chips for \a port itself and every port connected to it.
     *
     * Called from Scene::helpEvent in place of the native tooltip, so the chips appear and
     * disappear at the same wake-up delay a tooltip would. Replaces any chips already shown.
     */
    void showHoverLabels(Port *port);

    // --- Validation ---

    /**
     * \brief Returns \c true if a wire from \a startPort to \a endPort is permitted.
     *
     * A connection is rejected when:
     * - Either port is null.
     * - Both ports belong to the same element (self-loop).
     * - The ports are already connected (duplicate).
     * - \a endPort belongs to a wireless Rx node (physical wire would be ignored by simulation).
     * - \a startPort belongs to a wireless Tx node (tunnel convention: output drives channel only).
     */
    static bool isConnectionAllowed(OutputPort *startPort, InputPort *endPort);

    /// Returns a short, user-facing explanation of why the connection from \a startPort to
    /// \a endPort is disallowed, or an empty string if it is allowed. Single source of truth
    /// for isConnectionAllowed() and the status-bar feedback shown when a wire is rejected.
    static QString connectionRejectionReason(OutputPort *startPort, InputPort *endPort);

private:
    void setEditedConnection(Connection *connection);
    void deleteEditedConnection();

    void setHoverPort(Port *port);
    void releaseHoverPort();
    [[nodiscard]] Port *hoverPort();

    /// Returns the ports on the far end of every wire attached to \a port (skips dangling ends).
    [[nodiscard]] static QList<Port *> connectedPeers(Port *port);

    /// Removes and deletes all live label chips spawned by the current hover.
    void clearHoverLabels();

    Scene *m_scene = nullptr;

    /// ID of the in-progress wire (looked up via Scene::itemById).
    int m_editedConnectionId = 0;

    /// Hover-port tracking stored as element ID + port index so it survives undo/redo.
    int m_hoverPortElmId = 0;
    int m_hoverPortNumber = 0;

    /// Transient label chips shown for the hovered port and its connected peers.
    QList<QPointer<PortHoverLabel>> m_peerLabels;

    /// Peers actually given hoverEnter() by the current hover, stored as element ID + port
    /// index (same scheme as m_hoverPortElmId/m_hoverPortNumber — Port isn't a QObject,
    /// so it can't be tracked with QPointer, and a raw pointer would dangle if the peer's
    /// element is deleted mid-hover) so releaseHoverPort() can hoverLeave() exactly this set
    /// even if connectivity changes mid-hover.
    QList<QPair<int, int>> m_highlightedPeers;
};
