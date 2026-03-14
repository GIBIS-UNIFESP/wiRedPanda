// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>

#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QTextStream>
#include <QVector>

#include "App/Core/Enums.h"

class GraphicElement;
class IC;
class QNEInputPort;
class QNEPort;

/// Holds the mapping between a circuit pin and its generated SystemVerilog signal name.
class MappedPinSystemVerilog
{
public:
    MappedPinSystemVerilog() = default;
    MappedPinSystemVerilog(GraphicElement *elm, const QString &pin, const QString &varName, QNEPort *port, const int portNumber = 0)
        : m_elm(elm)
        , m_port(port)
        , m_pin(pin)
        , m_varName(varName)
        , m_portNumber(portNumber)
    {
    }

    GraphicElement *m_elm = nullptr;
    QNEPort *m_port = nullptr;
    QString m_pin;
    QString m_varName;
    int m_portNumber = 0;
};

/// Holds metadata for a single IC module during SystemVerilog hierarchical generation.
struct ICModuleInfo {
    QString moduleName;
    QString sourceFile;
    QVector<QString> inputNames;
    QVector<QString> outputNames;
    IC *prototypeIC = nullptr;
    ElementType detectedType = ElementType::Unknown;
    bool generated = false;
};

class SystemVerilogCodeGen
{
    Q_DECLARE_TR_FUNCTIONS(SystemVerilogCodeGen)

public:
    /// Constructs the code generator for the given output \a fileName and circuit \a elements.
    SystemVerilogCodeGen(const QString &fileName, const QVector<GraphicElement *> &elements);

    /// Generates the SystemVerilog output file for the circuit.
    void generate();

    /// Returns the mapped input pins after generation.
    const QVector<MappedPinSystemVerilog> &inputMap() const { return m_inputMap; }
    /// Returns the mapped output pins after generation.
    const QVector<MappedPinSystemVerilog> &outputMap() const { return m_outputMap; }
    /// Returns the top-level module name derived from the file name.
    const QString &moduleName() const { return m_fileName; }

private:
    static QString highLow(Status val);
    static QString removeForbiddenChars(const QString &input);
    static QString stripAccents(const QString &input);
    static bool isSimpleIdentifier(const QString &expr);

    QString otherPortName(QNEPort *port);
    QString otherPortNameImpl(QNEPort *port, QSet<QNEPort *> &visited);
    QString ensureSimpleSignal(const QString &expr);
    void assignVariablesRec(const QVector<GraphicElement *> &elements);
    void collectICTypes(const QVector<GraphicElement *> &elements);
    void declareAuxVariables();
    void declareAuxVariablesRec(const QVector<GraphicElement *> &elements);
    void declareInputs();
    void declareOutputs();
    void emitSequentialBlock(const QString &typeName,
                             const QString &clk, const QString &rawPrst, const QString &rawClr,
                             const std::function<void()> &emitPresetBody,
                             const std::function<void()> &emitClearBody,
                             const std::function<void()> &emitNormalBody);
    void generateICModules();
    void generateSingleICModule(ICModuleInfo &info);
    bool emitBehavioralICModule(ICModuleInfo &info);
    static ElementType detectSequentialICType(const ICModuleInfo &info);
    QString generateLogicExpression(GraphicElement *elm);
    QString generateLogicExpressionImpl(GraphicElement *elm, QSet<QNEPort *> &visited);
    void loop();

    QFile m_file;
    QHash<QNEPort *, QString> m_varMap;
    QHash<QString, QNEInputPort *> m_txInputPorts;
    QHash<QString, ICModuleInfo> m_icModules;
    QHash<IC *, QString> m_instanceNames;
    QStringList m_availablePins;
    QString m_fileName;
    QTextStream m_stream;
    QVector<MappedPinSystemVerilog> m_inputMap;
    QVector<MappedPinSystemVerilog> m_outputMap;
    const QVector<GraphicElement *> m_elements;
    bool m_generatingICModule = false;
    int m_globalCounter = 1;
};
