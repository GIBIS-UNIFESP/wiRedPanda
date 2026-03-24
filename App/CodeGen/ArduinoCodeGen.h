// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Arduino sketch code generator: translates a circuit into an uploadable .ino file.
 */

#pragma once

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
 * \class MappedPin
 * \brief Associates a circuit port with its generated Arduino variable name and physical pin.
 */
class MappedPin
{
public:
    /// Constructs a default (empty) MappedPin.
    MappedPin() = default;
    /**
     * \brief Constructs a MappedPin for a given element port.
     * \param elm        Owning graphic element.
     * \param pin        Arduino physical pin label (e.g. "A0", "2").
     * \param varName    Generated variable name used in the sketch.
     * \param port       The QNEPort this mapping refers to.
     * \param portNumber Index of the port within the element (default 0).
     */
    MappedPin(GraphicElement *elm, const QString &pin, const QString &varName, QNEPort *port, const int portNumber = 0)
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

/**
 * \struct ArduinoBoardConfig
 * \brief Describes an Arduino board's available GPIO pins.
 */
struct ArduinoBoardConfig
{
    QString name;          ///< Board name (e.g. "Arduino Uno").
    QStringList availablePins; ///< Ordered list of usable pin labels.
    QString description;   ///< Human-readable board description.
    /// Returns the total number of available pins.
    int maxPins() const { return static_cast<int>(availablePins.size()); }
};

/**
 * \class ArduinoCodeGen
 * \brief Generates an Arduino sketch from a flattened list of circuit elements.
 *
 * \details Translates combinational and simple sequential logic into C++ code
 * suitable for upload to an Arduino-compatible microcontroller.
 */
class ArduinoCodeGen
{
    Q_DECLARE_TR_FUNCTIONS(ArduinoCodeGen)

public:
    // --- Lifecycle ---

    /**
     * \brief Describes a single test vector for testbench generation.
     */
    struct TestVector {
        QVector<bool> inputs;   ///< Input pin values.
        QVector<bool> outputs;  ///< Expected output pin values.
    };

    /**
     * \brief Constructs the code generator targeting \a fileName.
     * \param fileName  Output file path for the generated sketch.
     * \param elements  Ordered list of graphic elements to translate.
     */
    ArduinoCodeGen(const QString &fileName, const QVector<GraphicElement *> &elements);

    // --- Code Generation ---

    /// Generates the Arduino sketch and writes it to the output file.
    void generate();
    void generateTestbench(const QString &tbFileName, const QVector<TestVector> &vectors);

private:
    // --- String Utilities ---

    /// Returns "HIGH" or "LOW" for Active/Inactive status values.
    static QString highLow(Status val);
    /// Replaces accented characters with their ASCII equivalents.
    static QString stripAccents(const QString &input);
    /// Strips characters that are illegal in C++ identifiers.
    static QString removeForbiddenChars(const QString &input);
    /// Returns \c true if \a name collides with an Arduino built-in identifier.
    static bool isArduinoReserved(const QString &name);

    // --- Board Selection ---

    /// Returns a board configuration with at least \a requiredPins available GPIO pins.
    ArduinoBoardConfig selectBoard(int requiredPins);
    /// Returns the list of known Arduino board configurations.
    QVector<ArduinoBoardConfig> getAvailableBoards();

    // --- Signal Name Resolution ---

    /// Builds a nested ternary expression for multiplexer select lines starting at \a startIndex.
    QString buildSelectExpression(GraphicElement *elm, int startIndex, int numSelectLines);
    /// Returns the variable name of the signal driving \a port.
    QString otherPortName(QNEPort *port);
    QString otherPortNameImpl(QNEPort *port, QSet<QNEPort *> &visited);
    void emitFlipFlopBlock(GraphicElement *elm, const QString &typeName, const QString &firstOut,
                           const QString &secondOut, int clockInputIndex, int presetInputIndex,
                           int clearInputIndex, const std::function<void()> &normalLogic);
    void emitDFlipFlop(GraphicElement *elm, const QString &firstOut);
    void emitDLatch(GraphicElement *elm, const QString &firstOut);
    void emitJKFlipFlop(GraphicElement *elm, const QString &firstOut);
    void emitSRFlipFlop(GraphicElement *elm, const QString &firstOut);
    void emitTFlipFlop(GraphicElement *elm, const QString &firstOut);
    void emitSRLatch(GraphicElement *elm, const QString &firstOut);
    void emitMux(GraphicElement *elm);
    void emitDemux(GraphicElement *elm);
    void emitTruthTable(GraphicElement *elm);
    void assignLogicOperator(GraphicElement *elm);

    // --- Variable / Declaration Emission ---

    /// Assigns variable names to all element ports in topological order.
    void assignVariablesRec(const QVector<GraphicElement *> &elements);
    /// Emits `bool` declarations for auxiliary internal variables.
    void declareAuxVariables();
    /// Recursive helper: emits auxiliary variable declarations for \a elements.
    void declareAuxVariablesRec(const QVector<GraphicElement *> &elements, const bool isBox = false, const QString &icPrefix = {});
    void declareInputs();
    /// Emits `pinMode(OUTPUT)` calls and variable declarations for circuit outputs.
    void declareOutputs();
    void emitComputeLogicFunction();
    void loop();
    /// Emits the `setup()` function: pin modes and serial initialization.
    void setup();

    // --- Members ---

    QFile m_file;                              ///< Output file handle.
    QHash<QNEPort *, QString> m_varMap;        ///< Port → generated variable name mapping.
    QHash<QString, QNEInputPort *> m_txInputPorts; ///< Wireless Tx label → input port mapping.
    QStringList m_availablePins;               ///< Remaining unassigned GPIO pin labels.
    QStringList m_declaredVariables;           ///< Names of already-declared variables (duplicate guard).
    QTextStream m_stream;                      ///< Text stream writing to m_file.
    QVector<MappedPin> m_inputMap;             ///< Final input pin mappings.
    QVector<MappedPin> m_outputMap;            ///< Final output pin mappings.
    const QVector<GraphicElement *> m_elements; ///< Topologically sorted circuit elements.
    IC *m_currentIC = nullptr;                 ///< IC currently being flattened (null at top level).
    ArduinoBoardConfig m_selectedBoard;        ///< Board configuration selected during generation.
};

