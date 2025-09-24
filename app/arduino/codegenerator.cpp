// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegenerator.h"

#include "buzzer.h"
#include "clock.h"
#include "common.h"
#include "elementfactory.h"
#include "graphicelement.h"
#include "ic.h"
#include "logictruthtable.h"
#include "qneconnection.h"
#include "qneport.h"
#include "truth_table.h"

#include <QRegularExpression>

CodeGenerator::CodeGenerator(const QString &fileName, const QVector<GraphicElement *> &elements)
    : m_file(fileName)
    , m_elements(elements)
{
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    m_stream.setDevice(&m_file);
}

static inline QString boolValue(const Status val)
{
    return (val == Status::Active) ? "true" : "false";
}

QString CodeGenerator::removeForbiddenChars(const QString &input)
{
    return input.toLower().trimmed().replace(' ', '_').replace('-', '_').replace(QRegularExpression("\\W"), "");
}

QString CodeGenerator::otherPortName(QNEPort *port)
{
    if (!port) {
        return "false";
    }

    if (port->connections().isEmpty()) {
        return boolValue(port->defaultValue());
    }

    auto *connection = port->connections().constFirst();
    if (!connection) {
        return boolValue(port->defaultValue());
    }

    auto *otherPort = connection->otherPort(port);
    if (!otherPort) {
        return boolValue(port->defaultValue());
    }

    QString result = m_varMap.value(otherPort);
    if (result.isEmpty()) {
        return boolValue(otherPort->defaultValue());
    }
    return result;
}

void CodeGenerator::generate()
{
    // Pre-flight check: validate pin requirements before generation
    int requiredInputPins = 0;
    int requiredOutputPins = 0;

    // Count input pins needed
    for (auto *elm : m_elements) {
        const auto type = elm->elementType();
        if ((type == ElementType::InputButton) || (type == ElementType::InputSwitch)) {
            requiredInputPins++;
        }
    }

    // Count output pins needed
    for (auto *elm : m_elements) {
        if (elm->elementGroup() == ElementGroup::Output) {
            requiredOutputPins += elm->inputs().size();
        }
    }

    int totalRequiredPins = requiredInputPins + requiredOutputPins;

    // Select the appropriate board based on pin requirements
    m_selectedBoard = selectBoard(totalRequiredPins);

    if (m_selectedBoard.name.isEmpty()) {
        qWarning() << "Error: Circuit requires" << totalRequiredPins << "pins but no available Arduino board can accommodate this";
        qWarning() << "Required:" << requiredInputPins << "input pins +" << requiredOutputPins << "output pins =" << totalRequiredPins << "total";
        qWarning() << "Maximum available: Arduino Mega 2560 with 70 pins";
        qWarning() << "Please reduce circuit complexity";
        return; // Exit before generating any code
    }

    // Update available pins to match selected board
    m_availablePins = m_selectedBoard.availablePins;

    qInfo() << "Selected board:" << m_selectedBoard.name;
    qInfo() << "Pin usage:" << totalRequiredPins << "/" << m_selectedBoard.maxPins() << "pins";
    qInfo() << "Board description:" << m_selectedBoard.description;

    m_stream << "// ==================================================================== //" << Qt::endl;
    m_stream << "// ======= This code was generated automatically by wiRedPanda ======== //" << Qt::endl;
    m_stream << "// ==================================================================== //" << Qt::endl;
    m_stream << "//" << Qt::endl;
    m_stream << QString("// Target Board: %1").arg(m_selectedBoard.name) << Qt::endl;
    m_stream << QString("// Pin Usage: %1/%2 pins").arg(totalRequiredPins).arg(m_selectedBoard.maxPins()) << Qt::endl;
    m_stream << QString("// %1").arg(m_selectedBoard.description) << Qt::endl;
    m_stream << "//" << Qt::endl;
    m_stream << Qt::endl
             << Qt::endl;
    /* Declaring input and output pins; */
    declareInputs();
    declareOutputs();
    declareAuxVariables();
    /* Setup section */
    setup();

    /* Loop section */
    loop();
}

