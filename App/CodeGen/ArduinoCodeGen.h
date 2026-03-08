// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QTextStream>
#include <QVector>

class GraphicElement;
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
     * \brief Constructs the code generator targeting \a fileName.
     * \param fileName  Output file path for the generated sketch.
     * \param elements  Ordered list of graphic elements to translate.
     */
    ArduinoCodeGen(const QString &fileName, const QVector<GraphicElement *> &elements);

    // --- Code Generation ---

    /// Generates the Arduino sketch and writes it to the output file.
    void generate();

private:
    // --- Internal helpers ---

    static QString removeForbiddenChars(const QString &input);

    QString otherPortName(QNEPort *port);
    void assignLogicOperator(GraphicElement *elm);
    void assignVariablesRec(const QVector<GraphicElement *> &elements);
    void declareAuxVariables();
    void declareAuxVariablesRec(const QVector<GraphicElement *> &elements, const bool isBox = false);
    void declareInputs();
    void declareOutputs();
    void loop();
    void setup();

    // --- Members ---

    QFile m_file;
    QHash<QNEPort *, QString> m_varMap;
    QStringList m_availablePins;
    QTextStream m_stream;
    QVector<MappedPin> m_inputMap;
    QVector<MappedPin> m_outputMap;
    const QVector<GraphicElement *> m_elements;
    int m_globalCounter = 1;
};
