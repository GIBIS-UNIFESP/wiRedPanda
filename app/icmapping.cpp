// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icmapping.h"

ICMapping::ICMapping(const ElementVector &elms, const QNEPortVector &inputs, const QNEPortVector &outputs)
    : ElementMapping(elms)
    , m_icInputs(inputs)
    , m_icOutputs(outputs)
{
}

ICMapping::~ICMapping() = default;

void ICMapping::initialize()
{
    ElementMapping::initialize();
    for (auto *port : qAsConst(m_icInputs)) {
        m_inputs.append(m_elementMap[port->graphicElement()]);
    }
    for (auto *port : qAsConst(m_icOutputs)) {
        m_outputs.append(m_elementMap[port->graphicElement()]);
    }
}

void ICMapping::clearConnections()
{
    for (auto *in : qAsConst(m_inputs)) {
        in->clearPredecessors();
    }
    for (auto *out : qAsConst(m_outputs)) {
        out->clearSucessors();
    }
}

LogicElement *ICMapping::getInput(int index)
{
    Q_ASSERT(index < m_icInputs.size());
    return m_inputs[index];
}

LogicElement *ICMapping::getOutput(int index)
{
    Q_ASSERT(index < m_icOutputs.size());
    return m_outputs[index];
}
