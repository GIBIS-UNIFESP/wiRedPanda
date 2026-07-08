// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/CodeGen/ArduinoCodeGen.h"

#include <algorithm>
#include <functional>

#include <QRegularExpression>
#include <QSet>

#include "App/CodeGen/CodeGenUtils.h"
#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/Buzzer.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/IC.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

ArduinoCodeGen::ArduinoCodeGen(const QString &fileName, const QVector<GraphicElement *> &elements)
    : m_file(fileName)
    , m_elements(elements)
{
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw PANDACEPTION("Could not open file for writing: %1", fileName);
    }
    m_stream.setDevice(&m_file);
}

QString ArduinoCodeGen::highLow(const Status val)
{
    return (val == Status::Active) ? "HIGH" : "LOW";
}

QString ArduinoCodeGen::removeForbiddenChars(const QString &input)
{
    return CodeGenUtils::removeForbiddenChars(input, true);
}

bool ArduinoCodeGen::isArduinoReserved(const QString &name)
{
    static const QSet<QString> reserved = {
        // C++ keywords
        "alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit",
        "atomic_noexcept", "auto", "bitand", "bitor", "bool", "break", "case", "catch",
        "char", "char8_t", "char16_t", "char32_t", "class", "compl", "concept",
        "const", "consteval", "constexpr", "constinit", "const_cast", "continue",
        "co_await", "co_return", "co_yield", "decltype", "default", "delete", "do",
        "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern",
        "false", "float", "for", "friend", "goto", "if", "inline", "int", "long",
        "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "or",
        "or_eq", "private", "protected", "public", "reflexpr", "register",
        "reinterpret_cast", "requires", "return", "short", "signed", "sizeof",
        "static", "static_assert", "static_cast", "struct", "switch", "synchronized",
        "template", "this", "thread_local", "throw", "true", "try", "typedef",
        "typeid", "typename", "union", "unsigned", "using", "virtual", "void",
        "volatile", "wchar_t", "while", "xor", "xor_eq",
        // Arduino built-ins and functions (lowercase to match post-removeForbiddenChars names)
        "high", "low", "input", "output", "input_pullup", "input_pulldown",
        "led_builtin", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
        "pinmode", "digitalwrite", "digitalread", "analogread", "analogwrite",
        "analogreadresolution", "analogwriteresolution", "serial",
        "tone", "notone", "delay", "millis", "micros", "setup", "loop"
    };
    return reserved.contains(name);
}

QString ArduinoCodeGen::otherPortName(Port *port)
{
    QSet<Port *> visited;
    return otherPortNameImpl(port, visited);
}

QString ArduinoCodeGen::otherPortNameImpl(Port *port, QSet<Port *> &visited)
{
    if (!port) {
        return "LOW";
    }

    // Cycle detection: if we've already visited this port, don't continue
    if (visited.contains(port)) {
        const QString mapped = m_varMap.value(port);
        return mapped.isEmpty() ? "LOW" : mapped;
    }

    if (port->connections().isEmpty()) {
        // Check m_varMap first — IC input boundary nodes have no connections but a mapped variable
        const QString mapped = m_varMap.value(port);
        if (!mapped.isEmpty()) {
            return mapped;
        }
        // Wireless Rx: resolve via the Tx node's input (what drives the transmitter)
        auto *elm = port->graphicElement();
        if (elm && elm->wirelessMode() == WirelessMode::Rx && !elm->label().isEmpty()) {
            auto *txInputPort = m_txInputPorts.value(elm->label(), nullptr);
            if (txInputPort) {
                return otherPortNameImpl(txInputPort, visited);
            }
        }
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

    // Cycle detection: if we've already visited the connected port, don't continue
    if (visited.contains(otherPort)) {
        const QString mapped = m_varMap.value(otherPort);
        return mapped.isEmpty() ? "LOW" : mapped;
    }

    // Mark both ports as visited
    visited.insert(port);
    visited.insert(otherPort);

    const QString result = m_varMap.value(otherPort);
    if (result.isEmpty()) {
        return highLow(otherPort->defaultValue());
    }
    return result;
}

void ArduinoCodeGen::generate()
{
    try {
        m_txInputPorts = Scene::wirelessTxInputPorts(m_elements);
        m_hasSequential = hasNativeMemory(m_elements);

        int requiredInputPins = 0;
        int requiredOutputPins = 0;
        for (auto *elm : m_elements) {
            const auto type = elm->elementType();
            if (type == ElementType::InputButton || type == ElementType::InputSwitch) {
                requiredInputPins++;
            } else if (type == ElementType::InputRotary) {
                requiredInputPins += elm->outputSize();
            }
        }
        for (auto *elm : m_elements) {
            if (elm->elementGroup() == ElementGroup::Output) {
                requiredOutputPins += static_cast<int>(elm->inputs().size());
            }
        }
        const int totalRequiredPins = requiredInputPins + requiredOutputPins;
        m_selectedBoard = selectBoard(totalRequiredPins);
        m_availablePins = m_selectedBoard.availablePins;

        m_stream << "// ==================================================================== //" << Qt::endl;
        m_stream << "// ======= This code was generated automatically by wiRedPanda ======== //" << Qt::endl;
        m_stream << "// ==================================================================== //" << Qt::endl;
        m_stream << "//" << Qt::endl;
        m_stream << QString("// Target Board: %1").arg(m_selectedBoard.name) << Qt::endl;
        m_stream << QString("// Pin Usage: %1/%2 pins").arg(totalRequiredPins).arg(m_selectedBoard.maxPins()) << Qt::endl;
        m_stream << "//" << Qt::endl;
        m_stream << Qt::endl
                 << Qt::endl;
        m_stream << "#include <elapsedMillis.h>" << Qt::endl;
        declareInputs();
        declareOutputs();
        declareAuxVariables();
        if (m_hasSequential) {
            // Phase flag for the non-blocking tick driver: flip-flops sample only
            // while true; the post-commit re-settle runs with it false.
            m_stream << "bool g_sample = true;" << Qt::endl << Qt::endl;
        }
        setup();
        emitComputeLogicFunction();
        if (m_hasSequential) {
            emitCommitFlipFlops();
        }
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

        if ((type == ElementType::InputButton) || (type == ElementType::InputSwitch)) {
            // Check if we have available pins before assigning
            if (m_availablePins.isEmpty()) {
                throw PANDACEPTION("Not enough pins available for all input elements");
            }

            QString varName = elm->objectName() + QString::number(counter);
            const QString label = elm->label();

            if (!label.isEmpty()) {
                varName += "_" + label;
            }

            varName = removeForbiddenChars(varName);
            if (isArduinoReserved(varName)) {
                varName.append('_');
            }
            m_stream << QString("const int %1 = %2;").arg(varName, m_availablePins.constFirst()) << Qt::endl;
            auto *outPort = elm->outputPort(0);
            if (outPort) {
                m_inputMap.append(MappedPin(elm, m_availablePins.constFirst(), varName, outPort, 0));
                m_varMap[outPort] = varName + QString("_val");
            }
            m_availablePins.removeFirst();
            ++counter;
        } else if (type == ElementType::InputRotary) {
            // Each rotary position maps to one digital input pin; exactly one is HIGH at a time.
            const QString label = elm->label();
            for (int i = 0; i < elm->outputSize(); ++i) {
                if (m_availablePins.isEmpty()) {
                    throw PANDACEPTION("Not enough pins available for all rotary switch positions");
                }
                QString varName = elm->objectName() + QString::number(counter) + "_pos" + QString::number(i);
                if (!label.isEmpty()) {
                    varName = elm->objectName() + QString::number(counter) + "_" + label + "_pos" + QString::number(i);
                }
                varName = removeForbiddenChars(varName);
                if (isArduinoReserved(varName)) {
                    varName.append('_');
                }
                m_stream << QString("const int %1 = %2;").arg(varName, m_availablePins.constFirst()) << Qt::endl;
                auto *outPort = elm->outputPort(i);
                if (outPort) {
                    m_inputMap.append(MappedPin(elm, m_availablePins.constFirst(), varName, outPort, i));
                    m_varMap[outPort] = varName + "_val";
                }
                m_availablePins.removeFirst();
            }
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
                Port *port = elm->inputPort(i);
                if (port && !port->name().isEmpty()) {
                    varName = QString("%1_%2").arg(varName, port->name());
                }
                varName = removeForbiddenChars(varName);
                if (isArduinoReserved(varName)) {
                    varName.append('_');
                }
                m_stream << QString("const int %1 = %2;").arg(varName, m_availablePins.constFirst()) << Qt::endl;
                m_outputMap.append(MappedPin(elm, m_availablePins.constFirst(), varName, port, i));
                m_availablePins.removeFirst();
            }
            ++counter;
        }
    }
    m_stream << Qt::endl;
}

