/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QFile>
#include <QHash>
#include <QTextStream>
#include <QVector>

class GraphicElement;
class QNEPort;

class MappedPin
{
public:
    MappedPin(GraphicElement *elm, const QString &pin, const QString &varName, QNEPort *port, int portNbr = 0)
        : elm(elm)
        , pin(pin)
        , varName(varName)
        , port(port)
        , portNbr(portNbr)
    {
    }

    MappedPin() = default;

    GraphicElement *elm;
    QString pin;
    QString varName;
    QNEPort *port;
    int portNbr;
};

class CodeGenerator
{
public:
    CodeGenerator(const QString &fileName, const QVector<GraphicElement *> &aElements);
    ~CodeGenerator();

    bool generate();

private:
    QString otherPortName(QNEPort *port);
    static QString removeForbiddenChars(const QString &input);
    void assignLogicOperator(GraphicElement *elm);
    void assignVariablesRec(const QVector<GraphicElement *> &elms);
    void declareAuxVariables();
    void declareAuxVariablesRec(const QVector<GraphicElement *> &elms, bool isBox = false);
    void declareInputs();
    void declareOutputs();
    void loop();
    void setup();

    QFile file;
    QHash<QNEPort *, QString> varMap;
    QTextStream out;
    QVector<MappedPin> inputMap, outputMap;
    QVector<QString> availablePins;
    const QVector<GraphicElement *> elements;
    unsigned int globalCounter;
};

