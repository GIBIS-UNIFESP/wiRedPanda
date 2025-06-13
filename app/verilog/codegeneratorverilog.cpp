// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeneratorverilog.h"

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

CodeGeneratorVerilog::CodeGeneratorVerilog(const QString &fileName, const QVector<GraphicElement *> &elements)
    : m_file(fileName)
    , m_elements(elements)
{
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    m_stream.setDevice(&m_file);

    /* Não é preciso para verilog */
    m_availablePins = QStringList{
        "sw0",
        "sw1",
        "sw1",
        "sw3",
        "sw4",
        "sw5",
        "sw6",
        "sw7",
        "sw8",
        "sw9",
        "sw10",
        "sw11",
        "sw12",
        "sw13",
    };

    QFileInfo info(fileName);
    m_fileName = info.completeBaseName();
}

static inline QString highLow(const Status val)
{
    return (val == Status::Active) ? "1'b1" : "1'b0";
}

QString CodeGeneratorVerilog::removeForbiddenChars(const QString &input)
{
    return input.toLower().trimmed().replace(' ', '_').replace('-', '_').replace(QRegularExpression("\\W"), "");
}

QString CodeGeneratorVerilog::stripAccents(const QString& input) {
    QString normalized = input.normalized(QString::NormalizationForm_D);
    // Remove combining marks (diacritics)
    QRegularExpression diacriticMarks("[\\p{Mn}]");
    return normalized.remove(diacriticMarks);
}

QString CodeGeneratorVerilog::otherPortName(QNEPort *port) {
    if (!port) return "1'b0";
    if (port->connections().isEmpty()) return highLow(port->defaultValue());

    auto *otherPort = port->connections().constFirst()->otherPort(port);
    if (!otherPort) return highLow(port->defaultValue());

    // Se a porta estiver conectada a um elemento lógico, retorna a expressão diretamente.
    auto *elm = otherPort->graphicElement();
    if (elm->elementType() == ElementType::And ||
        elm->elementType() == ElementType::Or ||
        elm->elementType() == ElementType::Nand ||
        elm->elementType() == ElementType::Nor ||
        elm->elementType() == ElementType::Xor ||
        elm->elementType() == ElementType::Xnor ||
        elm->elementType() == ElementType::Not ||
        elm->elementType() == ElementType::Node) {

        return generateLogicExpression(elm);
    }

    // Caso contrário, retorna o nome da variável mapeada.
    return m_varMap.value(otherPort);
}

void CodeGeneratorVerilog::generate()
{
    m_stream << "// ==================================================================== //" << endl;
    m_stream << "// ======= This code was generated automatically by wiRedPanda ======== //" << endl;
    m_stream << "// ==================================================================== //" << endl;
    m_stream << endl
             << endl;
    // m_stream << "#include <elapsedMillis.h>" << endl; // Não precisa em verilog
    m_stream << "module " << m_fileName << " (" << endl;
    /* Declaring input and output pins; */
    declareInputs();
    declareOutputs();
    declareAuxVariables();
    /* Setup section */
    setup();

    /* Loop section */
    loop();
}

void CodeGeneratorVerilog::declareInputs()
{
    int counter = 1;

    // Adicionado para controlar a questão da vírgula:
    int totalOutputs = 0;
    int currentOutput = 0;
    for (auto *elm : m_elements) {
        const auto type = elm->elementType();
        // Verifica se há um sinal de saída
        if (elm->elementGroup() == ElementGroup::Output){
            totalOutputs = INT_MAX;
            break;
        }
        // Verifica o número de sinais de entradas
        else if ((type == ElementType::InputButton) || (type == ElementType::InputSwitch) || (type == ElementType::Clock)) {        // Mudado para o clock
            totalOutputs += elm->outputs().size();
        }
    }

    m_stream << "/* ========= Inputs ========== */" << endl;

    for (auto *elm : m_elements) {
        const auto type = elm->elementType();

        if ((type == ElementType::InputButton) || (type == ElementType::InputSwitch) || (type == ElementType::Clock)) {
            QString varName = elm->objectName() + QString::number(counter);
            const QString label = elm->label();

            if (!label.isEmpty()) {
                varName += "_" + label;
            }

            varName = stripAccents(varName);
            varName = removeForbiddenChars(varName);

            currentOutput++;
            if (currentOutput < totalOutputs) {
                m_stream << QString("input %1,").arg(varName /*, m_availablePins.constFirst()*/) << endl;
            } else {
                m_stream << QString("input %1").arg(varName /*, m_availablePins.constFirst()*/) << endl;
            }

            m_inputMap.append(MappedPinVerilog(elm , /*m_availablePins.constFirst()*/ "", varName, elm->outputPort(0), 0));
            /*m_availablePins.removeFirst();*/
            m_varMap[elm->outputPort()] = varName + QString("_val");
            ++counter;
        }
    }

    m_stream << endl;
}

