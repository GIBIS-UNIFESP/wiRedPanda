// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/CodeGen/ArduinoCodeGen.h"

#include <QRegularExpression>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"

ArduinoCodeGen::ArduinoCodeGen(const QString &fileName, const QVector<GraphicElement *> &elements)
    : m_file(fileName)
    , m_elements(elements)
{
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw PANDACEPTION("Could not open file for writing: %1", fileName);
    }
    m_stream.setDevice(&m_file);
    // Pin pool for Arduino Uno/Nano.  Analog pins A0-A5 are listed first because
    // they are equally usable as digital GPIO and the circuit's inputs/outputs are
    // assigned in the order elements appear in the element list.
    // Pins 0 and 1 are commented out because they are shared with the hardware
    // UART (TX/RX) used for Serial communication and the USB bootloader; using them
    // would prevent sketch upload and break Serial.print() calls.
    m_availablePins = QStringList{
        "A0",
        "A1",
        "A2",
        "A3",
        "A4",
        "A5",
     // "0",  // UART RX — reserved for Serial / bootloader
     // "1",  // UART TX — reserved for Serial / bootloader
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "10",
        "11",
        "12",
        "13"
    };
}

static inline QString highLow(const Status val)
{
    return (val == Status::Active) ? "HIGH" : "LOW";
}

QString ArduinoCodeGen::removeForbiddenChars(const QString &input)
{
    static const QRegularExpression re("\\W");
    return input.toLower().trimmed().replace(' ', '_').replace('-', '_').remove(re);
}

QString ArduinoCodeGen::otherPortName(QNEPort *port)
{
    if (!port) {
        return "LOW";
    }

    if (port->connections().isEmpty()) {
        return highLow(port->defaultValue());
    }

    auto *connection = port->connections().constFirst();
    if (!connection) {
        return highLow(port->defaultValue());
    }

    auto *otherPort = connection->otherPort(port);
    if (!otherPort) {
        return highLow(port->defaultValue());
    }

    return m_varMap.value(otherPort);
}

void ArduinoCodeGen::generate()
{
    try {
        // Validate all elements before writing anything — avoids leaving a partial file on disk
        for (auto *elm : m_elements) {
            if (elm->elementType() == ElementType::IC) {
                throw PANDACEPTION("IC element not supported: %1", elm->objectName());
            }
        }

        m_stream << "// ==================================================================== //" << Qt::endl;
        m_stream << "// ======= This code was generated automatically by wiRedPanda ======== //" << Qt::endl;
        m_stream << "// ==================================================================== //" << Qt::endl;
        m_stream << Qt::endl
                 << Qt::endl;
        // elapsedMillis is an Arduino library that provides an auto-incrementing
        // millisecond counter; used to implement clock element timing without
        // blocking the Arduino loop() with delay() calls
        m_stream << "#include <elapsedMillis.h>" << Qt::endl;
        /* Declaring input and output pins; */
        declareInputs();
        declareOutputs();
        declareAuxVariables();
        /* Setup section */
        setup();
        loop();
    } catch (...) {
        m_file.close();
        m_file.remove();
        throw;
    }
}

void ArduinoCodeGen::declareInputs()
{
    int counter = 1;
    m_stream << "/* ========= Inputs ========== */" << Qt::endl;

    for (auto *elm : m_elements) {
        const auto type = elm->elementType();

        // Only switches and buttons map directly to Arduino digital input pins;
        // other input types (Clock, Vcc, Gnd) either have no physical pin or are
        // handled elsewhere (Clock gets elapsedMillis vars; Vcc/Gnd get HIGH/LOW literals)
        if ((type == ElementType::InputButton) || (type == ElementType::InputSwitch)) {
            // Check if we have available pins before assigning
            if (m_availablePins.isEmpty()) {
                throw PANDACEPTION("Not enough pins available for all input elements");
            }
            // Append the element counter to the object name to ensure uniqueness when
            // multiple switches of the same type exist without user-assigned labels
            QString varName = elm->objectName() + QString::number(counter);
            const QString label = elm->label();

            if (!label.isEmpty()) {
                varName += "_" + label;
            }

            varName = removeForbiddenChars(varName);
            // Emit "const int <pin_name> = <pin_number>;" — Arduino convention for
            // pin aliases; using const int rather than #define preserves type safety
            m_stream << QString("const int %1 = %2;").arg(varName, m_availablePins.constFirst()) << Qt::endl;
            auto *outPort = elm->outputPort(0);
            if (outPort) {
                m_inputMap.append(MappedPin(elm, m_availablePins.constFirst(), varName, outPort, 0));
                // The live value read from the pin is stored in a separate "_val" variable
                // (e.g. "sw1_val = digitalRead(sw1);") to keep pin alias and runtime value distinct
                m_varMap[outPort] = varName + QString("_val");
            }
            m_availablePins.removeFirst();
            ++counter;
        }
    }

    m_stream << Qt::endl;
}