void ArduinoCodeGen::declareAuxVariablesRec(const QVector<GraphicElement *> &elements, const bool isBox, const QString &icPrefix)
{
    int counter = 0;

    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = qobject_cast<IC *>(elm);
            if (!ic) continue;

            m_stream << "// IC: " << CodeGenUtils::sanitizeComment(ic->label()) << Qt::endl;

            // Include the full ancestor path (icPrefix) so names stay globally
            // unique across repeated/nested sub-IC instances (register files, RAM,
            // CPUs). Without it, structurally-identical sub-trees collide.
            QString baseVarName = QString("aux_%1%2_%3").arg(icPrefix, removeForbiddenChars(ic->label()), QString::number(counter++));

            for (int i = 0; i < ic->outputSize(); ++i) {
                Port *externalPort = ic->outputPort(i);
                if (externalPort) {
                    QString portVarName = baseVarName;
                    const QString portName = externalPort->name();
                    if (!portName.isEmpty()) {
                        portVarName += "_" + removeForbiddenChars(portName);
                        if (i > 0) {
                            portVarName += "_" + QString::number(i);
                        }
                    } else {
                        portVarName += "_out" + QString::number(i);
                    }
                    m_varMap[externalPort] = portVarName;
                    if (!m_declaredVariables.contains(portVarName)) {
                        m_stream << "bool " << portVarName << " = " << highLow(externalPort->defaultValue()) << ";" << Qt::endl;
                        m_declaredVariables.append(portVarName);
                    }
                }
            }

            if (!ic->internalElements().isEmpty()) {
                const QString nestedPrefix = QString("%1%2_%3_").arg(icPrefix, removeForbiddenChars(ic->label()), QString::number(counter - 1));
                declareAuxVariablesRec(ic->internalElements(), true, nestedPrefix);

                for (int i = 0; i < ic->internalInputs().size(); ++i) {
                    Port *internalPort = ic->internalInputs().at(i);
                    if (m_varMap.value(internalPort).isEmpty()) {
                        const QString portVarName = QString("aux_ic_input_%1%2_%3_%4").arg(icPrefix, removeForbiddenChars(ic->label()), QString::number(counter - 1), QString::number(i));
                        m_varMap[internalPort] = portVarName;
                        if (!m_declaredVariables.contains(portVarName)) {
                            m_stream << "bool " << portVarName << " = LOW;" << Qt::endl;
                            m_declaredVariables.append(portVarName);
                        }
                    }
                }
            }

            m_stream << "// End IC: " << CodeGenUtils::sanitizeComment(ic->label()) << Qt::endl;
            continue;
        }

        const auto outputs = elm->outputs();

        QString baseVarName;
        if (!icPrefix.isEmpty()) {
            baseVarName = QString("aux_%1%2_%3").arg(icPrefix, removeForbiddenChars(elm->objectName()), QString::number(counter++));
        } else {
            baseVarName = QString("aux_%1_%2").arg(removeForbiddenChars(elm->objectName()), QString::number(counter++));
        }

        if (outputs.size() == 1) {
            Port *port = outputs.constFirst();

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
            const QString varName2 = m_varMap.value(port);
            if (!m_declaredVariables.contains(varName2)) {
                m_stream << "bool " << varName2 << " = " << highLow(port->defaultValue()) << ";" << Qt::endl;
                m_declaredVariables.append(varName2);
            }

            // Staging variable for non-blocking sequential commit (see emitTickDriver).
            if (elm->elementGroup() == ElementGroup::Memory) {
                m_stream << "bool " << varName2 << "_next = " << highLow(port->defaultValue()) << ";" << Qt::endl;
            }

            switch (elm->elementType()) {
            case ElementType::Clock: {
                if (!isBox) {
                    auto *clk = qobject_cast<Clock *>(elm);
                    if (!clk) {
                        break;
                    }
                    m_stream << "elapsedMillis " << varName2 << "_elapsed = 0;" << Qt::endl;
                    m_stream << "int " << varName2 << "_interval = " << (std::max)(1, static_cast<int>(1000.0 / clk->frequency())) << ";" << Qt::endl;
                }
                break;
            }
            case ElementType::DFlipFlop: {
                m_stream << "bool " << varName2 << "_inclk = LOW;" << Qt::endl;
                m_stream << "bool " << varName2 << "_last = LOW;" << Qt::endl;
                break;
            }
            case ElementType::TFlipFlop:
            case ElementType::SRFlipFlop:
            case ElementType::JKFlipFlop: {
                m_stream << "bool " << varName2 << "_inclk = LOW;" << Qt::endl;
                break;
            }
            case ElementType::And:
            case ElementType::AudioBox:
            case ElementType::Buzzer:
            case ElementType::DLatch:
            case ElementType::Demux:
            case ElementType::Display14:
            case ElementType::Display16:
            case ElementType::Display7:
            case ElementType::IC:
            case ElementType::InputButton:
            case ElementType::InputGnd:
            case ElementType::InputRotary:
            case ElementType::InputSwitch:
            case ElementType::InputVcc:
            case ElementType::JKLatch:
            case ElementType::Led:
            case ElementType::Line:
            case ElementType::Mux:
            case ElementType::Nand:
            case ElementType::Node:
            case ElementType::Nor:
            case ElementType::Not:
            case ElementType::Or:
            case ElementType::SRLatch:
            case ElementType::Text:
            case ElementType::TruthTable:
            case ElementType::Unknown:
            case ElementType::Xnor:
            case ElementType::Xor:
                break;
            }
        }
    }
}

