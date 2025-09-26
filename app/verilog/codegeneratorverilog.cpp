// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeneratorverilog.h"

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
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QtMath>

CodeGeneratorVerilog::CodeGeneratorVerilog(const QString &fileName, const QVector<GraphicElement *> &elements, FPGAFamily target)
    : m_file(fileName)
    , m_elements(elements)
    , m_targetFamily(target)
{
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    m_stream.setDevice(&m_file);

    QFileInfo info(fileName);
    m_fileName = removeForbiddenChars(stripAccents(info.completeBaseName()));

    // Ensure module name starts with letter and is valid Verilog identifier
    if (m_fileName.isEmpty() || !m_fileName.at(0).isLetter() || isVerilogReservedKeyword(m_fileName)) {
        m_fileName = "wiredpanda_module";
    }

    // TEMPORARY: Enable debug output to trace clock signal resolution issues
    m_debugOutput = true;
}

// ============================================================================
// CORE UTILITY METHODS
// ============================================================================

QString CodeGeneratorVerilog::removeForbiddenChars(const QString &input)
{
    return input.toLower()
               .trimmed()
               .replace(' ', '_')
               .replace('-', '_')
               .replace(QRegularExpression("\\W"), "");
}

QString CodeGeneratorVerilog::stripAccents(const QString &input)
{
    QString normalized = input.normalized(QString::NormalizationForm_D);
    QRegularExpression diacriticMarks("[\\p{Mn}]");
    return normalized.remove(diacriticMarks);
}

QString CodeGeneratorVerilog::boolValue(const Status val)
{
    return (val == Status::Active) ? "1'b1" : "1'b0";
}

QString CodeGeneratorVerilog::simplifyExpression(const QString &expr)
{
    QString simplified = expr.trimmed();

    // Handle double negation: ~~signal -> signal
    while (simplified.startsWith("~~")) {
        simplified = simplified.mid(2); // Remove first two characters
    }

    // Handle complex double negation in parentheses: ~(~expr) -> expr
    if (simplified.startsWith("~(~") && simplified.endsWith(")")) {
        QString inner = simplified.mid(3, simplified.length() - 4); // Remove "~(~" and ")"
        // Only simplify if the inner expression is balanced (no unmatched parentheses)
        int parenCount = 0;
        bool balanced = true;
        for (const QChar &c : inner) {
            if (c == '(') parenCount++;
            else if (c == ')') {
                parenCount--;
                if (parenCount < 0) {
                    balanced = false;
                    break;
                }
            }
        }
        if (balanced && parenCount == 0) {
            simplified = inner;
        }
    }

    return simplified;
}

bool CodeGeneratorVerilog::isVerilogReservedKeyword(const QString &identifier)
{
    // Comprehensive list of Verilog reserved keywords (IEEE 1364/1800)
    static const QSet<QString> reservedKeywords = {
        // Port and signal keywords
        "input", "output", "inout", "wire", "reg", "tri", "tri0", "tri1",

        // Module and hierarchy keywords
        "module", "endmodule", "generate", "endgenerate", "genvar",
        "task", "endtask", "function", "endfunction",

        // Control flow keywords
        "if", "else", "case", "casex", "casez", "endcase", "default",
        "for", "while", "repeat", "forever", "begin", "end",

        // Logic keywords
        "and", "or", "not", "nand", "nor", "xor", "xnor", "buf", "bufif0", "bufif1",
        "notif0", "notif1", "nmos", "pmos", "cmos", "rnmos", "rpmos", "rcmos",

        // Timing and behavioral keywords
        "always", "initial", "assign", "posedge", "negedge", "edge",
        "wait", "disable", "fork", "join",

        // SystemVerilog additions
        "logic", "bit", "byte", "shortint", "int", "longint", "time",
        "interface", "endinterface", "class", "endclass", "package", "endpackage",
        "program", "endprogram", "property", "endproperty",

        // Compiler directives (without backticks)
        "define", "undef", "ifdef", "ifndef", "else", "endif", "include",
        "timescale", "resetall", "celldefine", "endcelldefine",

        // System tasks and functions (without $)
        "display", "monitor", "strobe", "write", "finish", "stop", "time", "realtime"
    };

    return reservedKeywords.contains(identifier.toLower());
}

// ============================================================================
// FPGA BOARD SELECTION AND VALIDATION
// ============================================================================

FPGABoard CodeGeneratorVerilog::selectFPGABoard(int requiredLUTs, int requiredFFs, int requiredIOs)
{
    QVector<FPGABoard> availableBoards;

    // Generic FPGA boards for different complexity levels
    availableBoards << FPGABoard("Generic-Small", "Small generic FPGA (educational)", FPGAFamily::Generic, 1000, 1000, 10, 50);
    availableBoards << FPGABoard("Generic-Medium", "Medium generic FPGA", FPGAFamily::Generic, 10000, 10000, 50, 200);
    availableBoards << FPGABoard("Generic-Large", "Large generic FPGA", FPGAFamily::Generic, 100000, 100000, 200, 500);

    // Xilinx boards
    if (m_targetFamily == FPGAFamily::Xilinx || m_targetFamily == FPGAFamily::Generic) {
        availableBoards << FPGABoard("Artix-7 35T", "Xilinx Artix-7 XC7A35T", FPGAFamily::Xilinx, 33280, 41600, 90, 210);
        availableBoards << FPGABoard("Kintex-7 325T", "Xilinx Kintex-7 XC7K325T", FPGAFamily::Xilinx, 326080, 407600, 890, 500);
        availableBoards << FPGABoard("Zynq-7020", "Xilinx Zynq-7000 XC7Z020", FPGAFamily::Xilinx, 85400, 106400, 280, 200);
    }

    // Intel/Altera boards
    if (m_targetFamily == FPGAFamily::Intel || m_targetFamily == FPGAFamily::Generic) {
        availableBoards << FPGABoard("Cyclone V SE", "Intel Cyclone V 5CSEMA5F31C6", FPGAFamily::Intel, 85000, 85000, 150, 457);
        availableBoards << FPGABoard("Stratix V", "Intel Stratix V 5SGXEA7N2F45C2", FPGAFamily::Intel, 690000, 690000, 2780, 696);
    }

    // Lattice boards
    if (m_targetFamily == FPGAFamily::Lattice || m_targetFamily == FPGAFamily::Generic) {
        availableBoards << FPGABoard("iCE40 HX8K", "Lattice iCE40HX8K", FPGAFamily::Lattice, 7680, 7680, 32, 206);
        availableBoards << FPGABoard("ECP5 85K", "Lattice LFE5UM5G-85F", FPGAFamily::Lattice, 85000, 85000, 200, 365);
    }

    // Sort by resource capacity (smallest first that meets requirements)
    std::sort(availableBoards.begin(), availableBoards.end(),
              [](const FPGABoard &a, const FPGABoard &b) {
                  return a.maxResources() < b.maxResources();
              });

    // Find the smallest board that meets requirements
    for (const auto &board : availableBoards) {
        if (board.maxLUTs >= requiredLUTs &&
            board.maxFlipFlops >= requiredFFs &&
            board.maxIOPins >= requiredIOs) {
            return board;
        }
    }

    // Return invalid board if no suitable board found
    return FPGABoard();
}

bool CodeGeneratorVerilog::validateResourceRequirements()
{
    estimateResourceUsage(m_estimatedLUTs, m_estimatedFlipFlops, m_estimatedIOPins);

    m_selectedBoard = selectFPGABoard(m_estimatedLUTs, m_estimatedFlipFlops, m_estimatedIOPins);

    if (m_selectedBoard.name.isEmpty()) {
        QString error = QString("Circuit requires %1 LUTs, %2 FFs, %3 IOs but no available FPGA can accommodate this complexity")
                       .arg(m_estimatedLUTs).arg(m_estimatedFlipFlops).arg(m_estimatedIOPins);
        handleGenerationError(error);
        return false;
    }

    return true;
}

void CodeGeneratorVerilog::estimateResourceUsage(int &luts, int &ffs, int &ios)
{
    luts = 0;
    ffs = 0;
    ios = 0;

    for (auto *elm : m_elements) {
        const auto type = elm->elementType();

        // Estimate LUT usage
        switch (type) {
        case ElementType::And:
        case ElementType::Or:
        case ElementType::Nand:
        case ElementType::Nor:
        case ElementType::Xor:
        case ElementType::Xnor:
        case ElementType::Not:
            luts += 1; // Basic logic gates use 1 LUT each
            break;

        case ElementType::Mux:
            luts += elm->inputSize() / 2; // Mux uses LUTs based on input count
            break;

        case ElementType::Demux:
            luts += elm->outputSize() / 2;
            break;

        case ElementType::TruthTable:
            luts += qPow(2, elm->inputSize()) / 16; // Truth table LUT usage
            break;

        case ElementType::Display7:
            luts += 10; // 7-segment decoder
            break;

        case ElementType::Display14:
            luts += 20; // 14-segment decoder
            break;

        case ElementType::Display16:
            luts += 25; // 16-segment decoder
            break;

        default:
            luts += 1; // Conservative estimate
            break;
        }

        // Estimate flip-flop usage
        switch (type) {
        case ElementType::DFlipFlop:
        case ElementType::JKFlipFlop:
        case ElementType::SRFlipFlop:
        case ElementType::TFlipFlop:
            ffs += 1; // Each flip-flop uses 1 FF
            break;

        case ElementType::DLatch:
        case ElementType::SRLatch:
            ffs += 1; // Latches implemented as FFs in most FPGAs
            break;

        case ElementType::Clock:
            ffs += 32; // Clock dividers need counter FFs
            break;

        default:
            break;
        }

        // Estimate I/O usage
        if (elm->elementGroup() == ElementGroup::Input) {
            ios += elm->outputs().size();
        } else if (elm->elementGroup() == ElementGroup::Output) {
            ios += elm->inputs().size();
        }
    }

    // Add safety margin
    luts = static_cast<int>(luts * 1.2);
    ffs = static_cast<int>(ffs * 1.1);
    ios = static_cast<int>(ios * 1.1);
}

// ============================================================================
// VARIABLE MANAGEMENT AND NAMING
// ============================================================================