void ArduinoCodeGen::declareOutputs()
{
    int counter = 1;
    m_stream << "/* ========= Outputs ========== */" << Qt::endl;
    for (auto *elm : m_elements) {
        if (elm->elementGroup() == ElementGroup::Output) {
            QString label = elm->label();
            for (int i = 0; i < elm->inputs().size(); ++i) {
                // Check if we have available pins before assigning
                if (m_availablePins.isEmpty()) {
                    throw PANDACEPTION("Not enough pins available for all output elements");
                }

                QString varName = elm->objectName() + QString::number(counter);
                if (!label.isEmpty()) {
                    varName = QString("%1_%2").arg(varName, label);
                }
                QNEPort *port = elm->inputPort(i);
                if (port && !port->name().isEmpty()) {
                    varName = QString("%1_%2").arg(varName, port->name());
                }
                varName = removeForbiddenChars(varName);
                m_stream << QString("const int %1 = %2;").arg(varName, m_availablePins.constFirst()) << Qt::endl;
                m_outputMap.append(MappedPin(elm, m_availablePins.constFirst(), varName, port, i));
                m_availablePins.removeFirst();
            }
        }
        ++counter;
    }
    m_stream << Qt::endl;
}

void ArduinoCodeGen::declareAuxVariablesRec(const QVector<GraphicElement *> &elements, const bool isBox)
{
    // isBox=true when called recursively for elements inside an IC; clock elements
    // inside a box do not get their own elapsedMillis variables because the IC's
    // internal timing is not directly mapped to real Arduino time
    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            //      IC *ic = qobject_cast<IC *>(elm);

            // FIXME: Get code generator to work again
            //      if (ic) {
            //        out << "// " << ic->getLabel() << Qt::endl;
            //        declareAuxVariablesRec(ic->getElements(), true);
            //        out << "// END of " << ic->getLabel() << Qt::endl;
            //        for (int i = 0; i < ic->outputSize(); ++i) {
            //          QNEPort *port = ic->outputMap.at(i);
            //          varMap[ic->outputPort(i)] = otherPortName(port);
            //        }
            //      }
        } else {
            // m_globalCounter ensures aux variable names are unique across the whole
            // generated sketch even when multiple elements share the same object name
            QString varName = QString("aux_%1_%2").arg(removeForbiddenChars(elm->objectName()), m_globalCounter++);
            const auto outputs = elm->outputs();

            if (outputs.size() == 1) {
                QNEPort *port = outputs.constFirst();

                if (elm->elementType() == ElementType::InputVcc) {
                    m_varMap[port] = "HIGH";
                    continue;
                }

                if (elm->elementType() == ElementType::InputGnd) {
                    m_varMap[port] = "LOW";
                    continue;
                }

                if (m_varMap.value(port).isEmpty()) {
                    m_varMap[port] = varName;
                }
            } else {
                int portCounter = 0;

                for (auto *port : outputs) {
                    QString portName = varName;
                    portName.append(QString("_%1").arg(portCounter++));

                    if (!port->name().isEmpty()) {
                        portName.append(QString("_%1").arg(removeForbiddenChars(port->name())));
                    }

                    m_varMap[port] = portName;
                }
            }

            for (auto *port : outputs) {
                QString varName2 = m_varMap.value(port);
                m_stream << "boolean " << varName2 << " = " << highLow(port->defaultValue()) << ";" << Qt::endl;

                switch (elm->elementType()) {
                case ElementType::Clock: {
                    if (!isBox) {
                        auto *clk = qobject_cast<Clock *>(elm);
                        if (!clk) {
                            break;
                        }
                        m_stream << "elapsedMillis " << varName2 << "_elapsed = 0;" << Qt::endl;
                        // Convert frequency (Hz) to half-period (ms): interval = 1000ms / freq.
                        // The clock toggles when elapsed > interval, producing a square wave
                        // with the requested frequency (each full cycle = 2 × interval ms).
                        m_stream << "int " << varName2 << "_interval = " << 1000 / clk->frequency() << ";" << Qt::endl;
                    }
                    break;
                }
                case ElementType::DFlipFlop: {
                    // _inclk tracks the clock level from the previous loop() iteration to
                    // detect rising edges; _last tracks the D input from the previous cycle
                    // so the flip-flop samples D at the moment the clock edge is detected
                    m_stream << "boolean " << varName2 << "_inclk = LOW;" << Qt::endl;
                    m_stream << "boolean " << varName2 << "_last = LOW;" << Qt::endl;
                    break;
                }
                case ElementType::TFlipFlop:
                case ElementType::SRFlipFlop:
                case ElementType::JKFlipFlop: {
                    // _inclk is the only extra state needed for these flip-flops: they
                    // sample their inputs on the rising clock edge, so only the previous
                    // clock level is required (unlike D flip-flop which also needs _last)
                    m_stream << "boolean " << varName2 << "_inclk = LOW;" << Qt::endl;
                    break;
                }

                default:
                    break;
                }
            }
        }
    }
}

