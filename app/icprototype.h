/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ICPROTOTYPE_H
#define ICPROTOTYPE_H

#include "icprototypeimpl.h"

#include <QString>
#include <QVector>

class ICMapping;
class GraphicElement;
class IC;

class ICPrototype
{
    QString m_fileName;

    ICPrototypeImpl ICImpl;
    QVector<IC *> icObservers;

public:
    ICPrototype(const QString &fileName);
    void reload();

    void fileName(QString newFileName);
    QString fileName() const;
    QString baseName() const;

    void insertICObserver(IC *ic);
    void removeICObserver(IC *ic);

    bool updateLocalIC(QString fileName, QString icDirName);

    int inputSize() const;
    int outputSize() const;

    QString inputLabel(int index) const;
    QString outputLabel(int index) const;

    bool defaultInputValue(int index);
    bool isInputRequired(int index);

    ICMapping *generateMapping() const;

private:
    void clear();
};

#endif // ICPROTOTYPE_H
