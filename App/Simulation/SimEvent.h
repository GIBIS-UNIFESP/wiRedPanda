// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Event struct and time-ordered queue for the unified event-driven engine.
 */

#pragma once

#include <queue>
#include <vector>

#include <QtGlobal>

#include "App/Simulation/SimTime.h"

class GraphicElement;

/// What a SimEvent does to its target when it fires.
enum class SimEventKind {
    Evaluate = 0, ///< Re-read live inputs and STAGE new outputs (no publication).
    Publish  = 1, ///< Apply the staged outputs, then evaluate the successors.
};

/**
 * \struct SimEvent
 * \brief A scheduled action on an element at a specific simulation time.
 *
 * \details Publish-side delay: an element evaluates *immediately* when an input it reads
 * changes, staging its new outputs, and schedules a Publish for `now + its own delay`. Only
 * one Publish per element is live at a time — scheduling another supersedes it via
 * \a generation, which is what makes the model genuinely inertial: an input that reverts
 * before the delay elapses re-stages the already-published value, and the superseding Publish
 * then changes nothing. Events carry no value: it lives in the element's staging buffer, and
 * supersession decides whether it is ever published.
 */
struct SimEvent
{
    SimTime time = 0;                 ///< When the event fires.
    int priority = -1;                ///< Topological priority of the target (higher = upstream).
    GraphicElement *target = nullptr; ///< Element to act on.
    SimEventKind kind = SimEventKind::Evaluate; ///< Evaluate (stage) or Publish (apply).
    quint64 generation = 0;           ///< Supersession stamp; a Publish with a stale one is dropped.

    /// Ordering for the min-heap: earliest time first, then HIGHEST topological priority first.
    /// Processing same-time events highest-priority-first means a gate is never evaluated until
    /// its upstream inputs have settled — so internally-generated (e.g. gated) clocks reach their
    /// final value before any flip-flop downstream samples them (no zero-delay glitches).
    bool operator>(const SimEvent &other) const
    {
        if (time != other.time) {
            return time > other.time;
        }
        // Evaluate before Publish at the same instant: every element that reads a signal has
        // staged its response before any staged value becomes visible. That phase separation is
        // what preserves non-blocking semantics at zero delay, where Publish lands on the same
        // timestamp as the Evaluate that scheduled it.
        if (kind != other.kind) {
            return kind > other.kind;
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
