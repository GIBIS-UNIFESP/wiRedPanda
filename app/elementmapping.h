/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "logicinput.h"

#include <QHash>
#include <QMap>
#include <QVector>

class Clock;
class GraphicElement;
class IC;
class Input;
class LogicElement;
class QNEPort;

class ElementMapping;
class ICMapping;

using ElementMap = QMap<GraphicElement *, LogicElement *>;
using InputMap = QMap<Input *, LogicElement *>;

class ElementMapping
{
public:
    explicit ElementMapping(const QVector<GraphicElement *> &elms);
    virtual ~ElementMapping();

    static QVector<GraphicElement *> sortGraphicElements(QVector<GraphicElement *> elms);

    ICMapping *getICMapping(IC *ic) const;
    LogicElement *getLogicElement(GraphicElement *elm) const;
    bool canInitialize() const;
    bool canRun() const;
    virtual void initialize();
    void clear();
    void sort();
    void update();

protected:
    // Methods
    static int calculatePriority(GraphicElement *elm, QHash<GraphicElement *, bool> &beingvisited, QHash<GraphicElement *, int> &priority);

    LogicElement *buildLogicElement(GraphicElement *elm);
    void applyConnection(GraphicElement *elm, QNEPort *in);
    void connectElements();
    void generateMap();
    void insertElement(GraphicElement *elm);
    void insertIC(IC *ic);
    void setDefaultValue(GraphicElement *elm, QNEPort *in);
    void sortLogicElements();
    void validateElements();

    // Attributes
    ElementMap m_elementMap;
    InputMap m_inputMap;
    LogicInput m_globalGND;
    LogicInput m_globalVCC;
    QMap<IC *, ICMapping *> m_icMappings;
    QString m_currentFile;
    QVector<Clock *> m_clocks;
    QVector<GraphicElement *> m_elements;
    QVector<LogicElement *> m_deletableElements;
    QVector<LogicElement *> m_logicElms;
    bool m_initialized;
};

