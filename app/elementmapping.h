// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicinput.h"

#include <QCoreApplication>
#include <QSet>
#include <memory>

class Clock;
class GraphicElement;
class GraphicElementInput;
class IC;
class ICMapping;
class QNEInputPort;
class QNEPort;

class ElementMapping
{
    Q_DECLARE_TR_FUNCTIONS(ElementMapping)

public:
    explicit ElementMapping(const QVector<GraphicElement *> &elements);
    ~ElementMapping();

    const QVector<std::shared_ptr<LogicElement>> &logicElms() const;
    void sort();
    
    // New method to access feedback groups
    const QVector<QVector<std::shared_ptr<LogicElement>>>& feedbackGroups() const;
    
    // Public method to re-detect feedback loops
    void detectFeedbackLoops();
    
    // Public method to establish connections on existing LogicElements
    void connectElements();

private:
    Q_DISABLE_COPY(ElementMapping)

    void applyConnection(GraphicElement *elm, QNEInputPort *inputPort);
    void generateLogic(GraphicElement *elm);
    void generateMap();
    void setDefaultValue(GraphicElement *elm, QNEPort *in);
    void sortLogicElements();
    void validateElements();
    
    // Private members for feedback circuit handling
    void findCycles(LogicElement *current, QSet<LogicElement*> &visited,
                    QSet<LogicElement*> &recursionStack, QVector<LogicElement*> &currentPath);
    void markFeedbackCycle(const QVector<LogicElement*> &path, LogicElement *cycleStart);

    LogicInput m_globalGND{false};
    LogicInput m_globalVCC{true};
    QVector<GraphicElement *> m_elements;
    QVector<std::shared_ptr<LogicElement>> m_logicElms;
    
    // New member for feedback circuit handling
    QVector<QVector<std::shared_ptr<LogicElement>>> m_feedbackGroups;
};
