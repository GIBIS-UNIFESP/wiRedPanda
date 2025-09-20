// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegenerator.h"

#include "clock.h"
#include "common.h"
#include "graphicelement.h"
#include "ic.h"
#include "qneconnection.h"
#include "qneport.h"
#include "logictruthtable.h"
#include "truth_table.h"
#include "buzzer.h"

#include <ElementFactory.h>
#include <QRegularExpression>


CodeGenerator::CodeGenerator(const QString &fileName, const QVector<GraphicElement *> &elements)
    : m_file(fileName)
    , m_elements(elements)
{
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    m_stream.setDevice(&m_file);
    m_availablePins = QStringList{
        "A0",
        "A1",
        "A2",
        "A3",
        "A4",
        "A5",
     // "0",
     // "1",
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

QString CodeGenerator::removeForbiddenChars(const QString &input)
{
    return input.toLower().trimmed().replace(' ', '_').replace('-', '_').replace(QRegularExpression("\\W"), "");
}

QString CodeGenerator::otherPortName(QNEPort *port)
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

    QString result = m_varMap.value(otherPort);
    if (result.isEmpty()) {
        return highLow(otherPort->defaultValue());
    }
    return result;
}

void CodeGenerator::generate()
{
    m_stream << "// ==================================================================== //" << Qt::endl;
    m_stream << "// ======= This code was generated automatically by wiRedPanda ======== //" << Qt::endl;
    m_stream << "// ==================================================================== //" << Qt::endl;
    m_stream << Qt::endl
             << Qt::endl;
    // m_stream << "#include <elapsedMillis.h>" << Qt::endl;
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

                if(elm->elementType() == ElementType::Display7) {
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

void CodeGenerator::declareAuxVariablesRec(const QVector<GraphicElement *> &elements, const bool isBox)
{
    int counter = 0;

    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = qobject_cast<IC *>(elm);
            if (!ic) continue;

            // Comment the IC block for readability
            m_stream << "// IC: " << ic->label() << Qt::endl;

            // Declare variables for external IC ports first
            QString baseVarName = QString("aux_%1_%2").arg(removeForbiddenChars(ic->objectName()), QString::number(counter++));

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
                        m_stream << "boolean " << portVarName << " = " << highLow(externalPort->defaultValue()) << ";" << Qt::endl;
                        m_declaredVariables.append(portVarName);
                        qDebug() << "Declared variable for external IC output port" << i << ":" << portVarName;
                    } else {
                        qDebug() << "Skipped duplicate declaration for external IC output port" << i << ":" << portVarName;
                    }
                }
            }

            // Recursively declare variables for internal elements
            if (!ic->m_icElements.isEmpty()) {
                qDebug() << "Before processing internal elements, m_varMap size:" << m_varMap.size();
                declareAuxVariablesRec(ic->m_icElements, true);
                qDebug() << "After processing internal elements, m_varMap size:" << m_varMap.size();

                // Debug: Check if internal ports got variables assigned
                for (int i = 0; i < ic->m_icInputs.size(); ++i) {
                    QNEPort *internalPort = ic->m_icInputs.at(i);
                    QString internalVar = m_varMap.value(internalPort);
                    GraphicElement *parentElement = internalPort->graphicElement();
                    QString elementType = parentElement ? ElementFactory::typeToText(parentElement->elementType()) : "null";
                    qDebug() << "Internal input port" << i << "variable in m_varMap:" << internalVar
                             << "parent element type:" << elementType
                             << "parent element name:" << (parentElement ? parentElement->objectName() : "null")
                             << "port is input:" << internalPort->isInput() << "output:" << internalPort->isOutput();
                }
                for (int i = 0; i < ic->m_icOutputs.size(); ++i) {
                    QNEPort *internalPort = ic->m_icOutputs.at(i);
                    QString internalVar = m_varMap.value(internalPort);
                    GraphicElement *parentElement = internalPort->graphicElement();
                    QString elementType = parentElement ? ElementFactory::typeToText(parentElement->elementType()) : "null";
                    qDebug() << "Internal output port" << i << "variable in m_varMap:" << internalVar
                             << "parent element type:" << elementType
                             << "parent element name:" << (parentElement ? parentElement->objectName() : "null")
                             << "port is input:" << internalPort->isInput() << "output:" << internalPort->isOutput();
                }

                // Ensure internal IC ports have variables assigned
                for (int i = 0; i < ic->m_icInputs.size(); ++i) {
                    QNEPort *internalPort = ic->m_icInputs.at(i);
                    if (m_varMap.value(internalPort).isEmpty()) {
                        QString portVarName = QString("aux_ic_input_%1_%2").arg(removeForbiddenChars(ic->objectName()), QString::number(i));
                        m_varMap[internalPort] = portVarName;
                        if (!m_declaredVariables.contains(portVarName)) {
                            m_stream << "boolean " << portVarName << " = LOW;" << Qt::endl;
                            m_declaredVariables.append(portVarName);
                        }
                    }
                }

            }

            m_stream << "// End IC: " << ic->label() << Qt::endl;
            continue;
        }

        const auto outputs = elm->outputs();

        QString baseVarName = QString("aux_%1_%2").arg(removeForbiddenChars(elm->objectName()), QString::number(counter++));

        if (elm->elementType() == ElementType::TruthTable && !outputs.isEmpty()) {
            QNEPort *outputPort = outputs.constFirst();
            QString ttVarName = QString("%1_output").arg(removeForbiddenChars(elm->objectName()));
            m_varMap[outputPort] = ttVarName;
            if (!m_declaredVariables.contains(ttVarName)) {
                m_stream << "boolean " << ttVarName << " = LOW;" << Qt::endl;
                m_declaredVariables.append(ttVarName);
            }

            continue;
        }

        // Lógica geral
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
                m_stream << "boolean " << varName2 << " = " << highLow(port->defaultValue()) << ";" << Qt::endl;
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
                        m_stream << "boolean " << inclkVar << " = LOW;" << Qt::endl;
                        m_declaredVariables.append(inclkVar);
                    }
                    if (!m_declaredVariables.contains(lastVar)) {
                        m_stream << "boolean " << lastVar << " = LOW;" << Qt::endl;
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
                        m_stream << "boolean " << inclkVar << " = LOW;" << Qt::endl;
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
            m_stream << "boolean " << valVarName << " = LOW;" << Qt::endl;
            m_declaredVariables.append(valVarName);
        }
    }

    declareAuxVariablesRec(m_elements);
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
        // Log element type for debugging
        QString elementTypeName = ElementFactory::typeToText(elm->elementType());
        qDebug() << "CodeGenerator::assignVariablesRec - Processing element type:" << elementTypeName
                 << "(" << static_cast<int>(elm->elementType()) << ") with label:" << elm->objectName();

        // Debug otherPortName for problematic elements
        if (elm->elementType() == ElementType::Node && !elm->inputs().isEmpty()) {
            QNEPort *inputPort = elm->inputPort(0);
            QString inputValue = otherPortName(inputPort);
            qDebug() << "Node" << elm->objectName() << "input value from otherPortName:" << inputValue;
            if (inputValue.isEmpty()) {
                qDebug() << "  Input port connections count:" << (inputPort ? inputPort->connections().size() : -1);
                if (inputPort && !inputPort->connections().isEmpty()) {
                    auto *conn = inputPort->connections().constFirst();
                    QNEPort *otherPort = nullptr;
                    if (conn->startPort() == inputPort) {
                        otherPort = conn->endPort();
                    } else {
                        otherPort = conn->startPort();
                    }
                    QString otherPortVar = m_varMap.value(otherPort);
                    qDebug() << "  Connected to port with variable:" << otherPortVar;
                    qDebug() << "  Other port parent element:" << (otherPort->graphicElement() ? otherPort->graphicElement()->objectName() : "null");
                }
            }
        }
        if (elm->elementType() == ElementType::IC) {
            auto *ic = qobject_cast<IC *>(elm);

            qDebug() << "CodeGenerator::assignVariablesRec - Processing IC:" << ic->label()
                     << "with" << ic->inputSize() << "inputs and" << ic->outputSize() << "outputs";
            qDebug() << "IC internal elements count:" << ic->m_icElements.size();
            qDebug() << "IC inputs mapping size:" << ic->m_icInputs.size();
            qDebug() << "IC outputs mapping size:" << ic->m_icOutputs.size();

            m_stream << "    // IC: " << ic->label() << Qt::endl;

            // Map IC inputs: external signals → internal IC input variables
            qDebug() << "Processing IC inputs...";
            for (int i = 0; i < ic->inputSize(); ++i) {
                QNEPort *externalPort = ic->inputPort(i);
                QNEPort *internalPort = ic->m_icInputs.at(i);

                QString externalPortTooltip = externalPort ? externalPort->toolTip() : "null";
                QString internalPortTooltip = internalPort ? internalPort->toolTip() : "null";
                qDebug() << "IC input" << i << "- External port tooltip:" << externalPortTooltip
                         << "Internal port tooltip:" << internalPortTooltip;

                QString externalValue = otherPortName(externalPort);

                QString internalVar = m_varMap.value(internalPort);
                qDebug() << "IC input" << i << "mapping: external=" << externalValue << "-> internal=" << internalVar;

                m_stream << "    " << internalVar << " = " << externalValue << ";" << Qt::endl;
            }

            // Process internal IC elements in correct dependency order
            if (!ic->m_icElements.isEmpty()) {
                qDebug() << "Processing" << ic->m_icElements.size() << "internal IC elements...";
                auto sortedElements = Common::sortGraphicElements(ic->m_icElements);
                qDebug() << "Sorted" << sortedElements.size() << "elements for processing";
                assignVariablesRec(sortedElements);
                qDebug() << "Finished processing internal IC elements";
            }

            // Map IC outputs: internal IC output variables → external signals
            for (int i = 0; i < ic->outputSize(); ++i) {
                QNEPort *externalPort = ic->outputPort(i);
                QNEPort *internalPort = ic->m_icOutputs.at(i);

                QString internalValue = m_varMap.value(internalPort);
                QString externalVar = m_varMap.value(externalPort);

                m_stream << "    " << externalVar << " = " << internalValue << ";" << Qt::endl;
            }

            qDebug() << "Finished processing IC:" << ic->label();
            m_stream << "    // End IC: " << ic->label() << Qt::endl;
            continue;
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
            m_stream << QString("    if (%1 && !%2) { ").arg(clk, inclk) << Qt::endl;
            m_stream << QString("        %1 = %2;").arg(firstOut, last) << Qt::endl;
            m_stream << QString("        %1 = !%2;").arg(secondOut, last) << Qt::endl;
            m_stream << QString("    }") << Qt::endl;
            QString prst = otherPortName(elm->inputPort(2));
            QString clr = otherPortName(elm->inputPort(3));
            m_stream << QString("    if (!%1 || !%2) { ").arg(prst, clr) << Qt::endl;
            m_stream << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << Qt::endl;
            m_stream << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << Qt::endl;
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
            m_stream << QString("            %1 = HIGH;").arg(firstOut) << Qt::endl;
            m_stream << QString("            %1 = LOW;").arg(secondOut) << Qt::endl;
            m_stream << QString("        } else if (%1) {").arg(k) << Qt::endl;
            m_stream << QString("            %1 = LOW;").arg(firstOut) << Qt::endl;
            m_stream << QString("            %1 = HIGH;").arg(secondOut) << Qt::endl;
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
            m_stream << QString("            %1 = HIGH;").arg(firstOut) << Qt::endl;
            m_stream << QString("            %1 = HIGH;").arg(secondOut) << Qt::endl;
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

                if (signalName == "LOW") {
                    signalName = "0";
                } else if (signalName == "HIGH") {
                    signalName = "1";
                }
                inputSignalNames << signalName;
            }

            QString outputVarName = m_varMap.value(elm->outputPort(0));

            m_stream << QString("    const uint8_t %1[%2] = {\n").arg(tableNameConst).arg(rows);
            for (int i = 0; i < rows; ++i) {
                m_stream << "        " << (propositions.testBit(i) ? "1" : "0");
                if (i != rows - 1) m_stream << ",";
                m_stream << " // entrada " << QString::number(i, 2).rightJustified(nInputs, '0') << "\n";
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

        // case ElementType::Display7: {}

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
    } else {
        /* ... */
    }
}

