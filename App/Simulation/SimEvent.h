// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Event struct and time-ordered queue for the unified event-driven engine.
 */

#pragma once

#include <queue>
#include <vector>

#include "App/Simulation/SimTime.h"

class GraphicElement;

/**
 * \struct SimEvent
 * \brief A scheduled re-evaluation of an element at a specific simulation time.
 *
 * \details Inertial-delay model: events carry no value. When an event fires, the
 * target re-reads its live predecessor outputs via updateLogic(), so a pulse shorter
 * than the gate's delay is naturally absorbed (the input has already settled back).
 */
struct SimEvent
{
    SimTime time = 0;                 ///< When the event fires.
    int priority = -1;                ///< Topological priority of the target (higher = upstream).
    GraphicElement *target = nullptr; ///< Element to re-evaluate.

    /// Ordering for the min-heap: earliest time first, then HIGHEST topological priority first.
    /// Processing same-time events highest-priority-first means a gate is never evaluated until
    /// its upstream inputs have settled — so internally-generated (e.g. gated) clocks reach their
    /// final value before any flip-flop downstream samples them (no zero-delay glitches).
    bool operator>(const SimEvent &other) const
    {
        if (time != other.time) {
            return time > other.time;
        }
        return priority < other.priority;
    }
};

/**
 * \class EventQueue
 * \brief Min-heap priority queue of simulation events, ordered by time.
 *
 * \note The engine pops one event at a time; ordering comes entirely from the heap
 * comparator (time ascending, then topological priority descending), so within a
 * timestamp the most-upstream element always drains first. Only FULL ties — same time
 * AND same priority, i.e. independent elements at equal topological depth — pop in
 * unspecified order, which cannot affect results: such elements never feed each other
 * within that timestamp, and each re-reads its live inputs when evaluated.
 */
class EventQueue
{
public:
    void schedule(const SimEvent &event) { m_queue.push(event); }

    SimEvent pop()
    {
        auto event = m_queue.top();
        m_queue.pop();
        return event;
    }

    bool empty() const { return m_queue.empty(); }
    SimTime nextTime() const { return m_queue.top().time; }
    void clear() { m_queue = {}; }
    int size() const { return static_cast<int>(m_queue.size()); }

private:
    std::priority_queue<SimEvent, std::vector<SimEvent>, std::greater<SimEvent>> m_queue;
};
