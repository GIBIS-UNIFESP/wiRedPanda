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

    GraphicElement *getElement(int index);
    ICMapping *generateMapping() const;
    QNEPort *getInput(int index);
    QNEPort *getOutput(int index);
    QString getInputLabel(int index) const;
    QString getOutputLabel(int index) const;
    int getInputSize() const;
    int getOutputSize() const;
    void clear();
    void loadFile(const QString &fileName);
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

