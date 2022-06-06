/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "logicinput.h"

#include <QCoreApplication>
#include <QMap>

class Clock;
class GraphicElement;
class GraphicElementInput;
class IC;
class ICMapping;
class QNEPort;

class ElementMapping
{
    Q_DECLARE_TR_FUNCTIONS(ElementMapping)

public:
    explicit ElementMapping(const QVector<GraphicElement *> &elms);
    virtual ~ElementMapping();

    static QVector<GraphicElement *> sortGraphicElements(QVector<GraphicElement *> elms);

    ICMapping *icMapping(IC *ic) const;
    bool canInitialize() const;
    bool canRun() const;
    virtual void initialize();
    void clear();
    void sort();
    void update();

private:
    static LogicElement *buildLogicElement(GraphicElement *elm);
    static int calculatePriority(GraphicElement *elm, QHash<GraphicElement *, bool> &beingVisited, QHash<GraphicElement *, int> &priority);

    void applyConnection(GraphicElement *elm, QNEPort *in);
    void connectElements();
    void generateMap();
    void insertElement(GraphicElement *elm);
    void insertIC(IC *ic);
    void setDefaultValue(GraphicElement *elm, QNEPort *in);
    void sortLogicElements();
    void validateElements();

    LogicInput m_globalGND{false};
    LogicInput m_globalVCC{true};
    QMap<IC *, ICMapping *> m_icMappings;
    QVector<Clock *> m_clocks;
    QVector<GraphicElement *> m_elements;
    QVector<GraphicElementInput *> m_inputs;
    QVector<LogicElement *> m_deletableElements;
    QVector<LogicElement *> m_logicElms;
    bool m_initialized = false;
};
