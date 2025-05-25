// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegenerator.h"

#include "clock.h"
#include "common.h"
#include "graphicelement.h"
#include "qneconnection.h"
#include "qneport.h"
#include "logictruthtable.h"
#include "truth_table.h"

#include <QRegularExpression>

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#define endl Qt::endl
#endif

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
    m_stream << "// ==================================================================== //" << endl;
    m_stream << "// ======= This code was generated automatically by wiRedPanda ======== //" << endl;
    m_stream << "// ==================================================================== //" << endl;
    m_stream << endl
             << endl;
    // m_stream << "#include <elapsedMillis.h>" << endl;
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
    m_stream << "/* ========= Inputs ========== */" << endl;

    for (auto *elm : m_elements) {
        const auto type = elm->elementType();

        if ((type == ElementType::InputButton) || (type == ElementType::InputSwitch)) {
            QString varName = elm->objectName() + QString::number(counter);
            const QString label = elm->label();

            if (!label.isEmpty()) {
                varName += "_" + label;
            }

            varName = removeForbiddenChars(varName);
            m_stream << QString("const int %1 = %2;").arg(varName, m_availablePins.constFirst()) << endl;
            m_inputMap.append(MappedPin(elm, m_availablePins.constFirst(), varName, elm->outputPort(0), 0));
            m_availablePins.removeFirst();
            m_varMap[elm->outputPort()] = varName + QString("_val");
            ++counter;
        }
    }

    m_stream << endl;
}

void CodeGenerator::declareOutputs()
{
    int counter = 1;
    m_stream << "/* ========= Outputs ========== */" << endl;
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
                m_stream << QString("const int %1 = %2;").arg(varName, m_availablePins.constFirst()) << endl;
                m_outputMap.append(MappedPin(elm, m_availablePins.constFirst(), varName, port, i));
                m_availablePins.removeFirst();
            }
        }
        ++counter;
    }
    m_stream << endl;
}

void CodeGenerator::declareAuxVariablesRec(const QVector<GraphicElement *> &elements, const bool isBox)
{
    int counter = 0;

    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            continue;
        }

        const auto outputs = elm->outputs();

        QString baseVarName = QString("aux_%1_%2").arg(removeForbiddenChars(elm->objectName()), QString::number(counter++));

        if (elm->elementType() == ElementType::TruthTable && !outputs.isEmpty()) {
            QNEPort *outputPort = outputs.constFirst();
            QString ttVarName = QString("tt_%1_output").arg(removeForbiddenChars(elm->objectName()));
            m_varMap[outputPort] = ttVarName;
            m_stream << "boolean " << ttVarName << " = LOW;" << endl;

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

            if (m_varMap.value(port).isEmpty()) {
                m_varMap[port] = baseVarName;
            }

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
            m_stream << "boolean " << varName2 << " = " << highLow(port->defaultValue()) << ";" << endl;

            switch (elm->elementType()) {
            case ElementType::Clock:
                if (!isBox) {
                    auto *clk = qgraphicsitem_cast<Clock *>(elm);
                    m_stream << "unsigned long " << varName2 << "_lastTime = 0;" << endl;
                    m_stream << "const unsigned long " << varName2 << "_interval = " << 1000UL / clk->frequency() << ";" << endl;
                }
                break;
            case ElementType::DFlipFlop:
                m_stream << "boolean " << varName2 << "_inclk = LOW;" << endl;
                m_stream << "boolean " << varName2 << "_last = LOW;" << endl;
                break;
            case ElementType::TFlipFlop:
            case ElementType::SRFlipFlop:
            case ElementType::JKFlipFlop:
                m_stream << "boolean " << varName2 << "_inclk = LOW;" << endl;
                break;
            default:
                break;
            }
        }
    }
}

void CodeGenerator::declareAuxVariables()
{
    m_stream << "/* ====== Aux. Variables ====== */" << endl;
    declareAuxVariablesRec(m_elements);
    m_stream << endl;
}