void CodeGenerator::loop()
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
            QString varName = m_varMap.value(elmOutputs.constFirst());
            // m_stream << QString("    if (%1_elapsed > %1_interval) {").arg(varName) << Qt::endl;
            // m_stream << QString("        %1_elapsed = 0;").arg(varName) << Qt::endl;
            // m_stream << QString("        %1 = ! %1;").arg(varName) << Qt::endl;
            // m_stream << QString("    }") << Qt::endl;

            m_stream << QString("    unsigned long now = millis();") << Qt::endl;
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

            // qDebug() << "Nota de áudio lida do Buzzer:" << note;

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

            m_stream << QString("    // Buzzer: %1 //").arg(elm->objectName()) << Qt::endl;
            m_stream << QString("    if (%1 == HIGH) {").arg(inputSignal) << Qt::endl;
            m_stream << QString("        tone(%1, %2);").arg(buzzerPinName).arg(frequency) << Qt::endl;
            m_stream << QString("    } else {") << Qt::endl;
            m_stream << QString("        noTone(%1);").arg(buzzerPinName) << Qt::endl;
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
        if (pin.m_elm->elementType() == ElementType::Buzzer) {
            continue;
        }

        QString varName = otherPortName(pin.m_port);
        m_stream << QString("    digitalWrite(%1, %2);").arg(pin.m_varName, varName) << Qt::endl;
    }
    m_stream << "}" << Qt::endl;
}