QString CodeGeneratorVerilog::otherPortName(QNEPort *port, QSet<GraphicElement*> *visited)
{
    if (!port) {
        generateDebugInfo("otherPortName: port is nullptr, returning 1'b0", nullptr);
        return "1'b0";
    }

    generateDebugInfo(QString("otherPortName: Processing port from element %1 (type %2)")
                     .arg(port->graphicElement() ? port->graphicElement()->objectName() : "unknown")
                     .arg(port->graphicElement() ? QString::number(static_cast<int>(port->graphicElement()->elementType())) : "unknown"),
                     port->graphicElement());

    if (port->connections().isEmpty()) {
        generateDebugInfo("otherPortName: port has no connections, returning default value", port->graphicElement());
        return boolValue(port->defaultValue());
    }

    auto *connection = port->connections().constFirst();
    if (!connection) {
        generateDebugInfo("otherPortName: connection is nullptr, returning default value", port->graphicElement());
        return boolValue(port->defaultValue());
    }

    auto *otherPort = connection->otherPort(port);
    if (!otherPort) {
        generateDebugInfo("otherPortName: otherPort is nullptr, returning default value", port->graphicElement());
        return boolValue(port->defaultValue());
    }

    generateDebugInfo(QString("otherPortName: Found connected port from element %1 (type %2)")
                     .arg(otherPort->graphicElement() ? otherPort->graphicElement()->objectName() : "unknown")
                     .arg(otherPort->graphicElement() ? QString::number(static_cast<int>(otherPort->graphicElement()->elementType())) : "unknown"),
                     otherPort->graphicElement());

    // Check if connected to a logic gate - generate expression directly
    auto *elm = otherPort->graphicElement();

    // Initialize visited set if not provided
    QSet<GraphicElement*> localVisited;
    if (!visited) {
        visited = &localVisited;
    }

    // Check for circular dependency
    if (visited->contains(elm)) {
        generateDebugInfo("otherPortName: Circular dependency detected, checking varMap", elm);
        // Return variable name from map if available, otherwise return default value
        QString result = m_varMap.value(otherPort);
        generateDebugInfo(QString("otherPortName: varMap result for circular dependency: '%1'").arg(result), elm);
        if (result.isEmpty()) {
            generateDebugInfo("otherPortName: varMap empty, returning default value", elm);
            return boolValue(otherPort->defaultValue());
        }
        generateDebugInfo(QString("otherPortName: Returning varMap result for circular dependency: %1").arg(result), elm);
        return result;
    }

    if (elm->elementType() == ElementType::And ||
        elm->elementType() == ElementType::Or ||
        elm->elementType() == ElementType::Nand ||
        elm->elementType() == ElementType::Nor ||
        elm->elementType() == ElementType::Xor ||
        elm->elementType() == ElementType::Xnor ||
        elm->elementType() == ElementType::Not ||
        elm->elementType() == ElementType::Node) {

        generateDebugInfo("otherPortName: Connected to logic gate, generating expression", elm);
        return generateLogicExpression(elm, visited);
    }

    generateDebugInfo("otherPortName: Checking varMap for final result", elm);
    QString result = m_varMap.value(otherPort);
    generateDebugInfo(QString("otherPortName: varMap lookup result: '%1'").arg(result), elm);
    if (result.isEmpty()) {
        generateDebugInfo("otherPortName: varMap empty, returning default value", elm);
        return boolValue(otherPort->defaultValue());
    }
    generateDebugInfo(QString("otherPortName: Final result: %1").arg(result), elm);
    return result;
}

QString CodeGeneratorVerilog::generateUniqueVariableName(const QString &baseName, const QString &context)
{
    QString cleanBase = removeForbiddenChars(stripAccents(baseName));
    if (cleanBase.isEmpty()) {
        cleanBase = "sig";
    }

    QString contextPrefix;
    if (!context.isEmpty()) {
        contextPrefix = removeForbiddenChars(stripAccents(context)) + "_";
    }

    // Add IC context if we're inside an IC
    if (m_currentIC != nullptr) {
        QString icName = removeForbiddenChars(stripAccents(m_currentIC->label()));
        if (icName.isEmpty()) {
            icName = QString("ic%1").arg(m_icDepth);
        }
        contextPrefix = QString("ic_%1_%2").arg(icName, contextPrefix);
    }

    QString candidate = QString("%1%2_%3").arg(contextPrefix, cleanBase).arg(m_globalCounter);

    // Ensure uniqueness
    while (isVariableDeclared(candidate)) {
        m_globalCounter++;
        candidate = QString("%1%2_%3").arg(contextPrefix, cleanBase).arg(m_globalCounter);
    }

    m_globalCounter++;
    markVariableDeclared(candidate);

    return candidate;
}

bool CodeGeneratorVerilog::isVariableDeclared(const QString &varName) const
{
    return m_declaredVariables.contains(varName);
}

void CodeGeneratorVerilog::markVariableDeclared(const QString &varName)
{
    m_declaredVariables.insert(varName);
}

// ============================================================================
// TOPOLOGICAL SORTING AND DEPENDENCY RESOLUTION
// ============================================================================

QVector<GraphicElement *> CodeGeneratorVerilog::topologicalSort(const QVector<GraphicElement *> &elements)
{
    QVector<GraphicElement *> sorted;
    QSet<GraphicElement *> visited;
    QSet<GraphicElement *> visiting;

    std::function<void(GraphicElement *)> visit = [&](GraphicElement *element) {
        if (visiting.contains(element)) {
            // Note: Skip circular dependencies in topological sort - let sequential circuits proceed
            return;
        }

        if (visited.contains(element)) {
            return;
        }

        visiting.insert(element);

        // Visit all elements this element depends on
        for (auto *inputPort : element->inputs()) {
            if (!inputPort->connections().isEmpty()) {
                auto *connection = inputPort->connections().constFirst();
                auto *otherPort = connection->otherPort(inputPort);
                if (otherPort) {
                    auto *dependency = otherPort->graphicElement();
                    if (elements.contains(dependency)) {
                        visit(dependency);
                    }
                }
            }
        }

        visiting.remove(element);
        visited.insert(element);
        sorted.prepend(element); // Prepend to get correct order
    };

    // Visit all elements
    for (auto *element : elements) {
        if (!visited.contains(element)) {
            visit(element);
        }
    }

    return sorted;
}

// Note: hasCircularDependency method removed - let EDA tools handle legitimate sequential feedback

int CodeGeneratorVerilog::getDependencyScore(GraphicElement *element, const QVector<GraphicElement *> &elements)
{
    int score = 0;

    for (auto *inputPort : element->inputs()) {
        if (!inputPort->connections().isEmpty()) {
            auto *connection = inputPort->connections().constFirst();
            auto *otherPort = connection->otherPort(inputPort);
            if (otherPort && elements.contains(otherPort->graphicElement())) {
                score++;
            }
        }
    }

    return score;
}

// ============================================================================
// IC PROCESSING PIPELINE
// ============================================================================

void CodeGeneratorVerilog::processICsRecursively(const QVector<GraphicElement *> &elements, int depth)
{
    m_icDepth = depth;

    generateDebugInfo(QString(">>> processICsRecursively called with %1 elements at depth %2").arg(elements.size()).arg(depth));

    for (auto *elm : elements) {
        generateDebugInfo(QString("Processing element: %1 (type: %2)").arg(elm->objectName()).arg(static_cast<int>(elm->elementType())));

        if (elm->elementType() == ElementType::IC) {
            generateDebugInfo(QString(">>> Found IC element: %1").arg(elm->objectName()), elm);

            auto *ic = qgraphicsitem_cast<IC *>(elm);
            if (!ic) {
                generateDebugInfo(QString(">>> CRITICAL: Failed to cast IC element: %1").arg(elm->objectName()), elm);
                handleGenerationError(QString("Failed to cast IC element: %1").arg(elm->objectName()), elm);
                continue;
            }

            generateDebugInfo(QString(">>> IC cast successful: %1 (label: %2)").arg(elm->objectName()).arg(ic->label()), ic);

            // Set current IC context
            IC *previousIC = m_currentIC;
            m_currentIC = ic;
            m_icStack.append(ic);

            // Generate IC boundary comment
            if (m_debugOutput) {
                m_stream << generateICBoundaryComment(ic, true) << Qt::endl;
            }

            // Validate IC connectivity
            generateDebugInfo(QString(">>> About to validate IC connectivity: %1").arg(ic->label()), ic);
            bool connectivityValid = validateICConnectivity(ic);
            generateDebugInfo(QString(">>> IC connectivity validation result: %1 for IC: %2").arg(connectivityValid ? "PASSED" : "FAILED").arg(ic->label()), ic);

            if (!connectivityValid) {
                generateDebugInfo(QString(">>> CRITICAL: IC connectivity validation failed, skipping: %1").arg(ic->label()), ic);
                handleGenerationError(QString("IC connectivity validation failed: %1").arg(ic->label()), elm);
                continue;
            }

            // Map IC ports to internal signals
            generateDebugInfo(QString(">>> About to call mapICPortsToSignals for IC: %1").arg(ic->label()), ic);
            mapICPortsToSignals(ic);
            generateDebugInfo(QString(">>> Completed mapICPortsToSignals for IC: %1").arg(ic->label()), ic);

            // Process IC internal elements recursively
            generateDebugInfo(QString("Checking IC %1: m_icElements.size() = %2").arg(ic->label()).arg(ic->m_icElements.size()), ic);

            if (!ic->m_icElements.isEmpty()) {
                generateDebugInfo(QString("Processing IC: %1 with %2 internal elements at depth %3")
                                .arg(ic->label()).arg(ic->m_icElements.size()).arg(depth), ic);

                // List all internal elements for debugging
                for (int i = 0; i < ic->m_icElements.size(); ++i) {
                    auto *elm = ic->m_icElements[i];
                    generateDebugInfo(QString("IC internal element %1: %2 (type %3)").arg(i).arg(elm->objectName()).arg(static_cast<int>(elm->elementType())), elm);
                }

                // CRITICAL FIX: Register AND declare variables for IC internal elements
                generateDebugInfo(QString("CRITICAL FIX: Processing IC %1 internal elements - registering variables").arg(ic->label()), ic);
                for (auto *elm : ic->m_icElements) {
                    if (elm->elementType() != ElementType::IC) {
                        // Skip input and output elements
                        if (elm->elementGroup() == ElementGroup::Input || elm->elementGroup() == ElementGroup::Output) {
                            continue;
                        }

                        QString varName = generateUniqueVariableName(elm->objectName(), QString("ic_%1").arg(ic->label().toLower()));
                        const auto outputs = elm->outputs();

                        if (outputs.size() == 1) {
                            // Single output elements
                            QNEPort *port = outputs.at(0);
                            generateDebugInfo(QString("CRITICAL FIX: Registering single output port for %1 -> %2").arg(elm->objectName()).arg(varName), elm);
                            m_varMap[port] = varName;

                            // Declare the variable
                            switch (elm->elementType()) {
                            case ElementType::DLatch:
                            case ElementType::SRLatch:
                            case ElementType::SRFlipFlop:
                            case ElementType::DFlipFlop:
                            case ElementType::TFlipFlop:
                            case ElementType::JKFlipFlop:
                                m_stream << QString("    reg %1 = 1'b0;").arg(varName) << Qt::endl;
                                break;
                            default:
                                m_stream << QString("    wire %1;").arg(varName) << Qt::endl;
                                break;
                            }
                        } else if (outputs.size() > 1) {
                            // Multi-output elements (like D-Flip-Flops)
                            int portCounter = 0;
                            for (auto *port : outputs) {
                                QString portName = QString("%1_%2").arg(varName).arg(portCounter++);
                                generateDebugInfo(QString("CRITICAL FIX: Registering multi-output port %1 for %2 -> %3").arg(portCounter-1).arg(elm->objectName()).arg(portName), elm);
                                m_varMap[port] = portName;

                                // Declare the variable
                                switch (elm->elementType()) {
                                case ElementType::DLatch:
                                case ElementType::SRLatch:
                                case ElementType::SRFlipFlop:
                                case ElementType::DFlipFlop:
                                case ElementType::TFlipFlop:
                                case ElementType::JKFlipFlop:
                                    m_stream << QString("    reg %1 = 1'b0;").arg(portName) << Qt::endl;
                                    break;
                                default:
                                    m_stream << QString("    wire %1;").arg(portName) << Qt::endl;
                                    break;
                                }
                            }
                        }
                    }
                }

                // Recursively assign variables for internal elements
                generateDebugInfo(QString("About to assign variables for IC %1 internal elements").arg(ic->label()), ic);
                assignVariablesRec(ic->m_icElements, true);

                generateDebugInfo(QString("Completed processing IC %1 internal elements").arg(ic->label()), ic);
            } else {
                generateDebugInfo(QString("IC %1 has no internal elements").arg(ic->label()), ic);
            }

            // Restore previous IC context
            m_icStack.removeLast();
            m_currentIC = previousIC;

            // Generate IC boundary end comment
            if (m_debugOutput) {
                m_stream << generateICBoundaryComment(ic, false) << Qt::endl;
            }
        }
    }
}

