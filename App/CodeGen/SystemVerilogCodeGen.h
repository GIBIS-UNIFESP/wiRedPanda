// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief SystemVerilog code generator: translates a circuit into a synthesisable module.
 */

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

/**
 * \class MappedPinSystemVerilog
 * \brief Maps a circuit port to its generated SystemVerilog signal name and module port.
 */
class MappedPinSystemVerilog
{
public:
    MappedPinSystemVerilog() = default;
    /**
     * \brief Constructs a pin mapping.
     * \param elm        Owning graphic element.
     * \param pin        Top-level module port name.
     * \param varName    Internal signal/wire name in the generated code.
     * \param port       Source QNEPort this mapping refers to.
     * \param portNumber Index of the port within the element (default 0).
     */
    MappedPinSystemVerilog(GraphicElement *elm, const QString &pin, const QString &varName, QNEPort *port, const int portNumber = 0)
        : m_elm(elm)
        , m_port(port)
        , m_pin(pin)
        , m_varName(varName)
        , m_portNumber(portNumber)
    {
    }

    GraphicElement *m_elm = nullptr; ///< Owning graphic element.
    QNEPort *m_port = nullptr;       ///< Source port in the circuit graph.
    QString m_pin;                   ///< Top-level module port name.
    QString m_varName;               ///< Generated internal signal name.
    int m_portNumber = 0;            ///< Port index within the element.
};

/**
 * \struct ICModuleInfo
 * \brief Metadata for a single IC module during SystemVerilog hierarchical generation.
 */
struct ICModuleInfo {
    QString moduleName;                        ///< Generated module identifier.
    QString sourceFile;                        ///< Original .panda file path (or blob name).
    QVector<QString> inputNames;               ///< Port names for the module's inputs.
    QVector<QString> outputNames;              ///< Port names for the module's outputs.
    IC *prototypeIC = nullptr;                 ///< Representative IC element (used to inspect sub-circuit).
    ElementType detectedType = ElementType::Unknown; ///< Detected sequential element type, if any.
    bool generated = false;                    ///< True once the module body has been emitted.
};

/**
 * \class SystemVerilogCodeGen
 * \brief Generates a synthesisable SystemVerilog module from a circuit's element graph.
 *
 * \details Walks the topologically sorted element list, maps each gate/flip-flop to
 * SystemVerilog constructs, and handles hierarchical IC modules by recursively
 * generating sub-modules.  Wireless Tx/Rx nodes are resolved via the same
 * label-matching used by the simulation engine.
 */
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
    // --- String Utilities ---

    /// Returns "1'b1" or "1'b0" for Active/Inactive status values.
    static QString highLow(Status val);
    /// Strips characters that are illegal in SystemVerilog identifiers.
    static QString removeForbiddenChars(const QString &input);
    /// Returns \c true if \a expr is already a simple identifier (no operators).
    static bool isSimpleIdentifier(const QString &expr);

    // --- Signal Name Resolution ---

    /// Returns the variable name of the signal driving \a port (resolves through wires and nodes).
    QString otherPortName(QNEPort *port);
    /// Recursive implementation of otherPortName() with cycle detection via \a visited.
    QString otherPortNameImpl(QNEPort *port, QSet<QNEPort *> &visited);
    /// Returns \a expr unchanged if simple, otherwise creates a temporary wire and assigns \a expr to it.
    QString ensureSimpleSignal(const QString &expr);

    // --- Variable / Declaration Emission ---

    /// Assigns variable names to all element ports in topological order.
    void assignVariablesRec(const QVector<GraphicElement *> &elements);
    /// Scans \a elements for IC instances and registers their module metadata in m_icModules.
    void collectICTypes(const QVector<GraphicElement *> &elements);
    /// Emits `logic` declarations for auxiliary internal signals.
    void declareAuxVariables();
    /// Recursive helper: emits auxiliary variable declarations for \a elements.
    void declareAuxVariablesRec(const QVector<GraphicElement *> &elements);
    /// Emits top-level `input` port declarations.
    void declareInputs();
    /// Emits top-level `output` port declarations.
    void declareOutputs();

    // --- Sequential Logic Emission ---

    /// Emits an `always_ff` block for a sequential element with preset/clear/normal branches.
    void emitSequentialBlock(const QString &typeName,
                             const QString &clk, const QString &rawPrst, const QString &rawClr,
                             const std::function<void()> &emitPresetBody,
                             const std::function<void()> &emitClearBody,
                             const std::function<void()> &emitNormalBody);

    // --- IC Module Generation ---

    /// Generates sub-module definitions for all collected IC types.
    void generateICModules();
    /// Generates the module body for a single IC type described by \a info.
    void generateSingleICModule(ICModuleInfo &info);
    /// Attempts to emit a behavioral (inlined) module for simple single-element ICs.
    bool emitBehavioralICModule(ICModuleInfo &info);
    /// Inspects \a info's sub-circuit to detect if it wraps a single sequential element type.
    static ElementType detectSequentialICType(const ICModuleInfo &info);

    // --- Combinational Logic ---

    /// Returns the combinational logic expression for \a elm's output.
    QString generateLogicExpression(GraphicElement *elm);
    /// Recursive implementation of generateLogicExpression() with cycle detection.
    QString generateLogicExpressionImpl(GraphicElement *elm, QSet<QNEPort *> &visited);

    // --- Top-Level Structure ---

    /// Emits the combinational `always_comb` / `assign` block and sequential blocks.
    void loop();

    // --- Members ---

    QFile m_file;                              ///< Output file handle.
    QHash<QNEPort *, QString> m_varMap;        ///< Port → generated signal name mapping.
    QHash<QString, QNEInputPort *> m_txInputPorts; ///< Wireless Tx label → input port mapping.
    QHash<QString, ICModuleInfo> m_icModules;  ///< IC source key → module metadata.
    QHash<IC *, QString> m_instanceNames;      ///< IC element → unique instance name.
    QStringList m_availablePins;               ///< Remaining unassigned top-level port names.
    QString m_fileName;                        ///< Sanitised top-level module name.
    QTextStream m_stream;                      ///< Text stream writing to m_file.
    QVector<MappedPinSystemVerilog> m_inputMap; ///< Final input pin mappings (populated by generate()).
    QVector<MappedPinSystemVerilog> m_outputMap; ///< Final output pin mappings (populated by generate()).
    const QVector<GraphicElement *> m_elements; ///< Topologically sorted circuit elements.
    bool m_generatingICModule = false;         ///< True while emitting an IC sub-module body.
    int m_globalCounter = 1;                   ///< Monotonic counter for unique signal names.
};