void ArduinoCodeGen::declareAuxVariables()
{
    m_stream << "/* ====== Aux. Variables ====== */" << Qt::endl;
    declareAuxVariablesRec(m_elements);
    m_stream << Qt::endl;
}

void ArduinoCodeGen::setup()
{
    m_stream << "void setup() {" << Qt::endl;
    for (const auto &pin : std::as_const(m_inputMap)) {
        m_stream << "    pinMode(" << pin.m_varName << ", INPUT);" << Qt::endl;
    }
    for (const auto &pin : std::as_const(m_outputMap)) {
        m_stream << "    pinMode(" << pin.m_varName << ", OUTPUT);" << Qt::endl;
    }
    m_stream << "}" << Qt::endl
             << Qt::endl;
}

void ArduinoCodeGen::assignVariablesRec(const QVector<GraphicElement *> &elements)
{
    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            throw PANDACEPTION("IC element not supported: %1", elm->objectName());
            // TODO: ArduinoCodeGen::assignVariablesRec for IC Element
            //      IC *ic = qobject_cast<IC *>(elm);
            //      out << "    // " << ic->getLabel() << Qt::endl;
            //      for (int i = 0; i < ic->inputSize(); ++i) {
            //          QNEPort *port = ic->inputPort(i);
            //          QNEPort *otherPort = port->connections().constFirst()->otherPort(port);
            //          QString value = highLow(port->defaultValue());
            //          if (!m_varMap.value(otherPort).isEmpty()) {
            //              value = m_varMap.value(otherPort);
            //          }
            //          out << "    " << m_varMap.value(ic->inputMap.at(i)) << " = " << value << ";" << Qt::endl;
            //      }
            //      QVector<GraphicElement*> icElms = ic->getElements();
            //      if (icElms.isEmpty()) {
            //          continue;
            //      }
            //      icElms = Simulation::sortElements(icElms);
            //      assignVariablesRec(icElms);
            //      out << "    // End of " << ic->getLabel() << Qt::endl;
        }
        if (elm->inputs().isEmpty() || elm->outputs().isEmpty()) {
            continue;
        }

        auto *outputPort0 = elm->outputPort(0);
        if (!outputPort0) {
            continue;
        }
        QString firstOut = m_varMap.value(outputPort0);
        switch (elm->elementType()) {
        case ElementType::DFlipFlop: {
            auto *outputPort1 = elm->outputPort(1);
            if (!outputPort1) break;
            QString secondOut = m_varMap.value(outputPort1);
            QString data = otherPortName(elm->inputPort(0));
            QString clk = otherPortName(elm->inputPort(1));
            QString inclk = firstOut + "_inclk";
            QString last = firstOut + "_last";
            m_stream << QString("    //D FlipFlop") << Qt::endl;
            // Rising-edge detection: clk is HIGH now and was LOW last iteration
            m_stream << QString("    if (%1 && !%2) { ").arg(clk, inclk) << Qt::endl;
            // On the rising edge, sample the D input that was registered in _last
            // (one iteration old), matching the setup-time semantics of real flip-flops
            m_stream << QString("        %1 = %2;").arg(firstOut, last) << Qt::endl;
            m_stream << QString("        %1 = !%2;").arg(secondOut, last) << Qt::endl;
            m_stream << QString("    }") << Qt::endl;
            // Preset (active-low) forces Q=1; Clear (active-low) forces Q=0.
            // These are asynchronous: checked every loop iteration, not just on clock edges.
            QString prst = otherPortName(elm->inputPort(2));
            QString clr = otherPortName(elm->inputPort(3));
            m_stream << QString("    if (!%1 || !%2) { ").arg(prst, clr) << Qt::endl;
            m_stream << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << Qt::endl;
            m_stream << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << Qt::endl;
            m_stream << QString("    }") << Qt::endl;

            /* Updating internal clock. */
            // Update _inclk and _last at the END of each iteration so they hold
            // the "previous" values when the next iteration evaluates the edge condition
            m_stream << "    " << inclk << " = " << clk << ";" << Qt::endl;
            m_stream << "    " << last << " = " << data << ";" << Qt::endl;
            m_stream << QString("    //End of D FlipFlop") << Qt::endl;

            break;
        }
        case ElementType::DLatch: {
            auto *outputPort1 = elm->outputPort(1);
            if (!outputPort1) break;
            QString secondOut = m_varMap.value(outputPort1);
            QString data = otherPortName(elm->inputPort(0));
            QString clk = otherPortName(elm->inputPort(1));
            m_stream << QString("    //D Latch") << Qt::endl;
            m_stream << QString("    if (%1) { ").arg(clk) << Qt::endl;
            m_stream << QString("        %1 = %2;").arg(firstOut, data) << Qt::endl;
            m_stream << QString("        %1 = !%2;").arg(secondOut, data) << Qt::endl;
            m_stream << QString("    }") << Qt::endl;
            m_stream << QString("    //End of D Latch") << Qt::endl;

            break;
        }
        case ElementType::JKFlipFlop: {
            auto *outputPort1 = elm->outputPort(1);
            if (!outputPort1) break;
            QString secondOut = m_varMap.value(outputPort1);
            QString j = otherPortName(elm->inputPort(0));
            QString clk = otherPortName(elm->inputPort(1));
            QString k = otherPortName(elm->inputPort(2));
            QString inclk = firstOut + "_inclk";
            m_stream << QString("    //JK FlipFlop") << Qt::endl;
            m_stream << QString("    if (%1 && !%2) { ").arg(clk, inclk) << Qt::endl;
            m_stream << QString("        if (%1 && %2) { ").arg(j, k) << Qt::endl;
            m_stream << QString("            boolean aux = %1;").arg(firstOut) << Qt::endl;
            m_stream << QString("            %1 = %2;").arg(firstOut, secondOut) << Qt::endl;
            m_stream << QString("            %1 = aux;").arg(secondOut) << Qt::endl;
            m_stream << QString("        } else if (%1) {").arg(j) << Qt::endl;
            m_stream << QString("            %1 = 1;").arg(firstOut) << Qt::endl;
            m_stream << QString("            %1 = 0;").arg(secondOut) << Qt::endl;
            m_stream << QString("        } else if (%1) {").arg(k) << Qt::endl;
            m_stream << QString("            %1 = 0;").arg(firstOut) << Qt::endl;
            m_stream << QString("            %1 = 1;").arg(secondOut) << Qt::endl;
            m_stream << QString("        }") << Qt::endl;
            m_stream << QString("    }") << Qt::endl;
            QString prst = otherPortName(elm->inputPort(3));
            QString clr = otherPortName(elm->inputPort(4));
            m_stream << QString("    if (!%1 || !%2) { ").arg(prst, clr) << Qt::endl;
            m_stream << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << Qt::endl;
            m_stream << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << Qt::endl;
            m_stream << QString("    }") << Qt::endl;

            /* Updating internal clock. */
            m_stream << "    " << inclk << " = " << clk << ";" << Qt::endl;
            m_stream << QString("    //End of JK FlipFlop") << Qt::endl;

            break;
        }
        case ElementType::SRFlipFlop: {
            auto *outputPort1 = elm->outputPort(1);
            if (!outputPort1) break;
            QString secondOut = m_varMap.value(outputPort1);
            QString s = otherPortName(elm->inputPort(0));
            QString clk = otherPortName(elm->inputPort(1));
            QString r = otherPortName(elm->inputPort(2));
            QString inclk = firstOut + "_inclk";
            m_stream << QString("    //SR FlipFlop") << Qt::endl;
            m_stream << QString("    if (%1 && !%2) { ").arg(clk, inclk) << Qt::endl;
            m_stream << QString("        if (%1 && %2) { ").arg(s, r) << Qt::endl;
            m_stream << QString("            %1 = 1;").arg(firstOut) << Qt::endl;
            m_stream << QString("            %1 = 1;").arg(secondOut) << Qt::endl;
            m_stream << QString("        } else if (%1 != %2) {").arg(s, r) << Qt::endl;
            m_stream << QString("            %1 = %2;").arg(firstOut, s) << Qt::endl;
            m_stream << QString("            %1 = %2;").arg(secondOut, r) << Qt::endl;
            m_stream << QString("        }") << Qt::endl;
            m_stream << QString("    }") << Qt::endl;
            QString prst = otherPortName(elm->inputPort(3));
            QString clr = otherPortName(elm->inputPort(4));
            m_stream << QString("    if (!%1 || !%2) { ").arg(prst, clr) << Qt::endl;
            m_stream << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << Qt::endl;
            m_stream << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << Qt::endl;
            m_stream << QString("    }") << Qt::endl;

            /* Updating internal clock. */
            m_stream << "    " << inclk << " = " << clk << ";" << Qt::endl;
            m_stream << QString("    //End of SR FlipFlop") << Qt::endl;

            break;
        }
        case ElementType::TFlipFlop: {
            auto *outputPort1 = elm->outputPort(1);
            if (!outputPort1) break;
            QString secondOut = m_varMap.value(outputPort1);
            QString t = otherPortName(elm->inputPort(0));
            QString clk = otherPortName(elm->inputPort(1));
            QString inclk = firstOut + "_inclk";
            //          QString last = firstOut + "_last";
            m_stream << QString("    //T FlipFlop") << Qt::endl;
            m_stream << QString("    if (%1 && !%2) { ").arg(clk, inclk) << Qt::endl;
            m_stream << QString("        if (%1) { ").arg(t) << Qt::endl;
            m_stream << QString("            %1 = !%1;").arg(firstOut) << Qt::endl;
            m_stream << QString("            %1 = !%2;").arg(secondOut, firstOut) << Qt::endl;
            m_stream << QString("        }") << Qt::endl;
            m_stream << QString("    }") << Qt::endl;
            QString prst = otherPortName(elm->inputPort(2));
            QString clr = otherPortName(elm->inputPort(3));
            m_stream << QString("    if (!%1 || !%2) { ").arg(prst, clr) << Qt::endl;
            m_stream << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << Qt::endl;
            m_stream << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << Qt::endl;
            m_stream << QString("    }") << Qt::endl;

            /* Updating internal clock. */
            m_stream << "    " << inclk << " = " << clk << ";" << Qt::endl;
            //          out << "    " << last << " = " << data << ";" << Qt::endl;
            m_stream << QString("    //End of T FlipFlop") << Qt::endl;

            break;
        }
        // All combinational gates are handled by a single helper that emits a C boolean
        // expression; the distinction between gate types is encoded in logicOperator/negate
        case ElementType::And:
        case ElementType::Or:
        case ElementType::Nand:
        case ElementType::Nor:
        case ElementType::Xor:
        case ElementType::Xnor:
        case ElementType::Not:
        case ElementType::Node: assignLogicOperator(elm); break;
        default:                throw PANDACEPTION("Element type not supported: %1", elm->objectName());
        }
    }
}