void ArduinoCodeGen::declareSequentialStateRec(const QVector<GraphicElement *> &elements, const bool topLevel)
{
    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            if (auto *ic = qobject_cast<IC *>(elm)) {
                declareSequentialStateRec(ic->internalElements(), false);
            }
            continue;
        }

        const auto outputs = elm->outputs();
        if (outputs.isEmpty()) {
            continue;
        }
        // Staging variables for non-blocking sequential commit, one per output.
        if (elm->elementGroup() == ElementGroup::Memory) {
            for (auto *port : outputs) {
                const QString v = m_varMap.value(port);
                if (!v.isEmpty()) {
                    m_stream << "bool " << v << "_next = " << highLow(port->defaultValue()) << ";" << Qt::endl;
                }
            }
        }

        const QString varName = m_varMap.value(outputs.constFirst());
        if (varName.isEmpty()) {
            continue;
        }

        switch (elm->elementType()) {
        case ElementType::Clock:
            // Mirror declareAuxVariablesRec: only top-level clocks are time-driven.
            if (topLevel) {
                m_stream << "elapsedMillis " << varName << "_elapsed = 0;" << Qt::endl;
                m_stream << "int " << varName << "_interval = 1000;" << Qt::endl;
            }
            break;
        case ElementType::DFlipFlop:
            m_stream << "bool " << varName << "_inclk = LOW;" << Qt::endl;
            m_stream << "bool " << varName << "_last = LOW;" << Qt::endl;
            break;
        case ElementType::TFlipFlop:
        case ElementType::SRFlipFlop:
        case ElementType::JKFlipFlop:
            m_stream << "bool " << varName << "_inclk = LOW;" << Qt::endl;
            break;
        case ElementType::And:
        case ElementType::AudioBox:
        case ElementType::Buzzer:
        case ElementType::DLatch:
        case ElementType::Demux:
        case ElementType::Display14:
        case ElementType::Display16:
        case ElementType::Display7:
        case ElementType::IC:
        case ElementType::InputButton:
        case ElementType::InputGnd:
        case ElementType::InputRotary:
        case ElementType::InputSwitch:
        case ElementType::InputVcc:
        case ElementType::JKLatch:
        case ElementType::Led:
        case ElementType::Line:
        case ElementType::Mux:
        case ElementType::Nand:
        case ElementType::Node:
        case ElementType::Nor:
        case ElementType::Not:
        case ElementType::Or:
        case ElementType::SRLatch:
        case ElementType::Text:
        case ElementType::TruthTable:
        case ElementType::Unknown:
        case ElementType::Xnor:
        case ElementType::Xor:
            break;
        }
    }
}

bool ArduinoCodeGen::hasNativeMemory(const QVector<GraphicElement *> &elements)
{
    for (auto *elm : elements) {
        if (elm->elementGroup() == ElementGroup::Memory) {
            return true;
        }
        if (elm->elementType() == ElementType::IC) {
            if (auto *ic = qobject_cast<IC *>(elm)) {
                if (hasNativeMemory(ic->internalElements())) {
                    return true;
                }
            }
        }
    }
    return false;
}

void ArduinoCodeGen::emitCommitFlipFlopsRec(const QVector<GraphicElement *> &elements)
{
    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            if (auto *ic = qobject_cast<IC *>(elm)) {
                emitCommitFlipFlopsRec(ic->internalElements());
            }
            continue;
        }
        if (elm->elementGroup() != ElementGroup::Memory) {
            continue;
        }
        for (auto *port : elm->outputs()) {
            const QString varName = m_varMap.value(port);
            if (!varName.isEmpty()) {
                m_stream << "    " << varName << " = " << varName << "_next;" << Qt::endl;
            }
        }
    }
}

void ArduinoCodeGen::emitCommitFlipFlops()
{
    m_stream << "void commitFlipFlops() {" << Qt::endl;
    emitCommitFlipFlopsRec(m_elements);
    m_stream << "}" << Qt::endl << Qt::endl;
}

void ArduinoCodeGen::emitTickDriver()
{
    // One simulation tick. Sequential sketches mirror the engine's non-blocking
    // semantics: settle combinational logic while flip-flops sample into staging
    // (_next) reading pre-edge state, commit all flip-flops at once, then settle
    // again so combinational outputs reflect the new committed state.
    if (m_hasSequential) {
        m_stream << "    g_sample = true;" << Qt::endl;
        m_stream << "    for (int s = 0; s < " << Simulation::kMaxSettleIterations << "; s++) { computeLogic(); }" << Qt::endl;
        m_stream << "    commitFlipFlops();" << Qt::endl;
        m_stream << "    g_sample = false;" << Qt::endl;
        m_stream << "    for (int s = 0; s < " << Simulation::kMaxSettleIterations << "; s++) { computeLogic(); }" << Qt::endl;
    } else {
        m_stream << "    computeLogic();" << Qt::endl;
    }
}

void ArduinoCodeGen::declareAuxVariables()
{
    m_stream << "/* ====== Aux. Variables ====== */" << Qt::endl;

    for (const auto &pin : std::as_const(m_inputMap)) {
        const QString valVarName = pin.m_varName + "_val";
        if (!m_declaredVariables.contains(valVarName)) {
            m_stream << "bool " << valVarName << " = LOW;" << Qt::endl;
            m_declaredVariables.append(valVarName);
        }
    }

    declareAuxVariablesRec(m_elements, false, {});
    m_stream << Qt::endl;
}

