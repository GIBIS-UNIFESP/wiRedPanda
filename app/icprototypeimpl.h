/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QVector>

class GraphicElement;
class QGraphicsItem;
class QNEPort;
class ICMapping;

class ICPrototypeImpl
{
public:
    ~ICPrototypeImpl();
    GraphicElement *element(int index);
    ICMapping *generateMapping() const;
    QNEPort *input(int index);
    QNEPort *output(int index);
    QString inputLabel(int index) const;
    QString outputLabel(int index) const;
    int inputSize() const;
    int outputSize() const;
    void clear();
    bool loadFile(const QString &fileName);
    void loadInputElement(GraphicElement *elm);
    void loadOutputElement(GraphicElement *elm);
    void setInputSize(int inputSize);
    void setOutputSize(int outputSize);

private:
    void loadInputs();
    void loadItem(QGraphicsItem *item);
    void loadOutputs();
    void sortPorts(QVector<QNEPort *> &map);

    QVector<GraphicElement *> m_elements;
    QVector<QNEPort *> m_inputs;
    QVector<QNEPort *> m_outputs;
    QVector<QString> m_inputLabels;
    QVector<QString> m_outputLabels;
};