void ArduinoCodeGen::assignLogicOperator(GraphicElement *elm)
{
    // negate=true wraps the expression in "!(...)" for NAND/NOR/XNOR/NOT.
    // parentheses=false for NOT because the single operand needs no grouping:
    // "!a" is unambiguous, while "!(a && b)" requires parentheses to negate the whole expression.
    bool negate = false;
    bool parentheses = true;
    QString logicOperator;
    switch (elm->elementType()) {
    case ElementType::And: {
        logicOperator = "&&";
        break;
    }
    case ElementType::Or: {
        logicOperator = "||";
        break;
    }
    case ElementType::Nand: {
        logicOperator = "&&";
        negate = true;
        break;
    }
    case ElementType::Nor: {
        logicOperator = "||";
        negate = true;
        break;
    }
    case ElementType::Xor: {
        logicOperator = "^";
        break;
    }
    case ElementType::Xnor: {
        logicOperator = "^";
        negate = true;
        break;
    }
    case ElementType::Not: {
        negate = true;
        parentheses = false;
        break;
    }
    default:
        break;
    }
    if (elm->outputs().size() == 1) {
        auto *outputPort = elm->outputPort();
        if (!outputPort) return;
        QString varName = m_varMap.value(outputPort);
        QNEPort *inPort = elm->inputPort();
        if (!inPort) return;
        m_stream << "    " << varName << " = ";
        if (negate) {
            m_stream << "!";
        }
        // Only open a parenthesis when both negating AND there are multiple operands to group;
        // for NOT (parentheses=false) the "!" applies directly to the single operand
        if (parentheses && negate) {
            m_stream << "(";
        }
        // Walk all input ports and join their driving variable names with the operator.
        // Unconnected ports fall back to their default value (HIGH/LOW) via otherPortName().
        if (!inPort->connections().isEmpty()) {
            m_stream << otherPortName(inPort);
            for (int i = 1; i < elm->inputs().size(); ++i) {
                inPort = elm->inputs().at(i);
                m_stream << " " << logicOperator << " ";
                m_stream << otherPortName(inPort);
            }
        }
        if (parentheses && negate) {
            m_stream << ")";
        }
        m_stream << ";" << Qt::endl;
    } else {
        // Multi-output combinational gates are not yet supported
        /* ... */
    }
}