void ArduinoCodeGen::emitFlipFlopBlock(GraphicElement *elm, const QString &typeName, const QString &firstOut,
                                       const QString &secondOut, int clockInputIndex, int presetInputIndex,
                                       int clearInputIndex, const std::function<void()> &edgeLogic,
                                       const std::function<void()> &stateEpilogue)
{
    QString clk = otherPortName(elm->inputPort(clockInputIndex));
    QString inclk = firstOut + "_inclk";
    QString firstOutNext = firstOut + "_next";
    QString secondOutNext = secondOut + "_next";

    m_stream << QString("    //%1 FlipFlop").arg(typeName) << Qt::endl;
    // Sample only during the settle phase (g_sample); the post-commit re-settle
    // re-evaluates combinational logic without re-clocking. Outputs are staged to
    // <out>_next and published together by commitFlipFlops() — non-blocking
    // semantics so gated clocks read pre-edge state, matching the engine.
    m_stream << QString("    if (g_sample) {") << Qt::endl;
    m_stream << QString("    if (%1 && !%2) { ").arg(clk, inclk) << Qt::endl;

    // Type-specific edge behavior (writes the _next staging variables).
    edgeLogic();

    m_stream << QString("    }") << Qt::endl;

    // Preset/Clear logic (common to all edge-triggered flip-flops), staged.
    QString prst = otherPortName(elm->inputPort(presetInputIndex));
    QString clr = otherPortName(elm->inputPort(clearInputIndex));
    m_stream << QString("    if (!%1 || !%2) { ").arg(prst, clr) << Qt::endl;
    m_stream << QString("        %1 = !%2; //Preset").arg(firstOutNext, prst) << Qt::endl;
    m_stream << QString("        %1 = !%2; //Clear").arg(secondOutNext, clr) << Qt::endl;
    m_stream << QString("    }") << Qt::endl;

    // Clock-level update for edge detection.
    m_stream << "    " << inclk << " = " << clk << ";" << Qt::endl;

    // Per-sample epilogue (e.g. the one-tick data latch for D/T flip-flops).
    if (stateEpilogue) {
        stateEpilogue();
    }

    m_stream << QString("    }") << Qt::endl; // end if (g_sample)
    m_stream << QString("    //End of %1 FlipFlop").arg(typeName) << Qt::endl;
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
            auto *ic = qobject_cast<IC *>(elm);
            if (!ic) continue;

            m_stream << "    // IC: " << CodeGenUtils::sanitizeComment(ic->label()) << Qt::endl;

            for (int i = 0; i < ic->inputSize(); ++i) {
                Port *externalPort = ic->inputPort(i);
                Port *internalPort = ic->internalInputs().at(i);
                const QString externalValue = otherPortName(externalPort);
                const QString internalVar = m_varMap.value(internalPort);
                m_stream << "    " << internalVar << " = " << externalValue << ";" << Qt::endl;
            }

            if (!ic->internalElements().isEmpty()) {
                IC *previousIC = m_currentIC;
                m_currentIC = ic;
                // Emit boundary input nodes before the rest so downstream gates
                // never read a stale input on the first settle pass. The
                // topological sort can place a boundary input after a consumer
                // when the IC has feedback (its priorities take the legacy
                // path), and a stale read transiently flips bistable latches —
                // wrong, and divergent from the simulation, which excludes
                // boundary inputs from its settle (IC::initializeSimulation).
                QSet<GraphicElement *> boundaryInputs;
                for (auto *port : ic->internalInputs()) {
                    if (auto *boundaryElement = port->graphicElement()) {
                        boundaryInputs.insert(boundaryElement);
                    }
                }
                auto sortedInternal = Scene::sortByTopology(ic->internalElements());
                std::stable_partition(sortedInternal.begin(), sortedInternal.end(),
                    [&boundaryInputs](GraphicElement *e) { return boundaryInputs.contains(e); });
                assignVariablesRec(sortedInternal);
                m_currentIC = previousIC;
            }

            for (int i = 0; i < ic->outputSize(); ++i) {
                Port *externalPort = ic->outputPort(i);
                Port *internalPort = ic->internalOutputs().at(i);
                const QString internalValue = m_varMap.value(internalPort);
                const QString externalVar = m_varMap.value(externalPort);
                m_stream << "    " << externalVar << " = " << internalValue << ";" << Qt::endl;
            }

            m_stream << "    // End IC: " << CodeGenUtils::sanitizeComment(ic->label()) << Qt::endl;
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
        case ElementType::DFlipFlop:  emitDFlipFlop(elm, firstOut);  break;
        case ElementType::DLatch:     emitDLatch(elm, firstOut);     break;
        case ElementType::JKFlipFlop: emitJKFlipFlop(elm, firstOut); break;
        case ElementType::SRFlipFlop: emitSRFlipFlop(elm, firstOut); break;
        case ElementType::TFlipFlop:  emitTFlipFlop(elm, firstOut);  break;
        case ElementType::SRLatch:    emitSRLatch(elm, firstOut);    break;
        case ElementType::Mux:        emitMux(elm);                  break;
        case ElementType::Demux:      emitDemux(elm);                break;
        case ElementType::TruthTable: emitTruthTable(elm);           break;
        case ElementType::And:
        case ElementType::Or:
        case ElementType::Nand:
        case ElementType::Nor:
        case ElementType::Xor:
        case ElementType::Xnor:
        case ElementType::Not:
        case ElementType::Node: assignLogicOperator(elm); break;
        case ElementType::AudioBox:
        case ElementType::Buzzer:
        case ElementType::Clock:
        case ElementType::Display14:
        case ElementType::Display16:
        case ElementType::Display7:
        case ElementType::IC:
        case ElementType::InputButton:
        case ElementType::InputGnd:
        case ElementType::InputRotary:
        case ElementType::InputSwitch:
        case ElementType::InputVcc:
        case ElementType::JKLatch:
        case ElementType::Led:
        case ElementType::Line:
        case ElementType::Text:
        case ElementType::Unknown:
            throw PANDACEPTION("Element type not supported: %1", elm->objectName());
        }
    }
}

