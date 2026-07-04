// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/CodeGen/SystemVerilogCodeGen.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>

#include "App/CodeGen/CodeGenUtils.h"
#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/IC.h"
#include "App/Scene/Scene.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

/// Returns a unique key identifying an IC for SystemVerilog code generation.
/// Embedded ICs use "embedded:blobName"; file-backed ICs use their canonical file path.
static QString icModuleKey(const IC *ic)
{
    if (ic->isEmbedded()) {
        return QStringLiteral("embedded:") + ic->blobName();
    }
    QString key = QFileInfo(ic->file()).canonicalFilePath();
    return key.isEmpty() ? ic->file() : key;
}

SystemVerilogCodeGen::SystemVerilogCodeGen(const QString &fileName, const QVector<GraphicElement *> &elements)
    : m_file(fileName)
    , m_elements(elements)
{
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        // Same contract as ArduinoCodeGen: a silent return would leave the
        // stream device-less, generate() would write into the void, and the
        // UI would report success with no file on disk.
        throw PANDACEPTION("Could not open file for writing: %1", fileName);
    }
    m_stream.setDevice(&m_file);

    // [QUALITY-10] Fixed: sw2 was duplicated as sw1 in original code
    m_availablePins = QStringList{
        "sw0",
        "sw1",
        "sw2",
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

QString SystemVerilogCodeGen::highLow(const Status val)
{
    return (val == Status::Active) ? "1'b1" : "1'b0";
}

// [QUALITY-14] Ensures result is a valid SystemVerilog identifier (starts with letter/underscore, non-empty)
QString SystemVerilogCodeGen::removeForbiddenChars(const QString &input)
{
    return CodeGenUtils::removeForbiddenChars(input);
}

// [BUG-1] Check whether a string is a simple SystemVerilog identifier (no operators/expressions)
bool SystemVerilogCodeGen::isSimpleIdentifier(const QString &expr)
{
    if (expr.isEmpty()) return false;
    static QRegularExpression re("^[a-zA-Z_][a-zA-Z0-9_]*$");
    return re.match(expr).hasMatch();
}

// [BUG-1] Ensure a signal is usable in a sensitivity list.
// If the expression is complex (not a simple identifier), creates an intermediate wire.
QString SystemVerilogCodeGen::ensureSimpleSignal(const QString &expr)
{
    if (expr.isEmpty() || expr == "1'b0" || expr == "1'b1") return expr;
    if (isSimpleIdentifier(expr)) return expr;

    // Create an intermediate wire for the complex expression
    QString wireName = QString("aux_async_%1").arg(m_globalCounter++);
    m_stream << "    wire " << wireName << " = " << expr << ";" << Qt::endl;
    return wireName;
}

QString SystemVerilogCodeGen::otherPortName(Port *port)
{
    QSet<Port *> visited;
    return otherPortNameImpl(port, visited);
}

QString SystemVerilogCodeGen::otherPortNameImpl(Port *port, QSet<Port *> &visited)
{
    if (!port) return "1'b0";

    // Cycle detection: if we've already visited this port, don't inline
    if (visited.contains(port)) {
        QString mapped = m_varMap.value(port);
        return mapped.isEmpty() ? "1'b0" : mapped;
    }

    if (port->connections().isEmpty()) {
        QString mapped = m_varMap.value(port);
        if (!mapped.isEmpty()) return mapped;
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

    auto *otherPort = port->connections().constFirst()->otherPort(port);
    if (!otherPort) return highLow(port->defaultValue());

    // Cycle detection: if we've already visited the connected port, don't inline
    if (visited.contains(otherPort)) {
        QString mapped = m_varMap.value(otherPort);
        return mapped.isEmpty() ? "1'b0" : mapped;
    }

    // Mark this port as visited to detect cycles
    visited.insert(port);

    auto *elm = otherPort->graphicElement();
    if (!elm) {
        QString mapped = m_varMap.value(otherPort);
        return mapped.isEmpty() ? highLow(port->defaultValue()) : mapped;
    }

    // Check m_varMap first — if a wire/variable was declared for this port, use it
    // directly instead of inlining. This prevents fan-out collision (multiple gates
    // sharing an upstream output) and enables IC-internal gate wire references.
    QString mapped = m_varMap.value(otherPort);
    if (!mapped.isEmpty()) {
        return mapped;
    }

    if (elm->elementType() == ElementType::And ||
        elm->elementType() == ElementType::Or ||
        elm->elementType() == ElementType::Nand ||
        elm->elementType() == ElementType::Nor ||
        elm->elementType() == ElementType::Xor ||
        elm->elementType() == ElementType::Xnor ||
        elm->elementType() == ElementType::Not ||
        elm->elementType() == ElementType::Node) {

        return generateLogicExpressionImpl(elm, visited);
    }

    return "1'b0";
}

// SystemVerilog reserved words that cannot be used as identifiers.
static bool isSystemVerilogReserved(const QString &name)
{
    static const QSet<QString> reserved = {
        "always", "and", "assign", "automatic", "begin", "buf", "bufif0", "bufif1",
        "case", "casex", "casez", "cell", "cmos", "config", "deassign", "default",
        "defparam", "design", "disable", "edge", "else", "end", "endcase",
        "endconfig", "endfunction", "endgenerate", "endmodule", "endprimitive",
        "endspecify", "endtable", "endtask", "event", "for", "force", "forever",
        "fork", "function", "generate", "genvar", "highz0", "highz1", "if",
        "ifnone", "incdir", "include", "initial", "inout", "input",
        "instance", "integer", "join", "large", "liblist", "library", "localparam",
        "macromodule", "medium", "module", "nand", "negedge", "nmos", "nor",
        "noshowcancelled", "not", "notif0", "notif1", "or", "output", "parameter",
        "pmos", "posedge", "primitive", "pull0", "pull1", "pulldown", "pullup",
        "pulsestyle_onevent", "pulsestyle_ondetect", "rcmos", "real", "realtime",
        "reg", "release", "repeat", "rnmos", "rpmos", "rtran", "rtranif0",
        "rtranif1", "scalared", "showcancelled", "signed", "small", "specify",
        "specparam", "strong0", "strong1", "supply0", "supply1", "table", "task",
        "time", "tran", "tranif0", "tranif1", "tri", "tri0", "tri1", "triand",
        "trior", "trireg", "unsigned", "use", "uwire", "vectored", "wait",
        "wand", "weak0", "weak1", "while", "wire", "wor", "xnor", "xor"
    };
    return reserved.contains(name);
}

// Recursively discover all unique IC types used in the circuit.
void SystemVerilogCodeGen::collectICTypes(const QVector<GraphicElement *> &elements)
{
    for (auto *elm : elements) {
        if (elm->elementType() != ElementType::IC) {
            continue;
        }

        auto *ic = qobject_cast<IC *>(elm);
        if (!ic) continue;

        // Use canonical file path as key for identity
        QString key = icModuleKey(ic);

        if (m_icModules.contains(key)) {
            continue;
        }

        ICModuleInfo info;
        info.sourceFile = key;
        info.prototypeIC = ic;

        // Derive module name from file basename
        QString baseName = ic->isEmbedded() ? ic->blobName() : QFileInfo(ic->file()).baseName();
        info.moduleName = removeForbiddenChars(CodeGenUtils::stripAccents(baseName));
        if (isSystemVerilogReserved(info.moduleName)) {
            info.moduleName = "m_" + info.moduleName;
        }

        // Build input port names from IC external port labels
        QSet<QString> usedNames;
        for (int i = 0; i < ic->inputSize(); ++i) {
            QString portLabel = ic->inputPort(i)->name();
            QString portName = removeForbiddenChars(CodeGenUtils::stripAccents(portLabel));
            if (portName.isEmpty() || portName == "_unnamed") {
                portName = QString("in_%1").arg(i);
            }
            // Escape SystemVerilog reserved words
            if (isSystemVerilogReserved(portName)) {
                portName = "p_" + portName;
            }
            // Deduplicate
            QString original = portName;
            int suffix = 1;
            while (usedNames.contains(portName)) {
                portName = QString("%1_%2").arg(original).arg(suffix++);
            }
            usedNames.insert(portName);
            info.inputNames.append(portName);
        }

        // Build output port names
        for (int i = 0; i < ic->outputSize(); ++i) {
            QString portLabel = ic->outputPort(i)->name();
            QString portName = removeForbiddenChars(CodeGenUtils::stripAccents(portLabel));
            if (portName.isEmpty() || portName == "_unnamed") {
                portName = QString("out_%1").arg(i);
            }
            // Escape SystemVerilog reserved words
            if (isSystemVerilogReserved(portName)) {
                portName = "p_" + portName;
            }
            QString original = portName;
            int suffix = 1;
            while (usedNames.contains(portName)) {
                portName = QString("%1_%2").arg(original).arg(suffix++);
            }
            usedNames.insert(portName);
            info.outputNames.append(portName);
        }

        m_icModules.insert(key, info);

        // Recurse into internal elements to discover nested IC types
        collectICTypes(ic->internalElements());
    }
}

// Returns the internal gate elements that sit on a combinational feedback loop
// (cross-coupled NOR/NAND latches). A plain `assign` comb loop x-locks at
// power-on in SystemVerilog and trips Verilator's UNOPTFLAT; these nodes are
// instead emitted as `reg` + `always @(*)` with a seed, so the simulator settles
// them. Detection is a reachability check over the gate graph: an element is on a
// loop iff it can reach itself following output→input edges.
QSet<GraphicElement *> SystemVerilogCodeGen::findFeedbackElements(const QVector<GraphicElement *> &elements)
{
    const QSet<GraphicElement *> elementSet(elements.cbegin(), elements.cend());

    auto isGate = [](GraphicElement *e) {
        switch (e->elementType()) {
        case ElementType::And:  case ElementType::Or:   case ElementType::Nand:
        case ElementType::Nor:  case ElementType::Xor:  case ElementType::Xnor:
        case ElementType::Not:  case ElementType::Node:
            return true;
        default:
            return false;
        }
    };

    // Gates driven by elm's outputs (restricted to this IC's internal gates).
    auto successors = [&](GraphicElement *elm) {
        QVector<GraphicElement *> succ;
        for (auto *outPort : elm->outputs()) {
            const auto conns = outPort->connections();
            for (auto *conn : conns) {
                if (!conn) continue;
                Port *other = conn->otherPort(outPort);
                if (!other) continue;
                GraphicElement *h = other->graphicElement();
                if (h && elementSet.contains(h) && isGate(h)) {
                    succ.append(h);
                }
            }
        }
        return succ;
    };

    QSet<GraphicElement *> feedback;
    for (auto *start : elements) {
        if (!isGate(start)) continue;
        // Depth-first: can `start` reach itself?
        QSet<GraphicElement *> seen;
        QVector<GraphicElement *> stack = successors(start);
        while (!stack.isEmpty()) {
            GraphicElement *n = stack.takeLast();
            if (n == start) {
                feedback.insert(start);
                break;
            }
            if (seen.contains(n)) continue;
            seen.insert(n);
            stack += successors(n);
        }
    }
    return feedback;
}

// Generate IC modules in topological order (leaves first).
void SystemVerilogCodeGen::generateICModules()
{
    if (m_icModules.isEmpty()) {
        return;
    }

    // Sort keys for deterministic module emission order.
    QStringList sortedKeys = m_icModules.keys();
    std::sort(sortedKeys.begin(), sortedKeys.end());

    bool progress = true;
    while (progress) {
        progress = false;
        for (const QString &key : std::as_const(sortedKeys)) {
            ICModuleInfo &info = m_icModules[key];
            if (info.generated) {
                continue;
            }

            // Check if all nested IC types are already generated
            bool allDepsReady = true;
            for (auto *elm : std::as_const(info.prototypeIC->internalElements())) {
                if (elm->elementType() != ElementType::IC) {
                    continue;
                }
                auto *nestedIC = qobject_cast<IC *>(elm);
                if (!nestedIC) continue;

                QString nestedKey = icModuleKey(nestedIC);
                if (!m_icModules.value(nestedKey).generated) {
                    allDepsReady = false;
                    break;
                }
            }

            if (allDepsReady) {
                generateSingleICModule(info);
                info.generated = true;
                progress = true;
            }
        }
    }
}

// Generate a complete SystemVerilog module for one IC type.
void SystemVerilogCodeGen::generateSingleICModule(ICModuleInfo &info)
{
    // Save context
    QHash<Port *, QString> savedVarMap = m_varMap;
    QHash<IC *, QString> savedInstanceNames = m_instanceNames;
    int savedCounter = m_globalCounter;

    m_varMap.clear();
    m_instanceNames.clear();
    m_globalCounter = 1;
    m_generatingICModule = true;

    IC *ic = info.prototypeIC;

    // Structural (gate-level) emission: every IC is translated from its actual
    // internal gate netlist — there is no behavioral/port-signature shortcut, so
    // the export can never drift from what the circuit does. Cross-coupled
    // feedback gates are handled by findFeedbackElements() (reg + always @(*)).

    // Emit module header
    const QString source = ic->isEmbedded() ? ic->blobName() : QFileInfo(ic->file()).fileName();
    m_stream << "// Module for " << ic->label() << " (generated from " << source << ")" << Qt::endl;
    m_stream << "module " << info.moduleName << " (" << Qt::endl;

    // Emit port list
    QStringList portDecls;
    for (int i = 0; i < info.inputNames.size(); ++i) {
        portDecls << QString("    input %1").arg(info.inputNames[i]);
    }
    for (int i = 0; i < info.outputNames.size(); ++i) {
        portDecls << QString("    output %1").arg(info.outputNames[i]);
    }
    m_stream << portDecls.join(",\n") << Qt::endl;
    m_stream << ");" << Qt::endl;

    // Map boundary input ports: for each IC input, the Node's output port
    // feeds internal logic, so map it to the module input name.
    // Also build a list of non-boundary elements for processing.
    QSet<GraphicElement *> boundaryNodes;
    for (int i = 0; i < ic->internalInputs().size(); ++i) {
        // m_internalInputs[i] is the Node's input port. Get the Node element.
        auto *nodeElm = ic->internalInputs()[i]->graphicElement();
        if (!nodeElm) {
            continue;
        }
        // The Node's output port is what internal elements connect to.
        m_varMap[nodeElm->outputPort(0)] = info.inputNames[i];
        boundaryNodes.insert(nodeElm);
    }

    // Map boundary output ports: the output Node's output port gets the output name.
    // The assign for these is emitted separately at end of module.
    for (int i = 0; i < ic->internalOutputs().size(); ++i) {
        auto *nodeElm = ic->internalOutputs()[i]->graphicElement();
        if (!nodeElm) {
            continue;
        }
        m_varMap[nodeElm->outputPort(0)] = info.outputNames[i];
        boundaryNodes.insert(nodeElm);
    }

    // Build list of internal (non-boundary) elements
    QVector<GraphicElement *> internalElements;
    for (auto *elm : std::as_const(ic->internalElements())) {
        if (!boundaryNodes.contains(elm)) {
            internalElements.append(elm);
        }
    }

    // Identify cross-coupled feedback gates so they're emitted as settling
    // `reg` + `always @(*)` instead of x-locking comb-loop `assign`s.
    m_feedbackElements = findFeedbackElements(internalElements);
    const bool hasFeedback = !m_feedbackElements.isEmpty();

    // Declare internal variables
    m_stream << Qt::endl;
    if (hasFeedback) {
        m_stream << "/* verilator lint_off UNOPTFLAT */ // intentional latch feedback" << Qt::endl;
    }
    declareAuxVariablesRec(internalElements);
    m_stream << Qt::endl;

    // Assign internal logic
    m_stream << "// Internal logic" << Qt::endl;
    assignVariablesRec(internalElements);

    // Emit output assignments: trace what drives each IC output node
    m_stream << Qt::endl;
    for (int i = 0; i < ic->internalOutputs().size(); ++i) {
        // m_internalOutputs[i] is the Node's output port. Get the Node element.
        auto *nodeElm = ic->internalOutputs()[i]->graphicElement();
        if (!nodeElm) {
            continue;
        }
        // The Node's input port receives from internal logic
        QString value = otherPortName(nodeElm->inputPort(0));
        m_stream << "assign " << info.outputNames[i] << " = " << value << ";" << Qt::endl;
    }

    if (hasFeedback) {
        m_stream << "/* verilator lint_on UNOPTFLAT */" << Qt::endl;
    }
    m_feedbackElements.clear();

    m_stream << "endmodule" << Qt::endl;
    m_stream << Qt::endl;

    // Restore context
    m_varMap = savedVarMap;
    m_instanceNames = savedInstanceNames;
    m_globalCounter = savedCounter;
    m_generatingICModule = false;
}

void SystemVerilogCodeGen::generate()
{
    m_txInputPorts = Scene::wirelessTxInputPorts(m_elements);

    m_stream << "// ==================================================================== //" << Qt::endl;
    m_stream << "// ======= This code was generated automatically by wiRedPanda ======== //" << Qt::endl;
    m_stream << "// ==================================================================== //" << Qt::endl;
    m_stream << Qt::endl
             << Qt::endl;

    // Discover and generate IC modules (bottom-up)
    collectICTypes(m_elements);

    // Resolve module name collisions: ensure no IC module name matches the top-level
    // module name, and no two IC modules share the same name.
    // Iterate in sorted key order for deterministic renaming.
    QSet<QString> usedModuleNames;
    usedModuleNames.insert(m_fileName);
    QStringList collisionKeys = m_icModules.keys();
    std::sort(collisionKeys.begin(), collisionKeys.end());
    for (const QString &key : std::as_const(collisionKeys)) {
        QString name = m_icModules[key].moduleName;
        while (usedModuleNames.contains(name)) {
            name += "_ic";
        }
        m_icModules[key].moduleName = name;
        usedModuleNames.insert(name);
    }

    generateICModules();

    // Top-level module
    m_stream << "module " << m_fileName << " (" << Qt::endl;
    // Declare input and output pins
    declareInputs();
    declareOutputs();
    declareAuxVariables();

    // Logic section
    loop();
}

// [ISSUE-9] Inputs are mapped directly by name (no redundant _val alias)
void SystemVerilogCodeGen::declareInputs()
{
    int counter = 1;

    // Count total inputs/outputs for comma placement
    int totalOutputs = 0;
    int currentOutput = 0;
    for (auto *elm : m_elements) {
        const auto type = elm->elementType();
        if (elm->elementGroup() == ElementGroup::Output) {
            totalOutputs = INT_MAX;
            break;
        } else if ((type == ElementType::InputButton) || (type == ElementType::InputSwitch) || (type == ElementType::Clock) || (type == ElementType::InputRotary)) {
            totalOutputs += static_cast<int>(elm->outputs().size());
        }
    }

    m_stream << "/* ========= Inputs ========== */" << Qt::endl;

    for (auto *elm : m_elements) {
        const auto type = elm->elementType();

        if ((type == ElementType::InputButton) || (type == ElementType::InputSwitch) || (type == ElementType::Clock) || (type == ElementType::InputRotary)) {
            QString baseName = elm->objectName() + QString::number(counter);
            const QString label = elm->label();

            if (!label.isEmpty()) {
                baseName += "_" + label;
            }

            baseName = CodeGenUtils::stripAccents(baseName);
            baseName = removeForbiddenChars(baseName);

            // One module input per output port. Button/Switch/Clock have a
            // single port and keep their unsuffixed name; a rotary (F23)
            // contributes one one-hot input per position — previously its
            // ports got undriven aux wires and the module floated.
            for (int port = 0; port < elm->outputSize(); ++port) {
                QString varName = (elm->outputSize() > 1) ? QString("%1_%2").arg(baseName).arg(port) : baseName;

                currentOutput++;
                if (currentOutput < totalOutputs) {
                    m_stream << QString("input %1,").arg(varName) << Qt::endl;
                } else {
                    m_stream << QString("input %1").arg(varName) << Qt::endl;
                }

                m_inputMap.append(MappedPinSystemVerilog(elm, "", varName, elm->outputPort(port), port));
                // [ISSUE-9] Map directly to input name, no _val indirection
                m_varMap[elm->outputPort(port)] = varName;
            }
            ++counter;
        }
    }

    m_stream << Qt::endl;
}

void SystemVerilogCodeGen::declareOutputs()
{
    int counter = 1;

    int totalOutputs = 0;
    int currentOutput = 0;
    for (auto *elm : m_elements) {
        if (elm->elementGroup() == ElementGroup::Output) {
            totalOutputs += static_cast<int>(elm->inputs().size());
        }
    }

    // [QUALITY-13] Outputs are declared as plain 'output' (wire by default).
    // This is correct because all outputs are driven by continuous 'assign' statements,
    // not by 'always' blocks. Internal reg variables handle the sequential logic.
    m_stream << "/* ========= Outputs ========== */" << Qt::endl;
    for (auto *elm : m_elements) {
        if (elm->elementGroup() == ElementGroup::Output) {
            QString label = elm->label();
            for (int i = 0; i < elm->inputs().size(); ++i) {
                currentOutput++;
                QString varName = elm->objectName() + QString::number(counter);
                if (!label.isEmpty()) {
                    varName = QString("%1_%2").arg(varName, label);
                }
                Port *port = elm->inputPort(i);
                if (!port->name().isEmpty()) {
                    varName = QString("%1_%2").arg(varName, port->name());
                }
                varName = CodeGenUtils::stripAccents(varName);
                varName = removeForbiddenChars(varName);
                if (currentOutput < totalOutputs) {
                    m_stream << QString("output %1,").arg(varName) << Qt::endl;
                } else {
                    m_stream << QString("output %1").arg(varName) << Qt::endl;
                }
                m_outputMap.append(MappedPinSystemVerilog(elm, "", varName, port, i));
            }
        }
        ++counter;
    }
    m_stream << ");" << Qt::endl;
}

void SystemVerilogCodeGen::declareAuxVariablesRec(const QVector<GraphicElement *> &elements)
{
    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = qobject_cast<IC *>(elm);
            if (!ic) continue;

            // Look up IC module info
            QString key = icModuleKey(ic);
            const ICModuleInfo &info = m_icModules.value(key);

            // Generate unique instance name
            QString instanceName = QString("%1_inst_%2").arg(info.moduleName).arg(m_globalCounter++);
            m_instanceNames[ic] = instanceName;

            m_stream << "// IC instance: " << ic->label() << " (" << info.moduleName << ")" << Qt::endl;

            // Declare output wires for this IC instance
            for (int i = 0; i < ic->outputSize(); ++i) {
                QString wireName = QString("w_%1_%2").arg(instanceName, info.outputNames.value(i, QString("out_%1").arg(i)));
                m_varMap[ic->outputPort(i)] = wireName;
                m_stream << "wire " << wireName << ";" << Qt::endl;
            }

            // Do NOT recurse into ic->internalElements() — the IC module handles its internals
        } else {
            // [ISSUE-9] Skip top-level input elements — they're already declared as module ports.
            // IC-internal inputs won't have pre-existing map entries, so they proceed normally.
            const auto type = elm->elementType();
            if ((type == ElementType::InputButton ||
                 type == ElementType::InputSwitch ||
                 type == ElementType::Clock ||
                 type == ElementType::InputRotary) &&
                !m_varMap.value(elm->outputPort()).isEmpty()) {
                continue;
            }

            // Skip wire declarations for top-level logic gates (their expressions
            // are inlined). Inside IC modules (m_generatingICModule), declare wires so that
            // feedback loops produce proper circular assign references instead of 1'b0.
            if (!m_generatingICModule &&
                (type == ElementType::And ||
                 type == ElementType::Or ||
                 type == ElementType::Nand ||
                 type == ElementType::Nor ||
                 type == ElementType::Xor ||
                 type == ElementType::Xnor ||
                 type == ElementType::Not ||
                 type == ElementType::Node)) {
                continue;
            }

            QString varName = QString("aux_%1_%2").arg(removeForbiddenChars(CodeGenUtils::stripAccents(elm->objectName()))).arg(m_globalCounter++);
            const auto outputs = elm->outputs();

            // Track which ports were already pre-mapped (e.g., IC module boundary ports)
            QSet<Port *> preMapped;

            if (outputs.size() == 1) {
                Port *port = outputs.constFirst();

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
                } else {
                    preMapped.insert(port);
                }
            } else {
                int portCounter = 0;

                for (auto *port : outputs) {
                    if (!m_varMap.value(port).isEmpty()) {
                        preMapped.insert(port);
                        portCounter++;
                        continue;
                    }

                    QString portName = varName;
                    portName.append(QString("_%1").arg(portCounter++));

                    if (!port->name().isEmpty()) {
                        portName.append(QString("_%1").arg(removeForbiddenChars(CodeGenUtils::stripAccents(port->name()))));
                    }

                    m_varMap[port] = portName;
                }
            }
            int aux = 0; // Initial values for flip-flop outputs (Q=0, Q̄=1)
            for (auto *port : outputs) {
                // Skip wire declaration for ports already mapped (e.g., module input/output ports)
                if (preMapped.contains(port)) {
                    aux++;
                    continue;
                }
                QString varName2 = m_varMap.value(port);

                switch (elm->elementType()) {

                case ElementType::DLatch:
                case ElementType::SRLatch:
                case ElementType::SRFlipFlop:
                case ElementType::DFlipFlop:
                case ElementType::TFlipFlop:
                case ElementType::JKFlipFlop: {
                    m_stream << "reg " << varName2 << QString(" = 1'b%1;").arg(aux) << Qt::endl;
                    aux++;
                    break;
                }

                case ElementType::TruthTable:
                    if (!outputs.isEmpty()) {
                        Port *outputPort = outputs.constFirst();
                        QString ttVarName = QString("%1_output").arg(removeForbiddenChars(elm->objectName()));
                        m_varMap[outputPort] = ttVarName;
                        m_stream << QString("reg ") << ttVarName << ";" << Qt::endl;

                        continue;
                    }
                    break;

                case ElementType::Mux:
                case ElementType::Demux: {
                    // Mux and Demux use always @(*) blocks, so outputs must be reg, not wire
                    m_stream << "reg " << varName2 << " = 1'b0;" << Qt::endl;
                    break;
                }

                case ElementType::And:
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
                case ElementType::Nand:
                case ElementType::Node:
                case ElementType::Nor:
                case ElementType::Not:
                case ElementType::Or:
                case ElementType::Text:
                case ElementType::Unknown:
                case ElementType::Xnor:
                case ElementType::Xor:
                    if (m_feedbackElements.contains(elm)) {
                        // Cross-coupled feedback node: a `reg` driven by
                        // `always @(*)` (below), seeded so it settles from a
                        // defined power-on state instead of x-locking.
                        m_stream << "reg " << varName2 << " = " << highLow(port->status()) << ";" << Qt::endl;
                    } else {
                        m_stream << "wire " << varName2 << ";" << Qt::endl;
                    }
                    break;
                }
            }
        }
    }
}