void CodeGeneratorVerilog::declareOutputs()
{
    int counter = 1;

    // Adicionado para controlar a questão da vírgula:
    int totalOutputs = 0;
    int currentOutput = 0;
    for (auto *elm : m_elements) {
        if (elm->elementGroup() == ElementGroup::Output) {
            totalOutputs += elm->inputs().size();
        }
    }

    m_stream << "/* ========= Outputs ========== */" << endl;
    for (auto *elm : m_elements) {
        if (elm->elementGroup() == ElementGroup::Output) {
            QString label = elm->label();
            for (int i = 0; i < elm->inputs().size(); ++i) {
                currentOutput++;
                QString varName = elm->objectName() + QString::number(counter);
                if (!label.isEmpty()) {
                    varName = QString("%1_%2").arg(varName, label);
                }
                QNEPort *port = elm->inputPort(i);
                if (!port->name().isEmpty()) {
                    varName = QString("%1_%2").arg(varName, port->name());
                }
                varName = stripAccents(varName);
                varName = removeForbiddenChars(varName);
                if (currentOutput < totalOutputs) {
                    m_stream << QString("output %1,").arg(varName) << endl;
                } else {
                    m_stream << QString("output %1").arg(varName) << endl;
                }
                m_outputMap.append(MappedPinVerilog(elm, /*m_availablePins.constFirst()*/ "", varName, port, i));
                /*m_availablePins.removeFirst();*/
            }
        }
        ++counter;
    }
    m_stream << ");" << endl; //Encerra o module
}

void CodeGeneratorVerilog::declareAuxVariablesRec(const QVector<GraphicElement *> &elements)
{
    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            //      IC *ic = qgraphicsitem_cast<IC *>(elm);

            // FIXME: Get code generator to work again
            //      if (ic) {
            //        out << "// " << ic->getLabel() << endl;
            //        declareAuxVariablesRec(ic->getElements(), true);
            //        out << "// END of " << ic->getLabel() << endl;
            //        for (int i = 0; i < ic->outputSize(); ++i) {
            //          QNEPort *port = ic->outputMap.at(i);
            //          varMap[ic->outputPort(i)] = otherPortName(port);
            //        }
            //      }
        } else {
            QString varName = QString("aux_%1_%2").arg(removeForbiddenChars(stripAccents(elm->objectName()))).arg(m_globalCounter++);
            const auto outputs = elm->outputs();

            if (outputs.size() == 1) {
                QNEPort *port = outputs.constFirst();

                if (elm->elementType() == ElementType::InputVcc) {
                    m_varMap[port] = "1'b1";
                    continue;
                }

                if (elm->elementType() == ElementType::InputGnd) {
                    m_varMap[port] = "1'b0";
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
                        portName.append(QString("_%1").arg(removeForbiddenChars(stripAccents(port->name()))));
                    }

                    m_varMap[port] = portName;
                }
            }
            int aux = 0; // Variável para inicializar os valores dos flip-flops
            for (auto *port : outputs) {
                QString varName2 = m_varMap.value(port);

                switch (elm->elementType()) {

                    /*
                case ElementType::Clock: {
                    if (!isBox) { // Para que serve isso?
                        // auto *clk = qgraphicsitem_cast<Clock *>(elm);
                        m_stream << "reg " << varName2 << ";" << endl;
                        m_stream << "reg [28:0]" << varName2 << "_elapse = 0;" << endl;
                        // m_stream << "elapsedMillis " << varName2 << "_elapsed = 0;" << endl;
                        // m_stream << "int " << varName2 << "_interval = " << 1000 / clk->frequency() << ";" << endl;
                    }
                    break;
                }
                */

                case ElementType::DLatch:
                case ElementType::SRLatch:
                case ElementType::SRFlipFlop:
                case ElementType::DFlipFlop:
                case ElementType::TFlipFlop:
                case ElementType::JKFlipFlop: {
                    m_stream << "reg " << varName2 << QString(" = 1'b%1;").arg(aux) << endl;
                    aux++;
                    break;
                }

                case ElementType::TruthTable:
                    if (!outputs.isEmpty()) {
                        QNEPort *outputPort = outputs.constFirst();
                        QString ttVarName = QString("%1_output").arg(removeForbiddenChars(elm->objectName()));
                        m_varMap[outputPort] = ttVarName;
                        m_stream << QString("reg ") << ttVarName << ";" << endl;

                        continue;
                    }
                    break;

                default:
                    m_stream << "wire " << varName2 << ";" << endl;
                    break;
                }
            }
        }
    }
}

