// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief SimulationHost: the narrow interface the Simulation engine needs from its scene.
 */

#pragma once

#include <QList>

class QGraphicsItem;

/**
 * \class SimulationHost
 * \brief Minimal interface the Simulation engine depends on, implemented by Scene.
 *
 * \details Lets Simulation pull the item list and mute audible outputs without a
 * concrete dependency on Scene (which sits above it). Mirrors the MainWindowHost /
 * DolphinHost injected-interface idiom.
 */
class SimulationHost
{
public:
    virtual ~SimulationHost() = default;

    /// All items the simulation should consider (the host's full item list).
    virtual QList<QGraphicsItem *> simulationItems() const = 0;

    /// Mutes or unmutes audible output elements (buzzers).
    virtual void setMuted(bool muted) = 0;
};
