// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QVector>

#include "App/Core/Enums.h"

class Element;
class Scene;

/**
 * \brief The engine a trace walk steps: a circuit is stepped one update at a time, and what its elements then drive is
 * read back.
 * \details master's tick engine is the first implementation, and the reference simulator of plan 5 the second; a walk
 * runs through this interface, so it reads either engine the same way.
 */
class TraceEngine
{
public:
    virtual ~TraceEngine() = default;

    /// Runs one step: the inputs' levels set since the last step take effect, and the circuit settles.
    virtual void step() = 0;

    /// Whether the engine's own timer runs, which would let clocks follow the wall clock.
    [[nodiscard]] virtual bool timerRuns() const = 0;

    /// What \a element drives, after the last step, read from the engine's state for it.
    [[nodiscard]] virtual QVector<Status> observed(const Element *element) const = 0;
};

/// The engine of master: \a scene's simulation, stepped by hand with its timer never started, so a clock keeps the
/// level the first update gives it.
std::unique_ptr<TraceEngine> makeMasterTraceEngine(Scene *scene);