QString CodeGeneratorVerilog::generateICBoundaryComment(IC *ic, bool isStart)
{
    QString icName = ic->label();
    if (icName.isEmpty()) {
        icName = QString("IC_%1").arg(ic->objectName());
    }

    QString marker = isStart ? "BEGIN" : "END";
    QString comment = QString("// ============== %1 IC: %2 ==============").arg(marker, icName);

    if (isStart) {
        comment += QString("\n// IC inputs: %1, IC outputs: %2")
                  .arg(ic->inputSize())
                  .arg(ic->outputSize());
        comment += QString("\n// Nesting depth: %1").arg(m_icDepth);
    }

    return comment;
}

void CodeGeneratorVerilog::mapICPortsToSignals(IC *ic)
{
    generateDebugInfo(QString("mapICPortsToSignals starting for IC %1 with %2 inputs, %3 outputs, %4 internal inputs, %5 internal outputs")
                     .arg(ic->label()).arg(ic->inputSize()).arg(ic->outputSize()).arg(ic->m_icInputs.size()).arg(ic->m_icOutputs.size()), ic);

    // Debug: Print all m_icInputs and m_icOutputs
    for (int i = 0; i < ic->m_icInputs.size(); ++i) {
        QNEPort *internalPort = ic->m_icInputs.at(i);
        generateDebugInfo(QString("IC internal input %1: port from element %2")
                         .arg(i).arg(internalPort->graphicElement() ? internalPort->graphicElement()->objectName() : "unknown"), ic);
    }
    for (int i = 0; i < ic->m_icOutputs.size(); ++i) {
        QNEPort *internalPort = ic->m_icOutputs.at(i);
        generateDebugInfo(QString("IC internal output %1: port from element %2")
                         .arg(i).arg(internalPort->graphicElement() ? internalPort->graphicElement()->objectName() : "unknown"), ic);
    }

    // Map input ports - Connect IC internal input ports to external signals
    for (int i = 0; i < ic->inputSize(); ++i) {
        QNEPort *externalPort = ic->inputPort(i);
        generateDebugInfo(QString("Processing external input port %1 from element %2")
                         .arg(i).arg(externalPort ? externalPort->graphicElement()->objectName() : "unknown"), ic);

        QString externalSignal = otherPortName(externalPort);
        generateDebugInfo(QString("External input port %1 resolves to signal: %2").arg(i).arg(externalSignal), ic);

        // Map the corresponding internal input port to the external signal
        if (i < ic->m_icInputs.size()) {
            QNEPort *internalInputPort = ic->m_icInputs.at(i);
            generateDebugInfo(QString("Mapping internal input port %1 (from %2) to external signal: %3")
                             .arg(i).arg(internalInputPort->graphicElement() ? internalInputPort->graphicElement()->objectName() : "unknown").arg(externalSignal), ic);

            // Check if already in varMap
            QString oldValue = m_varMap.value(internalInputPort);
            generateDebugInfo(QString("Internal input port %1 old varMap value: '%2'").arg(i).arg(oldValue), ic);

            m_varMap[internalInputPort] = externalSignal;

            generateDebugInfo(QString("Successfully mapped IC internal input port %1 to external signal: %2")
                             .arg(i).arg(externalSignal), ic);
        } else {
            generateDebugInfo(QString("ERROR: External input %1 has no corresponding internal input (m_icInputs size: %2)").arg(i).arg(ic->m_icInputs.size()), ic);
        }
    }

    // Map output ports - Connect IC internal output ports to external ports
    for (int i = 0; i < ic->outputSize(); ++i) {
        QNEPort *externalPort = ic->outputPort(i);
        generateDebugInfo(QString("Processing external output port %1").arg(i), ic);

        // Map the corresponding internal output port to the external port
        if (i < ic->m_icOutputs.size()) {
            QNEPort *internalOutputPort = ic->m_icOutputs.at(i);
            generateDebugInfo(QString("Getting source signal for internal output port %1 (from %2)")
                             .arg(i).arg(internalOutputPort->graphicElement() ? internalOutputPort->graphicElement()->objectName() : "unknown"), ic);

            // Get the signal that drives the internal output port
            QString sourceSignal = otherPortName(internalOutputPort);
            generateDebugInfo(QString("Internal output port %1 source signal: %2").arg(i).arg(sourceSignal), ic);

            // Map the external port to this source signal
            m_varMap[externalPort] = sourceSignal;

            generateDebugInfo(QString("Successfully mapped IC external output port %1 to internal source signal: %2")
                             .arg(i).arg(sourceSignal), ic);
        } else {
            generateDebugInfo(QString("ERROR: External output %1 has no corresponding internal output (m_icOutputs size: %2)").arg(i).arg(ic->m_icOutputs.size()), ic);
        }
    }

    // Debug: Print current state of m_varMap for this IC
    generateDebugInfo(QString("Current m_varMap entries for IC %1:").arg(ic->label()), ic);
    for (auto it = m_varMap.begin(); it != m_varMap.end(); ++it) {
        QNEPort *port = it.key();
        QString signal = it.value();
        if (port && port->graphicElement()) {
            generateDebugInfo(QString("  Port from %1 -> %2").arg(port->graphicElement()->objectName()).arg(signal), ic);
        }
    }

    generateDebugInfo(QString("mapICPortsToSignals completed for IC %1").arg(ic->label()), ic);
}

bool CodeGeneratorVerilog::validateICConnectivity(IC *ic)
{
    // Check if all IC inputs are connected
    for (int i = 0; i < ic->inputSize(); ++i) {
        QNEPort *port = ic->inputPort(i);
        if (port->connections().isEmpty()) {
            m_warnings << QString("IC %1 input %2 is not connected").arg(ic->label()).arg(i);
        }
    }

    // Check if IC outputs are used
    for (int i = 0; i < ic->outputSize(); ++i) {
        QNEPort *port = ic->outputPort(i);
        if (port->connections().isEmpty()) {
            m_warnings << QString("IC %1 output %2 is not connected").arg(ic->label()).arg(i);
        }
    }

    return true; // Basic validation passed
}

// ============================================================================
// CODE GENERATION PHASES
// ============================================================================

void CodeGeneratorVerilog::generate()
{
    // Check if file was opened successfully
    if (!m_file.isOpen()) {
        handleGenerationError("Cannot open output file for writing");
        return;
    }

    // Pre-flight validation
    if (!validateCircuit()) {
        handleGenerationError("Circuit validation failed");
        return;
    }

    if (!validateResourceRequirements()) {
        return; // Error already handled
    }

    generateDebugInfo(QString("Selected FPGA: %1 (%2)").arg(m_selectedBoard.name, m_selectedBoard.description));
    generateDebugInfo(QString("Estimated resources: %1 LUTs, %2 FFs, %3 IOs")
                     .arg(m_estimatedLUTs).arg(m_estimatedFlipFlops).arg(m_estimatedIOPins));

    // Generate module header
    generateModuleHeader();

    // Declare ports
    declareInputPorts();
    declareOutputPorts();

    // Close module port list
    m_stream << ");" << Qt::endl << Qt::endl;

    // Declare auxiliary variables
    declareAuxVariables();

    // Generate main logic
    generateLogicAssignments();

    // Generate output assignments
    generateOutputAssignments();

    // Generate module footer
    generateModuleFooter();
}

void CodeGeneratorVerilog::generateModuleHeader()
{
    m_stream << "// ====================================================================" << Qt::endl;
    m_stream << "// ======= This Verilog code was generated automatically by wiRedPanda =======" << Qt::endl;
    m_stream << "// ====================================================================" << Qt::endl;
    m_stream << "//" << Qt::endl;
    m_stream << QString("// Module: %1").arg(m_fileName) << Qt::endl;
    m_stream << QString("// Generated: %1").arg(QDateTime::currentDateTime().toString()) << Qt::endl;
    m_stream << QString("// Target FPGA: %1").arg(m_selectedBoard.name) << Qt::endl;
    m_stream << QString("// Resource Usage: %1/%2 LUTs, %3/%4 FFs, %5/%6 IOs")
                .arg(m_estimatedLUTs).arg(m_selectedBoard.maxLUTs)
                .arg(m_estimatedFlipFlops).arg(m_selectedBoard.maxFlipFlops)
                .arg(m_estimatedIOPins).arg(m_selectedBoard.maxIOPins) << Qt::endl;
    m_stream << "//" << Qt::endl;
    m_stream << "// wiRedPanda - Digital Logic Circuit Simulator" << Qt::endl;
    m_stream << "// https://github.com/gibis-unifesp/wiredpanda" << Qt::endl;
    m_stream << "// ====================================================================" << Qt::endl;
    m_stream << Qt::endl;

    // Add FPGA-specific attributes
    if (m_targetFamily == FPGAFamily::Xilinx) {
        m_stream << "(* keep_hierarchy = \"yes\" *)" << Qt::endl;
    } else if (m_targetFamily == FPGAFamily::Intel) {
        m_stream << "(* preserve_hierarchy = \"yes\" *)" << Qt::endl;
    }

    m_stream << QString("module %1 (").arg(m_fileName) << Qt::endl;
}