void SystemVerilogCodeGen::declareAuxVariables()
{
    m_stream << "/* ====== Aux. Variables ====== */" << Qt::endl;
    declareAuxVariablesRec(m_elements);
    m_stream << Qt::endl;
}

// [QUALITY-11] Shared helper for all edge-triggered flip-flop types.
// [BUG-1] Uses ensureSimpleSignal() to guarantee valid sensitivity list identifiers.
// [BUG-5] Uses ~ (bitwise NOT) consistently instead of ! (logical NOT).
// [BUG-6] Uses ~(expr) with parentheses for complex expressions.
void SystemVerilogCodeGen::emitSequentialBlock(
    const QString &typeName,
    const QString &clk, const QString &rawPrst, const QString &rawClr,
    const std::function<void()> &emitPresetBody,
    const std::function<void()> &emitClearBody,
    const std::function<void()> &emitNormalBody)
{
    bool hasPrst = (rawPrst != "1'b1" && rawPrst != "1'b0");
    bool hasClr = (rawClr != "1'b1" && rawClr != "1'b0");

    // [BUG-1] Ensure signals are simple identifiers for the sensitivity list.
    // Complex expressions like ~signal are invalid in always @(...) event controls.
    QString prst = hasPrst ? ensureSimpleSignal(rawPrst) : rawPrst;
    QString clr = hasClr ? ensureSimpleSignal(rawClr) : rawClr;

    m_stream << "    //" << typeName << Qt::endl;

    // Generate sensitivity list
    if (hasPrst && hasClr) {
        m_stream << "    always @(posedge " << clk << " or negedge " << prst << " or negedge " << clr << ")" << Qt::endl;
    } else if (hasPrst) {
        m_stream << "    always @(posedge " << clk << " or negedge " << prst << ")" << Qt::endl;
    } else if (hasClr) {
        m_stream << "    always @(posedge " << clk << " or negedge " << clr << ")" << Qt::endl;
    } else {
        m_stream << "    always @(posedge " << clk << ")" << Qt::endl;
    }

    m_stream << "    begin" << Qt::endl;

    if (!hasPrst && !hasClr) {
        // No async signals — just emit normal clock-edge logic
        emitNormalBody();
    } else {
        bool needsElse = false;

        if (hasPrst) {
            m_stream << "        if (~" << prst << ")" << Qt::endl;
            m_stream << "        begin" << Qt::endl;
            emitPresetBody();
            m_stream << "        end" << Qt::endl;
            needsElse = true;
        }

        if (hasClr) {
            if (needsElse) {
                m_stream << "        else if (~" << clr << ")" << Qt::endl;
            } else {
                m_stream << "        if (~" << clr << ")" << Qt::endl;
            }
            m_stream << "        begin" << Qt::endl;
            emitClearBody();
            m_stream << "        end" << Qt::endl;
        }

        m_stream << "        else" << Qt::endl;
        m_stream << "        begin" << Qt::endl;
        emitNormalBody();
        m_stream << "        end" << Qt::endl;
    }

    m_stream << "    end" << Qt::endl;
    m_stream << "    //End of " << typeName << Qt::endl;
}

