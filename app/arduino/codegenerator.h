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
    MappedPin() = default;
    MappedPin(GraphicElement *elm, const QString &pin, const QString &varName, QNEPort *port, const int portNbr = 0)
        : m_elm(elm)
        , m_port(port)
        , m_pin(pin)
        , m_varName(varName)
        , m_portNbr(portNbr)
    {
    }

    GraphicElement *m_elm = nullptr;
    QNEPort *m_port = nullptr;
    QString m_pin;
    QString m_varName;
    int m_portNbr = 0;
};

class CodeGenerator
{
    Q_DECLARE_TR_FUNCTIONS(CodeGenerator)

public:
    CodeGenerator(const QString &fileName, const QVector<GraphicElement *> &elements);

    void generate();

private:
    static QString removeForbiddenChars(const QString &input);

    QString otherPortName(QNEPort *port);
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