void CodeGenerator::setup()
{
    m_stream << "void setup() {" << endl;
    for (const auto &pin : std::as_const(m_inputMap)) {
        m_stream << "    pinMode(" << pin.m_varName << ", INPUT);" << endl;
    }
    for (const auto &pin : std::as_const(m_outputMap)) {
        m_stream << "    pinMode(" << pin.m_varName << ", OUTPUT);" << endl;
    }
    m_stream << "}" << endl
             << endl;
}

void CodeGenerator::assignVariablesRec(const QVector<GraphicElement *> &elements)
{
    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            throw Pandaception(tr("IC element not supported: ") + elm->objectName());
            // TODO: CodeGenerator::assignVariablesRec for IC Element
            //      IC *ic = qgraphicsitem_cast<IC *>(elm);
            //      out << "    // " << ic->getLabel() << endl;
            //      for (int i = 0; i < ic->inputSize(); ++i) {
            //          QNEPort *port = ic->inputPort(i);
            //          QNEPort *otherPort = port->connections().constFirst()->otherPort(port);
            //          QString value = highLow(port->defaultValue());
            //          if (!m_varMap.value(otherPort).isEmpty()) {
            //              value = m_varMap.value(otherPort);
            //          }
            //          out << "    " << m_varMap.value(ic->inputMap.at(i)) << " = " << value << ";" << endl;
            //      }
            //      QVector<GraphicElement*> icElms = ic->getElements();
            //      if (icElms.isEmpty()) {
            //          continue;
            //      }
            //      icElms = Simulation::sortElements(icElms);
            //      assignVariablesRec(icElms);
            //      out << "    // End of " << ic->getLabel() << endl;
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
            m_stream << QString("    //D FlipFlop") << endl;
            m_stream << QString("    if (%1 && !%2) { ").arg(clk, inclk) << endl;
            m_stream << QString("        %1 = %2;").arg(firstOut, last) << endl;
            m_stream << QString("        %1 = !%2;").arg(secondOut, last) << endl;
            m_stream << QString("    }") << endl;
            QString prst = otherPortName(elm->inputPort(2));
            QString clr = otherPortName(elm->inputPort(3));
            m_stream << QString("    if (!%1 || !%2) { ").arg(prst, clr) << endl;
            m_stream << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << endl;
            m_stream << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << endl;
            m_stream << QString("    }") << endl;

            /* Updating internal clock. */
            m_stream << "    " << inclk << " = " << clk << ";" << endl;
            m_stream << "    " << last << " = " << data << ";" << endl;
            m_stream << QString("    //End of D FlipFlop") << endl;

            break;
        }
        case ElementType::DLatch: {
            QString secondOut = m_varMap.value(elm->outputPort(1));
            QString data = otherPortName(elm->inputPort(0));
            QString clk = otherPortName(elm->inputPort(1));
            m_stream << QString("    //D Latch") << endl;
            m_stream << QString("    if (%1) { ").arg(clk) << endl;
            m_stream << QString("        %1 = %2;").arg(firstOut, data) << endl;
            m_stream << QString("        %1 = !%2;").arg(secondOut, data) << endl;
            m_stream << QString("    }") << endl;
            m_stream << QString("    //End of D Latch") << endl;

            break;
        }
        case ElementType::JKFlipFlop: {
            QString secondOut = m_varMap.value(elm->outputPort(1));
            QString j = otherPortName(elm->inputPort(0));
            QString clk = otherPortName(elm->inputPort(1));
            QString k = otherPortName(elm->inputPort(2));
            QString inclk = firstOut + "_inclk";
            m_stream << QString("    //JK FlipFlop") << endl;
            m_stream << QString("    if (%1 && !%2) { ").arg(clk, inclk) << endl;
            m_stream << QString("        if (%1 && %2) { ").arg(j, k) << endl;
            m_stream << QString("            boolean aux = %1;").arg(firstOut) << endl;
            m_stream << QString("            %1 = %2;").arg(firstOut, secondOut) << endl;
            m_stream << QString("            %1 = aux;").arg(secondOut) << endl;
            m_stream << QString("        } else if (%1) {").arg(j) << endl;
            m_stream << QString("            %1 = 1;").arg(firstOut) << endl;
            m_stream << QString("            %1 = 0;").arg(secondOut) << endl;
            m_stream << QString("        } else if (%1) {").arg(k) << endl;
            m_stream << QString("            %1 = 0;").arg(firstOut) << endl;
            m_stream << QString("            %1 = 1;").arg(secondOut) << endl;
            m_stream << QString("        }") << endl;
            m_stream << QString("    }") << endl;
            QString prst = otherPortName(elm->inputPort(3));
            QString clr = otherPortName(elm->inputPort(4));
            m_stream << QString("    if (!%1 || !%2) { ").arg(prst, clr) << endl;
            m_stream << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << endl;
            m_stream << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << endl;
            m_stream << QString("    }") << endl;

            /* Updating internal clock. */
            m_stream << "    " << inclk << " = " << clk << ";" << endl;
            m_stream << QString("    //End of JK FlipFlop") << endl;

            break;
        }
        case ElementType::SRFlipFlop: {
            QString secondOut = m_varMap.value(elm->outputPort(1));
            QString s = otherPortName(elm->inputPort(0));
            QString clk = otherPortName(elm->inputPort(1));
            QString r = otherPortName(elm->inputPort(2));
            QString inclk = firstOut + "_inclk";
            m_stream << QString("    //SR FlipFlop") << endl;
            m_stream << QString("    if (%1 && !%2) { ").arg(clk, inclk) << endl;
            m_stream << QString("        if (%1 && %2) { ").arg(s, r) << endl;
            m_stream << QString("            %1 = 1;").arg(firstOut) << endl;
            m_stream << QString("            %1 = 1;").arg(secondOut) << endl;
            m_stream << QString("        } else if (%1 != %2) {").arg(s, r) << endl;
            m_stream << QString("            %1 = %2;").arg(firstOut, s) << endl;
            m_stream << QString("            %1 = %2;").arg(secondOut, r) << endl;
            m_stream << QString("        }") << endl;
            m_stream << QString("    }") << endl;
            QString prst = otherPortName(elm->inputPort(3));
            QString clr = otherPortName(elm->inputPort(4));
            m_stream << QString("    if (!%1 || !%2) { ").arg(prst, clr) << endl;
            m_stream << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << endl;
            m_stream << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << endl;
            m_stream << QString("    }") << endl;

            /* Updating internal clock. */
            m_stream << "    " << inclk << " = " << clk << ";" << endl;
            m_stream << QString("    //End of SR FlipFlop") << endl;

            break;
        }
        case ElementType::TFlipFlop: {
            QString secondOut = m_varMap.value(elm->outputPort(1));
            QString t = otherPortName(elm->inputPort(0));
            QString clk = otherPortName(elm->inputPort(1));
            QString inclk = firstOut + "_inclk";
            //          QString last = firstOut + "_last";
            m_stream << QString("    //T FlipFlop") << endl;
            m_stream << QString("    if (%1 && !%2) { ").arg(clk, inclk) << endl;
            m_stream << QString("        if (%1) { ").arg(t) << endl;
            m_stream << QString("            %1 = !%1;").arg(firstOut) << endl;
            m_stream << QString("            %1 = !%2;").arg(secondOut, firstOut) << endl;
            m_stream << QString("        }") << endl;
            m_stream << QString("    }") << endl;
            QString prst = otherPortName(elm->inputPort(2));
            QString clr = otherPortName(elm->inputPort(3));
            m_stream << QString("    if (!%1 || !%2) { ").arg(prst, clr) << endl;
            m_stream << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << endl;
            m_stream << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << endl;
            m_stream << QString("    }") << endl;

            /* Updating internal clock. */
            m_stream << "    " << inclk << " = " << clk << ";" << endl;
            //          out << "    " << last << " = " << data << ";" << endl;
            m_stream << QString("    //End of T FlipFlop") << endl;

            break;
        }
        case ElementType::TruthTable: {
            auto* ttGraphic = dynamic_cast<TruthTable*>(elm);
            LogicTruthTable *ttLogic = dynamic_cast<LogicTruthTable *>(ttGraphic->logic());

            QBitArray propositions = ttLogic->proposition();
            const int nInputs = elm->inputSize();
            const int rows = 1 << nInputs;

            QString tableNameConst = QString("truth_table_%1").arg(removeForbiddenChars(elm->objectName()));

            QStringList inputSignalNames;
            for (int i = 0; i < nInputs; ++i) {
                QNEPort* ttInputPort = elm->inputPort(i);
                QString signalName = otherPortName(ttInputPort);

                if (signalName == "LOW") {
                    signalName = "0";
                } else if (signalName == "HIGH") {
                    signalName = "1";
                } else if (signalName.isEmpty()) {
                    m_stream << "// ATENÇÃO: Entrada " << i << " da Tabela Verdade '" << elm->objectName() << "' parece desconectada ou indefinida. Assumindo LOW." << endl;
                    signalName = "0";
                }
                inputSignalNames << signalName;
            }

            QString outputVarName = m_varMap.value(elm->outputPort(0));
            if (outputVarName.isEmpty()) {
                throw Pandaception(tr("Output variable not mapped for TruthTable: ") + elm->objectName());
            }

            m_stream << QString("const uint8_t %1[%2] = {\n").arg(tableNameConst).arg(rows);
            for (int i = 0; i < rows; ++i) {
                m_stream << "    " << (propositions.testBit(i) ? "1" : "0");
                if (i != rows - 1) m_stream << ",";
                m_stream << " // entrada " << QString::number(i, 2).rightJustified(nInputs, '0') << "\n";
            }
            m_stream << "};\n\n";

            QStringList bitExpressions;
            for (int i = 0; i < nInputs; ++i) {
                bitExpressions << QString("(%1 << %2)").arg(inputSignalNames[i]).arg(nInputs - 1 - i);
            }
            QString indexCalculation = bitExpressions.join(" | ");
            if (nInputs == 0) {
                indexCalculation = "0";
            }


            m_stream << QString("    uint8_t index = %1;").arg(indexCalculation) << endl;
            m_stream << QString("    %1 = %2[index];").arg(outputVarName, tableNameConst) << endl;
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
        default:                throw Pandaception(tr("Element type not supported: ") + elm->objectName());
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
        m_stream << ";" << endl;
    } else {
        /* ... */
    }
}