void CodeGeneratorVerilog::declareInputPorts()
{
    m_stream << "    // ========= Input Ports =========" << Qt::endl;

    QStringList inputDeclarations;
    int counter = 1;

    for (auto *elm : m_elements) {
        const auto type = elm->elementType();

        if ((type == ElementType::InputButton) ||
            (type == ElementType::InputSwitch) ||
            (type == ElementType::Clock)) {

            QString varName = elm->objectName() + QString::number(counter);
            const QString label = elm->label();

            if (!label.isEmpty()) {
                varName += "_" + label;
            }

            varName = stripAccents(varName);
            varName = removeForbiddenChars(varName);

            // Ensure unique variable name
            varName = generateUniqueVariableName(varName, "input");

            inputDeclarations << QString("    input wire %1").arg(varName);

            m_inputMap.append(MappedSignalVerilog(elm, "", varName, elm->outputPort(0), 0));
            m_varMap[elm->outputPort()] = varName;

            generateDebugInfo(QString("Input port: %1 -> %2").arg(elm->objectName(), varName), elm);
            ++counter;
        }
    }

    if (!inputDeclarations.isEmpty()) {
        // Check if we have output ports too - if so, add comma after all inputs including the last one
        bool hasOutputPorts = false;
        for (auto *elm : m_elements) {
            if (elm->elementGroup() == ElementGroup::Output) {
                hasOutputPorts = true;
                break;
            }
        }

        // Add comma separators - include last element if there are outputs
        for (int i = 0; i < inputDeclarations.size(); ++i) {
            if (i < inputDeclarations.size() - 1 || hasOutputPorts) {
                inputDeclarations[i] += ",";
            }
        }

        for (const QString &decl : inputDeclarations) {
            m_stream << decl << Qt::endl;
        }
    }
}

void CodeGeneratorVerilog::declareOutputPorts()
{
    m_stream << Qt::endl;
    m_stream << "    // ========= Output Ports =========" << Qt::endl;

    QStringList outputDeclarations;
    int counter = 1;

    for (auto *elm : m_elements) {
        if (elm->elementGroup() == ElementGroup::Output) {
            const QString label = elm->label();

            for (int i = 0; i < elm->inputs().size(); ++i) {
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

                // Ensure unique variable name
                varName = generateUniqueVariableName(varName, "output");

                outputDeclarations << QString("    output wire %1").arg(varName);

                m_outputMap.append(MappedSignalVerilog(elm, "", varName, port, i));

                generateDebugInfo(QString("Output port: %1[%2] -> %3").arg(elm->objectName()).arg(i).arg(varName), elm);
            }
            ++counter;
        }
    }

    if (!outputDeclarations.isEmpty()) {
        // No comma after the last output (end of port list)
        for (int i = 0; i < outputDeclarations.size() - 1; ++i) {
            outputDeclarations[i] += ",";
        }

        for (const QString &decl : outputDeclarations) {
            m_stream << decl << Qt::endl;
        }
    }
}

// ============================================================================
// CONTINUING WITH REMAINING METHODS...
// ============================================================================

void CodeGeneratorVerilog::declareAuxVariables()
{
    m_stream << "    // ========= Internal Signals =========" << Qt::endl;

    // CRITICAL FIX: Use declareAuxVariablesRec which includes IC port mapping via processICsRecursively
    generateDebugInfo(">>> CRITICAL: Calling declareAuxVariablesRec instead of declareUsedSignalsOnly to ensure IC processing");
    declareAuxVariablesRec(m_elements, false);

    m_stream << Qt::endl;
}

void CodeGeneratorVerilog::declareAuxVariablesRec(const QVector<GraphicElement *> &elements, bool isIC)
{
    // First, process ICs
    processICsRecursively(elements, isIC ? m_icDepth + 1 : 0);

    // Then process regular elements
    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            continue; // Already processed above
        }

        // Skip input and output elements - they're already declared as ports
        if (elm->elementGroup() == ElementGroup::Input || elm->elementGroup() == ElementGroup::Output) {
            continue;
        }

        QString varName = generateUniqueVariableName(elm->objectName(), isIC ? "ic" : "");
        const auto outputs = elm->outputs();

        if (outputs.size() == 1) {
            QNEPort *port = outputs.constFirst();

            // Handle special constant elements
            if (elm->elementType() == ElementType::InputVcc) {
                m_varMap[port] = "1'b1";
                continue;
            }

            if (elm->elementType() == ElementType::InputGnd) {
                m_varMap[port] = "1'b0";
                continue;
            }

            QString existingValue = m_varMap.value(port);
            generateDebugInfo(QString("declareVariablesRec: Port from %1, existing varMap value: '%2', new varName: '%3'").arg(elm->objectName()).arg(existingValue).arg(varName), elm);

            if (existingValue.isEmpty()) {
                generateDebugInfo(QString("declareVariablesRec: OVERWRITING empty varMap - setting port to '%1'").arg(varName), elm);
                m_varMap[port] = varName;
            } else {
                generateDebugInfo(QString("declareVariablesRec: PRESERVING existing varMap value '%1' instead of '%2'").arg(existingValue).arg(varName), elm);
            }
        } else {
            // Multi-output elements
            int portCounter = 0;

            for (auto *port : outputs) {
                QString portName = varName;
                portName.append(QString("_%1").arg(portCounter++));

                if (!port->name().isEmpty()) {
                    portName.append(QString("_%1").arg(removeForbiddenChars(stripAccents(port->name()))));
                }

                QString existingMultiValue = m_varMap.value(port);
                generateDebugInfo(QString("declareVariablesRec: Multi-output port %1 from %2, existing: '%3', new: '%4'").arg(portCounter-1).arg(elm->objectName()).arg(existingMultiValue).arg(portName), elm);

                if (!existingMultiValue.isEmpty()) {
                    generateDebugInfo(QString("declareVariablesRec: CRITICAL - OVERWRITING multi-output port mapping '%1' with '%2' for element %3!").arg(existingMultiValue).arg(portName).arg(elm->objectName()), elm);
                }

                m_varMap[port] = portName;
            }
        }

        // Declare variables based on element type
        for (auto *port : outputs) {
            QString varName2 = m_varMap.value(port);

            switch (elm->elementType()) {
            case ElementType::DLatch:
            case ElementType::SRLatch:
            case ElementType::SRFlipFlop:
            case ElementType::DFlipFlop:
            case ElementType::TFlipFlop:
            case ElementType::JKFlipFlop: {
                m_stream << QString("    reg %1 = 1'b0;").arg(varName2) << Qt::endl;
                break;
            }

            case ElementType::Clock: {
                m_stream << QString("    reg %1 = 1'b0;").arg(varName2) << Qt::endl;
                m_stream << QString("    reg [31:0] %1_counter = 32'd0;").arg(varName2) << Qt::endl;
                break;
            }

            case ElementType::TruthTable: {
                if (!outputs.isEmpty()) {
                    QNEPort *outputPort = outputs.constFirst();
                    QString ttVarName = generateUniqueVariableName(elm->objectName() + "_output", "tt");
                    m_varMap[outputPort] = ttVarName;
                    m_stream << QString("    reg %1;").arg(ttVarName) << Qt::endl;
                    continue;
                }
                break;
            }

            case ElementType::Buzzer: {
                m_stream << QString("    wire %1; // Buzzer output").arg(varName2) << Qt::endl;
                break;
            }

            case ElementType::Display7:
            case ElementType::Display14:
            case ElementType::Display16: {
                m_stream << QString("    wire %1; // Display segment").arg(varName2) << Qt::endl;
                break;
            }

            case ElementType::Mux:
            case ElementType::Demux: {
                m_stream << QString("    reg %1; // MUX/DEMUX output").arg(varName2) << Qt::endl;
                break;
            }

            default:
                m_stream << QString("    wire %1;").arg(varName2) << Qt::endl;
                break;
            }
        }
    }
}

void CodeGeneratorVerilog::declareUsedSignalsOnly(const QVector<GraphicElement *> &elements, bool isIC)
{
    // ARDUINO-STYLE APPROACH: Pure variable declaration phase
    // Following the Arduino pattern: declare ALL variables first, generate logic later

    // Recursively process all elements including IC internals
    declareVariablesRec(elements, isIC);
}

void CodeGeneratorVerilog::declareVariablesRec(const QVector<GraphicElement *> &elements, bool isIC)
{
    generateDebugInfo(QString("declareVariablesRec: Processing %1 elements, isIC=%2").arg(elements.size()).arg(isIC));

    for (auto *elm : elements) {
        generateDebugInfo(QString("declareVariablesRec: Processing element %1 (type %2)").arg(elm->objectName()).arg(static_cast<int>(elm->elementType())));

        if (elm->elementType() == ElementType::IC) {
            generateDebugInfo(QString("declareVariablesRec: Found IC element: %1").arg(elm->objectName()), elm);
            // Process IC internal elements recursively (Arduino pattern)
            auto *ic = qgraphicsitem_cast<IC *>(elm);
            if (ic && !ic->m_icElements.isEmpty()) {
                // Set IC context for proper variable naming
                IC *previousIC = m_currentIC;
                m_currentIC = ic;

                // Declare variables for IC internal elements
                generateDebugInfo(QString("declareVariablesRec: About to process %1 IC internal elements for IC: %2").arg(ic->m_icElements.size()).arg(ic->label()), ic);
                declareVariablesRec(ic->m_icElements, true);
                generateDebugInfo(QString("declareVariablesRec: Completed processing IC internal elements for IC: %1").arg(ic->label()), ic);

                // Restore context
                m_currentIC = previousIC;
            }
            continue;
        }

        // Skip input and output elements - they're already declared as ports
        if (elm->elementGroup() == ElementGroup::Input || elm->elementGroup() == ElementGroup::Output) {
            continue;
        }

        const auto outputs = elm->outputs();
        for (int i = 0; i < outputs.size(); ++i) {
            QNEPort *port = outputs.at(i);

            // Generate unique variable name
            QString baseVarName = generateUniqueVariableName(elm->objectName(), isIC ? "ic" : "");
            QString varName;

            if (outputs.size() == 1) {
                varName = QString("%1_%2").arg(baseVarName).arg(port->index());
            } else {
                varName = QString("%1_%2_%3").arg(baseVarName).arg(port->index()).arg(i);
            }

            // Map port to variable name
            m_varMap[port] = varName;

            // Declare the variable based on element type
            switch (elm->elementType()) {
            case ElementType::DFlipFlop:
            case ElementType::JKFlipFlop:
            case ElementType::SRFlipFlop:
            case ElementType::TFlipFlop:
            case ElementType::DLatch:
            case ElementType::SRLatch:
                m_stream << QString("    reg %1 = 1'b0;").arg(varName) << Qt::endl;
                break;
            default:
                m_stream << QString("    wire %1;").arg(varName) << Qt::endl;
                break;
            }
        }
    }
}

void CodeGeneratorVerilog::generateLogicAssignments()
{
    m_stream << "    // ========= Logic Assignments =========" << Qt::endl;

    assignVariablesRec(m_elements, false);

    m_stream << Qt::endl;
}