void ArduinoCodeGen::emitDFlipFlop(GraphicElement *elm, const QString &firstOut)
{
    auto *outputPort1 = elm->outputPort(1);
    if (!outputPort1) return;
    QString secondOut = m_varMap.value(outputPort1);
    QString firstOutNext = firstOut + "_next";
    QString secondOutNext = secondOut + "_next";
    QString data = otherPortName(elm->inputPort(0));
    QString last = firstOut + "_last";
    emitFlipFlopBlock(elm, "D", firstOut, secondOut, /*clk*/1, /*preset*/2, /*clear*/3,
        [this, &last, &firstOutNext, &secondOutNext]() {
            m_stream << QString("        %1 = %2;").arg(firstOutNext, last) << Qt::endl;
            m_stream << QString("        %1 = !%2;").arg(secondOutNext, last) << Qt::endl;
        },
        [this, &last, &data]() {
            m_stream << "        " << last << " = " << data << ";" << Qt::endl;
        });
}

void ArduinoCodeGen::emitDLatch(GraphicElement *elm, const QString &firstOut)
{
    auto *outputPort1 = elm->outputPort(1);
    if (!outputPort1) return;
    QString secondOut = m_varMap.value(outputPort1);
    QString firstOutNext = firstOut + "_next";
    QString secondOutNext = secondOut + "_next";
    QString data = otherPortName(elm->inputPort(0));
    QString clk = otherPortName(elm->inputPort(1));
    m_stream << QString("    //D Latch") << Qt::endl;
    m_stream << QString("    if (g_sample) {") << Qt::endl;
    m_stream << QString("    if (%1) { ").arg(clk) << Qt::endl;
    m_stream << QString("        %1 = %2;").arg(firstOutNext, data) << Qt::endl;
    m_stream << QString("        %1 = !%2;").arg(secondOutNext, data) << Qt::endl;
    m_stream << QString("    }") << Qt::endl;
    m_stream << QString("    }") << Qt::endl;
    m_stream << QString("    //End of D Latch") << Qt::endl;
}

void ArduinoCodeGen::emitJKFlipFlop(GraphicElement *elm, const QString &firstOut)
{
    auto *outputPort1 = elm->outputPort(1);
    if (!outputPort1) return;
    QString secondOut = m_varMap.value(outputPort1);
    QString firstOutNext = firstOut + "_next";
    QString secondOutNext = secondOut + "_next";
    QString j = otherPortName(elm->inputPort(0));
    QString k = otherPortName(elm->inputPort(2));
    emitFlipFlopBlock(elm, "JK", firstOut, secondOut, /*clk*/1, /*preset*/3, /*clear*/4,
        [this, &j, &k, &firstOut, &secondOut, &firstOutNext, &secondOutNext]() {
            m_stream << QString("        if (%1 && %2) { ").arg(j, k) << Qt::endl;
            m_stream << QString("            bool aux = %1;").arg(firstOut) << Qt::endl;
            m_stream << QString("            %1 = %2;").arg(firstOutNext, secondOut) << Qt::endl;
            m_stream << QString("            %1 = aux;").arg(secondOutNext) << Qt::endl;
            m_stream << QString("        } else if (%1) {").arg(j) << Qt::endl;
            m_stream << QString("            %1 = 1;").arg(firstOutNext) << Qt::endl;
            m_stream << QString("            %1 = 0;").arg(secondOutNext) << Qt::endl;
            m_stream << QString("        } else if (%1) {").arg(k) << Qt::endl;
            m_stream << QString("            %1 = 0;").arg(firstOutNext) << Qt::endl;
            m_stream << QString("            %1 = 1;").arg(secondOutNext) << Qt::endl;
            m_stream << QString("        }") << Qt::endl;
        });
}

void ArduinoCodeGen::emitSRFlipFlop(GraphicElement *elm, const QString &firstOut)
{
    auto *outputPort1 = elm->outputPort(1);
    if (!outputPort1) return;
    QString secondOut = m_varMap.value(outputPort1);
    QString firstOutNext = firstOut + "_next";
    QString secondOutNext = secondOut + "_next";
    QString s = otherPortName(elm->inputPort(0));
    QString r = otherPortName(elm->inputPort(2));
    emitFlipFlopBlock(elm, "SR", firstOut, secondOut, /*clk*/1, /*preset*/3, /*clear*/4,
        [this, &s, &r, &firstOutNext, &secondOutNext]() {
            m_stream << QString("        if (%1 && %2) { ").arg(s, r) << Qt::endl;
            m_stream << QString("            %1 = 1;").arg(firstOutNext) << Qt::endl;
            m_stream << QString("            %1 = 1;").arg(secondOutNext) << Qt::endl;
            m_stream << QString("        } else if (%1 != %2) {").arg(s, r) << Qt::endl;
            m_stream << QString("            %1 = %2;").arg(firstOutNext, s) << Qt::endl;
            m_stream << QString("            %1 = %2;").arg(secondOutNext, r) << Qt::endl;
            m_stream << QString("        }") << Qt::endl;
        });
}

void ArduinoCodeGen::emitTFlipFlop(GraphicElement *elm, const QString &firstOut)
{
    auto *outputPort1 = elm->outputPort(1);
    if (!outputPort1) return;
    QString secondOut = m_varMap.value(outputPort1);
    QString firstOutNext = firstOut + "_next";
    QString secondOutNext = secondOut + "_next";
    QString t = otherPortName(elm->inputPort(0));
    emitFlipFlopBlock(elm, "T", firstOut, secondOut, /*clk*/1, /*preset*/2, /*clear*/3,
        [this, &t, &firstOut, &firstOutNext, &secondOutNext]() {
            // Toggle reading the pre-edge Q: Q' = !Q, ~Q' = Q (old).
            m_stream << QString("        if (%1) { ").arg(t) << Qt::endl;
            m_stream << QString("            %1 = !%2;").arg(firstOutNext, firstOut) << Qt::endl;
            m_stream << QString("            %1 = %2;").arg(secondOutNext, firstOut) << Qt::endl;
            m_stream << QString("        }") << Qt::endl;
        });
}

