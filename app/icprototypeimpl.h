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
class ICMapping;

class ICPrototypeImpl
{
public:
    ~ICPrototypeImpl();
    void loadFile(const QString &fileName);
    void clear();

    int getInputSize() const;
    int getOutputSize() const;
    void setOutputSize(int outSize);
    void setInputSize(int inSize);

    void loadInputElement(GraphicElement *elm);
    void loadOutputElement(GraphicElement *elm);
    /**
     * @brief updateLocalIC: Updates all subIC paths inside the file of this prototype.
     * While saving a local project, WiredPanda copies all ICs e subICs to a project local directory.
     * Then, this function is responsible for updating the correct references of IC files to their subICs.
     */
    bool  updateLocalIC(const QString &src_fileName, const QString &tgt_fileName, const QString &icDirName);

    GraphicElement *getElement(int index);
    QString getInputLabel(int index) const;
    QString getOutputLabel(int index) const;
    QNEPort *getInput(int index);
    QNEPort *getOutput(int index);
    ICMapping *generateMapping() const;

private:
    void sortPorts(QVector<QNEPort *> &map);
    void loadItem(QGraphicsItem *item);
    void loadInputs();
    void loadOutputs();

    QVector<GraphicElement *> m_elements;
    QVector<QString> m_inputLabels;
    QVector<QString> m_outputLabels;

    QVector<QNEPort *> m_inputs;
    QVector<QNEPort *> m_outputs;
};

#endif // ICPROTOTYPEIMPL_H