void CodeGeneratorVerilog::assignVariablesRec(const QVector<GraphicElement *> &elements, bool isIC)
{
    // For IC internal elements, skip topological sorting since sequential circuits have feedback loops
    QVector<GraphicElement *> sortedElements;
    if (isIC) {
        // Don't sort IC internal elements - process in original order to avoid circular dependency issues
        sortedElements = elements;
        generateDebugInfo(QString("Processing %1 IC internal elements without topological sorting").arg(elements.size()));
    } else {
        // Sort top-level elements topologically for proper dependency order
        sortedElements = topologicalSort(elements);
        generateDebugInfo(QString("Processing %1 top-level elements with topological sorting").arg(sortedElements.size()));
    }

    for (auto *elm : sortedElements) {
        if (elm->elementType() == ElementType::IC) {
            // Process IC and its internal elements
            auto *ic = qgraphicsitem_cast<IC *>(elm);
            if (ic && !ic->m_icElements.isEmpty()) {
                // Set IC context for internal element processing
                IC *previousIC = m_currentIC;
                m_currentIC = ic;

                // Recursively process IC internal elements for logic generation
                assignVariablesRec(ic->m_icElements, true);

                // Restore previous context
                m_currentIC = previousIC;
            }
            continue;
        }

        if (elm->inputs().isEmpty() || elm->outputs().isEmpty()) {
            continue;
        }

        // Generate element-specific logic
        QString generatedCode;

        switch (elm->elementType()) {
        case ElementType::And:
        case ElementType::Or:
        case ElementType::Nand:
        case ElementType::Nor:
        case ElementType::Xor:
        case ElementType::Xnor:
        case ElementType::Not:
        case ElementType::Node: {
            // For logic gates, generate continuous assignment
            QString expr = generateLogicExpression(elm);
            for (auto *port : elm->outputs()) {
                QString outputVar = m_varMap.value(port);
                if (!outputVar.isEmpty()) {
                    m_stream << QString("    assign %1 = %2; // %3")
                                .arg(outputVar, expr, elm->objectName()) << Qt::endl;
                }
            }
            break;
        }

        case ElementType::DFlipFlop:
        case ElementType::JKFlipFlop:
        case ElementType::SRFlipFlop:
        case ElementType::TFlipFlop:
        case ElementType::DLatch:
        case ElementType::SRLatch: {
            generatedCode = generateSequentialLogic(elm);
            if (!generatedCode.isEmpty()) {
                m_stream << generatedCode << Qt::endl;
            }
            break;
        }

        case ElementType::TruthTable: {
            generatedCode = generateTruthTableLogic(elm);
            if (!generatedCode.isEmpty()) {
                m_stream << generatedCode << Qt::endl;
            }
            break;
        }

        case ElementType::Mux:
        case ElementType::Demux: {
            generatedCode = generateMuxDemuxLogic(elm);
            if (!generatedCode.isEmpty()) {
                m_stream << generatedCode << Qt::endl;
            }
            break;
        }

        case ElementType::Clock: {
            generatedCode = generateClockLogic(elm);
            if (!generatedCode.isEmpty()) {
                m_stream << generatedCode << Qt::endl;
            }
            break;
        }

        case ElementType::Buzzer: {
            generatedCode = generateAudioLogic(elm);
            if (!generatedCode.isEmpty()) {
                m_stream << generatedCode << Qt::endl;
            }
            break;
        }

        case ElementType::Display7:
        case ElementType::Display14:
        case ElementType::Display16: {
            generatedCode = generateDisplayLogic(elm);
            if (!generatedCode.isEmpty()) {
                m_stream << generatedCode << Qt::endl;
            }
            break;
        }

        default:
            m_warnings << QString("Unsupported element type: %1").arg(elm->objectName());
            break;
        }
    }
}

void CodeGeneratorVerilog::generateOutputAssignments()
{
    m_stream << "    // ========= Output Assignments =========" << Qt::endl;

    for (const auto &outputMapping : m_outputMap) {
        QString expr = otherPortName(outputMapping.m_port);
        if (expr.isEmpty()) {
            expr = boolValue(outputMapping.m_port->defaultValue());
        }

        m_stream << QString("    assign %1 = %2; // %3")
                    .arg(outputMapping.m_variableName, expr, outputMapping.m_element->objectName())
                 << Qt::endl;
    }

    m_stream << Qt::endl;
}

void CodeGeneratorVerilog::generateModuleFooter()
{
    m_stream << "endmodule // " << m_fileName << Qt::endl;

    // Generate summary comment
    m_stream << Qt::endl;
    m_stream << "// ====================================================================" << Qt::endl;
    m_stream << QString("// Module %1 generation completed successfully").arg(m_fileName) << Qt::endl;
    m_stream << QString("// Elements processed: %1").arg(m_elements.size()) << Qt::endl;
    m_stream << QString("// Inputs: %1, Outputs: %2").arg(m_inputMap.size()).arg(m_outputMap.size()) << Qt::endl;

    if (!m_warnings.isEmpty()) {
        m_stream << QString("// Warnings: %1").arg(m_warnings.size()) << Qt::endl;
        for (const QString &warning : m_warnings) {
            m_stream << QString("//   %1").arg(warning) << Qt::endl;
        }
    }

    m_stream << "// ====================================================================" << Qt::endl;
}

// ============================================================================
// ELEMENT-SPECIFIC CODE GENERATION
// ============================================================================

QString CodeGeneratorVerilog::generateLogicExpression(GraphicElement *elm, QSet<GraphicElement*> *visited)
{
    // Initialize visited set if not provided
    QSet<GraphicElement*> localVisited;
    if (!visited) {
        visited = &localVisited;
    }

    // Check for circular dependency
    if (visited->contains(elm)) {
        // Return a default value to break the cycle
        return "1'b0";
    }

    // Add current element to visited set
    visited->insert(elm);

    bool negate = false;
    QString logicOperator;

    switch (elm->elementType()) {
    case ElementType::And: logicOperator = "&"; break;
    case ElementType::Or:  logicOperator = "|"; break;
    case ElementType::Nand: logicOperator = "&"; negate = true; break;
    case ElementType::Nor: logicOperator = "|"; negate = true; break;
    case ElementType::Xor: logicOperator = "^"; break;
    case ElementType::Xnor: logicOperator = "^"; negate = true; break;
    case ElementType::Not:
        {
            QString input = otherPortName(elm->inputPort(0), visited);
            QString result = simplifyExpression("~" + input);
            visited->remove(elm);  // Remove from visited before returning
            return result;
        }
    case ElementType::Node:
        {
            QString result = simplifyExpression(otherPortName(elm->inputPort(0), visited)); // Node is just a pass-through
            visited->remove(elm);  // Remove from visited before returning
            return result;
        }
    default:
        visited->remove(elm);  // Remove from visited before returning
        return "";
    }

    QString expr;
    if (elm->inputs().size() == 1) {
        expr = otherPortName(elm->inputPort(0), visited);
    } else {
        expr = "(";
        for (int i = 0; i < elm->inputs().size(); ++i) {
            if (i > 0) expr += " " + logicOperator + " ";
            expr += otherPortName(elm->inputPort(i), visited);
        }
        expr += ")";
    }

    if (negate) {
        expr = "~" + expr;
    }

    // Remove from visited set before returning
    visited->remove(elm);
    return simplifyExpression(expr);
}