void ArduinoCodeGen::emitSRLatch(GraphicElement *elm, const QString &firstOut)
{
    auto *outputPort1 = elm->outputPort(1);
    if (!outputPort1) return;
    QString secondOut = m_varMap.value(outputPort1);
    QString firstOutNext = firstOut + "_next";
    QString secondOutNext = secondOut + "_next";
    QString s = otherPortName(elm->inputPort(0));
    QString r = otherPortName(elm->inputPort(1));
    m_stream << QString("    //SR Latch") << Qt::endl;
    m_stream << QString("    if (g_sample) {") << Qt::endl;
    m_stream << QString("    if (%1 && %2) { ").arg(s, r) << Qt::endl;
    m_stream << QString("        %1 = LOW;").arg(firstOutNext) << Qt::endl;
    m_stream << QString("        %1 = LOW;").arg(secondOutNext) << Qt::endl;
    m_stream << QString("    } else if (%1) { ").arg(s) << Qt::endl;
    m_stream << QString("        %1 = HIGH;").arg(firstOutNext) << Qt::endl;
    m_stream << QString("        %1 = LOW;").arg(secondOutNext) << Qt::endl;
    m_stream << QString("    } else if (%1) { ").arg(r) << Qt::endl;
    m_stream << QString("        %1 = LOW;").arg(firstOutNext) << Qt::endl;
    m_stream << QString("        %1 = HIGH;").arg(secondOutNext) << Qt::endl;
    m_stream << QString("    }") << Qt::endl;
    m_stream << QString("    }") << Qt::endl;
    m_stream << QString("    //End of SR Latch") << Qt::endl;
}

void ArduinoCodeGen::emitMux(GraphicElement *elm)
{
    const int totalInputs = elm->inputSize();
    int numSelectLines = 1;
    while (numSelectLines < 16 && (1 << numSelectLines) + numSelectLines < totalInputs) {
        numSelectLines++;
    }
    const int numDataInputs = totalInputs - numSelectLines;
    const QString output = m_varMap.value(elm->outputPort(0));
    const QString selectValue = buildSelectExpression(elm, numDataInputs, numSelectLines);

    m_stream << QString("    //Multiplexer") << Qt::endl;
    for (int i = 0; i < numDataInputs; ++i) {
        if (i == 0) {
            m_stream << QString("    if ((%1) == %2) {").arg(selectValue).arg(i) << Qt::endl;
        } else {
            m_stream << QString("    } else if ((%1) == %2) {").arg(selectValue).arg(i) << Qt::endl;
        }
        m_stream << QString("        %1 = %2;").arg(output, otherPortName(elm->inputPort(i))) << Qt::endl;
    }
    m_stream << QString("    } else {") << Qt::endl;
    m_stream << QString("        %1 = LOW;").arg(output) << Qt::endl;
    m_stream << QString("    }") << Qt::endl;
    m_stream << QString("    //End of Multiplexer") << Qt::endl;
}

void ArduinoCodeGen::emitDemux(GraphicElement *elm)
{
    const int numOutputs = elm->outputSize();
    int numSelectLines = 1;
    while (numSelectLines < 16 && (1 << numSelectLines) < numOutputs) {
        numSelectLines++;
    }
    const QString dataInput = otherPortName(elm->inputPort(0));
    const QString selectValue = buildSelectExpression(elm, 1, numSelectLines);

    m_stream << QString("    //Demultiplexer") << Qt::endl;
    for (int i = 0; i < numOutputs; ++i) {
        m_stream << QString("    %1 = LOW;").arg(m_varMap.value(elm->outputPort(i))) << Qt::endl;
    }
    for (int i = 0; i < numOutputs; ++i) {
        if (i == 0) {
            m_stream << QString("    if ((%1) == %2) {").arg(selectValue).arg(i) << Qt::endl;
        } else {
            m_stream << QString("    } else if ((%1) == %2) {").arg(selectValue).arg(i) << Qt::endl;
        }
        m_stream << QString("        %1 = %2;").arg(m_varMap.value(elm->outputPort(i)), dataInput) << Qt::endl;
    }
    m_stream << QString("    }") << Qt::endl;
    m_stream << QString("    //End of Demultiplexer") << Qt::endl;
}

void ArduinoCodeGen::emitTruthTable(GraphicElement *elm)
{
    auto *ttGraphic = dynamic_cast<TruthTable *>(elm);
    if (!ttGraphic) return;

    const QBitArray propositions = ttGraphic->key();
    const int nInputs = elm->inputSize();
    const int rows = 1 << nInputs;

    QStringList inputSignalNames;
    for (int i = 0; i < nInputs; ++i) {
        QString signalName = otherPortName(elm->inputPort(i));
        if (signalName == "LOW") {
            signalName = "0";
        } else if (signalName == "HIGH") {
            signalName = "1";
        } else if (signalName.isEmpty()) {
            m_stream << "// WARNING: Input " << i << " of TruthTable '" << elm->objectName() << "' appears disconnected. Assuming LOW." << Qt::endl;
            signalName = "0";
        }
        inputSignalNames << signalName;
    }

    QString indexCalculation;
    for (int i = 0; i < nInputs; ++i) {
        if (i == 0) {
            indexCalculation = inputSignalNames[i];
        } else {
            indexCalculation = inputSignalNames[i] + " + (" + indexCalculation + " << 1)";
        }
    }

    // One if/else chain per output (F19): output k reads key bits 256*k + row.
    // Emitting per-output keeps the single-output text byte-identical to the
    // historical form.
    for (int out = 0; out < elm->outputSize(); ++out) {
        const QString outputVarName = m_varMap.value(elm->outputPort(out));
        if (outputVarName.isEmpty()) {
            if (out == 0) {
                throw PANDACEPTION("Output variable not mapped for TruthTable: %1", elm->objectName());
            }
            m_stream << "// TruthTable '" << elm->objectName() << "' output " << out << " is disconnected — no code emitted." << Qt::endl;
            continue;
        }

        m_stream << QString("    //TruthTable") << Qt::endl;
        for (int i = 0; i < rows; ++i) {
            if (i == 0) {
                m_stream << QString("    if ((%1) == %2) {").arg(indexCalculation).arg(i) << Qt::endl;
            } else {
                m_stream << QString("    } else if ((%1) == %2) {").arg(indexCalculation).arg(i) << Qt::endl;
            }
            m_stream << QString("        %1 = %2;").arg(outputVarName, propositions.testBit(256 * out + i) ? "HIGH" : "LOW") << Qt::endl;
        }
        m_stream << QString("    } else {") << Qt::endl;
        m_stream << QString("        %1 = LOW;").arg(outputVarName) << Qt::endl;
        m_stream << QString("    }") << Qt::endl;
        m_stream << QString("    //End TruthTable") << Qt::endl;
    }
}

