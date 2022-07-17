// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicinput.h"

#include <QCoreApplication>
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
    explicit ElementMapping(const QVector<GraphicElement *> &elms);
    ~ElementMapping();

    const QVector<std::shared_ptr<LogicElement> > &logicElms() const;
    void sort();

private:
    Q_DISABLE_COPY(ElementMapping)

    void applyConnection(GraphicElement *elm, QNEInputPort *inputPort);
    void clear();
    void connectElements();
    void generateLogic(GraphicElement *elm);
    void generateMap();
    void setDefaultValue(GraphicElement *elm, QNEPort *in);
    void sortLogicElements();
    void validateElements();

    LogicInput m_globalGND{false};
    LogicInput m_globalVCC{true};
    QVector<GraphicElement *> m_elements;
    QVector<std::shared_ptr<LogicElement>> m_logicElms;
};