void CodeGeneratorVerilog::declareAuxVariables()
{
    m_stream << "/* ====== Aux. Variables ====== */" << endl;
    declareAuxVariablesRec(m_elements);
    m_stream << endl;
}

void CodeGeneratorVerilog::setup()
{
    // Não precisa em verilog
    /*
    m_stream << "void setup() {" << endl;
    for (const auto &pin : std::as_const(m_inputMap)) {
        m_stream << "    pinMode(" << pin.m_varName << ", INPUT);" << endl;
    }
    for (const auto &pin : std::as_const(m_outputMap)) {
        m_stream << "    pinMode(" << pin.m_varName << ", OUTPUT);" << endl;
    }
    m_stream << "}" << endl
             << endl;
    */
}

void CodeGeneratorVerilog::assignVariablesRec(const QVector<GraphicElement *> &elements) {
    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            throw Pandaception(tr("IC element not supported: ") + elm->objectName());
        }
        if (elm->inputs().isEmpty() || elm->outputs().isEmpty()) {
            continue;
        }

        // Se for um operador lógico, não gera variável auxiliar, apenas propaga a expressão.
        if (elm->elementType() == ElementType::And ||
            elm->elementType() == ElementType::Or ||
            elm->elementType() == ElementType::Nand ||
            elm->elementType() == ElementType::Nor ||
            elm->elementType() == ElementType::Xor ||
            elm->elementType() == ElementType::Xnor ||
            elm->elementType() == ElementType::Not ||
            elm->elementType() == ElementType::Node) {

            QString expr = generateLogicExpression(elm);
            for (auto *port : elm->outputs()) {
                m_varMap[port] = expr; // Armazena a expressão diretamente no mapa.
            }
        }
        // Flip-flops e outros elementos ainda usam variáveis auxiliares.
        else {
            QString firstOut = m_varMap.value(elm->outputPort(0));
            switch (elm->elementType()) {
            case ElementType::DLatch: {
                QString secondOut = m_varMap.value(elm->outputPort(1));
                QString data = otherPortName(elm->inputPort(0));
                QString enable = otherPortName(elm->inputPort(1));
                m_stream << QString("    //D Latch") << endl;
                m_stream << QString("    always @(*)") << endl;
                m_stream << QString("    begin") << endl;
                m_stream << QString("        if (%1)").arg(enable) << endl;
                m_stream << QString("        begin") << endl;
                m_stream << QString("            %1 = %2;").arg(firstOut, data) << endl;
                m_stream << QString("            %1 = !%2;").arg(secondOut, data) << endl;
                m_stream << QString("        end") << endl;
                m_stream << QString("    end") << endl;
                m_stream << QString("    //End of D Latch") << endl;

                break;
            }
            case ElementType::SRLatch: {
                QString secondOut = m_varMap.value(elm->outputPort(1));
                QString s = otherPortName(elm->inputPort(0));
                QString r = otherPortName(elm->inputPort(1));

                m_stream << QString("    //SR Latch") << endl;
                m_stream << QString("    always @(*)") << endl;
                m_stream << QString("    begin") << endl;
                m_stream << QString("        if (%1 && %2)").arg(s, r) << endl;
                m_stream << QString("        begin") << endl;
                m_stream << QString("            %1 = 1'b0;").arg(firstOut) << endl;
                m_stream << QString("            %1 = 1'b0;").arg(secondOut) << endl;
                m_stream << QString("        end") << endl;
                m_stream << QString("        else if (%1 != %2)").arg(s, r) << endl;
                m_stream << QString("        begin") << endl;
                m_stream << QString("            %1 = %2;").arg(firstOut, s) << endl;
                m_stream << QString("            %1 = %2;").arg(secondOut, r) << endl;
                m_stream << QString("        end") << endl;
                m_stream << QString("    end") << endl;
                m_stream << QString("    //End of SR Latch") << endl;

                break;
            }
            case ElementType::DFlipFlop: {
                QString secondOut = m_varMap.value(elm->outputPort(1));
                QString data = otherPortName(elm->inputPort(0));
                QString clk = otherPortName(elm->inputPort(1));
                QString prst = otherPortName(elm->inputPort(2));
                QString clr = otherPortName(elm->inputPort(3));

                m_stream << QString("    //D FlipFlop") << endl;
                // Caso o preset e o clear tenham sido ligados
                if(prst != "1'b1" && prst != "1'b0" && clr != "1'b1" && clr != "1'b0"){
                    m_stream << QString("    always @(posedge %1 or negedge %2 or negedge %3)").arg(clk, prst, clr) << endl;
                }
                // Caso haja somente o preset
                else if(prst != "1'b1" && prst != "1'b0"){
                    m_stream << QString("    always @(posedge %1 or negedge %2)").arg(clk, prst) << endl;
                }
                // Caso haja somente o clear
                else if(clr != "1'b1" && clr != "1'b0"){
                    m_stream << QString("    always @(posedge %1 or negedge %2)").arg(clk, clr) << endl;
                }
                // Caso não haja preset e clear
                else{
                    m_stream << QString("    always @(posedge %1)").arg(clk) << endl;
                }

                // Escreve o código de acordo com os casos acima

                // Caso não haja preset ou clear
                if((prst == "1'b1" || prst == "1'b0") && (clr == "1'b1" || clr == "1'b0")){
                    m_stream << QString("    begin") << endl;
                    m_stream << QString("        %1 <= %2;").arg(firstOut, data) << endl;
                    m_stream << QString("        %1 <= !%2;").arg(secondOut, data) << endl;
                    m_stream << QString("    end") << endl;
                    m_stream << QString("    //End of D FlipFlop") << endl;

                }
                else{
                    m_stream << QString("    begin") << endl;
                    // Caso haja o preset
                    if(prst != "1'b1" && prst != "1'b0"){
                        m_stream << QString("        if(!%1)").arg(prst) << endl;
                        m_stream << QString("        begin") << endl;
                        m_stream << QString("            %1 <= 1'b1;").arg(firstOut) << endl;
                        m_stream << QString("            %1 <= !%2;").arg(secondOut, clr) << endl;
                        m_stream << QString("        end") << endl;

                        if(clr != "1'b1" && clr != "1'b0"){
                            m_stream << "        else ";
                        }
                        else{
                            m_stream << "        ";
                        }
                    }
                    else{
                        m_stream << "        ";
                    }
                    // Caso haja o clear
                    if(clr != "1'b1" && clr != "1'b0"){
                        m_stream << QString("if(!%1)").arg(clr) << endl;
                        m_stream << QString("        begin") << endl;
                        m_stream << QString("            %1 <= 1'b0;").arg(firstOut) << endl;
                        m_stream << QString("            %1 <= 1'b1;").arg(secondOut) << endl;
                        m_stream << QString("        end") << endl;
                        m_stream << "        ";
                    }
                    m_stream << QString("else") << endl;
                    m_stream << QString("        begin") << endl;
                    m_stream << QString("            %1 <= %2;").arg(firstOut, data) << endl;
                    m_stream << QString("            %1 <= !%2;").arg(secondOut, data) << endl;
                    m_stream << QString("        end") << endl;
                    m_stream << QString("    end") << endl;
                    m_stream << QString("    //End of D FlipFlop") << endl;
                }

                // Código completo para o caso de preset e clear
                /*
                m_stream << QString("    begin") << endl;
                m_stream << QString("        if(!%1)").arg(prst) << endl;
                m_stream << QString("        begin") << endl;
                m_stream << QString("            %1 <= 1'b1;").arg(firstOut) << endl;
                m_stream << QString("            %1 <= !%2;").arg(secondOut, clr) << endl;
                m_stream << QString("        end") << endl;
                m_stream << QString("        else if(!%1)").arg(clr) << endl;
                m_stream << QString("        begin") << endl;
                m_stream << QString("            %1 <= 1'b0;").arg(firstOut) << endl;
                m_stream << QString("            %1 <= 1'b1;").arg(secondOut) << endl;
                m_stream << QString("        end") << endl;
                m_stream << QString("        else") << endl;
                m_stream << QString("        begin") << endl;
                m_stream << QString("            %1 <= %2;").arg(firstOut, data) << endl;
                m_stream << QString("            %1 <= !%2;").arg(secondOut, data) << endl;
                m_stream << QString("        end") << endl;
                m_stream << QString("    end") << endl;
                m_stream << QString("    //End of D FlipFlop") << endl;
                */

                break;
            }
            case ElementType::JKFlipFlop: {
                QString secondOut = m_varMap.value(elm->outputPort(1));
                QString j = otherPortName(elm->inputPort(0));
                QString clk = otherPortName(elm->inputPort(1));
                QString k = otherPortName(elm->inputPort(2));
                QString prst = otherPortName(elm->inputPort(3));
                QString clr = otherPortName(elm->inputPort(4));

                m_stream << QString("    //JK FlipFlop") << endl;
                // Caso o preset e o clear tenham sido ligados
                if(prst != "1'b1" && prst != "1'b0" && clr != "1'b1" && clr != "1'b0"){
                    m_stream << QString("    always @(posedge %1 or negedge %2 or negedge %3)").arg(clk, prst, clr) << endl;
                }
                // Caso haja somente o preset
                else if(prst != "1'b1" && prst != "1'b0"){
                    m_stream << QString("    always @(posedge %1 or negedge %2)").arg(clk, prst) << endl;
                }
                // Caso haja somente o clear
                else if(clr != "1'b1" && clr != "1'b0"){
                    m_stream << QString("    always @(posedge %1 or negedge %2)").arg(clk, clr) << endl;
                }
                // Caso não haja preset e clear
                else{
                    m_stream << QString("    always @(posedge %1)").arg(clk) << endl;
                }

                // Caso não haja preset ou clear
                if((prst == "1'b1" || prst == "1'b0") && (clr == "1'b1" || clr == "1'b0")){
                    m_stream << QString("    begin") << endl;
                    m_stream << QString("        if(%1 && %2)").arg(j, k) << endl;
                    m_stream << QString("        begin") << endl;
                    m_stream << QString("            %1 <= %2;").arg(firstOut, secondOut) << endl;
                    m_stream << QString("            %1 <= %2;").arg(secondOut, firstOut) << endl;
                    m_stream << QString("        end") << endl;
                    m_stream << QString("        else if(%1 && !%2)").arg(j, k) << endl;
                    m_stream << QString("        begin") << endl;
                    m_stream << QString("            %1 <= 1'b1;").arg(firstOut) << endl;
                    m_stream << QString("            %1 <= 1'b0;").arg(secondOut) << endl;
                    m_stream << QString("        end") << endl;
                    m_stream << QString("        else if(!%1 && %2)").arg(j, k) << endl;
                    m_stream << QString("        begin") << endl;
                    m_stream << QString("            %1 <= 1'b0;").arg(firstOut) << endl;
                    m_stream << QString("            %1 <= 1'b1;").arg(secondOut) << endl;
                    m_stream << QString("        end") << endl;
                    m_stream << QString("    end") << endl;
                    m_stream << QString("    //End of JK FlipFlop") << endl;

                }
                else{
                    m_stream << QString("    begin") << endl;
                    // Caso haja o preset
                    if(prst != "1'b1" && prst != "1'b0"){
                        m_stream << QString("        if(!%1)").arg(prst) << endl;
                        m_stream << QString("        begin") << endl;
                        m_stream << QString("            %1 <= 1'b1;").arg(firstOut) << endl;
                        m_stream << QString("            %1 <= !%2;").arg(secondOut, clr) << endl;
                        m_stream << QString("        end") << endl;

                        if(clr != "1'b1" && clr != "1'b0"){
                            m_stream << "        else ";
                        }
                        else{
                            m_stream << "        ";
                        }
                    }
                    else{
                        m_stream << "        ";
                    }
                    // Caso haja o clear
                    if(clr != "1'b1" && clr != "1'b0"){
                        m_stream << QString("if(!%1)").arg(clr) << endl;
                        m_stream << QString("        begin") << endl;
                        m_stream << QString("            %1 <= 1'b0;").arg(firstOut) << endl;
                        m_stream << QString("            %1 <= 1'b1;").arg(secondOut) << endl;
                        m_stream << QString("        end") << endl;
                        m_stream << "        ";
                    }
                    m_stream << QString("else") << endl;
                    m_stream << QString("        begin") << endl;
                    m_stream << QString("            if(%1 && %2)").arg(j, k) << endl;
                    m_stream << QString("            begin") << endl;
                    m_stream << QString("                %1 <= %2;").arg(firstOut, secondOut) << endl;
                    m_stream << QString("                %1 <= %2;").arg(secondOut, firstOut) << endl;
                    m_stream << QString("            end") << endl;
                    m_stream << QString("            else if(%1 && !%2)").arg(j, k) << endl;
                    m_stream << QString("            begin") << endl;
                    m_stream << QString("                %1 <= 1'b1;").arg(firstOut) << endl;
                    m_stream << QString("                %1 <= 1'b0;").arg(secondOut) << endl;
                    m_stream << QString("            end") << endl;
                    m_stream << QString("            else if(!%1 && %2)").arg(j, k) << endl;
                    m_stream << QString("            begin") << endl;
                    m_stream << QString("                %1 <= 1'b0;").arg(firstOut) << endl;
                    m_stream << QString("                %1 <= 1'b1;").arg(secondOut) << endl;
                    m_stream << QString("            end") << endl;
                    m_stream << QString("        end") << endl;
                    m_stream << QString("    end") << endl;
                    m_stream << QString("    //End of JK FlipFlop") << endl;
                }

                // Código completo para o caso de preset e clear
                /*
                m_stream << QString("    begin") << endl;
                m_stream << QString("        if(!%1)").arg(prst) << endl;
                m_stream << QString("        begin") << endl;
                m_stream << QString("            %1 <= 1'b1;").arg(firstOut) << endl;
                m_stream << QString("            %1 <= !%2;").arg(secondOut, clr) << endl;
                m_stream << QString("        end") << endl;
                m_stream << QString("        else if(!%1)").arg(clr) << endl;
                m_stream << QString("        begin") << endl;
                m_stream << QString("            %1 <= 1'b0;").arg(firstOut) << endl;
                m_stream << QString("            %1 <= 1'b1;").arg(secondOut) << endl;
                m_stream << QString("        end") << endl;
                m_stream << QString("        else") << endl;
                m_stream << QString("        begin") << endl;
                m_stream << QString("            if(%1 && %2)").arg(j, k) << endl;
                m_stream << QString("            begin") << endl;
                m_stream << QString("                %1 <= %2;").arg(firstOut, secondOut) << endl;
                m_stream << QString("                %1 <= %2;").arg(secondOut, firstOut) << endl;
                m_stream << QString("            end") << endl;
                m_stream << QString("            else if(%1 && !%2)").arg(j, k) << endl;
                m_stream << QString("            begin") << endl;
                m_stream << QString("                %1 <= 1'b1;").arg(firstOut) << endl;
                m_stream << QString("                %1 <= 1'b0;").arg(secondOut) << endl;
                m_stream << QString("            end") << endl;
                m_stream << QString("            else if(!%1 && %2)").arg(j, k) << endl;
                m_stream << QString("            begin") << endl;
                m_stream << QString("                %1 <= 1'b0;").arg(firstOut) << endl;
                m_stream << QString("                %1 <= 1'b1;").arg(secondOut) << endl;
                m_stream << QString("            end") << endl;
                m_stream << QString("        end") << endl;
                m_stream << QString("    end") << endl;
                m_stream << QString("    //End of JK FlipFlop") << endl;
                */

                break;
            }
            case ElementType::TFlipFlop: {
                QString secondOut = m_varMap.value(elm->outputPort(1));
                QString t = otherPortName(elm->inputPort(0));
                QString clk = otherPortName(elm->inputPort(1));
                QString prst = otherPortName(elm->inputPort(2));
                QString clr = otherPortName(elm->inputPort(3));

                m_stream << QString("    //T FlipFlop") << endl;
                // Caso o preset e o clear tenham sido ligados
                if(prst != "1'b1" && prst != "1'b0" && clr != "1'b1" && clr != "1'b0"){
                    m_stream << QString("    always @(posedge %1 or negedge %2 or negedge %3)").arg(clk, prst, clr) << endl;
                }
                // Caso haja somente o preset
                else if(prst != "1'b1" && prst != "1'b0"){
                    m_stream << QString("    always @(posedge %1 or negedge %2)").arg(clk, prst) << endl;
                }
                // Caso haja somente o clear
                else if(clr != "1'b1" && clr != "1'b0"){
                    m_stream << QString("    always @(posedge %1 or negedge %2)").arg(clk, clr) << endl;
                }
                // Caso não haja preset e clear
                else{
                    m_stream << QString("    always @(posedge %1)").arg(clk) << endl;
                }

                // Caso não haja preset ou clear
                if((prst == "1'b1" || prst == "1'b0") && (clr == "1'b1" || clr == "1'b0")){
                    m_stream << QString("    begin") << endl;
                    m_stream << QString("        if(%1)").arg(t) << endl;
                    m_stream << QString("        begin") << endl;
                    m_stream << QString("            %1 <= %2;").arg(firstOut, secondOut) << endl;
                    m_stream << QString("            %1 <= %2;").arg(secondOut, firstOut) << endl;
                    m_stream << QString("        end") << endl;
                    m_stream << QString("    end") << endl;
                    m_stream << QString("    //End of T FlipFlop") << endl;

                }
                else{
                    m_stream << QString("    begin") << endl;
                    // Caso haja o preset
                    if(prst != "1'b1" && prst != "1'b0"){
                        m_stream << QString("        if(!%1)").arg(prst) << endl;
                        m_stream << QString("        begin") << endl;
                        m_stream << QString("            %1 <= 1'b1;").arg(firstOut) << endl;
                        m_stream << QString("            %1 <= !%2;").arg(secondOut, clr) << endl;
                        m_stream << QString("        end") << endl;

                        if(clr != "1'b1" && clr != "1'b0"){
                            m_stream << "        else ";
                        }
                        else{
                            m_stream << "        ";
                        }
                    }
                    else{
                        m_stream << "        ";
                    }
                    // Caso haja o clear
                    if(clr != "1'b1" && clr != "1'b0"){
                        m_stream << QString("if(!%1)").arg(clr) << endl;
                        m_stream << QString("        begin") << endl;
                        m_stream << QString("            %1 <= 1'b0;").arg(firstOut) << endl;
                        m_stream << QString("            %1 <= 1'b1;").arg(secondOut) << endl;
                        m_stream << QString("        end") << endl;
                        m_stream << "        ";
                    }
                    m_stream << QString("else if(%1)").arg(t) << endl;
                    m_stream << QString("        begin") << endl;
                    m_stream << QString("            %1 <= %2;").arg(firstOut, secondOut) << endl;
                    m_stream << QString("            %1 <= %2;").arg(secondOut, firstOut) << endl;
                    m_stream << QString("        end") << endl;
                    m_stream << QString("    end") << endl;
                    m_stream << QString("    //End of T FlipFlop") << endl;
                }

                // Código completo para o caso de preset e clear
                /*
                m_stream << QString("    begin") << endl;
                m_stream << QString("        if(!%1)").arg(prst) << endl;
                m_stream << QString("        begin") << endl;
                m_stream << QString("            %1 <= 1'b1;").arg(firstOut) << endl;
                m_stream << QString("            %1 <= !%2;").arg(secondOut, clr) << endl;
                m_stream << QString("        end") << endl;
                m_stream << QString("        else if(!%1)").arg(clr) << endl;
                m_stream << QString("        begin") << endl;
                m_stream << QString("            %1 <= 1'b0;").arg(firstOut) << endl;
                m_stream << QString("            %1 <= 1'b1;").arg(secondOut) << endl;
                m_stream << QString("        end") << endl;
                m_stream << QString("        else if(%1)").arg(t) << endl;
                m_stream << QString("        begin") << endl;
                m_stream << QString("            %1 <= %2;").arg(firstOut, secondOut) << endl;
                m_stream << QString("            %1 <= %2;").arg(secondOut, firstOut) << endl;
                m_stream << QString("        end") << endl;
                m_stream << QString("    end") << endl;
                m_stream << QString("    //End of T FlipFlop") << endl;
                */

                break;
            }
            case ElementType::TruthTable: {
                auto* ttGraphic = dynamic_cast<TruthTable*>(elm);
                LogicTruthTable *ttLogic = dynamic_cast<LogicTruthTable *>(ttGraphic->logic());

                QBitArray propositions = ttLogic->proposition();
                const int nInputs = elm->inputSize();
                const int rows = 1 << nInputs;

                // Verifica se a saída está conectada
                QString outputVarName = m_varMap.value(elm->outputPort(0));
                if (outputVarName.isEmpty()) {
                    throw Pandaception(tr("Output variable not mapped for TruthTable: ") + elm->objectName());
                }

                // Tratando os sinais de entrada
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

                // Escrevendo os sinais de entrada para o case
                QStringList bitExpressions;
                bitExpressions << "{";
                for (int i = 0; i < nInputs; ++i) {
                    if(i < nInputs - 1) {
                        bitExpressions << QString("%1, ").arg(inputSignalNames[i]);
                    } else {
                        bitExpressions << QString("%1}").arg(inputSignalNames[i]);
                    }
                }

                // Transforma bitExpression em string para ser escrita
                QString indexCalculation = bitExpressions.join("");
                m_stream << QString("    //TruthTable") << endl;
                m_stream << QString("    always @(*)") << endl;
                m_stream << QString("    begin") << endl;
                m_stream << QString("        case(%1)").arg(indexCalculation) << endl;


                for (int i = 0; i < rows; ++i) {
                    m_stream << QString("            %1'b").arg(nInputs) << QString::number(i, 2).rightJustified(nInputs, '0') << ": " << outputVarName << " = 1'b" << (propositions.testBit(i) ? "1" : "0") << ";" << endl;
                }
                m_stream << QString("        endcase") << endl;
                m_stream << QString("    end") << endl;
                m_stream << QString("    //End TruthTable") << endl;

                break;
            }

            default:                throw Pandaception(tr("Element type not supported: ") + elm->objectName());
            }
        }
    }
}

