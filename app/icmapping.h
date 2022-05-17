/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "elementmapping.h"
#include "graphicelement.h"
#include "qneport.h"

class LogicElement;

class ICMapping : public ElementMapping
{
public:
    ICMapping(const ElementVector &elms, const QNEPortVector &inputs, const QNEPortVector &outputs);
    ~ICMapping() override;

    LogicElement *input(const int index);
    LogicElement *output(const int index);
    void clearConnections();
    void initialize() override;

private:
    QNEPortVector m_icInputs;
    QNEPortVector m_icOutputs;
    QVector<LogicElement *> m_inputs;
    QVector<LogicElement *> m_outputs;
};

