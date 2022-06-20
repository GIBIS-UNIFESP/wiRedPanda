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
    ICPrototypeImpl() = default;
    ~ICPrototypeImpl();

    GraphicElement *element(const int index);
    ICMapping *generateMapping() const;
    QNEPort *input(const int index);
    QNEPort *output(const int index);
    QString inputLabel(const int index) const;
    QString outputLabel(const int index) const;
    int inputSize() const;
    int outputSize() const;
    void clear();
    void loadFile(const QString &fileName);
    void loadInputElement(GraphicElement *elm);
    void loadOutputElement(GraphicElement *elm);
    void setInputSize(const int inputSize);
    void setOutputSize(const int outputSize);

private:
    Q_DISABLE_COPY(ICPrototypeImpl)

    static void sortPorts(QVector<QNEPort *> &map);

    void loadInputs();
    void loadItem(QGraphicsItem *item);
    void loadOutputs();

    QVector<GraphicElement *> m_elements;
    QVector<QNEPort *> m_inputs;
    QVector<QNEPort *> m_outputs;
    QVector<QString> m_inputLabels;
    QVector<QString> m_outputLabels;
};