void SystemVerilogCodeGen::assignVariablesRec(const QVector<GraphicElement *> &elements)
{
    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = qobject_cast<IC *>(elm);
            if (!ic) continue;

            // Look up IC module info and instance name
            QString key = icModuleKey(ic);
            const ICModuleInfo &info = m_icModules.value(key);
            QString instanceName = m_instanceNames.value(ic);

            // Emit module instantiation
            m_stream << info.moduleName << " " << instanceName << " (" << Qt::endl;

            // Connect input ports
            for (int i = 0; i < ic->inputSize(); ++i) {
                QString inputValue = otherPortName(ic->inputPort(i));
                m_stream << "    ." << info.inputNames.value(i, QString("in_%1").arg(i))
                         << "(" << inputValue << ")";
                if (i < ic->inputSize() - 1 || ic->outputSize() > 0) {
                    m_stream << ",";
                }
                m_stream << Qt::endl;
            }

            // Connect output ports
            for (int i = 0; i < ic->outputSize(); ++i) {
                QString outputWire = m_varMap.value(ic->outputPort(i));
                m_stream << "    ." << info.outputNames.value(i, QString("out_%1").arg(i))
                         << "(" << outputWire << ")";
                if (i < ic->outputSize() - 1) {
                    m_stream << ",";
                }
                m_stream << Qt::endl;
            }

            m_stream << ");" << Qt::endl;
            continue;
        }
        if (elm->inputs().isEmpty() || elm->outputs().isEmpty()) {
            continue;
        }

        // Logic gates: propagate expressions directly (top-level) or emit
        // assign statements (IC module internal, where wires were declared).
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
                QString existingVar = m_varMap.value(port);
                if (!existingVar.isEmpty() && m_generatingICModule) {
                    const bool isFeedback = m_feedbackElements.contains(elm);
                    if (isFeedback) {
                        // Cross-coupled feedback node: drive the seeded `reg`
                        // combinationally so the loop settles (vs. a comb-loop
                        // `assign` that x-locks / trips Verilator UNOPTFLAT).
                        m_stream << "always @(*) " << existingVar << " = " << expr << ";" << Qt::endl;
                    } else {
                        // IC module internal: wire was declared, emit assign statement
                        m_stream << "assign " << existingVar << " = " << expr << ";" << Qt::endl;
                    }
                } else {
                    // Top-level: inline the expression
                    m_varMap[port] = expr;
                }
            }
        }
        // Flip-flops and other stateful elements use auxiliary variables
        else {
            QString firstOut = m_varMap.value(elm->outputPort(0));
            switch (elm->elementType()) {
            case ElementType::DLatch: {
                QString secondOut = m_varMap.value(elm->outputPort(1));
                QString data = otherPortName(elm->inputPort(0));
                QString enable = otherPortName(elm->inputPort(1));
                m_stream << QString("    //D Latch") << Qt::endl;
                m_stream << QString("    always @(*)") << Qt::endl;
                m_stream << QString("    begin") << Qt::endl;
                m_stream << QString("        if (%1)").arg(enable) << Qt::endl;
                m_stream << QString("        begin") << Qt::endl;
                // [BUG-5] Use ~ (bitwise NOT) instead of ! (logical NOT)
                m_stream << QString("            %1 = %2;").arg(firstOut, data) << Qt::endl;
                QString dataBar = data.startsWith("~") ? data.mid(1) : ("~" + data);
                m_stream << QString("            %1 = %2;").arg(secondOut, dataBar) << Qt::endl;
                m_stream << QString("        end") << Qt::endl;
                m_stream << QString("    end") << Qt::endl;
                m_stream << QString("    //End of D Latch") << Qt::endl;

                break;
            }
            case ElementType::SRLatch: {
                QString secondOut = m_varMap.value(elm->outputPort(1));
                QString s = otherPortName(elm->inputPort(0));
                QString r = otherPortName(elm->inputPort(1));

                m_stream << QString("    //SR Latch") << Qt::endl;
                m_stream << QString("    always @(*)") << Qt::endl;
                m_stream << QString("    begin") << Qt::endl;
                m_stream << QString("        if (%1 && %2)").arg(s, r) << Qt::endl;
                m_stream << QString("        begin") << Qt::endl;
                m_stream << QString("            %1 = 1'b0;").arg(firstOut) << Qt::endl;
                m_stream << QString("            %1 = 1'b0;").arg(secondOut) << Qt::endl;
                m_stream << QString("        end") << Qt::endl;
                m_stream << QString("        else if (%1 != %2)").arg(s, r) << Qt::endl;
                m_stream << QString("        begin") << Qt::endl;
                m_stream << QString("            %1 = %2;").arg(firstOut, s) << Qt::endl;
                m_stream << QString("            %1 = %2;").arg(secondOut, r) << Qt::endl;
                m_stream << QString("        end") << Qt::endl;
                m_stream << QString("    end") << Qt::endl;
                m_stream << QString("    //End of SR Latch") << Qt::endl;

                break;
            }

            // [QUALITY-11] All edge-triggered flip-flops use the shared emitSequentialBlock helper.
            // This fixes BUG-1 (sensitivity lists), BUG-5 (!/~), BUG-6 (!expr) in one place.

            case ElementType::SRFlipFlop: {
                QString secondOut = m_varMap.value(elm->outputPort(1));
                QString s = otherPortName(elm->inputPort(0));
                QString clk = otherPortName(elm->inputPort(1));
                QString r = otherPortName(elm->inputPort(2));
                QString prst = otherPortName(elm->inputPort(3));
                QString clr = otherPortName(elm->inputPort(4));

                emitSequentialBlock("SR FlipFlop", clk, prst, clr,
                    [&]() { // Preset: Q=1, Q̄=0
                        m_stream << "            " << firstOut << " <= 1'b1;" << Qt::endl;
                        m_stream << "            " << secondOut << " <= 1'b0;" << Qt::endl;
                    },
                    [&]() { // Clear: Q=0, Q̄=1
                        m_stream << "            " << firstOut << " <= 1'b0;" << Qt::endl;
                        m_stream << "            " << secondOut << " <= 1'b1;" << Qt::endl;
                    },
                    [&]() { // Normal: SR logic on clock edge
                        m_stream << "            if (" << s << " && ~" << r << ")" << Qt::endl;
                        m_stream << "            begin" << Qt::endl;
                        m_stream << "                " << firstOut << " <= 1'b1;" << Qt::endl;
                        m_stream << "                " << secondOut << " <= 1'b0;" << Qt::endl;
                        m_stream << "            end" << Qt::endl;
                        m_stream << "            else if (~" << s << " && " << r << ")" << Qt::endl;
                        m_stream << "            begin" << Qt::endl;
                        m_stream << "                " << firstOut << " <= 1'b0;" << Qt::endl;
                        m_stream << "                " << secondOut << " <= 1'b1;" << Qt::endl;
                        m_stream << "            end" << Qt::endl;
                    }
                );

                break;
            }
            case ElementType::DFlipFlop: {
                QString secondOut = m_varMap.value(elm->outputPort(1));
                QString data = otherPortName(elm->inputPort(0));
                QString clk = otherPortName(elm->inputPort(1));
                QString prst = otherPortName(elm->inputPort(2));
                QString clr = otherPortName(elm->inputPort(3));

                emitSequentialBlock("D FlipFlop", clk, prst, clr,
                    [&]() { // Preset: Q=1, Q̄=0
                        m_stream << "            " << firstOut << " <= 1'b1;" << Qt::endl;
                        m_stream << "            " << secondOut << " <= 1'b0;" << Qt::endl;
                    },
                    [&]() { // Clear: Q=0, Q̄=1
                        m_stream << "            " << firstOut << " <= 1'b0;" << Qt::endl;
                        m_stream << "            " << secondOut << " <= 1'b1;" << Qt::endl;
                    },
                    [&]() { // Normal: capture data on clock edge
                        m_stream << "            " << firstOut << " <= " << data << ";" << Qt::endl;
                        QString dataBar = data.startsWith("~") ? data.mid(1) : ("~" + data);
                        m_stream << "            " << secondOut << " <= " << dataBar << ";" << Qt::endl;
                    }
                );

                break;
            }
            case ElementType::JKFlipFlop: {
                QString secondOut = m_varMap.value(elm->outputPort(1));
                QString j = otherPortName(elm->inputPort(0));
                QString clk = otherPortName(elm->inputPort(1));
                QString k = otherPortName(elm->inputPort(2));
                QString prst = otherPortName(elm->inputPort(3));
                QString clr = otherPortName(elm->inputPort(4));

                emitSequentialBlock("JK FlipFlop", clk, prst, clr,
                    [&]() { // Preset: Q=1, Q̄=0
                        m_stream << "            " << firstOut << " <= 1'b1;" << Qt::endl;
                        m_stream << "            " << secondOut << " <= 1'b0;" << Qt::endl;
                    },
                    [&]() { // Clear: Q=0, Q̄=1
                        m_stream << "            " << firstOut << " <= 1'b0;" << Qt::endl;
                        m_stream << "            " << secondOut << " <= 1'b1;" << Qt::endl;
                    },
                    [&]() { // Normal: JK logic on clock edge
                        m_stream << "            if (" << j << " && " << k << ")" << Qt::endl;
                        m_stream << "            begin" << Qt::endl;
                        m_stream << "                " << firstOut << " <= " << secondOut << ";" << Qt::endl;
                        m_stream << "                " << secondOut << " <= " << firstOut << ";" << Qt::endl;
                        m_stream << "            end" << Qt::endl;
                        m_stream << "            else if (" << j << " && ~" << k << ")" << Qt::endl;
                        m_stream << "            begin" << Qt::endl;
                        m_stream << "                " << firstOut << " <= 1'b1;" << Qt::endl;
                        m_stream << "                " << secondOut << " <= 1'b0;" << Qt::endl;
                        m_stream << "            end" << Qt::endl;
                        m_stream << "            else if (~" << j << " && " << k << ")" << Qt::endl;
                        m_stream << "            begin" << Qt::endl;
                        m_stream << "                " << firstOut << " <= 1'b0;" << Qt::endl;
                        m_stream << "                " << secondOut << " <= 1'b1;" << Qt::endl;
                        m_stream << "            end" << Qt::endl;
                    }
                );

                break;
            }
            case ElementType::TFlipFlop: {
                QString secondOut = m_varMap.value(elm->outputPort(1));
                QString t = otherPortName(elm->inputPort(0));
                QString clk = otherPortName(elm->inputPort(1));
                QString prst = otherPortName(elm->inputPort(2));
                QString clr = otherPortName(elm->inputPort(3));

                emitSequentialBlock("T FlipFlop", clk, prst, clr,
                    [&]() { // Preset: Q=1, Q̄=0
                        m_stream << "            " << firstOut << " <= 1'b1;" << Qt::endl;
                        m_stream << "            " << secondOut << " <= 1'b0;" << Qt::endl;
                    },
                    [&]() { // Clear: Q=0, Q̄=1
                        m_stream << "            " << firstOut << " <= 1'b0;" << Qt::endl;
                        m_stream << "            " << secondOut << " <= 1'b1;" << Qt::endl;
                    },
                    [&]() { // Normal: toggle on clock edge if T is high
                        m_stream << "            if (" << t << ")" << Qt::endl;
                        m_stream << "            begin" << Qt::endl;
                        m_stream << "                " << firstOut << " <= " << secondOut << ";" << Qt::endl;
                        m_stream << "                " << secondOut << " <= " << firstOut << ";" << Qt::endl;
                        m_stream << "            end" << Qt::endl;
                    }
                );

                break;
            }
            case ElementType::Mux: {
                // [ISSUE-7] Cleaner mux select line calculation.
                // Solves: 2^select + select = totalInputs
                int totalInputs = elm->inputSize();
                int numSelectLines = 1;
                while ((1 << numSelectLines) + numSelectLines < totalInputs) {
                    numSelectLines++;
                }
                int numDataInputs = totalInputs - numSelectLines;

                QString output = m_varMap.value(elm->outputPort(0));
                m_stream << QString("    //Multiplexer") << Qt::endl;
                m_stream << QString("    always @(*)") << Qt::endl;
                m_stream << QString("    begin") << Qt::endl;
                m_stream << QString("        case({");

                // Build select signal concatenation
                for (int i = numSelectLines - 1; i >= 0; --i) {
                    m_stream << otherPortName(elm->inputPort(numDataInputs + i));
                    if (i > 0) m_stream << ", ";
                }
                m_stream << "})" << Qt::endl;

                // Generate case statements for each select value
                for (int i = 0; i < numDataInputs; ++i) {
                    m_stream << QString("            %1'd%2: %3 = %4;")
                        .arg(numSelectLines)
                        .arg(i)
                        .arg(output)
                        .arg(otherPortName(elm->inputPort(i))) << Qt::endl;
                }
                m_stream << QString("            default: %1 = 1'b0;").arg(output) << Qt::endl;
                m_stream << QString("        endcase") << Qt::endl;
                m_stream << QString("    end") << Qt::endl;
                m_stream << QString("    //End of Multiplexer") << Qt::endl;

                break;
            }
            case ElementType::Demux: {
                // Demultiplexer: 1 data input + select lines -> N outputs
                int numOutputs = elm->outputSize();
                int numSelectLines = 1;
                while ((1 << numSelectLines) < numOutputs) {
                    numSelectLines++;
                }

                QString dataInput = otherPortName(elm->inputPort(0));
                m_stream << QString("    //Demultiplexer") << Qt::endl;
                m_stream << QString("    always @(*)") << Qt::endl;
                m_stream << QString("    begin") << Qt::endl;

                // Initialize all outputs to 0
                for (int i = 0; i < numOutputs; ++i) {
                    m_stream << QString("        %1 = 1'b0;").arg(m_varMap.value(elm->outputPort(i))) << Qt::endl;
                }

                // Build select signal concatenation for case
                m_stream << QString("        case({");
                for (int i = numSelectLines - 1; i >= 0; --i) {
                    m_stream << otherPortName(elm->inputPort(1 + i));
                    if (i > 0) m_stream << ", ";
                }
                m_stream << "})" << Qt::endl;

                // Generate case statements for each select value
                for (int i = 0; i < numOutputs; ++i) {
                    m_stream << QString("            %1'd%2: %3 = %4;")
                        .arg(numSelectLines)
                        .arg(i)
                        .arg(m_varMap.value(elm->outputPort(i)))
                        .arg(dataInput) << Qt::endl;
                }
                m_stream << QString("        endcase") << Qt::endl;
                m_stream << QString("    end") << Qt::endl;
                m_stream << QString("    //End of Demultiplexer") << Qt::endl;

                break;
            }
            case ElementType::TruthTable: {
                auto *ttGraphic = dynamic_cast<TruthTable *>(elm);
                if (!ttGraphic) break;

                QBitArray propositions = ttGraphic->key();
                const int nInputs = elm->inputSize();
                const int rows = 1 << nInputs;

                // Resolve input signal names
                QStringList inputSignalNames;
                for (int i = 0; i < nInputs; ++i) {
                    Port *ttInputPort = elm->inputPort(i);
                    QString signalName = otherPortName(ttInputPort);

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

                // Build concatenated input expression for case selector
                QStringList bitExpressions;
                bitExpressions << "{";
                for (int i = 0; i < nInputs; ++i) {
                    if (i < nInputs - 1) {
                        bitExpressions << QString("%1, ").arg(inputSignalNames[i]);
                    } else {
                        bitExpressions << QString("%1}").arg(inputSignalNames[i]);
                    }
                }

                QString indexCalculation = bitExpressions.join("");

                // One always block per output (F19): output k reads key bits 256*k + row.
                // Emitting per-output keeps the single-output text byte-identical to the
                // historical form.
                for (int out = 0; out < elm->outputSize(); ++out) {
                    QString outputVarName = m_varMap.value(elm->outputPort(out));
                    if (outputVarName.isEmpty()) {
                        if (out == 0) {
                            throw PANDACEPTION("Output variable not mapped for TruthTable: %1", elm->objectName());
                        }
                        m_stream << "// TruthTable '" << elm->objectName() << "' output " << out << " is disconnected — no code emitted." << Qt::endl;
                        continue;
                    }

                    m_stream << QString("    //TruthTable") << Qt::endl;
                    m_stream << QString("    always @(*)") << Qt::endl;
                    m_stream << QString("    begin") << Qt::endl;
                    m_stream << QString("        case(%1)").arg(indexCalculation) << Qt::endl;

                    for (int i = 0; i < rows; ++i) {
                        m_stream << QString("            %1'b").arg(nInputs) << QString::number(i, 2).rightJustified(nInputs, '0') << ": " << outputVarName << " = 1'b" << (propositions.testBit(256 * out + i) ? "1" : "0") << ";" << Qt::endl;
                    }
                    // [ISSUE-8] Defensive default for X/Z input states
                    m_stream << QString("            default: %1 = 1'b0;").arg(outputVarName) << Qt::endl;
                    m_stream << QString("        endcase") << Qt::endl;
                    m_stream << QString("    end") << Qt::endl;
                    m_stream << QString("    //End TruthTable") << Qt::endl;
                }

                break;
            }

            case ElementType::And:
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
            case ElementType::Nand:
            case ElementType::Node:
            case ElementType::Nor:
            case ElementType::Not:
            case ElementType::Or:
            case ElementType::Text:
            case ElementType::Unknown:
            case ElementType::Xnor:
            case ElementType::Xor:
                throw PANDACEPTION("Element type not supported: %1", elm->objectName());
            }
        }
    }
}

