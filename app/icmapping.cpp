// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icmapping.h"

ICMapping::ICMapping(QString file, const ElementVector &elms, const QNEPortVector &inputs, const QNEPortVector &outputs)
    : ElementMapping(elms, file)
    , icInputs(inputs)
    , icOutputs(outputs)
{
}

ICMapping::~ICMapping() = default;

void ICMapping::initialize()
{
    ElementMapping::initialize();
    for (QNEPort *port : qAsConst(icInputs)) {
        inputs.append(map[port->graphicElement()]);
    }
    for (QNEPort *port : qAsConst(icOutputs)) {
        outputs.append(map[port->graphicElement()]);
    }
}

void ICMapping::clearConnections()
{
    for (LogicElement *in : qAsConst(inputs)) {
        in->clearPredecessors();
    }
    for (LogicElement *out : qAsConst(outputs)) {
        out->clearSucessors();
    }
}

LogicElement *ICMapping::getInput(int index)
{
    Q_ASSERT(index < icInputs.size());
    return (inputs[index]);
}

LogicElement *ICMapping::getOutput(int index)
{
    Q_ASSERT(index < icOutputs.size());
    return (outputs[index]);
}