void ArduinoCodeGen::loop()
{
    m_stream << "void loop() {" << Qt::endl;
    m_stream << "    // Reading input data //." << Qt::endl;
    for (const auto &pin : std::as_const(m_inputMap)) {
        m_stream << QString("    %1_val = digitalRead(%1);").arg(pin.m_varName) << Qt::endl;
    }
    m_stream << Qt::endl;
    m_stream << "    // Updating clocks. //" << Qt::endl;
    for (auto *elm : m_elements) {
        if (elm->elementType() == ElementType::Clock) {
            const auto elmOutputs = elm->outputs();
            if (elmOutputs.isEmpty()) {
                continue;
            }
            QString varName = m_varMap.value(elmOutputs.constFirst());
            m_stream << QString("    if (%1_elapsed > %1_interval) {").arg(varName) << Qt::endl;
            m_stream << QString("        %1_elapsed = 0;").arg(varName) << Qt::endl;
            m_stream << QString("        %1 = ! %1;").arg(varName) << Qt::endl;
            m_stream << QString("    }") << Qt::endl;
        }
    }
    /* Aux variables. */
    m_stream << Qt::endl;
    m_stream << "    // Assigning aux variables. //" << Qt::endl;
    assignVariablesRec(m_elements);
    m_stream << "\n";
    m_stream << "    // Writing output data. //\n";
    for (const auto &pin : std::as_const(m_outputMap)) {
        QString varName = otherPortName(pin.m_port);
        if (varName.isEmpty()) {
            varName = highLow(pin.m_port->defaultValue());
        }
        m_stream << QString("    digitalWrite(%1, %2);").arg(pin.m_varName, varName) << Qt::endl;
    }
    m_stream << "}" << Qt::endl;
}

