// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegenerator.h"

#include "clock.h"
#include "common.h"
#include "graphicelement.h"
#include "qneconnection.h"
#include "qneport.h"

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

    auto *otherPort = port->connections().constFirst()->otherPort(port);

    if (!otherPort) {
        return highLow(port->defaultValue());
    }

    return m_varMap.value(otherPort);
}

void CodeGenerator::generate()
{
    m_stream << "// ==================================================================== //" << Qt::endl;
    m_stream << "// ======= This code was generated automatically by wiRedPanda ======== //" << Qt::endl;
    m_stream << "// ==================================================================== //" << Qt::endl;
    m_stream << Qt::endl
             << Qt::endl;
    m_stream << "#include <elapsedMillis.h>" << Qt::endl;
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
            m_stream << QString("const int %1 = %2;").arg(varName, m_availablePins.constFirst()) << Qt::endl;
            m_inputMap.append(MappedPin(elm, m_availablePins.constFirst(), varName, elm->outputPort(0), 0));
            m_availablePins.removeFirst();
            m_varMap[elm->outputPort()] = varName + QString("_val");
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
                QString varName = elm->objectName() + QString::number(counter);
                if (!label.isEmpty()) {
                    varName = QString("%1_%2").arg(varName, label);
                }
                QNEPort *port = elm->inputPort(i);
                if (!port->name().isEmpty()) {
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

void CodeGenerator::declareAuxVariablesRec(const QVector<GraphicElement *> &elements, const bool isBox)
{
    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            //      IC *ic = qgraphicsitem_cast<IC *>(elm);

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
                        auto *clk = qgraphicsitem_cast<Clock *>(elm);
                        m_stream << "elapsedMillis " << varName2 << "_elapsed = 0;" << Qt::endl;
                        m_stream << "int " << varName2 << "_interval = " << 1000 / clk->frequency() << ";" << Qt::endl;
                    }
                    break;
                }
                case ElementType::DFlipFlop: {
                    m_stream << "boolean " << varName2 << "_inclk = LOW;" << Qt::endl;
                    m_stream << "boolean " << varName2 << "_last = LOW;" << Qt::endl;
                    break;
                }
                case ElementType::TFlipFlop:
                case ElementType::SRFlipFlop:
                case ElementType::JKFlipFlop: {
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

void CodeGenerator::declareAuxVariables()
{
    m_stream << "/* ====== Aux. Variables ====== */" << Qt::endl;
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
        if (elm->elementType() == ElementType::IC) {
            throw PANDACEPTION("IC element not supported: %1", elm->objectName());
            // TODO: CodeGenerator::assignVariablesRec for IC Element
            //      IC *ic = qgraphicsitem_cast<IC *>(elm);
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

        QString firstOut = m_varMap.value(elm->outputPort(0));
        switch (elm->elementType()) {
        case ElementType::DFlipFlop: {
            QString secondOut = m_varMap.value(elm->outputPort(1));
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
            QString secondOut = m_varMap.value(elm->outputPort(1));
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
            QString secondOut = m_varMap.value(elm->outputPort(1));
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
            QString secondOut = m_varMap.value(elm->outputPort(1));
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
            QString secondOut = m_varMap.value(elm->outputPort(1));
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
    default:
        break;
    }
    if (elm->outputs().size() == 1) {
        QString varName = m_varMap.value(elm->outputPort());
        QNEPort *inPort = elm->inputPort();
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
