/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ICPROTOTYPEIMPL_H
#define ICPROTOTYPEIMPL_H

#include <QVector>

class GraphicElement;
class QGraphicsItem;
class QNEPort;

class ICPrototypeImpl
{
public:
    QVector<GraphicElement *> elements;
    QVector<QString> inputLabels;
    QVector<QString> outputLabels;

    QVector<QNEPort *> inputs;
    QVector<QNEPort *> outputs;

    ~ICPrototypeImpl();
    void loadFile(const QString &fileName);
    void clear();

    int getInputSize() const;
    int getOutputSize() const;
    void setOutputSize(int outSize);
    void setInputSize(int inSize);

    void loadInputElement(GraphicElement *elm);
    void loadOutputElement(GraphicElement *elm);
    bool updateLocalIC(const QString &fileName, const QString &icDirName);

private:
    void sortPorts(QVector<QNEPort *> &map);
    void loadItem(QGraphicsItem *item);
    void loadInputs();
    void loadOutputs();
};

#endif // ICPROTOTYPEIMPL_H