void CodeGenerator::loop()
{
    m_stream << "void loop() {" << endl;
    m_stream << "    // Reading input data //." << endl;
    for (const auto &pin : std::as_const(m_inputMap)) {
        m_stream << QString("    %1_val = digitalRead(%1);").arg(pin.m_varName) << endl;
    }
    m_stream << endl;
    m_stream << "    // Updating clocks. //" << endl;
    for (auto *elm : m_elements) {
        if (elm->elementType() == ElementType::Clock) {
            const auto elmOutputs = elm->outputs();
            QString varName = m_varMap.value(elmOutputs.constFirst());
            // m_stream << QString("    if (%1_elapsed > %1_interval) {").arg(varName) << endl;
            // m_stream << QString("        %1_elapsed = 0;").arg(varName) << endl;
            // m_stream << QString("        %1 = ! %1;").arg(varName) << endl;
            // m_stream << QString("    }") << endl;

            m_stream << QString("    unsigned long now = millis();") << endl;
            m_stream << QString("    if (now - %1_lastTime >= %1_interval) {").arg(varName) << endl;
            m_stream << QString("        %1_lastTime = now;").arg(varName) << endl;
            m_stream << QString("        %1 = !%1;").arg(varName) << endl;
            m_stream << "    }" << endl;
        }
    }
    /* Aux variables. */
    m_stream << endl;
    m_stream << "    // Assigning aux variables. //" << endl;
    assignVariablesRec(m_elements);
    m_stream << "\n";
    m_stream << "    // Writing output data. //\n";
    for (const auto &pin : std::as_const(m_outputMap)) {
        QString varName = otherPortName(pin.m_port);
        if (varName.isEmpty()) {
            varName = highLow(pin.m_port->defaultValue());
        }
        m_stream << QString("    digitalWrite(%1, %2);").arg(pin.m_varName, varName) << endl;
    }
    m_stream << "}" << endl;
}
