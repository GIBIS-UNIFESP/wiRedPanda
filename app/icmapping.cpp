// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icmapping.h"

ICMapping::ICMapping(const QVector<GraphicElement *> &elms, const QVector<QNEPort *> &inputs, const QVector<QNEPort *> &outputs)
    : ElementMapping(elms)
    , m_icInputs(inputs)
    , m_icOutputs(outputs)
{
}

void ICMapping::initialize()
{
    ElementMapping::initialize();
    for (auto *port : qAsConst(m_icInputs)) {
        m_inputs.append(port->logicElement());
    }
    for (auto *port : qAsConst(m_icOutputs)) {
        m_outputs.append(port->logicElement());
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

LogicElement *ICMapping::input(const int index)
{
    return m_inputs[index];
}

LogicElement *ICMapping::output(const int index)
{
    return m_outputs[index];
}
