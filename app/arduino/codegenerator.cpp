// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegenerator.h"

#include "clock.h"
#include "common.h"
#include "graphicelement.h"
#include "qneconnection.h"
#include "qneport.h"

#include <QRegularExpression>
#include <stdexcept>

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
using Qt::endl;
#endif

CodeGenerator::CodeGenerator(const QString &fileName, const QVector<GraphicElement *> &elements)
    : file(fileName)
    , elements(elements)
{
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    globalCounter = 1;
    out.setDevice(&file);
    availablePins =
    {
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

CodeGenerator::~CodeGenerator()
{
    file.close();
}

static inline QString highLow(int val)
{
    return val == 1 ? "HIGH" : "LOW";
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

    QNEPort *other = port->connections().front()->otherPort(port);

    if (!other) {
        return highLow(port->defaultValue());
    }

    return varMap.value(other);
}

bool CodeGenerator::generate()
{
    out << "// ==================================================================== //" << endl;
    out << "// ======= This code was generated automatically by WiRedPanda ======== //" << endl;
    out << "// ==================================================================== //" << endl;
    out << endl
        << endl;
    out << "#include <elapsedMillis.h>" << endl;
    /* Declaring input and output pins; */
    declareInputs();
    declareOutputs();
    declareAuxVariables();
    /* Setup section */
    setup();

    /* Loop section */
    loop();
    return true;
}

void CodeGenerator::declareInputs()
{
    int counter = 1;
    out << "/* ========= Inputs ========== */" << endl;
    for (auto *elm : elements) {
        const auto type = elm->elementType();
        if (type == ElementType::Button || type == ElementType::Switch) {
            QString varName = elm->objectName() + QString::number(counter);
            QString label = elm->getLabel();
            if (!label.isEmpty()) {
                varName += "_" + label;
            }
            varName = removeForbiddenChars(varName);
            out << QString("const int %1 = %2;").arg(varName, availablePins.front()) << endl;
            inputMap.append(MappedPin(elm, availablePins.front(), varName, elm->output(0), 0));
            availablePins.pop_front();
            varMap[elm->output()] = varName + QString("_val");
            counter++;
        }
    }
    out << endl;
}

void CodeGenerator::declareOutputs()
{
    int counter = 1;
    out << "/* ========= Outputs ========== */" << endl;
    for (auto *elm : elements) {
        if (elm->elementGroup() == ElementGroup::Output) {
            QString label = elm->getLabel();
            for (int i = 0; i < elm->inputs().size(); ++i) {
                QString varName = elm->objectName() + QString::number(counter);
                if (!label.isEmpty()) {
                    varName = QString("%1_%2").arg(varName, label);
                }
                QNEPort *port = elm->input(i);
                if (!port->getName().isEmpty()) {
                    varName = QString("%1_%2").arg(varName, port->getName());
                }
                varName = removeForbiddenChars(varName);
                out << QString("const int %1 = %2;").arg(varName, availablePins.front()) << endl;
                outputMap.append(MappedPin(elm, availablePins.front(), varName, port, i));
                availablePins.pop_front();
            }
        }
        counter++;
    }
    out << endl;
}

void CodeGenerator::declareAuxVariablesRec(const QVector<GraphicElement *> &elms, bool isBox)
{
    for (auto *elm : elms) {
        if (elm->elementType() == ElementType::IC) {
            //      Box *box = qgraphicsitem_cast<Box*>(elm);

            // FIXME: Get code generator to work again
            //      if (box) {
            //        out << "// " << box->getLabel() << endl;
            //        declareAuxVariablesRec(box->getElements(), true);
            //        out << "// END of " << box->getLabel() << endl;
            //        for (int i = 0; i < box->outputSize(); ++i) {
            //          QNEPort *port = box->outputMap.at(i);
            //          varMap[box->output(i)] = otherPortName(port);
            //        }
            //      }
        } else {
            QString varName = QString("aux_%1_%2").arg(removeForbiddenChars(elm->objectName()), globalCounter++);
            const auto outputs = elm->outputs();
            if (outputs.size() == 1) {
                QNEPort *port = outputs.first();
                if (elm->elementType() == ElementType::Vcc) {
                    varMap[port] = "HIGH";
                    continue;
                }
                if (elm->elementType() == ElementType::Gnd) {
                    varMap[port] = "LOW";
                    continue;
                } else if (varMap[port].isEmpty()) {
                    varMap[port] = varName;
                }
            } else {
                int portCounter = 0;
                for (auto *port : outputs) {
                    QString portName = varName;
                    portName.append(QString("_%1").arg(portCounter++));
                    if (!port->getName().isEmpty()) {
                        portName.append(QString("_%1").arg(removeForbiddenChars(port->getName())));
                    }
                    varMap[port] = portName;
                }
            }
            for (auto *port : outputs) {
                QString varName2 = varMap[port];
                out << "boolean " << varName2 << " = " << highLow(port->defaultValue()) << ";" << endl;
                switch (elm->elementType()) {
                case ElementType::Clock: {
                    if (!isBox) {
                        auto *clk = qgraphicsitem_cast<Clock *>(elm);
                        out << "elapsedMillis " << varName2 << "_elapsed = 0;" << endl;
                        out << "int " << varName2 << "_interval = " << 1000 / clk->getFrequency() << ";" << endl;
                    }
                    break;
                }
                case ElementType::DFlipFlop: {
                    out << "boolean " << varName2 << "_inclk = LOW;" << endl;
                    out << "boolean " << varName2 << "_last = LOW;" << endl;
                    break;
                }
                case ElementType::TFlipFlop:
                case ElementType::SRFlipFlop:
                case ElementType::JKFlipFlop: {
                    out << "boolean " << varName2 << "_inclk = LOW;" << endl;
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
    out << "/* ====== Aux. Variables ====== */" << endl;
    declareAuxVariablesRec(elements);
    out << endl;
}

void CodeGenerator::setup()
{
    out << "void setup() {" << endl;
    for (const auto &pin : qAsConst(inputMap)) {
        out << "    pinMode(" << pin.varName << ", INPUT);" << endl;
    }
    for (const auto &pin : qAsConst(outputMap)) {
        out << "    pinMode(" << pin.varName << ", OUTPUT);" << endl;
    }
    out << "}" << endl
        << endl;
}

void CodeGenerator::assignVariablesRec(const QVector<GraphicElement *> &elms)
{
    for (auto *elm : elms) {
        if (elm->elementType() == ElementType::IC) {
            throw std::runtime_error(QString("BOX element not supported : %1").arg(elm->objectName()).toStdString());
            // TODO: CodeGenerator::assignVariablesRec for Box Element
            //      Box *box = qgraphicsitem_cast<Box*>(elm);
            //      out << "    // " << box->getLabel() << endl;
            //      for (int i = 0; i < box->inputSize(); ++i) {
            //          QNEPort *port = box->input(i);
            //          QNEPort *otherPort = port->connections().first()->otherPort(port);
            //          QString value = highLow(port->defaultValue());
            //          if (!varMap[otherPort].isEmpty()) {
            //              value = varMap[otherPort];
            //          }
            //          out << "    " << varMap[box->inputMap.at(i)] << " = " << value << ";" << endl;
            //      }
            //      QVector<GraphicElement*> boxElms = box->getElements();
            //      if (boxElms.isEmpty()) {
            //          continue;
            //      }
            //      boxElms = SimulationController::sortElements(boxElms);
            //      assignVariablesRec(boxElms);
            //      out << "    // End of " << box->getLabel() << endl;
        }
        if (elm->inputs().isEmpty() || elm->outputs().isEmpty()) {
            continue;
        } else {
            QString firstOut = varMap[elm->output(0)];
            switch (elm->elementType()) {
            case ElementType::DFlipFlop: {
                QString secondOut = varMap[elm->output(1)];
                QString data = otherPortName(elm->input(0));
                QString clk = otherPortName(elm->input(1));
                QString inclk = firstOut + "_inclk";
                QString last = firstOut + "_last";
                out << QString("    //D FlipFlop") << endl;
                out << QString("    if (%1 && !%2) { ").arg(clk, inclk) << endl;
                out << QString("        %1 = %2;").arg(firstOut, last) << endl;
                out << QString("        %1 = !%2;").arg(secondOut, last) << endl;
                out << QString("    }") << endl;
                QString prst = otherPortName(elm->input(2));
                QString clr = otherPortName(elm->input(3));
                out << QString("    if (!%1 || !%2) { ").arg(prst, clr) << endl;
                out << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << endl;
                out << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << endl;
                out << QString("    }") << endl;

                /* Updating internal clock. */
                out << "    " << inclk << " = " << clk << ";" << endl;
                out << "    " << last << " = " << data << ";" << endl;
                out << QString("    //End of D FlipFlop") << endl;

                break;
            }
            case ElementType::DLatch: {
                QString secondOut = varMap[elm->output(1)];
                QString data = otherPortName(elm->input(0));
                QString clk = otherPortName(elm->input(1));
                out << QString("    //D Latch") << endl;
                out << QString("    if (%1) { ").arg(clk) << endl;
                out << QString("        %1 = %2;").arg(firstOut, data) << endl;
                out << QString("        %1 = !%2;").arg(secondOut, data) << endl;
                out << QString("    }") << endl;
                out << QString("    //End of D Latch") << endl;

                break;
            }
            case ElementType::JKFlipFlop: {
                QString secondOut = varMap[elm->output(1)];
                QString j = otherPortName(elm->input(0));
                QString clk = otherPortName(elm->input(1));
                QString k = otherPortName(elm->input(2));
                QString inclk = firstOut + "_inclk";
                out << QString("    //JK FlipFlop") << endl;
                out << QString("    if (%1 && !%2) { ").arg(clk, inclk) << endl;
                out << QString("        if (%1 && %2) { ").arg(j, k) << endl;
                out << QString("            boolean aux = %1;").arg(firstOut) << endl;
                out << QString("            %1 = %2;").arg(firstOut, secondOut) << endl;
                out << QString("            %1 = aux;").arg(secondOut) << endl;
                out << QString("        } else if (%1) {").arg(j) << endl;
                out << QString("            %1 = 1;").arg(firstOut) << endl;
                out << QString("            %1 = 0;").arg(secondOut) << endl;
                out << QString("        } else if (%1) {").arg(k) << endl;
                out << QString("            %1 = 0;").arg(firstOut) << endl;
                out << QString("            %1 = 1;").arg(secondOut) << endl;
                out << QString("        }") << endl;
                out << QString("    }") << endl;
                QString prst = otherPortName(elm->input(3));
                QString clr = otherPortName(elm->input(4));
                out << QString("    if (!%1 || !%2) { ").arg(prst, clr) << endl;
                out << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << endl;
                out << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << endl;
                out << QString("    }") << endl;

                /* Updating internal clock. */
                out << "    " << inclk << " = " << clk << ";" << endl;
                out << QString("    //End of JK FlipFlop") << endl;

                break;
            }
            case ElementType::SRFlipFlop: {
                QString secondOut = varMap[elm->output(1)];
                QString s = otherPortName(elm->input(0));
                QString clk = otherPortName(elm->input(1));
                QString r = otherPortName(elm->input(2));
                QString inclk = firstOut + "_inclk";
                out << QString("    //SR FlipFlop") << endl;
                out << QString("    if (%1 && !%2) { ").arg(clk, inclk) << endl;
                out << QString("        if (%1 && %2) { ").arg(s, r) << endl;
                out << QString("            %1 = 1;").arg(firstOut) << endl;
                out << QString("            %1 = 1;").arg(secondOut) << endl;
                out << QString("        } else if (%1 != %2) {").arg(s, r) << endl;
                out << QString("            %1 = %2;").arg(firstOut, s) << endl;
                out << QString("            %1 = %2;").arg(secondOut, r) << endl;
                out << QString("        }") << endl;
                out << QString("    }") << endl;
                QString prst = otherPortName(elm->input(3));
                QString clr = otherPortName(elm->input(4));
                out << QString("    if (!%1 || !%2) { ").arg(prst, clr) << endl;
                out << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << endl;
                out << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << endl;
                out << QString("    }") << endl;

                /* Updating internal clock. */
                out << "    " << inclk << " = " << clk << ";" << endl;
                out << QString("    //End of SR FlipFlop") << endl;

                break;
            }
            case ElementType::TFlipFlop: {
                QString secondOut = varMap[elm->output(1)];
                QString t = otherPortName(elm->input(0));
                QString clk = otherPortName(elm->input(1));
                QString inclk = firstOut + "_inclk";
                //          QString last = firstOut + "_last";
                out << QString("    //T FlipFlop") << endl;
                out << QString("    if (%1 && !%2) { ").arg(clk, inclk) << endl;
                out << QString("        if (%1) { ").arg(t) << endl;
                out << QString("            %1 = !%1;").arg(firstOut) << endl;
                out << QString("            %1 = !%2;").arg(secondOut, firstOut) << endl;
                out << QString("        }") << endl;
                out << QString("    }") << endl;
                QString prst = otherPortName(elm->input(2));
                QString clr = otherPortName(elm->input(3));
                out << QString("    if (!%1 || !%2) { ").arg(prst, clr) << endl;
                out << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << endl;
                out << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << endl;
                out << QString("    }") << endl;

                /* Updating internal clock. */
                out << "    " << inclk << " = " << clk << ";" << endl;
                //          out << "    " << last << " = " << data << ";" << endl;
                out << QString("    //End of T FlipFlop") << endl;

                break;
            }
            case ElementType::And:
            case ElementType::Or:
            case ElementType::Nand:
            case ElementType::Nor:
            case ElementType::Xor:
            case ElementType::XNor:
            case ElementType::Not:
            case ElementType::Node: assignLogicOperator(elm); break;
            default:                throw std::runtime_error(ERRORMSG(QString("Element type not supported : %1").arg(elm->objectName()).toStdString()));
            }
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
    case ElementType::XNor: {
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
        QString varName = varMap[elm->output()];
        QNEPort *inPort = elm->input();
        out << "    " << varName << " = ";
        if (negate) {
            out << "!";
        }
        if (parentheses && negate) {
            out << "(";
        }
        if (!inPort->connections().isEmpty()) {
            out << otherPortName(inPort);
            for (int i = 1; i < elm->inputs().size(); ++i) {
                const auto elmInputs = elm->inputs();
                inPort = elmInputs[i];
                out << " " << logicOperator << " ";
                out << otherPortName(inPort);
            }
        }
        if (parentheses && negate) {
            out << ")";
        }
        out << ";" << endl;
    } else {
        /* ... */
    }
}

void CodeGenerator::loop()
{
    out << "void loop() {" << endl;
    out << "    // Reading input data //." << endl;
    for (const auto &pin : qAsConst(inputMap)) {
        out << QString("    %1_val = digitalRead(%1);").arg(pin.varName) << endl;
    }
    out << endl;
    out << "    // Updating clocks. //" << endl;
    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::Clock) {
            const auto elm_outputs = elm->outputs();
            QString varName = varMap[elm_outputs.first()];
            out << QString("    if (%1_elapsed > %1_interval){").arg(varName) << endl;
            out << QString("        %1_elapsed = 0;").arg(varName) << endl;
            out << QString("        %1 = ! %1;").arg(varName) << endl;
            out << QString("    }") << endl;
        }
    }
    /* Aux variables. */
    out << endl;
    out << "    // Assigning aux variables. //" << endl;
    assignVariablesRec(elements);
    out << "\n";
    out << "    // Writing output data. //\n";
    for (const auto &pin : qAsConst(outputMap)) {
        QString varName = otherPortName(pin.port);
        if (varName.isEmpty()) {
            varName = highLow(pin.port->defaultValue());
        }
        out << QString("    digitalWrite(%1, %2);").arg(pin.varName, varName) << endl;
    }
    out << "}" << endl;
}
