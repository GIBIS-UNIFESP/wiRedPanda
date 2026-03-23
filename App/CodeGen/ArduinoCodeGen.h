// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QTextStream>
#include <QVector>

#include "App/Core/Enums.h"

class GraphicElement;
class IC;
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
    // --- Internal helpers ---

    static QString highLow(Status val);
    static QString stripAccents(const QString &input);
    static QString removeForbiddenChars(const QString &input);
    static bool isArduinoReserved(const QString &name);

    ArduinoBoardConfig selectBoard(int requiredPins);
    QVector<ArduinoBoardConfig> getAvailableBoards();
    QString buildSelectExpression(GraphicElement *elm, int startIndex, int numSelectLines);
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
    void assignVariablesRec(const QVector<GraphicElement *> &elements);
    void declareAuxVariables();
    void declareAuxVariablesRec(const QVector<GraphicElement *> &elements, const bool isBox = false, const QString &icPrefix = {});
    void declareInputs();
    void declareOutputs();
    void emitComputeLogicFunction();
    void loop();
    void setup();

    // --- Members ---

    QFile m_file;
    QHash<QNEPort *, QString> m_varMap;
    QStringList m_availablePins;
    QStringList m_declaredVariables;
    QTextStream m_stream;
    QVector<MappedPin> m_inputMap;
    QVector<MappedPin> m_outputMap;
    const QVector<GraphicElement *> m_elements;
    int m_globalCounter = 1;
    IC *m_currentIC = nullptr;
    ArduinoBoardConfig m_selectedBoard;
};

