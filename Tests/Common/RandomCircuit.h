// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

class Element;
class WorkSpace;
class TraceEngine;

/**
 * \brief The source of a generator's choices: a seeded sequence in the tests, a fuzzer's bytes in plan 6.
 */
class ChoiceSource
{
public:
    virtual ~ChoiceSource() = default;

    /// A value from 0 to \a bound - 1.
    virtual int below(int bound) = 0;

    /// Whether a choice of \a percent in 100 comes up.
    bool chance(int percent) { return below(100) < percent; }
};

/// A seeded 31-bit linear congruential generator: the same sequence on every platform and compiler.
class SeededChoices final : public ChoiceSource
{
public:
    explicit SeededChoices(quint32 seed);

    int below(int bound) override;

private:
    quint32 next();

    quint32 m_state;
};

/// What a generated circuit contains, counted, so a test can check that every feature is generated at all.
struct RandomCircuitFeatures {
    int openInputs = 0;         ///< Inputs left unwired.
    int doubleDriven = 0;       ///< Inputs with two wires, a net of two drivers.
    int feedbackWires = 0;      ///< Wires from an element to one at or before it in creation order.
    int lockedClocks = 0;       ///< Clocks whose lock is set.
    int memoryElements = 0;     ///< Flip-flops and latches.
    int gateElements = 0;       ///< Logic gates.
    int muxElements = 0;        ///< Multiplexers and demultiplexers.
    int truthTables = 0;        ///< Truth tables.
    int delayedElements = 0;    ///< Logic elements given a nonzero delay for temporal mode.
    int wirelessNodes = 0;      ///< Wireless nodes, transmitters and receivers.
    int wiredReceivers = 0;     ///< Receivers that a wire also reaches.
    int memoryClockOwn = 0;     ///< Memory clocks taken from the element's own output.
    int memoryClockInput = 0;   ///< Memory clocks taken from an input element.
    int memoryClockClock = 0;   ///< Memory clocks taken from a clock.
    int icInstances = 0;        ///< Integrated circuits from the building-block files.
    int rippleChains = 0;       ///< Chains of toggle flip-flops past the round budget.
};

/**
 * \brief A generated circuit: its workspace, and the handles a walk and an oracle use.
 */
struct RandomCircuit
{
    std::unique_ptr<WorkSpace> workspace;

    /// Every element in creation order, the order a description and the wiring follow.
    QList<Element *> elements;

    /// Driven by a walk: switches, buttons and rotaries. Constant sources and clocks are not.
    QList<Element *> inputs;

    /// Clocks, which a walk never drives.
    QList<Element *> clocks;

    /// Every element that computes or shows a level, in creation order: what a walk observes.
    QList<Element *> observed;

    /// The delay of each logic element in nanoseconds for temporal mode; the elements carry none of their own.
    QHash<const Element *, int> delays;

    RandomCircuitFeatures features;
};

/**
 * \brief Writes the building-block ICs into \a dir, the first toggle flip-flop before the counter that nests it, and
 * returns their paths; a file already there is kept.
 */
QStringList buildingBlockFiles(const QString &dir);

/**
 * \brief Builds a random circuit from \a choices: sources, gates, memory, multiplexers, truth tables, wireless nodes,
 * LEDs and, given \a blockFiles from buildingBlockFiles(), instances of the building blocks. It is wired at random with
 * some inputs open, some doubly driven, and feedback where the wiring makes it. Some circuits add a chain of toggle
 * flip-flops past the round budget.
 */
RandomCircuit makeRandomCircuit(ChoiceSource &choices, const QStringList &blockFiles = {});

/**
 * \brief Describes \a circuit's elements and wires in creation order, so two circuits of one seed read the same.
 */
QStringList describeRandomCircuit(const RandomCircuit &circuit);

/**
 * \brief Drives \a circuit through \a steps steps of a walk on \a engine: each step changes some inputs at random, then
 * steps the engine, and records what each observed element shows.
 * \return One line per step, led by the step's index.
 */
QStringList randomCircuitWalk(RandomCircuit &circuit, TraceEngine &engine, ChoiceSource &choices, int steps);