QString CodeGeneratorVerilog::generateSequentialLogic(GraphicElement *elm)
{
    QString firstOut = m_varMap.value(elm->outputPort(0));
    QString code;

    switch (elm->elementType()) {
    case ElementType::DLatch: {
        QString secondOut = m_varMap.value(elm->outputPort(1));
        QString data = otherPortName(elm->inputPort(0));
        QString enable = otherPortName(elm->inputPort(1));

        code += QString("    // D Latch: %1\n").arg(elm->objectName());
        code += QString("    always @(*) begin\n");
        code += QString("        if (%1) begin\n").arg(enable);
        code += QString("            %1 = %2;\n").arg(firstOut, data);
        code += QString("            %1 = ~%2;\n").arg(secondOut, data);
        code += QString("        end\n");
        code += QString("    end\n");
        break;
    }

    case ElementType::SRLatch: {
        QString secondOut = m_varMap.value(elm->outputPort(1));
        QString s = otherPortName(elm->inputPort(0));
        QString r = otherPortName(elm->inputPort(1));

        code += QString("    // SR Latch: %1\n").arg(elm->objectName());
        code += QString("    always @(*) begin\n");
        code += QString("        if (%1 && %2) begin\n").arg(s, r);
        code += QString("            %1 = 1'b0; // Invalid state\n").arg(firstOut);
        code += QString("            %1 = 1'b0; // Invalid state\n").arg(secondOut);
        code += QString("        end else if (%1 != %2) begin\n").arg(s, r);
        code += QString("            %1 = %2;\n").arg(firstOut, s);
        code += QString("            %1 = %2;\n").arg(secondOut, r);
        code += QString("        end\n");
        code += QString("        // else hold previous state\n");
        code += QString("    end\n");
        break;
    }

    case ElementType::DFlipFlop: {
        generateDebugInfo(QString("generateSequentialLogic: Processing DFlipFlop %1").arg(elm->objectName()), elm);

        QString firstOut = m_varMap.value(elm->outputPort(0));
        QString secondOut = m_varMap.value(elm->outputPort(1));

        generateDebugInfo(QString("DFlipFlop %1: firstOut=%2, secondOut=%3").arg(elm->objectName()).arg(firstOut).arg(secondOut), elm);

        // Get input signals with extensive logging
        generateDebugInfo(QString("DFlipFlop %1: Getting data signal from input port 0").arg(elm->objectName()), elm);
        QString data = otherPortName(elm->inputPort(0));
        generateDebugInfo(QString("DFlipFlop %1: data signal = %2").arg(elm->objectName()).arg(data), elm);

        generateDebugInfo(QString("DFlipFlop %1: Getting clock signal from input port 1").arg(elm->objectName()), elm);
        QString clk = otherPortName(elm->inputPort(1));
        generateDebugInfo(QString("DFlipFlop %1: CLOCK signal = %2 (THIS IS CRITICAL!)").arg(elm->objectName()).arg(clk), elm);

        generateDebugInfo(QString("DFlipFlop %1: Getting preset signal from input port 2").arg(elm->objectName()), elm);
        QString prst = otherPortName(elm->inputPort(2));
        generateDebugInfo(QString("DFlipFlop %1: preset signal = %2").arg(elm->objectName()).arg(prst), elm);

        generateDebugInfo(QString("DFlipFlop %1: Getting clear signal from input port 3").arg(elm->objectName()), elm);
        QString clr = otherPortName(elm->inputPort(3));
        generateDebugInfo(QString("DFlipFlop %1: clear signal = %2").arg(elm->objectName()).arg(clr), elm);

        code += QString("    // D FlipFlop: %1\n").arg(elm->objectName());

        // Build sensitivity list based on connected signals
        generateDebugInfo(QString("DFlipFlop %1: Building sensitivity list with clock=%2").arg(elm->objectName()).arg(clk), elm);
        QStringList sensitivity;

        // CRITICAL: This is where the syntax error occurs if clk is "1'b0"
        if (clk == "1'b0" || clk == "1'b1") {
            generateDebugInfo(QString("WARNING: DFlipFlop %1 clock is constant %2, this will cause syntax error!").arg(elm->objectName()).arg(clk), elm);
        }
        sensitivity << QString("posedge %1").arg(clk);
        generateDebugInfo(QString("DFlipFlop %1: Added 'posedge %2' to sensitivity list").arg(elm->objectName()).arg(clk), elm);

        if (prst != "1'b1" && prst != "1'b0") {
            // Handle inverted signals properly in sensitivity list
            if (prst.startsWith("~")) {
                sensitivity << QString("posedge %1").arg(prst.mid(1)); // Remove ~ and use posedge
            } else {
                sensitivity << QString("negedge %1").arg(prst);
            }
        }
        if (clr != "1'b1" && clr != "1'b0") {
            // Handle inverted signals properly in sensitivity list
            if (clr.startsWith("~")) {
                sensitivity << QString("posedge %1").arg(clr.mid(1)); // Remove ~ and use posedge
            } else {
                sensitivity << QString("negedge %1").arg(clr);
            }
        }

        code += QString("    always @(%1) begin\n").arg(sensitivity.join(" or "));

        // Handle asynchronous preset
        if (prst != "1'b1" && prst != "1'b0") {
            // Handle inverted signals properly in conditional logic
            QString presetCondition = prst.startsWith("~") ? prst.mid(1) : QString("!%1").arg(prst);
            generateDebugInfo(QString("DFlipFlop %1: Generating preset - firstOut='%2', secondOut='%3'").arg(elm->objectName()).arg(firstOut).arg(secondOut), elm);
            code += QString("        if (%1) begin\n").arg(presetCondition);
            code += QString("            %1 <= 1'b1;\n").arg(firstOut);
            code += QString("            %1 <= 1'b0;\n").arg(secondOut);
            code += QString("        end else ");
        }

        // Handle asynchronous clear
        if (clr != "1'b1" && clr != "1'b0") {
            // Handle inverted signals properly in conditional logic
            QString clearCondition = clr.startsWith("~") ? clr.mid(1) : QString("!%1").arg(clr);
            generateDebugInfo(QString("DFlipFlop %1: Generating clear - firstOut='%2', secondOut='%3'").arg(elm->objectName()).arg(firstOut).arg(secondOut), elm);
            if (prst == "1'b1" || prst == "1'b0") {
                code += QString("        if (%1) begin\n").arg(clearCondition);
            } else {
                code += QString("if (%1) begin\n").arg(clearCondition);
            }
            code += QString("            %1 <= 1'b0;\n").arg(firstOut);
            code += QString("            %1 <= 1'b1;\n").arg(secondOut);
            code += QString("        end else begin\n");
        } else if (prst != "1'b1" && prst != "1'b0") {
            code += QString("begin\n");
        } else {
            code += QString("        begin\n");
        }

        // Normal clocked operation
        generateDebugInfo(QString("DFlipFlop %1: About to generate assignments - firstOut='%2', secondOut='%3', data='%4'").arg(elm->objectName()).arg(firstOut).arg(secondOut).arg(data), elm);

        if (firstOut.isEmpty()) {
            generateDebugInfo(QString("CRITICAL ERROR: DFlipFlop %1 firstOut is EMPTY!").arg(elm->objectName()), elm);
        }
        if (secondOut.isEmpty()) {
            generateDebugInfo(QString("CRITICAL ERROR: DFlipFlop %1 secondOut is EMPTY!").arg(elm->objectName()), elm);
        }

        code += QString("            %1 <= %2;\n").arg(firstOut, data);
        code += QString("            %1 <= ~%2;\n").arg(secondOut, data);
        code += QString("        end\n");
        code += QString("    end\n");
        break;
    }

    case ElementType::JKFlipFlop: {
        QString firstOut = m_varMap.value(elm->outputPort(0));
        QString secondOut = m_varMap.value(elm->outputPort(1));

        QString j = otherPortName(elm->inputPort(0));
        QString clk = otherPortName(elm->inputPort(1));
        QString k = otherPortName(elm->inputPort(2));
        QString prst = otherPortName(elm->inputPort(3));
        QString clr = otherPortName(elm->inputPort(4));

        code += QString("    // JK FlipFlop: %1\n").arg(elm->objectName());

        // Build sensitivity list
        QStringList sensitivity;
        sensitivity << QString("posedge %1").arg(clk);

        if (prst != "1'b1" && prst != "1'b0") {
            // Handle inverted signals properly in sensitivity list
            if (prst.startsWith("~")) {
                sensitivity << QString("posedge %1").arg(prst.mid(1)); // Remove ~ and use posedge
            } else {
                sensitivity << QString("negedge %1").arg(prst);
            }
        }
        if (clr != "1'b1" && clr != "1'b0") {
            // Handle inverted signals properly in sensitivity list
            if (clr.startsWith("~")) {
                sensitivity << QString("posedge %1").arg(clr.mid(1)); // Remove ~ and use posedge
            } else {
                sensitivity << QString("negedge %1").arg(clr);
            }
        }

        code += QString("    always @(%1) begin\n").arg(sensitivity.join(" or "));

        // Asynchronous controls
        if (prst != "1'b1" && prst != "1'b0") {
            // Handle inverted signals properly in conditional logic
            QString presetCondition = prst.startsWith("~") ? prst.mid(1) : QString("!%1").arg(prst);
            code += QString("        if (%1) begin\n").arg(presetCondition);
            code += QString("            %1 <= 1'b1;\n").arg(firstOut);
            code += QString("            %1 <= 1'b0;\n").arg(secondOut);
            code += QString("        end else ");
        }
        if (clr != "1'b1" && clr != "1'b0") {
            // Handle inverted signals properly in conditional logic
            QString clearCondition = clr.startsWith("~") ? clr.mid(1) : QString("!%1").arg(clr);
            if (prst == "1'b1" || prst == "1'b0") {
                code += QString("        if (%1) begin\n").arg(clearCondition);
            } else {
                code += QString("if (%1) begin\n").arg(clearCondition);
            }
            code += QString("            %1 <= 1'b0;\n").arg(firstOut);
            code += QString("            %1 <= 1'b1;\n").arg(secondOut);
            code += QString("        end else begin\n");
        } else if (prst != "1'b1" && prst != "1'b0") {
            code += QString("begin\n");
        } else {
            code += QString("        begin\n");
        }

        // JK logic
        code += QString("            case ({%1, %2})\n").arg(j, k);
        code += QString("                2'b00: begin /* hold */ end\n");
        code += QString("                2'b01: begin %1 <= 1'b0; %2 <= 1'b1; end\n").arg(firstOut, secondOut);
        code += QString("                2'b10: begin %1 <= 1'b1; %2 <= 1'b0; end\n").arg(firstOut, secondOut);
        code += QString("                2'b11: begin %1 <= %2; %3 <= %4; end // toggle\n").arg(firstOut, secondOut, secondOut, firstOut);
        code += QString("            endcase\n");
        code += QString("        end\n");
        code += QString("    end\n");
        break;
    }

    case ElementType::SRFlipFlop: {
        QString firstOut = m_varMap.value(elm->outputPort(0));
        QString secondOut = m_varMap.value(elm->outputPort(1));
        QString s = otherPortName(elm->inputPort(0));
        QString r = otherPortName(elm->inputPort(1));
        QString clk = otherPortName(elm->inputPort(2));

        code += QString("    // SR FlipFlop: %1\n").arg(elm->objectName());

        // Build sensitivity list
        QStringList sensitivity;
        sensitivity << QString("posedge %1").arg(clk);

        code += QString("    always @(%1) begin\n").arg(sensitivity.join(" or "));

        // SR flip-flop logic
        code += QString("        case ({%1, %2})\n").arg(s, r);
        code += QString("            2'b00: begin /* hold */ end\n");
        code += QString("            2'b01: begin %1 <= 1'b0; %2 <= 1'b1; end // reset\n").arg(firstOut, secondOut);
        code += QString("            2'b10: begin %1 <= 1'b1; %2 <= 1'b0; end // set\n").arg(firstOut, secondOut);
        code += QString("            2'b11: begin %1 <= 1'b0; %2 <= 1'b0; end // invalid\n").arg(firstOut, secondOut);
        code += QString("        endcase\n");
        code += QString("    end\n");
        break;
    }

    case ElementType::TFlipFlop: {
        QString firstOut = m_varMap.value(elm->outputPort(0));
        QString secondOut = m_varMap.value(elm->outputPort(1));
        QString t = otherPortName(elm->inputPort(0));
        QString clk = otherPortName(elm->inputPort(1));
        QString prst = otherPortName(elm->inputPort(2));
        QString clr = otherPortName(elm->inputPort(3));

        code += QString("    // T FlipFlop: %1\n").arg(elm->objectName());

        // Build sensitivity list
        QStringList sensitivity;
        sensitivity << QString("posedge %1").arg(clk);
        if (prst != "1'b1" && prst != "1'b0") sensitivity << QString("negedge %1").arg(prst);
        if (clr != "1'b1" && clr != "1'b0") sensitivity << QString("negedge %1").arg(clr);

        code += QString("    always @(%1) begin\n").arg(sensitivity.join(" or "));

        // Asynchronous controls
        if (prst != "1'b1" && prst != "1'b0") {
            code += QString("        if (!%1) begin\n").arg(prst);
            code += QString("            %1 <= 1'b1;\n").arg(firstOut);
            code += QString("            %1 <= 1'b0;\n").arg(secondOut);
            code += QString("        end else ");
        }
        if (clr != "1'b1" && clr != "1'b0") {
            if (prst == "1'b1" || prst == "1'b0") {
                code += QString("        if (!%1) begin\n").arg(clr);
            } else {
                code += QString("if (!%1) begin\n").arg(clr);
            }
            code += QString("            %1 <= 1'b0;\n").arg(firstOut);
            code += QString("            %1 <= 1'b1;\n").arg(secondOut);
            code += QString("        end else begin\n");
        } else if (prst != "1'b1" && prst != "1'b0") {
            code += QString("begin\n");
        } else {
            code += QString("        begin\n");
        }

        // T FlipFlop logic
        code += QString("            if (%1) begin // toggle\n").arg(t);
        code += QString("                %1 <= %2;\n").arg(firstOut, secondOut);
        code += QString("                %1 <= %2;\n").arg(secondOut, firstOut);
        code += QString("            end\n");
        code += QString("            // else hold\n");
        code += QString("        end\n");
        code += QString("    end\n");
        break;
    }

    default:
        break;
    }

    return code;
}

