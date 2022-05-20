/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QTextStream>
#include <QVector>

class GraphicElement;
class QNEPort;

class MappedPin
{
public:
    MappedPin(GraphicElement *elm, const QString &pin, const QString &varName, QNEPort *port, const int portNbr = 0)
        : m_elm(elm)
        , m_port(port)
        , m_pin(pin)
        , m_varName(varName)
        , m_portNbr(portNbr)
    {
    }

    MappedPin() = default;

    GraphicElement *m_elm;
    QNEPort *m_port;
    QString m_pin;
    QString m_varName;
    int m_portNbr;
};

class CodeGenerator
{
    Q_DECLARE_TR_FUNCTIONS(CodeGenerator)

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
    void declareAuxVariablesRec(const QVector<GraphicElement *> &elms, const bool isBox = false);
    void declareInputs();
    void declareOutputs();
    void loop();
    void setup();

    QFile m_file;
    QHash<QNEPort *, QString> m_varMap;
    QStringList m_availablePins;
    QTextStream m_stream;
    QVector<MappedPin> m_inputMap;
    QVector<MappedPin> m_outputMap;
    const QVector<GraphicElement *> m_elements;
    int m_globalCounter = 1;
};

