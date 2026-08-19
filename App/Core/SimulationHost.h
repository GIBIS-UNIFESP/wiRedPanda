// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Interface exposing the narrow slice of Scene that Simulation depends on.
 */

#pragma once

#include <QList>

class QGraphicsItem;

/**
 * \class SimulationHost
 * \brief Narrow interface letting Simulation reach its host scene without naming
 * the concrete Scene class.
 */
class SimulationHost
{
public:
    virtual ~SimulationHost() = default;

    /// Returns every item currently in the host scene.
    virtual QList<QGraphicsItem *> simulationItems() const = 0;

    /// Mutes or unmutes audio-producing elements in the host scene.
    virtual void setMuted(bool muted) = 0;

    /// Pauses or resumes audio-producing elements' hardware playback in the host scene,
    /// independent of setMuted(): pausing stops the transport (so playback position
    /// freezes with the simulation clock) rather than merely silencing already-running
    /// playback.
    virtual void setPaused(bool paused) = 0;
};
