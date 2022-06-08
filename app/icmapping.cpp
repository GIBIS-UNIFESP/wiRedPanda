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
}

void ICMapping::clearConnections()
{
    for (auto *portIn : qAsConst(m_icInputs)) {
        portIn->logicElement()->clearPredecessors();
    }

    for (auto *portOut : qAsConst(m_icOutputs)) {
        portOut->logicElement()->clearSucessors();
    }
}

LogicElement *ICMapping::input(const int index)
{
    return m_icInputs.at(index)->logicElement();
}

LogicElement *ICMapping::output(const int index)
{
    return m_icOutputs.at(index)->logicElement();
}