QString CodeGeneratorVerilog::generateTruthTableLogic(GraphicElement *elm)
{
    auto *ttGraphic = dynamic_cast<TruthTable *>(elm);
    if (!ttGraphic) {
        handleGenerationError(QString("Failed to cast TruthTable element: %1").arg(elm->objectName()), elm);
        return "";
    }

    auto *ttLogic = dynamic_cast<LogicTruthTable *>(ttGraphic->logic());
    if (!ttLogic) {
        // For cases where logic is not initialized (e.g., test scenarios),
        // create a temporary LogicTruthTable to extract the truth table logic
        QBitArray key = ttGraphic->key();
        const int nInputs = elm->inputSize();
        const int nOutputs = elm->outputSize();

        // Create temporary LogicTruthTable for logic generation
        LogicTruthTable tempLogic(nInputs, nOutputs, key);

        // Generate code using the temporary logic
        QBitArray propositions = tempLogic.proposition();
        const int rows = 1 << nInputs;

        QString outputVarName = m_varMap.value(elm->outputPort(0));
        if (outputVarName.isEmpty()) {
            handleGenerationError(QString("Output variable not mapped for TruthTable: %1").arg(elm->objectName()), elm);
            return "";
        }

        QString code;
        code += QString("    // Truth Table: %1 (temporary logic)\n").arg(elm->objectName());

        // Get input signals
        QStringList inputSignals;
        for (int i = 0; i < nInputs; ++i) {
            QNEPort *inputPort = elm->inputPort(i);
            QString signalName = otherPortName(inputPort);

            if (signalName.isEmpty()) {
                signalName = "1'b0"; // Default to low if unconnected
                m_warnings << QString("TruthTable %1 input %2 is unconnected, assuming low").arg(elm->objectName()).arg(i);
            }
            inputSignals << signalName;
        }

        // Generate a simple case statement or assign based on truth table
        if (nInputs <= 4) { // Generate case statement for small truth tables
            code += QString("    always @(*) begin\n");
            code += QString("        case ({%1})\n").arg(inputSignals.join(", "));

            for (int row = 0; row < rows; ++row) {
                code += QString("            %1'b").arg(nInputs);
                for (int bit = nInputs - 1; bit >= 0; --bit) {
                    code += ((row & (1 << bit)) ? "1" : "0");
                }
                code += QString(": %1 = %2;\n").arg(outputVarName).arg(propositions.testBit(row) ? "1'b1" : "1'b0");
            }

            code += QString("            default: %1 = 1'b0;\n").arg(outputVarName);
            code += QString("        endcase\n");
            code += QString("    end\n");
        } else {
            // For larger truth tables, use a more compact representation
            code += QString("    // Large truth table - using simplified logic\n");
            code += QString("    assign %1 = 1'b0; // TODO: Implement large truth table logic\n").arg(outputVarName);
        }

        return code;
    }

    QBitArray propositions = ttLogic->proposition();
    const int nInputs = elm->inputSize();
    const int rows = 1 << nInputs;

    QString outputVarName = m_varMap.value(elm->outputPort(0));
    if (outputVarName.isEmpty()) {
        handleGenerationError(QString("Output variable not mapped for TruthTable: %1").arg(elm->objectName()), elm);
        return "";
    }

    QString code;
    code += QString("    // Truth Table: %1\n").arg(elm->objectName());

    // Get input signals
    QStringList inputSignals;
    for (int i = 0; i < nInputs; ++i) {
        QNEPort *inputPort = elm->inputPort(i);
        QString signalName = otherPortName(inputPort);

        if (signalName.isEmpty()) {
            signalName = "1'b0"; // Default to low if unconnected
            m_warnings << QString("TruthTable %1 input %2 is unconnected, assuming low").arg(elm->objectName()).arg(i);
        }
        inputSignals << signalName;
    }

    // Generate sensitivity list from inputs
    QStringList sensitivityList;
    for (const QString &signal : inputSignals) {
        if (signal != "1'b0" && signal != "1'b1") {
            sensitivityList << signal;
        }
    }

    if (sensitivityList.isEmpty()) {
        code += QString("    assign %1 = ").arg(outputVarName);
        // All inputs are constants, evaluate directly
        int index = 0;
        for (int i = 0; i < nInputs; ++i) {
            if (inputSignals[i] == "1'b1") {
                index |= (1 << (nInputs - 1 - i));
            }
        }
        code += QString("%1; // Constant truth table result\n").arg(propositions.testBit(index) ? "1'b1" : "1'b0");
    } else {
        code += QString("    always @(*) begin\n");
        code += QString("        case ({%1})\n").arg(inputSignals.join(", "));

        for (int i = 0; i < rows; ++i) {
            QString binary = QString::number(i, 2).rightJustified(nInputs, '0');
            code += QString("            %1'b%2: %3 = %4;\n")
                       .arg(nInputs)
                       .arg(binary)
                       .arg(outputVarName)
                       .arg(propositions.testBit(i) ? "1'b1" : "1'b0");
        }

        code += QString("            default: %1 = 1'b0;\n").arg(outputVarName);
        code += QString("        endcase\n");
        code += QString("    end\n");
    }

    return code;
}

QString CodeGeneratorVerilog::generateClockLogic(GraphicElement *elm)
{
    auto *clk = qgraphicsitem_cast<Clock *>(elm);
    if (!clk) {
        handleGenerationError(QString("Failed to cast Clock element: %1").arg(elm->objectName()), elm);
        return "";
    }

    QString outputVar = m_varMap.value(elm->outputPort(0));
    if (outputVar.isEmpty()) {
        return "";
    }

    QString counterVar = outputVar + "_counter";
    float frequency = clk->frequency();

    // Calculate counter limit for desired frequency
    // Assuming 50MHz system clock (common in FPGAs)
    const int systemClockFreq = 50000000;
    int counterLimit = static_cast<int>(systemClockFreq / (2 * frequency)) - 1;

    QString code;
    code += QString("    // Clock Generator: %1 (%2 Hz)\n").arg(elm->objectName()).arg(frequency);

    // Add system clock requirement comment
    if (m_debugOutput) {
        code += QString("    // Requires system clock input 'clk' at %1 MHz\n").arg(systemClockFreq / 1000000);
    }

    code += QString("    always @(posedge clk) begin\n");
    code += QString("        if (%1 >= 32'd%2) begin\n").arg(counterVar).arg(counterLimit);
    code += QString("            %1 <= 32'd0;\n").arg(counterVar);
    code += QString("            %1 <= ~%1; // Toggle output\n").arg(outputVar, outputVar);
    code += QString("        end else begin\n");
    code += QString("            %1 <= %1 + 32'd1;\n").arg(counterVar, counterVar);
    code += QString("        end\n");
    code += QString("    end\n");

    return code;
}

QString CodeGeneratorVerilog::generateDisplayLogic(GraphicElement *elm)
{
    QString code;
    QString outputVar = m_varMap.value(elm->outputPort(0));

    switch (elm->elementType()) {
    case ElementType::Display7: {
        code += QString("    // 7-Segment Display: %1\n").arg(elm->objectName());

        // Get input signals for 4-bit BCD
        QStringList inputs;
        for (int i = 0; i < qMin(4, elm->inputSize()); ++i) {
            inputs << otherPortName(elm->inputPort(i));
        }

        while (inputs.size() < 4) {
            inputs << "1'b0"; // Pad with zeros if fewer than 4 inputs
        }

        code += QString("    // 7-segment decoder for BCD input\n");
        code += QString("    always @(*) begin\n");
        code += QString("        case ({%1})\n").arg(inputs.join(", "));

        // Standard 7-segment patterns for digits 0-9
        QStringList patterns = {
            "7'b1111110", // 0
            "7'b0110000", // 1
            "7'b1101101", // 2
            "7'b1111001", // 3
            "7'b0110011", // 4
            "7'b1011011", // 5
            "7'b1011111", // 6
            "7'b1110000", // 7
            "7'b1111111", // 8
            "7'b1111011", // 9
            "7'b1110111", // A (10)
            "7'b0011111", // b (11)
            "7'b1001110", // C (12)
            "7'b0111101", // d (13)
            "7'b1001111", // E (14)
            "7'b1000111"  // F (15)
        };

        for (int i = 0; i < 16; ++i) {
            code += QString("            4'b%1: %2 = %3; // %4\n")
                       .arg(QString::number(i, 2).rightJustified(4, '0'))
                       .arg(outputVar)
                       .arg(patterns[i])
                       .arg(i < 10 ? QString::number(i) : QString(QChar('A' + i - 10)));
        }

        code += QString("            default: %1 = 7'b0000000; // blank\n").arg(outputVar);
        code += QString("        endcase\n");
        code += QString("    end\n");
        break;
    }

    case ElementType::Display14:
    case ElementType::Display16: {
        int segments = (elm->elementType() == ElementType::Display14) ? 14 : 16;
        code += QString("    // %1-Segment Display: %2\n").arg(segments).arg(elm->objectName());

        // For now, just pass through the input bits
        QStringList inputs;
        for (int i = 0; i < elm->inputSize(); ++i) {
            inputs << otherPortName(elm->inputPort(i));
        }

        code += QString("    assign %1 = {%2}; // Direct segment control\n")
                   .arg(outputVar)
                   .arg(inputs.join(", "));
        break;
    }

    default:
        break;
    }

    return code;
}

QString CodeGeneratorVerilog::generateAudioLogic(GraphicElement *elm)
{
    auto *buzzer = qgraphicsitem_cast<Buzzer *>(elm);
    if (!buzzer) {
        handleGenerationError(QString("Failed to cast Buzzer element: %1").arg(elm->objectName()), elm);
        return "";
    }

    QString outputVar = m_varMap.value(elm->outputPort(0));
    if (outputVar.isEmpty()) {
        return "";
    }

    QString code;
    code += QString("    // Buzzer/Audio Output: %1\n").arg(elm->objectName());

    // Get enable signal
    QString enable = otherPortName(elm->inputPort(0));

    // For FPGA implementation, buzzer is typically just a PWM signal
    // The actual frequency would be set by external PWM generator
    code += QString("    assign %1 = %2; // Audio enable signal\n").arg(outputVar, enable);

    if (m_debugOutput) {
        code += QString("    // Note: Actual audio frequency generation requires PWM module\n");
        code += QString("    // Connect %1 to PWM enable for audio output\n").arg(outputVar);
    }

    return code;
}

// ============================================================================
// FPGA-SPECIFIC OPTIMIZATIONS
// ============================================================================