void CodeGenerator::declareInputs()
{
    int counter = 1;
    m_stream << "/* ========= Inputs ========== */" << Qt::endl;

    for (auto *elm : m_elements) {
        const auto type = elm->elementType();

        if ((type == ElementType::InputButton) || (type == ElementType::InputSwitch)) {
            QString varName = elm->objectName() + QString::number(counter);
            const QString label = elm->label();

            if (!label.isEmpty()) {
                varName += "_" + label;
            }

            varName = removeForbiddenChars(varName);
            if (m_availablePins.isEmpty()) {
                qWarning() << "Error: Ran out of available Arduino pins for input:" << varName;
                qWarning() << "Circuit has too many inputs for Arduino hardware";
                return; // Exit gracefully instead of crashing
            }
            const QString pinName = m_availablePins.constFirst();
            m_stream << QString("const int %1 = %2;").arg(varName, pinName) << Qt::endl;
            auto *outPort = elm->outputPort(0);
            if (outPort) {
                m_inputMap.append(MappedPin(elm, pinName, varName, outPort, 0));
                QString valVarName = varName + QString("_val");
                m_varMap[outPort] = valVarName;
            }
            m_availablePins.removeFirst();
            ++counter;
        }
    }

    m_stream << Qt::endl;
}

void CodeGenerator::declareOutputs()
{
    int counter = 1;
    m_stream << "/* ========= Outputs ========== */" << Qt::endl;
    for (auto *elm : m_elements) {
        if (elm->elementGroup() == ElementGroup::Output) {
            QString label = elm->label();
            for (int i = 0; i < elm->inputs().size(); ++i) {
                QString varName = elm->objectName();

                if (elm->elementType() == ElementType::Display7) {
                    varName.replace("7", "seven");
                }

                varName += QString::number(counter);

                if (!label.isEmpty()) {
                    varName = QString("%1_%2").arg(varName, label);
                }
                QNEPort *port = elm->inputPort(i);
                if (port && !port->name().isEmpty()) {
                    varName = QString("%1_%2").arg(varName, port->name());
                }
                varName = removeForbiddenChars(varName);
                if (m_availablePins.isEmpty()) {
                    qWarning() << "Error: Ran out of available Arduino pins for output:" << varName;
                    qWarning() << "Circuit has too many outputs for Arduino hardware";
                    return; // Exit gracefully instead of crashing
                }
                const QString pinName = m_availablePins.constFirst();
                m_stream << QString("const int %1 = %2;").arg(varName, pinName) << Qt::endl;
                m_outputMap.append(MappedPin(elm, pinName, varName, port, i));
                m_availablePins.removeFirst();
            }
        }
        ++counter;
    }
    m_stream << Qt::endl;
}

