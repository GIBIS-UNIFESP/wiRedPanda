// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icmapping.h"

ICMapping::ICMapping(QString file, const ElementVector &elms, const QNEPortVector &inputs, const QNEPortVector &outputs)
    : ElementMapping(elms, file)
    , m_icInputs(inputs)
    , m_icOutputs(outputs)
{
}

ICMapping::~ICMapping() = default;

void ICMapping::initialize()
{
    ElementMapping::initialize();
    for (QNEPort *port : qAsConst(m_icInputs)) {
        m_inputs.append(map[port->graphicElement()]);
    }
    for (QNEPort *port : qAsConst(m_icOutputs)) {
        m_outputs.append(map[port->graphicElement()]);
    }
}

void ICMapping::clearConnections()
{
    for (LogicElement *in : qAsConst(m_inputs)) {
        in->clearPredecessors();
    }
    for (LogicElement *out : qAsConst(m_outputs)) {
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
