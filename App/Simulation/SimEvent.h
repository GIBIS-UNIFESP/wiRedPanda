// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Event struct and priority queue for the temporal simulation engine.
 */

#pragma once

#include <queue>
#include <vector>

#include "App/Simulation/SimTime.h"

class LogicElement;

/**
 * \struct SimEvent
 * \brief A scheduled re-evaluation of a logic element at a specific simulation time.
 *
 * \details In the inertial delay model, events simply mark "re-evaluate this
 * element at this time."  The element reads live predecessor outputs when
 * evaluated, so no value needs to be carried in the event.
 */
struct SimEvent
{
    SimTime time = 0;              ///< When the event fires (nanoseconds).
    LogicElement *target = nullptr; ///< Element to re-evaluate.

    /// Min-heap comparison: earliest time first; ties broken by insertion order (stable).
    bool operator>(const SimEvent &other) const { return time > other.time; }
};

/**
 * \class EventQueue
 * \brief Min-heap priority queue of simulation events, ordered by time.
 */
class EventQueue
{
public:
    /// Schedules an event for future processing.
    void schedule(const SimEvent &event) { m_queue.push(event); }

    /// Removes and returns the earliest event.
    SimEvent pop()
    {
        auto event = m_queue.top();
        m_queue.pop();
        return event;
    }

    /// Returns \c true if no events are pending.
    bool empty() const { return m_queue.empty(); }

    /// Returns the time of the earliest pending event.
    SimTime nextTime() const { return m_queue.top().time; }

    /// Removes all pending events.
    void clear() { m_queue = {}; }

    /// Returns the number of pending events.
    int size() const { return static_cast<int>(m_queue.size()); }

private:
    std::priority_queue<SimEvent, std::vector<SimEvent>, std::greater<SimEvent>> m_queue;
};
