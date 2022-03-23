// Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegenerator.h"
#include "clock.h"
#include "common.h"
#include "graphicelement.h"
#include "qneconnection.h"
#include "qneport.h"

#include <QRegularExpression>
#include <stdexcept>

CodeGenerator::CodeGenerator(const QString &fileName, const QVector<GraphicElement *> &elements)
    : file(fileName)
    , elements(elements)
{
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    globalCounter = 1;
    out.setDevice(&file);
    availablePins = {"A0",
                     "A1",
                     "A2",
                     "A3",
                     "A4",
                     "A5",
                     /*"0", "1",*/ "2",
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
                     "13"};
}

CodeGenerator::~CodeGenerator()
{
    file.close();
}

static inline QString highLow(int val)
{
    return val == 1 ? "HIGH" : "LOW";
}

QString clearString(const QString &input)
{
    return input.toLower().trimmed().replace(" ", "_").replace("-", "_").replace(QRegularExpression("\\W"), "");
}

QString CodeGenerator::otherPortName(QNEPort *port)
{
    if (port) {
        if (port->connections().isEmpty()) {
            return highLow(port->defaultValue());
        }
        QNEPort *other = port->connections().front()->otherPort(port);
        if (other) {
            return varMap[other];
        } else {
            return highLow(port->defaultValue());
        }

    } else {
        return "LOW";
    }
}

bool CodeGenerator::generate()
{
    out << "// ==================================================================== //" << Qt::endl;
    out << "// ======= This code was generated automatically by wiRED PANDA ======= //" << Qt::endl;
    out << "// ==================================================================== //" << Qt::endl;
    out << Qt::endl << Qt::endl;
    out << "#include <elapsedMillis.h>" << Qt::endl;
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
    out << "/* ========= Inputs ========== */" << Qt::endl;
    for (GraphicElement *elm : elements) {
        if ((elm->elementType() == ElementType::BUTTON) || (elm->elementType() == ElementType::SWITCH)) {
            QString varName = elm->objectName() + QString::number(counter);
            QString label = elm->getLabel();
            if (!label.isEmpty()) {
                varName = QString("%1_%2").arg(varName, label);
            }
            varName = clearString(varName);
            out << QString("const int %1 = %2;").arg(varName, availablePins.front()) << Qt::endl;
            inputMap.append(MappedPin(elm, availablePins.front(), varName, elm->output(0), 0));
            availablePins.pop_front();
            varMap[elm->output()] = varName + QString("_val");
            counter++;
        }
    }
    out << Qt::endl;
}

void CodeGenerator::declareOutputs()
{
    int counter = 1;
    out << "/* ========= Outputs ========== */" << Qt::endl;
    for (GraphicElement *elm : elements) {
        if (elm->elementGroup() == ElementGroup::OUTPUT) {
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
                varName = clearString(varName);
                out << QString("const int %1 = %2;").arg(varName, availablePins.front()) << Qt::endl;
                outputMap.append(MappedPin(elm, availablePins.front(), varName, port, i));
                availablePins.pop_front();
            }
        }
        counter++;
    }
    out << Qt::endl;
}

