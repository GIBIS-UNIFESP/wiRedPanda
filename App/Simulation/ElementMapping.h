// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Maps graphic elements to their logic counterparts and wires up connections.
 */

#pragma once

#include <memory>

#include <QCoreApplication>
#include <QHash>
#include <QSet>

#include "App/Element/LogicElements/LogicSource.h"

class Clock;
class GraphicElement;
class GraphicElementInput;
class IC;
class ICMapping;
class QNEInputPort;
class QNEPort;

/**
 * \class ElementMapping
 * \brief Builds and maintains the flat simulation graph from a scene's elements.
 *
 * \details During construction, ElementMapping calls generateMap() to create
 * shared_ptr<LogicElement> instances for every graphic element, then
 * connectElements() wires them according to the QNEConnection topology.
 * sort() assigns topological priorities for deterministic update ordering.
 */
class ElementMapping
{
    Q_DECLARE_TR_FUNCTIONS(ElementMapping)

public:
    /**
     * \brief Constructs and initializes the mapping from \a elements.
     * \param elements All graphic elements currently in the scene.
     * \param gnd  Optional shared GND source; created internally if null.
     * \param vcc  Optional shared VCC source; created internally if null.
     */
    explicit ElementMapping(const QVector<GraphicElement *> &elements,
                            std::shared_ptr<LogicSource> gnd = {},
                            std::shared_ptr<LogicSource> vcc = {});

    /// Destructor.
    ~ElementMapping() = default;

    // --- Query ---

    /// Returns the ordered vector of logic elements.
    const QVector<std::shared_ptr<LogicElement>> &logicElms() const;

    // --- Topology queries ---

    /// Returns the topological priority for \a logic (-1 if unknown).
    int priority(const LogicElement *logic) const;

    /// Returns \c true if \a logic participates in a combinational feedback loop.
    bool isInFeedbackLoop(const LogicElement *logic) const;

    /// Returns \c true if any logic element in the mapping is part of a feedback loop.
    bool hasFeedbackElements() const;

    // --- Topology ---

    /// Topologically sorts the logic elements by priority.
    void sort();

private:
    Q_DISABLE_COPY(ElementMapping)

    // --- Mapping generation ---

    void generateMap();

    // --- Connection setup ---

    void connectElements();
    /// Overrides physical predecessors on Rx nodes with their matching Tx node.
    /// Must be called after connectElements() so wireless always wins, and before sort().
    void connectWirelessElements();
    void applyConnection(QNEInputPort *inputPort);

    // --- Sorting ---

    void sortLogicElements();

    // --- Members ---

    std::shared_ptr<LogicSource> m_globalGND;
    std::shared_ptr<LogicSource> m_globalVCC;
    QVector<GraphicElement *> m_elements;
    QVector<std::shared_ptr<LogicElement>> m_logicElms;
    QHash<const LogicElement *, int> m_priorities;
    QSet<const LogicElement *> m_feedbackNodes;
};