QVector<ArduinoBoardConfig> ArduinoCodeGen::availableBoards() const
{
    static const QVector<ArduinoBoardConfig> boards = []() {
        QVector<ArduinoBoardConfig> b;

        b.append({
            "Arduino UNO R3/R4",
            {"A0", "A1", "A2", "A3", "A4", "A5", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13"},
            "Standard Arduino board with 20 I/O pins"
        });

        b.append({
            "Arduino Nano",
            {"A0", "A1", "A2", "A3", "A4", "A5", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13"},
            "Compact Arduino board with 20 I/O pins"
        });

        QStringList megaPins;
        for (int i = 0; i <= 15; ++i) {
            megaPins.append(QString("A%1").arg(i));
        }
        for (int i = 2; i <= 53; ++i) {
            megaPins.append(QString::number(i));
        }
        b.append({
            "Arduino Mega 2560",
            megaPins,
            "High I/O count Arduino board with 70 I/O pins"
        });

        b.append({
            "ESP32",
            {"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10", "A11", "A12", "A13", "A14", "A15", "A16", "A17",
             "2", "4", "5", "12", "13", "14", "15", "16", "17", "18", "19", "21", "22", "23", "25", "26", "27", "32", "33", "34"},
            "WiFi/Bluetooth enabled board with 36 I/O pins"
        });

        return b;
    }();
    return boards;
}

ArduinoBoardConfig ArduinoCodeGen::selectBoard(int requiredPins)
{
    const auto boards = availableBoards();
    for (const auto &board : boards) {
        if (board.maxPins() >= requiredPins) {
            return board;
        }
    }
    return boards.last();
}

QString ArduinoCodeGen::buildSelectExpression(GraphicElement *elm, int startIndex, int numSelectLines)
{
    QString selectValue;
    for (int i = numSelectLines - 1; i >= 0; --i) {
        QString selectBit = otherPortName(elm->inputPort(startIndex + i));
        if (selectBit == "HIGH") selectBit = "1";
        else if (selectBit == "LOW") selectBit = "0";
        if (i == numSelectLines - 1) {
            selectValue = selectBit;
        } else {
            selectValue = selectBit + " + (" + selectValue + " << 1)";
        }
    }
    return selectValue;
}

void ArduinoCodeGen::assignLogicOperator(GraphicElement *elm)
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
        if (elm->outputs().size() == 1 && elm->inputs().size() == 1) {
            auto *outputPort = elm->outputPort(0);
            auto *inputPort = elm->inputPort(0);
            if (outputPort && inputPort) {
                QString varName = m_varMap.value(outputPort);
                QString inputValue = otherPortName(inputPort);
                if (inputValue.isEmpty() && m_currentIC) {
                    for (int i = 0; i < m_currentIC->internalInputs().size(); ++i) {
                        if (m_currentIC->internalInputs().at(i) == inputPort) {
                            inputValue = m_varMap.value(inputPort);
                            break;
                        }
                    }
                }
                m_stream << "    " << varName << " = " << inputValue << ";" << Qt::endl;
            }
        }
        return;
    }
    case ElementType::AudioBox:
    case ElementType::Buzzer:
    case ElementType::Clock:
    case ElementType::DFlipFlop:
    case ElementType::DLatch:
    case ElementType::Demux:
    case ElementType::Display14:
    case ElementType::Display16:
    case ElementType::Display7:
    case ElementType::IC:
    case ElementType::InputButton:
    case ElementType::InputGnd:
    case ElementType::InputRotary:
    case ElementType::InputSwitch:
    case ElementType::InputVcc:
    case ElementType::JKFlipFlop:
    case ElementType::JKLatch:
    case ElementType::Led:
    case ElementType::Line:
    case ElementType::Mux:
    case ElementType::SRFlipFlop:
    case ElementType::SRLatch:
    case ElementType::TFlipFlop:
    case ElementType::Text:
    case ElementType::TruthTable:
    case ElementType::Unknown:
        break;
    }
    if (elm->outputs().size() == 1) {
        auto *outputPort = elm->outputPort();
        if (!outputPort) return;
        QString varName = m_varMap.value(outputPort);
        Port *inPort = elm->inputPort();
        if (!inPort) return;

        // Build the inner expression first
        QString innerExpr;
        innerExpr = otherPortName(inPort);
        for (int i = 1; i < elm->inputs().size(); ++i) {
            inPort = elm->inputs().at(i);
            innerExpr += " " + logicOperator + " ";
            innerExpr += otherPortName(inPort);
        }

        QString finalExpr = innerExpr;
        if (negate) {
            if (parentheses) {
                finalExpr = "!(" + innerExpr + ")";
            } else {
                finalExpr = "!" + innerExpr;
            }
        }

        m_stream << "    " << varName << " = " << finalExpr << ";" << Qt::endl;
    } else {
        qWarning() << "assignLogicOperator: element" << elm->objectName() << "has" << elm->outputs().size() << "outputs (expected 1) — skipping";
    }
}

void ArduinoCodeGen::emitComputeLogicFunction()
{
    m_stream << "void computeLogic() {" << Qt::endl;
    m_stream << "    // Assigning aux variables. //" << Qt::endl;
    assignVariablesRec(m_elements);
    m_stream << "}" << Qt::endl
             << Qt::endl;
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
    m_stream << Qt::endl;
    emitTickDriver();
    m_stream << Qt::endl;
    m_stream << "    // Writing output data. //" << Qt::endl;
    for (const auto &pin : std::as_const(m_outputMap)) {
        if (pin.m_elm->elementType() == ElementType::Buzzer) {
            auto *buzzer = qobject_cast<Buzzer *>(pin.m_elm);
            if (!buzzer) continue;
            const QString inputSignal = otherPortName(buzzer->inputPort(0));
            const int frequency = static_cast<int>(buzzer->frequency());
            m_stream << QString("    if (%1) {").arg(inputSignal) << Qt::endl;
            m_stream << QString("        tone(%1, %2);").arg(pin.m_varName).arg(frequency) << Qt::endl;
            m_stream << QString("    } else {") << Qt::endl;
            m_stream << QString("        noTone(%1);").arg(pin.m_varName) << Qt::endl;
            m_stream << QString("    }") << Qt::endl;
            continue;
        }
        QString varName = otherPortName(pin.m_port);
        if (varName.isEmpty()) {
            varName = highLow(pin.m_port->defaultValue());
        }
        m_stream << QString("    digitalWrite(%1, %2);").arg(pin.m_varName, varName) << Qt::endl;
    }
    m_stream << "}" << Qt::endl;
}

