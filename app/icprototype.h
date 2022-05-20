/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "icprototypeimpl.h"

#include <QString>
#include <QVector>

class ICMapping;
class GraphicElement;
class IC;

class ICPrototype
{
public:
    explicit ICPrototype(const QString &fileName);
    ~ICPrototype() = default;

    ICMapping *generateMapping() const;
    QString baseName() const;
    QString fileName() const;
    QString inputLabel(const int index) const;
    QString outputLabel(const int index) const;
    bool defaultInputValue(const int index);
    bool isInputRequired(const int index);
    int inputSize() const;
    int outputSize() const;
    void fileName(const QString &newFileName);
    void insertICObserver(IC *ic);
    void reload();
    void removeICObserver(IC *ic);

private:
    void clear();

    ICPrototypeImpl m_ICImpl;
    QString m_fileName;
    QVector<IC *> m_icObservers;
};

