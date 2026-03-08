// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Maps graphic elements to their logic counterparts and wires up connections.
 */

#pragma once

#include <memory>

#include <QCoreApplication>

#include "App/Element/LogicElements/LogicInput.h"

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
     */
    explicit ElementMapping(const QVector<GraphicElement *> &elements);

    /// Destructor.
    ~ElementMapping() = default;

    // --- Query ---

    /// Returns the ordered vector of logic elements.
    const QVector<std::shared_ptr<LogicElement>> &logicElms() const;

    // --- Topology ---

    /// Topologically sorts the logic elements by priority.
    void sort();

private:
    Q_DISABLE_COPY(ElementMapping)

    // --- Mapping generation ---

    void generateMap();
    void generateLogic(GraphicElement *elm);

    // --- Connection setup ---

    void connectElements();
    void applyConnection(QNEInputPort *inputPort);

    // --- Validation & sorting ---

    void validateElements();
    void sortLogicElements();

    // --- Members ---

    LogicInput m_globalGND{false};
    LogicInput m_globalVCC{true};
    QVector<GraphicElement *> m_elements;
    QVector<std::shared_ptr<LogicElement>> m_logicElms;
};