void ArduinoCodeGen::generateTestbench(const QString &tbFileName, const QVector<TestVector> &vectors)
{
    QFile tbFile(tbFileName);
    if (!tbFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw PANDACEPTION("Could not open testbench file: %1", tbFileName);
    }

    QIODevice *savedDevice = m_stream.device();
    m_stream.setDevice(&tbFile);
    m_hasSequential = hasNativeMemory(m_elements);

    try {
        m_stream << "// ============================================================ //" << Qt::endl;
        m_stream << "// ====== Testbench generated automatically by wiRedPanda ====== //" << Qt::endl;
        m_stream << "// ============================================================ //" << Qt::endl;
        m_stream << Qt::endl;
        m_stream << "#include <avr/sleep.h>" << Qt::endl;
        m_stream << "#include <elapsedMillis.h>" << Qt::endl;
        m_stream << Qt::endl;

        // Input pin stubs (same variable names as production sketch; not used for I/O)
        m_stream << "/* ========= Inputs ========== */" << Qt::endl;
        for (const auto &pin : std::as_const(m_inputMap)) {
            m_stream << "const int " << pin.m_varName << " = 0;" << Qt::endl;
        }
        m_stream << Qt::endl;

        // Output pin stubs
        m_stream << "/* ========= Outputs ========== */" << Qt::endl;
        for (const auto &pin : std::as_const(m_outputMap)) {
            m_stream << "const int " << pin.m_varName << " = 0;" << Qt::endl;
        }
        m_stream << Qt::endl;

        // Aux variable declarations (same as production sketch)
        m_stream << "/* ====== Aux. Variables ====== */" << Qt::endl;
        for (const auto &varName : std::as_const(m_declaredVariables)) {
            m_stream << "bool " << varName << " = LOW;" << Qt::endl;
        }
        // Extra state variables for clocks and flip-flops, including those nested
        // inside ICs — computeLogic() recurses into them, so their _inclk/_last
        // state must be declared at every depth (not just top level).
        declareSequentialStateRec(m_elements, /*topLevel*/ true);
        if (m_hasSequential) {
            m_stream << "bool g_sample = true;" << Qt::endl;
        }
        m_stream << Qt::endl;

        // computeLogic() function — identical to production sketch
        emitComputeLogicFunction();
        if (m_hasSequential) {
            emitCommitFlipFlops();
        }

        // Build the list of output variable names (what feeds each output pin)
        QStringList outputVarNames;
        for (const auto &pin : std::as_const(m_outputMap)) {
            QString varName = otherPortName(pin.m_port);
            if (varName.isEmpty()) {
                varName = highLow(pin.m_port->defaultValue());
            }
            outputVarNames.append(varName);
        }

        // Test vector table
        const int numInputs = static_cast<int>(m_inputMap.size());
        const int numOutputs = static_cast<int>(m_outputMap.size());
        const int numTests = static_cast<int>(vectors.size());

        m_stream << "/* ====== Test Vectors ====== */" << Qt::endl;
        m_stream << "struct TestVec {" << Qt::endl;
        m_stream << "    bool in[" << (std::max)(1, numInputs) << "];" << Qt::endl;
        m_stream << "    bool out[" << (std::max)(1, numOutputs) << "];" << Qt::endl;
        m_stream << "};" << Qt::endl;
        m_stream << Qt::endl;
        m_stream << "const int NUM_TESTS = " << numTests << ";" << Qt::endl;
        m_stream << "const TestVec VECTORS[" << (std::max)(1, numTests) << "] = {" << Qt::endl;
        for (const auto &v : vectors) {
            m_stream << "    {{";
            for (int j = 0; j < v.inputs.size(); ++j) {
                if (j > 0) m_stream << ", ";
                m_stream << (v.inputs[j] ? "HIGH" : "LOW");
            }
            m_stream << "}, {";
            for (int k = 0; k < v.outputs.size(); ++k) {
                if (k > 0) m_stream << ", ";
                m_stream << (v.outputs[k] ? "HIGH" : "LOW");
            }
            m_stream << "}}," << Qt::endl;
        }
        m_stream << "};" << Qt::endl;
        m_stream << Qt::endl;

        // setup() — runs all test vectors, reports via Serial
        m_stream << "void setup() {" << Qt::endl;
        m_stream << "    Serial.begin(9600);" << Qt::endl;
        m_stream << "    bool allPassed = true;" << Qt::endl;
        m_stream << "    for (int t = 0; t < NUM_TESTS; t++) {" << Qt::endl;
        for (int j = 0; j < numInputs; ++j) {
            m_stream << "        " << m_inputMap.at(j).m_varName << "_val = VECTORS[t].in[" << j << "];" << Qt::endl;
        }
        // One simulation tick. For sequential circuits this mirrors the engine's
        // non-blocking commit: settle while flip-flops sample into _next (reading
        // pre-edge state), commit all at once, then re-settle so combinational
        // outputs reflect the new state. Combinational/gate-built circuits just
        // settle to a fixed point (same bound as Simulation::iterativeSettle).
        if (m_hasSequential) {
            m_stream << "        g_sample = true;" << Qt::endl;
            m_stream << "        for (int s = 0; s < " << Simulation::kMaxSettleIterations
                     << "; s++) { computeLogic(); }" << Qt::endl;
            m_stream << "        commitFlipFlops();" << Qt::endl;
            m_stream << "        g_sample = false;" << Qt::endl;
            m_stream << "        for (int s = 0; s < " << Simulation::kMaxSettleIterations
                     << "; s++) { computeLogic(); }" << Qt::endl;
        } else {
            m_stream << "        for (int s = 0; s < " << Simulation::kMaxSettleIterations
                     << "; s++) { computeLogic(); }" << Qt::endl;
        }
        m_stream << "        bool pass = true;" << Qt::endl;
        for (int k = 0; k < numOutputs; ++k) {
            m_stream << "        pass = pass && (" << outputVarNames.at(k) << " == VECTORS[t].out[" << k << "]);" << Qt::endl;
        }
        m_stream << "        if (!pass) {" << Qt::endl;
        m_stream << "            allPassed = false;" << Qt::endl;
        m_stream << "            Serial.print(\"FAIL vector \");" << Qt::endl;
        m_stream << "            Serial.println(t);" << Qt::endl;
        m_stream << "        }" << Qt::endl;
        m_stream << "    }" << Qt::endl;
        m_stream << "    Serial.println(allPassed ? \"ALL PASS\" : \"SOME FAILED\");" << Qt::endl;
        m_stream << "    Serial.flush();" << Qt::endl;
        m_stream << "    set_sleep_mode(SLEEP_MODE_PWR_DOWN);" << Qt::endl;
        m_stream << "    sleep_mode();" << Qt::endl;
        m_stream << "}" << Qt::endl;
        m_stream << Qt::endl;

        m_stream << "void loop() {}" << Qt::endl;
    } catch (...) {
        m_stream.setDevice(savedDevice);
        tbFile.close();
        throw;
    }

    m_stream.setDevice(savedDevice);
    tbFile.close();
}
