// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ConnectionManager: wire-connection validation shared by every canvas frontend.
 */

#pragma once

#include <QCoreApplication>
#include <QString>

class InputPort;
class OutputPort;

/**
 * \class ConnectionManager
 * \brief Static wire-connection validation rules, shared by every canvas frontend.
 *
 * \details Originally also owned the interactive wire-creation/hover workflow for the Qt
 * Widgets Scene; that half is gone now that GraphicElement/Port/Connection aren't
 * QGraphicsItems (CanvasItem.cpp reimplements the equivalent drag-to-connect workflow
 * directly against its own SpatialIndex, reusing only these two static rules). Kept as a
 * static-only utility class rather than free functions to avoid a call-site rename.
 */
class ConnectionManager
{
    Q_DECLARE_TR_FUNCTIONS(ConnectionManager)

public:
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
};
