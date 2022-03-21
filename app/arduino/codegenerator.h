/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <QFile>
#include <QHash>
#include <QTextStream>
#include <QVector>

class GraphicElement;
class QNEPort;
class MappedPin
{
public:
    GraphicElement *elm;
    QString pin;
    QString varName;
    QNEPort *port;
    int portNbr;
    MappedPin(GraphicElement *elm, const QString &pin, const QString &varName, QNEPort *port, int portNbr = 0)
        : elm(elm)
        , pin(pin)
        , varName(varName)
        , port(port)
        , portNbr(portNbr)
    {
    }

    MappedPin() = default;
};

class CodeGenerator
{
public:
    CodeGenerator(const QString &fileName, const QVector<GraphicElement *> &aElements);
    ~CodeGenerator();
    bool generate();

private:
    void declareInputs();
    void declareOutputs();
    void declareAuxVariables();

    QFile file;
    QTextStream out;
    const QVector<GraphicElement *> elements;
    QVector<MappedPin> inputMap, outputMap;
    QHash<QNEPort *, QString> varMap;
    //! carmesim: fix typo in availablePins
    QVector<QString> availablePins;
    void setup();
    void loop();
    unsigned int globalCounter;
    void declareAuxVariablesRec(const QVector<GraphicElement *> &elms, bool isBox = false);
    void assignVariablesRec(const QVector<GraphicElement *> &elms);
    void assignLogicOperator(GraphicElement *elm);
    QString otherPortName(QNEPort *port);
};

#endif /* CODEGENERATOR_H */