void CodeGenerator::declareAuxVariablesRec(const QVector<GraphicElement *> &elements, const bool isBox, const QString &icPrefix)
{
    int counter = 0;

    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = qobject_cast<IC *>(elm);
            if (!ic) continue;

            // Comment the IC block for readability
            m_stream << "// IC: " << ic->label() << Qt::endl;

            // Declare variables for external IC ports first
            QString baseVarName = QString("aux_%1_%2").arg(removeForbiddenChars(ic->label()), QString::number(counter++));

            // Declare variables for external IC output ports
            for (int i = 0; i < ic->outputSize(); ++i) {
                QNEPort *externalPort = ic->outputPort(i);
                if (externalPort) {
                    QString portVarName = baseVarName;
                    QString portName = externalPort->name();

                    if (!portName.isEmpty()) {
                        portVarName += "_" + removeForbiddenChars(portName);
                        // Ensure uniqueness by adding index for subsequent ports with same name
                        if (i > 0) {
                            portVarName += "_" + QString::number(i);
                        }
                    } else {
                        portVarName += "_out" + QString::number(i);
                    }
                    m_varMap[externalPort] = portVarName;
                    if (!m_declaredVariables.contains(portVarName)) {
                        m_stream << "bool " << portVarName << " = " << (externalPort->defaultValue() == Status::Active ? "true" : "false") << ";" << Qt::endl;
                        m_declaredVariables.append(portVarName);
                    }
                }
            }

            // Recursively declare variables for internal elements
            if (!ic->m_icElements.isEmpty()) {
                QString nestedIcPrefix = QString("%1_%2_").arg(removeForbiddenChars(ic->label()), QString::number(counter-1));
                declareAuxVariablesRec(ic->m_icElements, true, nestedIcPrefix);

                // Ensure internal IC ports have variables assigned
                for (int i = 0; i < ic->m_icInputs.size(); ++i) {
                    QNEPort *internalPort = ic->m_icInputs.at(i);
                    if (m_varMap.value(internalPort).isEmpty()) {
                        QString portVarName = QString("aux_ic_input_%1_%2").arg(removeForbiddenChars(ic->label()), QString::number(i));
                        m_varMap[internalPort] = portVarName;
                        if (!m_declaredVariables.contains(portVarName)) {
                            m_stream << "bool " << portVarName << " = false;" << Qt::endl;
                            m_declaredVariables.append(portVarName);
                        }
                    }
                }

            }

            m_stream << "// End IC: " << ic->label() << Qt::endl;
            continue;
        }

        const auto outputs = elm->outputs();

        QString baseVarName;
        if (!icPrefix.isEmpty()) {
            baseVarName = QString("aux_%1%2_%3").arg(icPrefix, removeForbiddenChars(elm->objectName()), QString::number(counter++));
        } else {
            baseVarName = QString("aux_%1_%2").arg(removeForbiddenChars(elm->objectName()), QString::number(counter++));
        }

        if (elm->elementType() == ElementType::TruthTable && !outputs.isEmpty()) {
            QNEPort *outputPort = outputs.constFirst();
            QString ttVarName = QString("%1_output").arg(removeForbiddenChars(elm->objectName()));
            m_varMap[outputPort] = ttVarName;
            if (!m_declaredVariables.contains(ttVarName)) {
                m_stream << "bool " << ttVarName << " = false;" << Qt::endl;
                m_declaredVariables.append(ttVarName);
            }

            continue;
        }

        // General logic
        if (outputs.size() == 1) {
            QNEPort *port = outputs.constFirst();

            if (elm->elementType() == ElementType::InputVcc) {
                m_varMap[port] = "true";
                continue;
            }

            if (elm->elementType() == ElementType::InputGnd) {
                m_varMap[port] = "false";
                continue;
            }

            m_varMap[port] = baseVarName;

        } else {
            int portCounter = 0;
            for (auto *port : outputs) {
                QString portName = baseVarName + QString("_%1").arg(portCounter++);
                if (!port->name().isEmpty()) {
                    portName += "_" + removeForbiddenChars(port->name());
                }
                m_varMap[port] = portName;
            }
        }

        for (auto *port : outputs) {
            QString varName2 = m_varMap.value(port);
            if (!m_declaredVariables.contains(varName2)) {
                m_stream << "bool " << varName2 << " = " << (port->defaultValue() == Status::Active ? "true" : "false") << ";" << Qt::endl;
                m_declaredVariables.append(varName2);
            }

            switch (elm->elementType()) {
            case ElementType::Clock:
                if (!isBox) {
                    auto *clk = qobject_cast<Clock *>(elm);
                    if (!clk) {
                        break;
                    }
                    QString lastTimeVar = varName2 + "_lastTime";
                    QString intervalVar = varName2 + "_interval";
                    if (!m_declaredVariables.contains(lastTimeVar)) {
                        m_stream << "unsigned long " << lastTimeVar << " = 0;" << Qt::endl;
                        m_declaredVariables.append(lastTimeVar);
                    }
                    if (!m_declaredVariables.contains(intervalVar)) {
                        m_stream << "const unsigned long " << intervalVar << " = " << 1000UL / clk->frequency() << ";" << Qt::endl;
                        m_declaredVariables.append(intervalVar);
                    }
                }
                break;
            case ElementType::DFlipFlop:
                {
                    QString inclkVar = varName2 + "_inclk";
                    QString lastVar = varName2 + "_last";
                    if (!m_declaredVariables.contains(inclkVar)) {
                        m_stream << "bool " << inclkVar << " = false;" << Qt::endl;
                        m_declaredVariables.append(inclkVar);
                    }
                    if (!m_declaredVariables.contains(lastVar)) {
                        m_stream << "bool " << lastVar << " = false;" << Qt::endl;
                        m_declaredVariables.append(lastVar);
                    }
                }
                break;
            case ElementType::TFlipFlop:
            case ElementType::SRFlipFlop:
            case ElementType::JKFlipFlop:
                {
                    QString inclkVar = varName2 + "_inclk";
                    if (!m_declaredVariables.contains(inclkVar)) {
                        m_stream << "bool " << inclkVar << " = false;" << Qt::endl;
                        m_declaredVariables.append(inclkVar);
                    }
                }
                break;
            default:
                break;
            }
        }
    }
}

void CodeGenerator::declareAuxVariables()
{
    m_stream << "/* ====== Aux. Variables ====== */" << Qt::endl;

    // Declare input value variables
    for (const auto &pin : std::as_const(m_inputMap)) {
        QString valVarName = pin.m_varName + "_val";
        if (!m_declaredVariables.contains(valVarName)) {
            m_stream << "bool " << valVarName << " = false;" << Qt::endl;
            m_declaredVariables.append(valVarName);
        }
    }

    declareAuxVariablesRec(m_elements, false, "");
    m_stream << Qt::endl;
}