QString SystemVerilogCodeGen::generateLogicExpression(GraphicElement *elm)
{
    QSet<Port *> visited;
    return generateLogicExpressionImpl(elm, visited);
}

// [BUG-2] Cancels double negations: ~~expr -> expr
QString SystemVerilogCodeGen::generateLogicExpressionImpl(GraphicElement *elm, QSet<Port *> &visited)
{
    bool negate = false;
    QString logicOperator;

    switch (elm->elementType()) {
    case ElementType::And:  logicOperator = "&"; break;
    case ElementType::Or:   logicOperator = "|"; break;
    case ElementType::Nand: logicOperator = "&"; negate = true; break;
    case ElementType::Nor:  logicOperator = "|"; negate = true; break;
    case ElementType::Xor:  logicOperator = "^"; break;
    case ElementType::Xnor: logicOperator = "^"; negate = true; break;
    case ElementType::Not: {
        QString inner = otherPortNameImpl(elm->inputPort(0), visited);
        // [BUG-2] Cancel double negation: ~~x -> x
        if (inner.startsWith("~")) {
            return inner.mid(1);
        }
        return "~" + inner;
    }
    case ElementType::Node: return otherPortNameImpl(elm->inputPort(0), visited);
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
        return "";
    }

    QString expr;
    if (elm->inputs().size() == 1) {
        expr = otherPortNameImpl(elm->inputPort(0), visited);
    } else {
        // Group multiple inputs with parentheses
        expr = "(";
        for (int i = 0; i < elm->inputs().size(); ++i) {
            if (i > 0) expr += " " + logicOperator + " ";
            expr += otherPortNameImpl(elm->inputPort(i), visited);
        }
        expr += ")";
    }

    if (negate) {
        // [BUG-2] Cancel double negation: ~(~expr) -> expr without outer ~
        if (expr.startsWith("~")) {
            expr = expr.mid(1);
        } else {
            expr = "~" + expr;
        }
    }

    return expr;
}

void SystemVerilogCodeGen::loop()
{
    // [ISSUE-9] No redundant _val assigns — inputs are used directly by name

    m_stream << "\n// Assigning aux variables. //" << Qt::endl;
    assignVariablesRec(m_elements);

    m_stream << "\n// Writing output data. //" << Qt::endl;
    for (const auto &pin : std::as_const(m_outputMap)) {
        QString expr = otherPortName(pin.m_port);
        if (expr.isEmpty()) {
            expr = highLow(pin.m_port->defaultValue());
        }
        m_stream << QString("assign %1 = %2;").arg(pin.m_varName, expr) << Qt::endl;
    }
    m_stream << "endmodule" << Qt::endl;
}
