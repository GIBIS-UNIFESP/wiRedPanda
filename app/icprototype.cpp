// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icprototype.h"

#include "ic.h"
#include "icmapping.h"
#include "qneport.h"

#include <QFileInfo>

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

int ICPrototype::inputSize() const
{
    return m_ICImpl.inputSize();
}

int ICPrototype::outputSize() const
{
    return m_ICImpl.outputSize();
}

QString ICPrototype::inputLabel(const int index) const
{
    return m_ICImpl.inputLabel(index);
}

QString ICPrototype::outputLabel(const int index) const
{
    return m_ICImpl.outputLabel(index);
}

bool ICPrototype::defaultInputValue(const int index)
{
    return m_ICImpl.input(index)->value();
}

bool ICPrototype::isInputRequired(const int index)
{
    return m_ICImpl.input(index)->isRequired();
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
    // verifyRecursion(fileName);
    clear();
    m_ICImpl.loadFile(m_fileName);
    for (auto *ic : qAsConst(m_icObservers)) {
        ic->loadFile(m_fileName);
    }
}
