// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icprototype.h"

#include <QFileInfo>

#include "ic.h"
#include "icmapping.h"

ICPrototype::ICPrototype(const QString &fileName)
    : m_fileName(fileName)
{
}

void ICPrototype::fileName(QString newFileName)
{
    m_fileName = newFileName;
}

QString ICPrototype::fileName() const
{
    return (m_fileName);
}

QString ICPrototype::baseName() const
{
    return (QFileInfo(m_fileName).baseName());
}

void ICPrototype::insertICObserver(IC *ic)
{
    if (!icObservers.contains(ic)) {
        icObservers.append(ic);
    }
}

void ICPrototype::removeICObserver(IC *ic)
{
    if (icObservers.contains(ic)) {
        icObservers.removeAll(ic);
    }
}

bool ICPrototype::updateLocalIC(QString fileName, QString dirName)
{
    return (ICImpl.updateLocalIC(fileName, dirName));
}

int ICPrototype::inputSize() const
{
    return (ICImpl.getInputSize());
}

int ICPrototype::outputSize() const
{
    return (ICImpl.getOutputSize());
}

QString ICPrototype::inputLabel(int index) const
{
    return (ICImpl.inputLabels[index]);
}

QString ICPrototype::outputLabel(int index) const
{
    return (ICImpl.outputLabels[index]);
}

bool ICPrototype::defaultInputValue(int index)
{
    return (ICImpl.inputs[index]->value());
}

bool ICPrototype::isInputRequired(int index)
{
    return (ICImpl.inputs[index]->isRequired());
}

ICMapping *ICPrototype::generateMapping() const
{
    return (new ICMapping(fileName(), ICImpl.elements, ICImpl.inputs, ICImpl.outputs));
}

void ICPrototype::clear()
{
    ICImpl.clear();
}

void ICPrototype::reload()
{
    // TODO: Verify file recursion
    //  verifyRecursion( fname );
    clear();
    ICImpl.loadFile(m_fileName);
    for (IC *ic : qAsConst(icObservers)) {
        ic->loadFile(m_fileName);
    }
}
