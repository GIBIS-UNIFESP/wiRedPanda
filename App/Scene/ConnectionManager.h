// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ConnectionManager: manages wire creation, deletion, validation and hover feedback.
 */

#pragma once

#include <QPointF>

class GraphicElement;
class QNEConnection;
class QNEInputPort;
class QNEOutputPort;
class QNEPort;
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
public:
    explicit ConnectionManager(Scene *scene);

    // --- Wire creation workflow ---

    /// Begins a new in-progress wire anchored at \a startPort (output port).
    void startFromOutput(QNEOutputPort *startPort);

    /// Begins a new in-progress wire anchored at \a endPort (input port).
    void startFromInput(QNEInputPort *endPort);

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
    void detach(QNEInputPort *endPort);

    // --- In-progress wire state ---

    /// Returns true if a wire is currently being drawn.
    [[nodiscard]] bool hasEditedConnection() const;

    /// Returns the in-progress wire, or nullptr.
    [[nodiscard]] QNEConnection *editedConnection() const;

    /// Updates the free end of the in-progress wire to follow \a scenePos.
    void updateEditedEnd(const QPointF &scenePos);

    // --- Hover feedback ---

    /// Updates the hover state for the port under \a scenePos.
    void updateHover(const QPointF &scenePos);

    /// Clears the current hover state (unhighlights the port, resets cursor).
    void clearHover();

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
    static bool isConnectionAllowed(QNEOutputPort *startPort, QNEInputPort *endPort);

private:
    void setEditedConnection(QNEConnection *connection);
    void deleteEditedConnection();

    void setHoverPort(QNEPort *port);
    void releaseHoverPort();
    [[nodiscard]] QNEPort *hoverPort();

    Scene *m_scene = nullptr;

    /// ID of the in-progress wire (looked up via Scene::itemById).
    int m_editedConnectionId = 0;

    /// Hover-port tracking stored as element ID + port index so it survives undo/redo.
    int m_hoverPortElmId = 0;
    int m_hoverPortNumber = 0;
};

