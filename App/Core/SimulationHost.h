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
};