void CodeGenerator::setup()
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

void CodeGenerator::assignVariablesRec(const QVector<GraphicElement *> &elements)
{
    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = qobject_cast<IC *>(elm);

            m_stream << "    // IC: " << ic->label() << Qt::endl;

            // Map IC inputs: external signals → internal IC input variables
            for (int i = 0; i < ic->inputSize(); ++i) {
                QNEPort *externalPort = ic->inputPort(i);
                QNEPort *internalPort = ic->m_icInputs.at(i);

                QString externalValue = otherPortName(externalPort);
                QString internalVar = m_varMap.value(internalPort);

                m_stream << "    " << internalVar << " = " << externalValue << ";" << Qt::endl;
            }

            // Process internal IC elements in correct dependency order
            if (!ic->m_icElements.isEmpty()) {
                auto sortedElements = Common::sortGraphicElements(ic->m_icElements);

                // Set IC context for internal element processing
                IC *previousIC = m_currentIC;
                m_currentIC = ic;
                assignVariablesRec(sortedElements);
                m_currentIC = previousIC; // Restore previous context
            }

            // Map IC outputs: internal IC output variables → external signals
            for (int i = 0; i < ic->outputSize(); ++i) {
                QNEPort *externalPort = ic->outputPort(i);
                QNEPort *internalPort = ic->m_icOutputs.at(i);

                QString internalValue = m_varMap.value(internalPort);
                QString externalVar = m_varMap.value(externalPort);

                m_stream << "    " << externalVar << " = " << internalValue << ";" << Qt::endl;
            }

            m_stream << "    // End IC: " << ic->label() << Qt::endl;
            continue;
        }
        // Skip elements that don't have the required ports, except input elements which only need outputs
        bool isInputElement = (elm->elementType() == ElementType::InputButton || elm->elementType() == ElementType::InputSwitch);
        if (elm->outputs().isEmpty() || (!isInputElement && elm->inputs().isEmpty())) {
            continue;
        }

        auto *outputPort0 = elm->outputPort(0);
        if (!outputPort0) {
            continue;
        }
        QString firstOut = m_varMap.value(outputPort0);
        switch (elm->elementType()) {
        case ElementType::InputButton:
        case ElementType::InputSwitch: {
            // Connect input _val variable to circuit logic variable
            // Find the corresponding _val variable in m_varMap for this element's output port
            QString valVarName;
            for (const auto &pin : std::as_const(m_inputMap)) {
                if (pin.m_elm == elm) {
                    valVarName = pin.m_varName + "_val";
                    break;
                }
            }
            if (!valVarName.isEmpty()) {
                m_stream << QString("    %1 = %2;").arg(firstOut, valVarName) << Qt::endl;
            }
            break;
        }
        case ElementType::DFlipFlop: {
            auto *outputPort1 = elm->outputPort(1);
            if (!outputPort1) break;
            QString secondOut = m_varMap.value(outputPort1);
            QString data = otherPortName(elm->inputPort(0));
            QString clk = otherPortName(elm->inputPort(1));
            QString inclk = firstOut + "_inclk";
            QString last = firstOut + "_last";
            m_stream << QString("    //D FlipFlop") << Qt::endl;
            m_stream << QString("    if (%1 && !%2) { ").arg(clk, inclk) << Qt::endl;
            m_stream << QString("        %1 = %2;").arg(firstOut, last) << Qt::endl;
            m_stream << QString("        %1 = !%2;").arg(secondOut, last) << Qt::endl;
            m_stream << QString("    }") << Qt::endl;
            QString prst = otherPortName(elm->inputPort(2));
            QString clr = otherPortName(elm->inputPort(3));
            m_stream << QString("    if (!%1) { ").arg(prst) << Qt::endl;
            m_stream << QString("        %1 = true; // Preset").arg(firstOut) << Qt::endl;
            m_stream << QString("        %1 = false;").arg(secondOut) << Qt::endl;
            m_stream << QString("    } else if (!%1) { ").arg(clr) << Qt::endl;
            m_stream << QString("        %1 = false; // Clear").arg(firstOut) << Qt::endl;
            m_stream << QString("        %1 = true;").arg(secondOut) << Qt::endl;
            m_stream << QString("    }") << Qt::endl;

            /* Updating internal clock. */
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
        case ElementType::SRLatch: {
            auto *outputPort1 = elm->outputPort(1);
            if (!outputPort1) break;
            QString secondOut = m_varMap.value(outputPort1);
            QString s = otherPortName(elm->inputPort(0));
            QString r = otherPortName(elm->inputPort(1));
            m_stream << QString("    //SR Latch") << Qt::endl;
            m_stream << QString("    if (%1 && %2) { ").arg(s, r) << Qt::endl;
            m_stream << QString("        // Invalid state: both Set and Reset active") << Qt::endl;
            m_stream << QString("        %1 = false;").arg(firstOut) << Qt::endl;
            m_stream << QString("        %1 = false;").arg(secondOut) << Qt::endl;
            m_stream << QString("    } else if (%1) { ").arg(s) << Qt::endl;
            m_stream << QString("        // Set: Q = 1, ~Q = 0").arg(firstOut) << Qt::endl;
            m_stream << QString("        %1 = true;").arg(firstOut) << Qt::endl;
            m_stream << QString("        %1 = false;").arg(secondOut) << Qt::endl;
            m_stream << QString("    } else if (%1) { ").arg(r) << Qt::endl;
            m_stream << QString("        // Reset: Q = 0, ~Q = 1") << Qt::endl;
            m_stream << QString("        %1 = false;").arg(firstOut) << Qt::endl;
            m_stream << QString("        %1 = true;").arg(secondOut) << Qt::endl;
            m_stream << QString("    }") << Qt::endl;
            m_stream << QString("    // Hold state when both S and R are 0") << Qt::endl;
            m_stream << QString("    //End of SR Latch") << Qt::endl;

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

            // Optimize JK logic based on constant inputs
            bool jIsTrue = (j == "true");
            bool jIsFalse = (j == "false");
            bool kIsTrue = (k == "true");
            bool kIsFalse = (k == "false");

            if (jIsTrue && kIsTrue) {
                // J=1, K=1: Toggle mode
                m_stream << QString("        // Toggle mode (J=1, K=1)") << Qt::endl;
                m_stream << QString("        bool aux = %1;").arg(firstOut) << Qt::endl;
                m_stream << QString("        %1 = %2;").arg(firstOut, secondOut) << Qt::endl;
                m_stream << QString("        %1 = aux;").arg(secondOut) << Qt::endl;
            } else if (jIsTrue && kIsFalse) {
                // J=1, K=0: Set mode
                m_stream << QString("        // Set mode (J=1, K=0)") << Qt::endl;
                m_stream << QString("        %1 = true;").arg(firstOut) << Qt::endl;
                m_stream << QString("        %1 = false;").arg(secondOut) << Qt::endl;
            } else if (jIsFalse && kIsTrue) {
                // J=0, K=1: Reset mode
                m_stream << QString("        // Reset mode (J=0, K=1)") << Qt::endl;
                m_stream << QString("        %1 = false;").arg(firstOut) << Qt::endl;
                m_stream << QString("        %1 = true;").arg(secondOut) << Qt::endl;
            } else if (jIsFalse && kIsFalse) {
                // J=0, K=0: Hold mode (no change)
                m_stream << QString("        // Hold mode (J=0, K=0) - no change") << Qt::endl;
            } else {
                // General case with variable inputs
                m_stream << QString("        if (%1 && %2) { ").arg(j, k) << Qt::endl;
                m_stream << QString("            // Toggle mode") << Qt::endl;
                m_stream << QString("            bool aux = %1;").arg(firstOut) << Qt::endl;
                m_stream << QString("            %1 = %2;").arg(firstOut, secondOut) << Qt::endl;
                m_stream << QString("            %1 = aux;").arg(secondOut) << Qt::endl;
                m_stream << QString("        } else if (%1) {").arg(j) << Qt::endl;
                m_stream << QString("            // Set mode") << Qt::endl;
                m_stream << QString("            %1 = true;").arg(firstOut) << Qt::endl;
                m_stream << QString("            %1 = false;").arg(secondOut) << Qt::endl;
                m_stream << QString("        } else if (%1) {").arg(k) << Qt::endl;
                m_stream << QString("            // Reset mode") << Qt::endl;
                m_stream << QString("            %1 = false;").arg(firstOut) << Qt::endl;
                m_stream << QString("            %1 = true;").arg(secondOut) << Qt::endl;
                m_stream << QString("        }") << Qt::endl;
            }
            m_stream << QString("    }") << Qt::endl;

            // Preset and Clear logic - only generate if not constantly disabled
            QString prst = otherPortName(elm->inputPort(3));
            QString clr = otherPortName(elm->inputPort(4));
            bool prstIsTrue = (prst == "true");
            bool clrIsTrue = (clr == "true");

            if (!prstIsTrue || !clrIsTrue) {
                if (!prstIsTrue && !clrIsTrue) {
                    // Both preset and clear can be active
                    m_stream << QString("    if (!%1) { ").arg(prst) << Qt::endl;
                    m_stream << QString("        %1 = true; // Preset").arg(firstOut) << Qt::endl;
                    m_stream << QString("        %1 = false;").arg(secondOut) << Qt::endl;
                    m_stream << QString("    } else if (!%1) { ").arg(clr) << Qt::endl;
                    m_stream << QString("        %1 = false; // Clear").arg(firstOut) << Qt::endl;
                    m_stream << QString("        %1 = true;").arg(secondOut) << Qt::endl;
                    m_stream << QString("    }") << Qt::endl;
                } else if (!prstIsTrue) {
                    // Only preset can be active
                    m_stream << QString("    if (!%1) { ").arg(prst) << Qt::endl;
                    m_stream << QString("        %1 = true; // Preset").arg(firstOut) << Qt::endl;
                    m_stream << QString("        %1 = false;").arg(secondOut) << Qt::endl;
                    m_stream << QString("    }") << Qt::endl;
                } else if (!clrIsTrue) {
                    // Only clear can be active
                    m_stream << QString("    if (!%1) { ").arg(clr) << Qt::endl;
                    m_stream << QString("        %1 = false; // Clear").arg(firstOut) << Qt::endl;
                    m_stream << QString("        %1 = true;").arg(secondOut) << Qt::endl;
                    m_stream << QString("    }") << Qt::endl;
                }
            }

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
            m_stream << QString("            %1 = true;").arg(firstOut) << Qt::endl;
            m_stream << QString("            %1 = true;").arg(secondOut) << Qt::endl;
            m_stream << QString("        } else if (%1 != %2) {").arg(s, r) << Qt::endl;
            m_stream << QString("            %1 = %2;").arg(firstOut, s) << Qt::endl;
            m_stream << QString("            %1 = %2;").arg(secondOut, r) << Qt::endl;
            m_stream << QString("        }") << Qt::endl;
            m_stream << QString("    }") << Qt::endl;
            QString prst = otherPortName(elm->inputPort(3));
            QString clr = otherPortName(elm->inputPort(4));
            m_stream << QString("    if (!%1) { ").arg(prst) << Qt::endl;
            m_stream << QString("        %1 = true; // Preset").arg(firstOut) << Qt::endl;
            m_stream << QString("        %1 = false;").arg(secondOut) << Qt::endl;
            m_stream << QString("    } else if (!%1) { ").arg(clr) << Qt::endl;
            m_stream << QString("        %1 = false; // Clear").arg(firstOut) << Qt::endl;
            m_stream << QString("        %1 = true;").arg(secondOut) << Qt::endl;
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
            m_stream << QString("    //T FlipFlop") << Qt::endl;
            m_stream << QString("    if (%1 && !%2) { ").arg(clk, inclk) << Qt::endl;
            m_stream << QString("        if (%1) { ").arg(t) << Qt::endl;
            m_stream << QString("            %1 = !%1;").arg(firstOut) << Qt::endl;
            m_stream << QString("            %1 = !%2;").arg(secondOut, firstOut) << Qt::endl;
            m_stream << QString("        }") << Qt::endl;
            m_stream << QString("    }") << Qt::endl;
            QString prst = otherPortName(elm->inputPort(2));
            QString clr = otherPortName(elm->inputPort(3));
            m_stream << QString("    if (!%1) { ").arg(prst) << Qt::endl;
            m_stream << QString("        %1 = true; // Preset").arg(firstOut) << Qt::endl;
            m_stream << QString("        %1 = false;").arg(secondOut) << Qt::endl;
            m_stream << QString("    } else if (!%1) { ").arg(clr) << Qt::endl;
            m_stream << QString("        %1 = false; // Clear").arg(firstOut) << Qt::endl;
            m_stream << QString("        %1 = true;").arg(secondOut) << Qt::endl;
            m_stream << QString("    }") << Qt::endl;

            /* Updating internal clock. */
            m_stream << "    " << inclk << " = " << clk << ";" << Qt::endl;
            m_stream << QString("    //End of T FlipFlop") << Qt::endl;

            break;
        }
        case ElementType::TruthTable: {
            auto* ttGraphic = dynamic_cast<TruthTable*>(elm);
            LogicTruthTable *ttLogic = dynamic_cast<LogicTruthTable *>(ttGraphic->logic());

            QBitArray propositions = ttLogic->proposition();
            const int nInputs = elm->inputSize();
            const int rows = 1 << nInputs;

            QString tableNameConst = QString("%1").arg(removeForbiddenChars(elm->objectName()));

            QStringList inputSignalNames;
            for (int i = 0; i < nInputs; ++i) {
                QNEPort* ttInputPort = elm->inputPort(i);
                QString signalName = otherPortName(ttInputPort);

                if (signalName == "false") {
                    signalName = "0";
                } else if (signalName == "true") {
                    signalName = "1";
                }
                inputSignalNames << signalName;
            }

            QString outputVarName = m_varMap.value(elm->outputPort(0));

            m_stream << QString("    const uint8_t %1[%2] = {\n").arg(tableNameConst).arg(rows);
            for (int i = 0; i < rows; ++i) {
                m_stream << "        " << (propositions.testBit(i) ? "1" : "0");
                if (i != rows - 1) m_stream << ",";
                m_stream << " // input " << QString::number(i, 2).rightJustified(nInputs, '0') << "\n";
            }
            m_stream << "    };\n\n";

            QStringList bitExpressions;
            for (int i = 0; i < nInputs; ++i) {
                bitExpressions << QString("(%1 << %2)").arg(inputSignalNames[i]).arg(nInputs - 1 - i);
            }
            QString indexCalculation = bitExpressions.join(" | ");
            if (nInputs == 0) {
                indexCalculation = "0";
            }

            m_stream << QString("    uint8_t index = %1;").arg(indexCalculation) << Qt::endl;
            m_stream << QString("    %1 = %2[index];").arg(outputVarName, tableNameConst) << Qt::endl;
            break;
        }

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

void CodeGenerator::assignLogicOperator(GraphicElement *elm)
{
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
    case ElementType::Node: {
        // Nodes are pass-through connections - handle them specially
        if (elm->outputs().size() == 1 && elm->inputs().size() == 1) {
            auto *outputPort = elm->outputPort(0);
            auto *inputPort = elm->inputPort(0);
            if (outputPort && inputPort) {
                QString varName = m_varMap.value(outputPort);
                QString inputValue = otherPortName(inputPort);

                // Special handling for IC internal input nodes
                if (inputValue == "false" && m_currentIC) {
                    // Check if this node's input port is an IC internal input
                    for (int i = 0; i < m_currentIC->m_icInputs.size(); ++i) {
                        QNEPort *icInputPort = m_currentIC->m_icInputs.at(i);
                        if (icInputPort == inputPort) {
                            // This is an IC internal input node - use the corresponding external input variable
                            inputValue = QString("aux_ic_input_%1_%2").arg(removeForbiddenChars(m_currentIC->label()), QString::number(i));
                            break;
                        }
                    }
                }

                m_stream << "    " << varName << " = " << inputValue << ";" << Qt::endl;
            }
        }
        return; // Early return to skip the general logic below
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
        if (parentheses && negate) {
            m_stream << "(";
        }
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
    }
}

void CodeGenerator::loop()
{
    m_stream << "void loop() {" << Qt::endl;
    m_stream << "    // Read input data" << Qt::endl;
    for (const auto &pin : std::as_const(m_inputMap)) {
        m_stream << QString("    %1_val = digitalRead(%1);").arg(pin.m_varName) << Qt::endl;
    }
    m_stream << Qt::endl;
    m_stream << "    // Update clocks" << Qt::endl;
    bool hasClocks = false;
    for (auto *elm : m_elements) {
        if (elm->elementType() == ElementType::Clock) {
            if (!hasClocks) {
                m_stream << QString("    unsigned long now = millis();") << Qt::endl;
                hasClocks = true;
            }
            const auto elmOutputs = elm->outputs();
            QString varName = m_varMap.value(elmOutputs.constFirst());
            m_stream << QString("    if (now - %1_lastTime >= %1_interval) {").arg(varName) << Qt::endl;
            m_stream << QString("        %1_lastTime = now;").arg(varName) << Qt::endl;
            m_stream << QString("        %1 = !%1;").arg(varName) << Qt::endl;
            m_stream << "    }" << Qt::endl;
        }

        if (elm->elementType() == ElementType::Buzzer) {
            auto *buzzer = qobject_cast<Buzzer *>(elm);
            QString inputSignal = otherPortName(buzzer->inputPort(0));

            QString buzzerPinName;
            for (const auto &pin : std::as_const(m_outputMap)) {
                if (pin.m_elm == elm) {
                    buzzerPinName = pin.m_varName;
                    break;
                }
            }

            QString note = buzzer->audio();

            int frequency = 0;
            if (note == "C6") frequency = 1047;
            else if (note == "D6") frequency = 1175;
            else if (note == "E6") frequency = 1319;
            else if (note == "F6") frequency = 1397;
            else if (note == "G6") frequency = 1568;
            else if (note == "A7") frequency = 3520;
            else if (note == "B7") frequency = 3951;
            else if (note == "C7") frequency = 2093;
            else frequency = 1047;

            m_stream << QString("    // Buzzer: %1").arg(elm->objectName()) << Qt::endl;
            m_stream << QString("    if (%1) {").arg(inputSignal) << Qt::endl;
            m_stream << QString("        tone(%1, %2);").arg(buzzerPinName).arg(frequency) << Qt::endl;
            m_stream << QString("    } else {") << Qt::endl;
            m_stream << QString("        noTone(%1);").arg(buzzerPinName) << Qt::endl;
            m_stream << QString("    }") << Qt::endl;
        }
    }
    /* Aux variables. */
    m_stream << Qt::endl;
    m_stream << "    // Update logic variables" << Qt::endl;
    assignVariablesRec(m_elements);
    m_stream << "\n";
    m_stream << "    // Write output data\n";
    for (const auto &pin : std::as_const(m_outputMap)) {
        if (pin.m_elm->elementType() == ElementType::Buzzer) {
            continue;
        }

        QString varName = otherPortName(pin.m_port);
        m_stream << QString("    digitalWrite(%1, %2);").arg(pin.m_varName, varName) << Qt::endl;
    }
    m_stream << "}" << Qt::endl;
}

QVector<ArduinoBoardConfig> CodeGenerator::getAvailableBoards()
{
    QVector<ArduinoBoardConfig> boards;

    // Arduino UNO R3/R4 - 20 pins
    boards.append({
        "Arduino UNO R3/R4",
        {"A0", "A1", "A2", "A3", "A4", "A5", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13"},
        "Standard Arduino board with 20 I/O pins (14 digital + 6 analog)"
    });

    // Arduino Nano - 20 pins (same as UNO)
    boards.append({
        "Arduino Nano",
        {"A0", "A1", "A2", "A3", "A4", "A5", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13"},
        "Compact Arduino board with 20 I/O pins (14 digital + 6 analog)"
    });

    // Arduino Mega 2560 - 70 pins
    QStringList megaPins;
    // Analog pins A0-A15
    for (int i = 0; i <= 15; ++i) {
        megaPins.append(QString("A%1").arg(i));
    }
    // Digital pins 2-53 (avoiding 0,1 for serial)
    for (int i = 2; i <= 53; ++i) {
        megaPins.append(QString::number(i));
    }
    boards.append({
        "Arduino Mega 2560",
        megaPins,
        "High I/O count Arduino board with 70 I/O pins (54 digital + 16 analog)"
    });

    // ESP32 - 36 pins
    boards.append({
        "ESP32",
        {"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10", "A11", "A12", "A13", "A14", "A15", "A16", "A17",
         "2", "4", "5", "12", "13", "14", "15", "16", "17", "18", "19", "21", "22", "23", "25", "26", "27", "32", "33", "34"},
        "WiFi/Bluetooth enabled board with 36 I/O pins (18 digital + 18 analog)"
    });

    return boards;
}

ArduinoBoardConfig CodeGenerator::selectBoard(int requiredPins)
{
    auto boards = getAvailableBoards();

    // Find the smallest board that can accommodate the required pins
    ArduinoBoardConfig selectedBoard;
    for (const auto &board : boards) {
        if (board.maxPins() >= requiredPins) {
            selectedBoard = board;
            break;
        }
    }

    // If no board found, return the largest available (ESP32/Mega 2560)
    if (selectedBoard.name.isEmpty() && !boards.isEmpty()) {
        selectedBoard = boards.last(); // Last board should have highest pin count
    }

    return selectedBoard;
}