void CodeGenerator::declareAuxVariablesRec(const QVector<GraphicElement *> &elms, bool isBox)
{
    for (GraphicElement *elm : elms) {
        if (elm->elementType() == ElementType::IC) {
            //      Box *box = qgraphicsitem_cast< Box* >( elm );

            // FIXME: Get code generator to work again
            //      if( box ) {
            //        out << "// " << box->getLabel( ) << Qt::endl;
            //        declareAuxVariablesRec( box->getElements( ), true );
            //        out << "// END of " << box->getLabel( ) << Qt::endl;
            //        for( int i = 0; i < box->outputSize( ); ++i ) {
            //          QNEPort *port = box->outputMap.at( i );
            //          varMap[ box->output( i ) ] = otherPortName( port );
            //        }
            //      }
        } else {
            QString varName = QString("aux_%1_%2").arg(clearString(elm->objectName())).arg(globalCounter++);
            auto const outputs = elm->outputs();
            if (outputs.size() == 1) {
                QNEPort *port = outputs.first();
                if (elm->elementType() == ElementType::VCC) {
                    varMap[port] = "HIGH";
                    continue;
                }
                if (elm->elementType() == ElementType::GND) {
                    varMap[port] = "LOW";
                    continue;
                } else if (varMap[port].isEmpty()) {
                    varMap[port] = varName;
                }
            } else {
                int portCounter = 0;
                for (QNEPort *port : outputs) {
                    QString portName = varName;
                    portName.append(QString("_%1").arg(portCounter++));
                    if (!port->getName().isEmpty()) {
                        portName.append(QString("_%1").arg(clearString(port->getName())));
                    }
                    varMap[port] = portName;
                }
            }
            for (QNEPort *port : outputs) {
                QString varName2 = varMap[port];
                out << "boolean " << varName2 << " = " << highLow(port->defaultValue()) << ";" << Qt::endl;
                switch (elm->elementType()) {
                case ElementType::CLOCK: {
                    if (!isBox) {
                        auto *clk = qgraphicsitem_cast<Clock *>(elm);
                        out << "elapsedMillis " << varName2 << "_elapsed = 0;" << Qt::endl;
                        out << "int " << varName2 << "_interval = " << 1000 / clk->getFrequency() << ";" << Qt::endl;
                    }
                    break;
                }
                case ElementType::DFLIPFLOP: {
                    out << "boolean " << varName2 << "_inclk = LOW;" << Qt::endl;
                    out << "boolean " << varName2 << "_last = LOW;" << Qt::endl;
                    break;
                }
                case ElementType::TFLIPFLOP:
                case ElementType::SRFLIPFLOP:
                case ElementType::JKFLIPFLOP: {
                    out << "boolean " << varName2 << "_inclk = LOW;" << Qt::endl;
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
    out << "/* ====== Aux. Variables ====== */" << Qt::endl;
    declareAuxVariablesRec(elements);
    out << Qt::endl;
}

void CodeGenerator::setup()
{
    out << "void setup( ) {" << Qt::endl;
    for (const MappedPin &pin : qAsConst(inputMap)) {
        out << "    pinMode( " << pin.varName << ", INPUT );" << Qt::endl;
    }
    for (const MappedPin &pin : qAsConst(outputMap)) {
        out << "    pinMode( " << pin.varName << ", OUTPUT );" << Qt::endl;
    }
    out << "}" << Qt::endl << Qt::endl;
}

void CodeGenerator::assignVariablesRec(const QVector<GraphicElement *> &elms)
{
    for (GraphicElement *elm : elms) {
        if (elm->elementType() == ElementType::IC) {
            throw std::runtime_error(QString("BOX element not supported : %1").arg(elm->objectName()).toStdString());
            // TODO: CodeGenerator::assignVariablesRec for Box Element
            //      Box *box = qgraphicsitem_cast< Box* >( elm );
            //      out << "    // " << box->getLabel( ) << Qt::endl;
            //      for( int i = 0; i < box->inputSize( ); ++i ) {
            //        QNEPort *port = box->input( i );
            //        QNEPort *otherPort = port->connections( ).first( )->otherPort( port );
            //        QString value = highLow( port->defaultValue( ) );
            //        if( !varMap[ otherPort ].isEmpty( ) ) {
            //          value = varMap[ otherPort ];
            //        }
            //        out << "    " << varMap[ box->inputMap.at( i ) ] << " = " << value << ";" << Qt::endl;
            //      }
            //      QVector< GraphicElement* > boxElms = box->getElements( );
            //      if( boxElms.isEmpty( ) ) {
            //        continue;
            //      }
            //      boxElms = SimulationController::sortElements( boxElms );
            //      assignVariablesRec( boxElms );
            //      out << "    // End of " << box->getLabel( ) << Qt::endl;
        }
        if (elm->inputs().isEmpty() || elm->outputs().isEmpty()) {
            continue;
        } else {
            QString firstOut = varMap[elm->output(0)];
            switch (elm->elementType()) {
            case ElementType::DFLIPFLOP: {
                QString secondOut = varMap[elm->output(1)];
                QString data = otherPortName(elm->input(0));
                QString clk = otherPortName(elm->input(1));
                QString inclk = firstOut + "_inclk";
                QString last = firstOut + "_last";
                out << QString("    //D FlipFlop") << Qt::endl;
                out << QString("    if( %1 && !%2) { ").arg(clk, inclk) << Qt::endl;
                out << QString("        %1 = %2;").arg(firstOut, last) << Qt::endl;
                out << QString("        %1 = !%2;").arg(secondOut, last) << Qt::endl;
                out << QString("    }") << Qt::endl;
                QString prst = otherPortName(elm->input(2));
                QString clr = otherPortName(elm->input(3));
                out << QString("    if( !%1 || !%2) { ").arg(prst, clr) << Qt::endl;
                out << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << Qt::endl;
                out << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << Qt::endl;
                out << QString("    }") << Qt::endl;

                /* Updating internal clock. */
                out << "    " << inclk << " = " << clk << ";" << Qt::endl;
                out << "    " << last << " = " << data << ";" << Qt::endl;
                out << QString("    //End of D FlipFlop") << Qt::endl;

                break;
            }
            case ElementType::DLATCH: {
                QString secondOut = varMap[elm->output(1)];
                QString data = otherPortName(elm->input(0));
                QString clk = otherPortName(elm->input(1));
                out << QString("    //D Latch") << Qt::endl;
                out << QString("    if( %1 ) { ").arg(clk) << Qt::endl;
                out << QString("        %1 = %2;").arg(firstOut, data) << Qt::endl;
                out << QString("        %1 = !%2;").arg(secondOut, data) << Qt::endl;
                out << QString("    }") << Qt::endl;
                out << QString("    //End of D Latch") << Qt::endl;
                break;
            }
            case ElementType::JKFLIPFLOP: {
                QString secondOut = varMap[elm->output(1)];
                QString j = otherPortName(elm->input(0));
                QString clk = otherPortName(elm->input(1));
                QString k = otherPortName(elm->input(2));
                QString inclk = firstOut + "_inclk";
                out << QString("    //JK FlipFlop") << Qt::endl;
                out << QString("    if( %1 && !%2 ) { ").arg(clk, inclk) << Qt::endl;
                out << QString("        if( %1 && %2) { ").arg(j, k) << Qt::endl;
                out << QString("            boolean aux = %1;").arg(firstOut) << Qt::endl;
                out << QString("            %1 = %2;").arg(firstOut, secondOut) << Qt::endl;
                out << QString("            %1 = aux;").arg(secondOut) << Qt::endl;
                out << QString("        } else if ( %1 ) {").arg(j) << Qt::endl;
                out << QString("            %1 = 1;").arg(firstOut) << Qt::endl;
                out << QString("            %1 = 0;").arg(secondOut) << Qt::endl;
                out << QString("        } else if ( %1 ) {").arg(k) << Qt::endl;
                out << QString("            %1 = 0;").arg(firstOut) << Qt::endl;
                out << QString("            %1 = 1;").arg(secondOut) << Qt::endl;
                out << QString("        }") << Qt::endl;
                out << QString("    }") << Qt::endl;
                QString prst = otherPortName(elm->input(3));
                QString clr = otherPortName(elm->input(4));
                out << QString("    if( !%1 || !%2 ) { ").arg(prst, clr) << Qt::endl;
                out << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << Qt::endl;
                out << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << Qt::endl;
                out << QString("    }") << Qt::endl;

                /* Updating internal clock. */
                out << "    " << inclk << " = " << clk << ";" << Qt::endl;
                out << QString("    //End of JK FlipFlop") << Qt::endl;
                break;
            }
            case ElementType::SRFLIPFLOP: {
                QString secondOut = varMap[elm->output(1)];
                QString s = otherPortName(elm->input(0));
                QString clk = otherPortName(elm->input(1));
                QString r = otherPortName(elm->input(2));
                QString inclk = firstOut + "_inclk";
                out << QString("    //SR FlipFlop") << Qt::endl;
                out << QString("    if( %1 && !%2 ) { ").arg(clk, inclk) << Qt::endl;
                out << QString("        if( %1 && %2) { ").arg(s, r) << Qt::endl;
                out << QString("            %1 = 1;").arg(firstOut) << Qt::endl;
                out << QString("            %1 = 1;").arg(secondOut) << Qt::endl;
                out << QString("        } else if ( %1 != %2) {").arg(s, r) << Qt::endl;
                out << QString("            %1 = %2;").arg(firstOut, s) << Qt::endl;
                out << QString("            %1 = %2;").arg(secondOut, r) << Qt::endl;
                out << QString("        }") << Qt::endl;
                out << QString("    }") << Qt::endl;
                QString prst = otherPortName(elm->input(3));
                QString clr = otherPortName(elm->input(4));
                out << QString("    if( !%1 || !%2 ) { ").arg(prst, clr) << Qt::endl;
                out << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << Qt::endl;
                out << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << Qt::endl;
                out << QString("    }") << Qt::endl;

                /* Updating internal clock. */
                out << "    " << inclk << " = " << clk << ";" << Qt::endl;
                out << QString("    //End of SR FlipFlop") << Qt::endl;
                break;
            }
            case ElementType::TFLIPFLOP: {
                QString secondOut = varMap[elm->output(1)];
                QString t = otherPortName(elm->input(0));
                QString clk = otherPortName(elm->input(1));
                QString inclk = firstOut + "_inclk";
                //          QString last = firstOut + "_last";
                out << QString("    //T FlipFlop") << Qt::endl;
                out << QString("    if( %1 && !%2) { ").arg(clk,inclk) << Qt::endl;
                out << QString("        if( %1 ) { ").arg(t) << Qt::endl;
                out << QString("            %1 = !%1;").arg(firstOut) << Qt::endl;
                out << QString("            %1 = !%2;").arg(secondOut, firstOut) << Qt::endl;
                out << QString("        }") << Qt::endl;
                out << QString("    }") << Qt::endl;
                QString prst = otherPortName(elm->input(2));
                QString clr = otherPortName(elm->input(3));
                out << QString("    if( !%1 || !%2) { ").arg(prst, clr) << Qt::endl;
                out << QString("        %1 = !%2; //Preset").arg(firstOut, prst) << Qt::endl;
                out << QString("        %1 = !%2; //Clear").arg(secondOut, clr) << Qt::endl;
                out << QString("    }") << Qt::endl;

                /* Updating internal clock. */
                out << "    " << inclk << " = " << clk << ";" << Qt::endl;
                //          out << "    " << last << " = " << data << ";" << Qt::endl;
                out << QString("    //End of T FlipFlop") << Qt::endl;

                break;
            }
            case ElementType::AND:
            case ElementType::OR:
            case ElementType::NAND:
            case ElementType::NOR:
            case ElementType::XOR:
            case ElementType::XNOR:
            case ElementType::NOT:
            case ElementType::NODE:
                assignLogicOperator(elm);
                break;
            default:
                throw std::runtime_error(ERRORMSG(QString("Element type not supported : %1").arg(elm->objectName()).toStdString()));
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
    case ElementType::AND: {
        logicOperator = "&&";
        break;
    }
    case ElementType::OR: {
        logicOperator = "||";
        break;
    }
    case ElementType::NAND: {
        logicOperator = "&&";
        negate = true;
        break;
    }
    case ElementType::NOR: {
        logicOperator = "||";
        negate = true;
        break;
    }
    case ElementType::XOR: {
        logicOperator = "^";
        break;
    }
    case ElementType::XNOR: {
        logicOperator = "^";
        negate = true;
        break;
    }
    case ElementType::NOT: {
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
            out << "( ";
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
            out << " )";
        }
        out << ";" << Qt::endl;
    } else {
        /* ... */
    }
}

void CodeGenerator::loop()
{
    out << "void loop( ) {" << Qt::endl;
    out << "    // Reading input data //." << Qt::endl;
    for (const MappedPin &pin : qAsConst(inputMap)) {
        out << QString("    %1_val = digitalRead( %1 );").arg(pin.varName) << Qt::endl;
    }
    out << Qt::endl;
    out << "    // Updating clocks. //" << Qt::endl;
    for (GraphicElement *elm : elements) {
        if (elm->elementType() == ElementType::CLOCK) {
            const auto elm_outputs = elm->outputs();
            QString varName = varMap[elm_outputs.first()];
            out << QString("    if( %1_elapsed > %1_interval ){").arg(varName) << Qt::endl;
            out << QString("        %1_elapsed = 0;").arg(varName) << Qt::endl;
            out << QString("        %1 = ! %1;").arg(varName) << Qt::endl;
            out << QString("    }") << Qt::endl;
        }
    }
    /* Aux variables. */
    out << Qt::endl;
    out << "    // Assigning aux variables. //" << Qt::endl;
    assignVariablesRec(elements);
    out << "\n";
    out << "    // Writing output data. //\n";
    for (const MappedPin& pin : qAsConst(outputMap)) {
        QString varName = otherPortName(pin.port);
        if (varName.isEmpty()) {
            varName = highLow(pin.port->defaultValue());
        }
        out << QString("    digitalWrite( %1, %2 );").arg(pin.varName, varName) << Qt::endl;
    }
    out << "}" << Qt::endl;
}