QString CodeGeneratorVerilog::generateLogicExpression(GraphicElement *elm) {
    bool negate = false;
    QString logicOperator;

    switch (elm->elementType()) {
    case ElementType::And: logicOperator = "&"; break;
    case ElementType::Or:  logicOperator = "|"; break;
    case ElementType::Nand: logicOperator = "&"; negate = true; break;
    case ElementType::Nor: logicOperator = "|"; negate = true; break;
    case ElementType::Xor: logicOperator = "^"; break;
    case ElementType::Xnor: logicOperator = "^"; negate = true; break;
    case ElementType::Not: return "~" + otherPortName(elm->inputPort(0)); // NOT não precisa de parênteses extras
    default: return "";
    }

    QString expr;
    if (elm->inputs().size() == 1) {
        // Se houver apenas uma entrada, não precisa de parênteses
        expr = otherPortName(elm->inputPort(0));
    } else {
        // Para múltiplas entradas, agrupa com parênteses
        expr = "(";
        for (int i = 0; i < elm->inputs().size(); ++i) {
            if (i > 0) expr += " " + logicOperator + " ";
            expr += otherPortName(elm->inputPort(i));
        }
        expr += ")";
    }

    if (negate) {
        expr = "~" + expr;
    }

    return expr;
}

void CodeGeneratorVerilog::loop() {
    m_stream << "// Reading input data //." << endl;
    for (const auto &pin : std::as_const(m_inputMap)) {
        m_stream << QString("assign %1_val = %1;").arg(pin.m_varName) << endl;
    }

    /*
    m_stream << "\n// Updating clocks. //" << endl;
    for (auto *elm : m_elements) {
        if (elm->elementType() == ElementType::Clock) {
            auto *clk = qgraphicsitem_cast<Clock *>(elm);
            const auto elmOutputs = elm->outputs();
            QString varName = m_varMap.value(elmOutputs.constFirst());

            const float frequency = 50000000/clk->frequency();

            m_stream << QString("always @(posedge clock_50MHz)") << endl;
            m_stream << QString("begin") << endl;
            m_stream << QString("    if (%1_elapsed == 29'd%2)").arg(varName).arg(frequency, 0, 'f', 2) << endl;
            m_stream << QString("    begin") << endl;
            m_stream << QString("        %1_elapsed <= 29d'0;").arg(varName) << endl;
            m_stream << QString("        %1 = ! %1;").arg(varName) << endl;
            m_stream << QString("    end") << endl;
            m_stream << QString("    else %1_elapsed <= %1_elapsed + 1;").arg(varName) << endl;
            m_stream << QString("end") << endl;
        }
    }
    */

    m_stream << "\n// Assigning aux variables. //" << endl;
    assignVariablesRec(m_elements);

    m_stream << "\n// Writing output data. //" << endl;
    for (const auto &pin : std::as_const(m_outputMap)) {
        QString expr = otherPortName(pin.m_port);
        if (expr.isEmpty()) {
            expr = highLow(pin.m_port->defaultValue());
        }
        m_stream << QString("assign %1 = %2;").arg(pin.m_varName, expr) << endl;
    }
    m_stream << "endmodule" << endl;
}
