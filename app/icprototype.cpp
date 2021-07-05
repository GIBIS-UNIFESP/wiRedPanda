// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icprototype.h"

#include <QFileInfo>

#include "ic.h"
#include "icmapping.h"
#include "qneport.h"

ICPrototype::ICPrototype(const QString &fileName)
    : m_fileName(fileName)
{
}

void ICPrototype::fileName(const QString &newFileName)
{
    m_fileName = newFileName;
}

QString ICPrototype::fileName() const
{
    return m_fileName;
}

QString ICPrototype::baseName() const
{
    return QFileInfo(m_fileName).baseName();
}

void ICPrototype::insertICObserver(IC *ic)
{
    if (!m_icObservers.contains(ic)) {
        m_icObservers.append(ic);
    }
}

void ICPrototype::removeICObserver(IC *ic)
{
    if (m_icObservers.contains(ic)) {
        m_icObservers.removeAll(ic);
    }
}

bool ICPrototype::updateLocalIC(const QString &fileName, const QString &dirName)
{
    return m_ICImpl.updateLocalIC(fileName, dirName);
}

int ICPrototype::inputSize() const
{
    return m_ICImpl.getInputSize();
}

int ICPrototype::outputSize() const
{
    return m_ICImpl.getOutputSize();
}

QString ICPrototype::inputLabel(int index) const
{
    return m_ICImpl.getInputLabel(index);
}

QString ICPrototype::outputLabel(int index) const
{
    return m_ICImpl.getOutputLabel(index);
}

bool ICPrototype::defaultInputValue(int index)
{
    return m_ICImpl.getInput(index)->value();
}

bool ICPrototype::isInputRequired(int index)
{
    return m_ICImpl.getInput(index)->isRequired();
}

ICMapping *ICPrototype::generateMapping() const
{
    return m_ICImpl.generateMapping();
}

void ICPrototype::clear()
{
    m_ICImpl.clear();
}

void ICPrototype::reload()
{
    // TODO: Verify file recursion
    //  verifyRecursion( fname );
    clear();
    m_ICImpl.loadFile(m_fileName);
    for (IC *ic : qAsConst(m_icObservers)) {
        ic->loadFile(m_fileName);
    }
}
