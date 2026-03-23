// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Maps graphic elements to logic elements for simulation.
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
 * \brief Translates a set of GraphicElements into a flat vector of LogicElements.
 */
class ElementMapping
{
    Q_DECLARE_TR_FUNCTIONS(ElementMapping)

public:
    explicit ElementMapping(const QVector<GraphicElement *> &elements);

    ~ElementMapping() = default;

    // --- Query ---

    const QVector<std::shared_ptr<LogicElement>> &logicElms() const;

    // --- Topology queries ---

    int priority(const LogicElement *logic) const;
    bool isInFeedbackLoop(const LogicElement *logic) const;
    bool hasFeedbackElements() const;

    // --- Topology ---

    void sort();

private:
    Q_DISABLE_COPY(ElementMapping)

    void generateMap();
    void connectElements();
    void applyConnection(QNEInputPort *inputPort);
    void validateElements();
    void sortLogicElements();

    // --- Members ---

    LogicSource m_globalGND{false};
    LogicSource m_globalVCC{true};
    QVector<GraphicElement *> m_elements;
    QVector<std::shared_ptr<LogicElement>> m_logicElms;
    QHash<const LogicElement *, int> m_priorities;
    QSet<const LogicElement *> m_feedbackNodes;
};

