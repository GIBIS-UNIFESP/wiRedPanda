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
public:
    ICPrototype(const QString &fileName);
    void reload();

    void fileName(const QString &newFileName);
    QString fileName() const;
    QString baseName() const;

    void insertICObserver(IC *ic);
    void removeICObserver(IC *ic);

    /**
     * @brief updateLocalIC: Updates all subIC paths inside the file of this prototype.
     * While saving a local project, WiredPanda copies all ICs e subICs to a project local directory.
     * Then, this function is responsible for updating the correct references of IC files to their subICs.
     */
    bool updateLocalIC(const QString &src_fileName, const QString &tgt_fileName, const QString &icDirName);

    int inputSize() const;
    int outputSize() const;

    QString inputLabel(int index) const;
    QString outputLabel(int index) const;

    bool defaultInputValue(int index);
    bool isInputRequired(int index);

    ICMapping *generateMapping() const;

private:
    void clear();

    QString m_fileName;

    ICPrototypeImpl m_ICImpl;
    QVector<IC *> m_icObservers;
};

#endif // ICPROTOTYPE_H