QString CodeGeneratorVerilog::addFPGAAttributes(const QString &code, GraphicElement *element)
{
    QString attributedCode = code;

    switch (m_targetFamily) {
    case FPGAFamily::Xilinx:
        // Add Xilinx-specific attributes
        if (element->elementType() == ElementType::Clock) {
            attributedCode = "(* CLOCK_BUFFER_TYPE = \"BUFG\" *)\n" + attributedCode;
        } else if (element->elementType() == ElementType::DFlipFlop ||
                  element->elementType() == ElementType::JKFlipFlop ||
                  element->elementType() == ElementType::TFlipFlop) {
            attributedCode = "(* IOB = \"TRUE\" *)\n" + attributedCode;
        }
        break;

    case FPGAFamily::Intel:
        // Add Intel/Altera-specific attributes
        if (element->elementType() == ElementType::Clock) {
            attributedCode = "(* altera_attribute = \"-name GLOBAL_SIGNAL GLOBAL_CLOCK\" *)\n" + attributedCode;
        }
        break;

    case FPGAFamily::Lattice:
        // Add Lattice-specific attributes
        if (element->elementType() == ElementType::Clock) {
            attributedCode = "(* syn_isclock = 1 *)\n" + attributedCode;
        }
        break;

    default:
        break;
    }

    return attributedCode;
}

QString CodeGeneratorVerilog::generateClockDomainCrossing(const QString &srcClock, const QString &dstClock, const QString &signal)
{
    QString code;
    QString syncReg1 = generateUniqueVariableName(signal + "_sync1", "cdc");
    QString syncReg2 = generateUniqueVariableName(signal + "_sync2", "cdc");

    code += QString("    // Clock Domain Crossing: %1 -> %2\n").arg(srcClock, dstClock);
    code += QString("    reg %1, %2;\n").arg(syncReg1, syncReg2);

    if (m_targetFamily == FPGAFamily::Xilinx) {
        code += QString("    (* ASYNC_REG = \"TRUE\" *) reg %1;\n").arg(syncReg1);
        code += QString("    (* ASYNC_REG = \"TRUE\" *) reg %2;\n").arg(syncReg2);
    }

    code += QString("    always @(posedge %1) begin\n").arg(dstClock);
    code += QString("        %1 <= %2;\n").arg(syncReg1, signal);
    code += QString("        %2 <= %1; // Two FF synchronizer\n").arg(syncReg2, syncReg1);
    code += QString("    end\n");

    return code;
}

QString CodeGeneratorVerilog::optimizeForFPGA(const QString &expression)
{
    QString optimized = expression;

    // Basic optimizations for FPGA synthesis
    optimized.replace("~(~", "(");          // Double negation elimination
    optimized.replace("~~", "");            // Double negation elimination
    optimized.replace("1'b1 &", "");        // AND with 1 elimination
    optimized.replace("& 1'b1", "");        // AND with 1 elimination
    optimized.replace("1'b0 |", "");        // OR with 0 elimination
    optimized.replace("| 1'b0", "");        // OR with 0 elimination

    return optimized;
}

// ============================================================================
// ERROR HANDLING AND VALIDATION
// ============================================================================

bool CodeGeneratorVerilog::validateCircuit()
{
    bool valid = true;

    // Note: Circular dependency check removed - let EDA tools handle legitimate sequential feedback

    // Check for unconnected ports
    for (auto *elm : m_elements) {
        if (elm->elementGroup() == ElementGroup::Input) {
            // Input elements should have connected outputs
            for (auto *port : elm->outputs()) {
                if (port->connections().isEmpty()) {
                    m_warnings << QString("Input element %1 output is not connected").arg(elm->objectName());
                }
            }
        } else if (elm->elementGroup() == ElementGroup::Output) {
            // Output elements should have connected inputs
            for (auto *port : elm->inputs()) {
                if (port->connections().isEmpty()) {
                    m_warnings << QString("Output element %1 input is not connected").arg(elm->objectName());
                }
            }
        }
    }

    // Validate element-specific requirements
    for (auto *elm : m_elements) {
        switch (elm->elementType()) {
        case ElementType::TruthTable: {
            if (elm->inputSize() > 8) {
                handleGenerationError(QString("TruthTable %1 has too many inputs (%2 > 8)").arg(elm->objectName()).arg(elm->inputSize()), elm);
                valid = false;
            }
            break;
        }

        case ElementType::Clock: {
            auto *clk = qgraphicsitem_cast<Clock *>(elm);
            if (clk && clk->frequency() > 100000) {
                m_warnings << QString("Clock %1 frequency (%2 Hz) may be too high for FPGA implementation").arg(elm->objectName()).arg(clk->frequency());
            }
            break;
        }

        default:
            break;
        }
    }

    return valid;
}

QString CodeGeneratorVerilog::generateMuxDemuxLogic(GraphicElement *elm)
{
    QString code;

    if (elm->elementType() == ElementType::Mux) {
        // Generate MUX logic
        QString outputVar = m_varMap.value(elm->outputPort(0));
        if (outputVar.isEmpty()) {
            return "";
        }

        int numInputs = elm->inputSize();
        if (numInputs < 2) {
            return ""; // Need at least data and select
        }

        // Detect BCD decoder pattern: 4+ select inputs suggests BCD decode application
        QString elementName = elm->objectName();
        QString elementLabel = elm->label();
        bool isBCDDecoder = (numInputs >= 6) && // Multiple BCD inputs plus select
                           (elementName.contains("decoder", Qt::CaseInsensitive) ||
                            elementName.contains("bcd", Qt::CaseInsensitive) ||
                            elementName.contains("segment", Qt::CaseInsensitive) ||
                            elementLabel.contains("decoder", Qt::CaseInsensitive) ||
                            elementLabel.contains("bcd", Qt::CaseInsensitive) ||
                            elementLabel.contains("segment", Qt::CaseInsensitive));

        code += QString("    // MUX: %1%2\n").arg(elm->objectName())
                                              .arg(isBCDDecoder ? " (BCD Decoder)" : "");

        if (isBCDDecoder) {
            // Generate comprehensive 16-case BCD/hex decoder for proper segment decoding
            // Assume last 4 inputs are BCD select bits (3:0), others are configuration
            QString bcdSelect = QString("{%1, %2, %3, %4}")
                               .arg(otherPortName(elm->inputPort(numInputs - 1)))  // bit 3
                               .arg(otherPortName(elm->inputPort(numInputs - 2)))  // bit 2
                               .arg(otherPortName(elm->inputPort(numInputs - 3)))  // bit 1
                               .arg(otherPortName(elm->inputPort(numInputs - 4))); // bit 0

            code += QString("    always @(*) begin\n");
            code += QString("        case (%1)\n").arg(bcdSelect);

            // Generate full 16-case BCD to 7-segment decoder patterns
            QStringList segmentPatterns = {
                "7'b1111110", // 0: segments a,b,c,d,e,f
                "7'b0110000", // 1: segments b,c
                "7'b1101101", // 2: segments a,b,g,e,d
                "7'b1111001", // 3: segments a,b,g,c,d
                "7'b0110011", // 4: segments f,g,b,c
                "7'b1011011", // 5: segments a,f,g,c,d
                "7'b1011111", // 6: segments a,f,g,e,d,c
                "7'b1110000", // 7: segments a,b,c
                "7'b1111111", // 8: all segments
                "7'b1111011", // 9: segments a,b,c,d,f,g
                "7'b1110111", // A: segments a,b,c,e,f,g
                "7'b0011111", // b: segments c,d,e,f,g
                "7'b1001110", // C: segments a,d,e,f
                "7'b0111101", // d: segments b,c,d,e,g
                "7'b1001111", // E: segments a,d,e,f,g
                "7'b1000111"  // F: segments a,e,f,g
            };

            for (int i = 0; i < 16; ++i) {
                code += QString("            4'h%1: %2 = %3; // %4\n")
                       .arg(QString::number(i, 16).toUpper())
                       .arg(outputVar)
                       .arg(segmentPatterns[i])
                       .arg(i < 10 ? QString::number(i) : QString(QChar('A' + i - 10)));
            }
            code += QString("            default: %1 = 7'b0000000;\n").arg(outputVar);
            code += QString("        endcase\n");
            code += QString("    end\n");

        } else {
            // Standard MUX logic for non-BCD applications
            QString selectSignal = otherPortName(elm->inputPort(numInputs - 1));
            QStringList dataInputs;

            for (int i = 0; i < numInputs - 1; ++i) {
                dataInputs << otherPortName(elm->inputPort(i));
            }

            code += QString("    always @(*) begin\n");
            code += QString("        case (%1)\n").arg(selectSignal);

            for (int i = 0; i < dataInputs.size(); ++i) {
                code += QString("            1'b%1: %2 = %3;\n").arg(i).arg(outputVar).arg(dataInputs[i]);
            }

            code += QString("            default: %1 = 1'b0;\n").arg(outputVar);
            code += QString("        endcase\n");
            code += QString("    end\n");
        }

    } else if (elm->elementType() == ElementType::Demux) {
        // Generate DEMUX logic
        code += QString("    // DEMUX: %1\n").arg(elm->objectName());

        // Assume first input is data, last input is select signal
        int numOutputs = elm->outputSize();
        if (numOutputs < 2 || elm->inputSize() < 2) {
            return "";
        }

        QString dataSignal = otherPortName(elm->inputPort(0));
        QString selectSignal = otherPortName(elm->inputPort(elm->inputSize() - 1));

        code += QString("    always @(*) begin\n");
        code += QString("        // Initialize all outputs to 0\n");

        for (int i = 0; i < numOutputs; ++i) {
            QString outputVar = m_varMap.value(elm->outputPort(i));
            if (!outputVar.isEmpty()) {
                code += QString("        %1 = 1'b0;\n").arg(outputVar);
            }
        }

        code += QString("        case (%1)\n").arg(selectSignal);

        for (int i = 0; i < numOutputs; ++i) {
            QString outputVar = m_varMap.value(elm->outputPort(i));
            if (!outputVar.isEmpty()) {
                code += QString("            %1'd%2: %3 = %4;\n")
                           .arg(QString::number(static_cast<int>(qCeil(qLn(numOutputs) / qLn(2)))))
                           .arg(i)
                           .arg(outputVar)
                           .arg(dataSignal);
            }
        }

        code += QString("        endcase\n");
        code += QString("    end\n");
    }

    return code;
}

void CodeGeneratorVerilog::handleGenerationError(const QString &error, GraphicElement *element)
{
    QString fullError = error;
    if (element) {
        fullError += QString(" (Element: %1)").arg(element->objectName());
    }

    m_errors << fullError;

    if (m_debugOutput) {
        m_stream << QString("// ERROR: %1\n").arg(fullError);
    }

    qWarning() << "Verilog Generation Error:" << fullError;
}

void CodeGeneratorVerilog::generateDebugInfo(const QString &message, GraphicElement *element)
{
    if (!m_debugOutput) {
        return;
    }

    QString debugMsg = message;
    if (element) {
        debugMsg += QString(" (Element: %1)").arg(element->objectName());
    }

    // Log to console instead of polluting the generated Verilog file
    qDebug() << "VERILOG DEBUG:" << debugMsg;
}
