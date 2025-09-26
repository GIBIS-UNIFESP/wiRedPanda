// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "enums.h"

#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QSet>
#include <QTextStream>
#include <QVector>

class GraphicElement;
class IC;
class QNEPort;

/**
 * @brief Target FPGA families and their specific characteristics
 */
enum class FPGAFamily {
    Generic,      // Generic Verilog compatible with most tools
    Xilinx,       // Xilinx-specific optimizations and attributes
    Intel,        // Intel/Altera-specific features
    Lattice,      // Lattice-specific optimizations
    Microsemi     // Microsemi/Microchip-specific features
};

/**
 * @brief FPGA board specifications with resource constraints
 */
struct FPGABoard {
    QString name;
    QString description;
    FPGAFamily family;
    int maxLUTs = 0;
    int maxFlipFlops = 0;
    int maxBRAMs = 0;
    int maxIOPins = 0;
    QStringList availableClocks;

    FPGABoard() = default;
    FPGABoard(const QString &n, const QString &desc, FPGAFamily fam, int luts, int ffs, int brams, int ios, const QStringList &clks = {})
        : name(n), description(desc), family(fam), maxLUTs(luts), maxFlipFlops(ffs), maxBRAMs(brams), maxIOPins(ios), availableClocks(clks) {}

    int maxResources() const { return maxLUTs + maxFlipFlops + maxBRAMs; }
};

/**
 * @brief Mapped signal information for Verilog generation
 */
class MappedSignalVerilog
{
public:
    MappedSignalVerilog() = default;
    MappedSignalVerilog(GraphicElement *elm, const QString &sigName, const QString &varName, QNEPort *port, const int portNumber = 0)
        : m_element(elm)
        , m_port(port)
        , m_signalName(sigName)
        , m_variableName(varName)
        , m_portNumber(portNumber)
    {
    }

    GraphicElement *m_element = nullptr;
    QNEPort *m_port = nullptr;
    QString m_signalName;        // Original signal name
    QString m_variableName;      // Generated variable name
    int m_portNumber = 0;        // Port index for multi-port elements
    int m_bitWidth = 1;          // Signal bit width (for buses)
};

/**
 * @brief Comprehensive Verilog code generator with advanced features
 *
 * This class generates professional-grade Verilog HDL code from wiRedPanda circuit designs,
 * supporting complex hierarchical circuits with nested ICs, advanced sequential logic,
 * and FPGA-specific optimizations.
 *
 * Key Features:
 * - Complete IC processing with recursive hierarchies
 * - Advanced sequential logic (flip-flops, latches, state machines)
 * - Topological sorting for dependency resolution
 * - Multi-FPGA target support with family-specific optimizations
 * - Comprehensive error handling and validation
 * - Memory-optimized variable management
 * - Professional code formatting with IEEE 1364 compliance
 */
class CodeGeneratorVerilog
{
    Q_DECLARE_TR_FUNCTIONS(CodeGeneratorVerilog)

public:
    /**
     * @brief Construct code generator for specified file and circuit elements
     * @param fileName Output Verilog file path
     * @param elements Circuit elements to process
     * @param target Target FPGA family (default: Generic)
     */
    CodeGeneratorVerilog(const QString &fileName, const QVector<GraphicElement *> &elements, FPGAFamily target = FPGAFamily::Generic);

    /**
     * @brief Generate complete Verilog module code
     */
    void generate();

    /**
     * @brief Set target FPGA family for optimizations
     * @param family Target FPGA family
     */
    void setFPGAFamily(FPGAFamily family) { m_targetFamily = family; }

    /**
     * @brief Get currently selected FPGA board
     * @return Selected board specification
     */
    FPGABoard getSelectedBoard() const { return m_selectedBoard; }

    /**
     * @brief Enable/disable debug output generation
     * @param enable True to enable debug comments
     */
    void setDebugOutput(bool enable) { m_debugOutput = enable; }

private:
    // ============================================================================
    // CORE UTILITY METHODS
    // ============================================================================

    /**
     * @brief Remove forbidden characters from Verilog identifiers
     * @param input Input string to clean
     * @return Clean Verilog identifier
     */
    static QString removeForbiddenChars(const QString &input);

    /**
     * @brief Remove accents and diacritics from text
     * @param input Input string with accents
     * @return String without accents
     */
    static QString stripAccents(const QString &input);

    /**
     * @brief Convert status value to Verilog constant
     * @param val Status value to convert
     * @return Verilog bit constant (1'b0 or 1'b1)
     */
    static QString boolValue(const Status val);

    /**
     * @brief Simplify Verilog expressions to prevent double negation
     * @param expr Input expression that may have redundant negations
     * @return Simplified expression
     */
    static QString simplifyExpression(const QString &expr);

    /**
     * @brief Check if identifier is a Verilog reserved keyword
     * @param identifier Identifier to check
     * @return True if identifier is reserved
     */
    static bool isVerilogReservedKeyword(const QString &identifier);

    // ============================================================================
    // FPGA BOARD SELECTION AND VALIDATION
    // ============================================================================

    /**
     * @brief Select optimal FPGA board based on resource requirements
     * @param requiredLUTs Required LUT count
     * @param requiredFFs Required flip-flop count
     * @param requiredIOs Required I/O pins
     * @return Selected board, or invalid board if requirements cannot be met
     */
    FPGABoard selectFPGABoard(int requiredLUTs, int requiredFFs, int requiredIOs);

    /**
     * @brief Validate circuit complexity against target FPGA resources
     * @return True if circuit fits target device
     */
    bool validateResourceRequirements();

    /**
     * @brief Estimate resource usage for circuit elements
     * @param luts Output estimated LUT usage
     * @param ffs Output estimated flip-flop usage
     * @param ios Output estimated I/O usage
     */
    void estimateResourceUsage(int &luts, int &ffs, int &ios);

    // ============================================================================
    // VARIABLE MANAGEMENT AND NAMING
    // ============================================================================

    /**
     * @brief Get variable name for connected port, handling complex expressions
     * @param port Input port to analyze
     * @param visited Set of visited elements to prevent infinite recursion
     * @return Variable name or expression
     */
    QString otherPortName(QNEPort *port, QSet<GraphicElement*> *visited = nullptr);

    /**
     * @brief Generate unique variable name with collision prevention
     * @param baseName Base name for variable
     * @param context Optional context string (IC name, etc.)
     * @return Unique variable name
     */
    QString generateUniqueVariableName(const QString &baseName, const QString &context = QString());

    /**
     * @brief Check if variable name is already declared
     * @param varName Variable name to check
     * @return True if already declared
     */
    bool isVariableDeclared(const QString &varName) const;

    /**
     * @brief Mark variable as declared to prevent collisions
     * @param varName Variable name to mark as declared
     */
    void markVariableDeclared(const QString &varName);

    // ============================================================================
    // TOPOLOGICAL SORTING AND DEPENDENCY RESOLUTION
    // ============================================================================

    /**
     * @brief Sort elements by dependency order using topological sort
     * @param elements Elements to sort
     * @return Topologically sorted elements
     */
    QVector<GraphicElement *> topologicalSort(const QVector<GraphicElement *> &elements);

    /**
     * @brief Detect circular dependencies in circuit
     * @param elements Elements to analyze
     * @return True if circular dependency detected
     */
    bool hasCircularDependency(const QVector<GraphicElement *> &elements);

    /**
     * @brief Get dependency score for element (higher = depends on more elements)
     * @param element Element to analyze
     * @param elements All elements in circuit
     * @return Dependency score
     */
    int getDependencyScore(GraphicElement *element, const QVector<GraphicElement *> &elements);

    // ============================================================================
    // IC PROCESSING PIPELINE
    // ============================================================================

    /**
     * @brief Process IC elements recursively with hierarchy tracking
     * @param elements Elements to process (may include ICs)
     * @param depth Current nesting depth
     */
    void processICsRecursively(const QVector<GraphicElement *> &elements, int depth = 0);

    /**
     * @brief Generate IC boundary comments for code organization
     * @param ic IC element
     * @param isStart True for start marker, false for end marker
     * @return Formatted boundary comment
     */
    QString generateICBoundaryComment(IC *ic, bool isStart);

    /**
     * @brief Map IC external ports to internal signals
     * @param ic IC to process
     */
    void mapICPortsToSignals(IC *ic);

    /**
     * @brief Propagate external signal mapping to all connected Node elements inside IC
     * @param startPort Starting port from IC input
     * @param externalSignal External signal to propagate
     * @param ic IC context
     */
    void propagateSignalToConnectedNodes(QNEPort *startPort, const QString &externalSignal, IC *ic);

    /**
     * @brief Validate IC connectivity and signal flow
     * @param ic IC to validate
     * @return True if IC is properly connected
     */
    bool validateICConnectivity(IC *ic);

    // ============================================================================
    // CODE GENERATION PHASES
    // ============================================================================

    /**
     * @brief Generate module header with ports and parameters
     */
    void generateModuleHeader();

    /**
     * @brief Declare input ports with proper naming
     */
    void declareInputPorts();

    /**
     * @brief Declare output ports with proper naming
     */
    void declareOutputPorts();

    /**
     * @brief Declare auxiliary variables recursively
     * @param elements Elements to process
     * @param isIC True if processing IC internal elements
     */
    void declareAuxVariablesRec(const QVector<GraphicElement *> &elements, bool isIC = false);

    /**
     * @brief Declare only internal signals that will actually be used (OPTIMIZATION)
     * @param elements Elements to process for signal declarations
     * @param isIC True if processing IC internal elements
     */
    void declareUsedSignalsOnly(const QVector<GraphicElement *> &elements, bool isIC = false);

    /**
     * @brief Recursively declare variables for elements (Arduino-style approach)
     * @param elements Elements to process for variable declarations
     * @param isIC True if processing IC internal elements
     */
    void declareVariablesRec(const QVector<GraphicElement *> &elements, bool isIC = false);

    /**
     * @brief Declare all auxiliary variables and wires
     */
    void declareAuxVariables();

    /**
     * @brief Generate assignments for variables recursively
     * @param elements Elements to process
     * @param isIC True if processing IC internal elements
     */
    void assignVariablesRec(const QVector<GraphicElement *> &elements, bool isIC = false);

    /**
     * @brief Generate main logic assignment section
     */
    void generateLogicAssignments();

    /**
     * @brief Generate output port assignments
     */
    void generateOutputAssignments();

    /**
     * @brief Generate module footer and cleanup
     */
    void generateModuleFooter();

    // ============================================================================
    // ELEMENT-SPECIFIC CODE GENERATION
    // ============================================================================

    /**
     * @brief Generate logic expression for combinational elements
     * @param elm Element to process
     * @param visited Set of visited elements to prevent infinite recursion
     * @return Verilog expression string
     */
    QString generateLogicExpression(GraphicElement *elm, QSet<GraphicElement*> *visited = nullptr);

    /**
     * @brief Generate sequential logic for flip-flops and latches
     * @param elm Sequential element
     * @return Generated Verilog code block
     */
    QString generateSequentialLogic(GraphicElement *elm);

    /**
     * @brief Generate truth table logic as case statement
     * @param elm Truth table element
     * @return Generated Verilog case statement
     */
    QString generateTruthTableLogic(GraphicElement *elm);

    /**
     * @brief Generate clock domain logic
     * @param elm Clock element
     * @return Generated clock logic
     */
    QString generateClockLogic(GraphicElement *elm);

    /**
     * @brief Generate multiplexer and demultiplexer logic
     * @param elm MUX or DEMUX element
     * @return Generated Verilog logic with case statements
     */
    QString generateMuxDemuxLogic(GraphicElement *elm);

    /**
     * @brief Generate display element logic (7-segment, etc.)
     * @param elm Display element
     * @return Generated display controller logic
     */
    QString generateDisplayLogic(GraphicElement *elm);

    /**
     * @brief Generate audio/buzzer logic (adapted for FPGA)
     * @param elm Buzzer element
     * @return Generated audio logic
     */
    QString generateAudioLogic(GraphicElement *elm);

    /**
     * @brief Check if Node assignment is redundant (no fan-out)
     * @param port Output port of the Node element
     * @param expr Expression that would be assigned
     * @return True if assignment is redundant and should be skipped
     */
    bool isRedundantNodeAssignment(QNEPort *port, const QString &expr);

    /**
     * @brief Predict which signals will actually be used in assignments or expressions
     * @param elements Elements to analyze for signal usage
     * @param usedSignals Output set of ports that will be used
     */
    void predictUsedSignals(const QVector<GraphicElement *> &elements, QSet<QNEPort*> &usedSignals);

    // ============================================================================
    // IC BOUNDARY HANDLING (ARDUINO-STYLE ARCHITECTURE)
    // ============================================================================

    /**
     * @brief Process IC with proper boundary crossing (Arduino-style)
     * @param ic IC element to process
     */
    void processICWithBoundaries(IC *ic);

    /**
     * @brief Map IC inputs: external signals → internal IC input variables
     * @param ic IC element
     */
    void mapICInputBoundaries(IC *ic);

    /**
     * @brief Map IC outputs: internal IC output variables → external signals
     * @param ic IC element
     */
    void mapICOutputBoundaries(IC *ic);

    /**
     * @brief Get IC boundary variable name for internal port
     * @param ic IC context
     * @param port Internal IC port
     * @param isInput True for input boundary, false for output boundary
     * @return Variable name for boundary crossing
     */
    QString getICBoundaryVariable(IC *ic, QNEPort *port, bool isInput);

    // ============================================================================
    // FPGA-SPECIFIC OPTIMIZATIONS
    // ============================================================================

    /**
     * @brief Add FPGA-specific attributes and optimizations
     * @param code Base Verilog code
     * @param element Target element
     * @return Code with FPGA-specific attributes
     */
    QString addFPGAAttributes(const QString &code, GraphicElement *element);

    /**
     * @brief Generate clock domain crossing logic
     * @param srcClock Source clock domain
     * @param dstClock Destination clock domain
     * @param signal Signal to cross
     * @return CDC logic
     */
    QString generateClockDomainCrossing(const QString &srcClock, const QString &dstClock, const QString &signal);

    /**
     * @brief Optimize logic expressions for FPGA synthesis
     * @param expression Input expression
     * @return Optimized expression
     */
    QString optimizeForFPGA(const QString &expression);

    // ============================================================================
    // ERROR HANDLING AND VALIDATION
    // ============================================================================

    /**
     * @brief Validate circuit before generation
     * @return True if circuit is valid for Verilog generation
     */
    bool validateCircuit();

    /**
     * @brief Handle generation errors gracefully
     * @param error Error message
     * @param element Problem element (if applicable)
     */
    void handleGenerationError(const QString &error, GraphicElement *element = nullptr);

    /**
     * @brief Generate diagnostic information for debugging
     * @param message Debug message
     * @param element Related element (if applicable)
     */
    void generateDebugInfo(const QString &message, GraphicElement *element = nullptr);

    // ============================================================================
    // MEMBER VARIABLES
    // ============================================================================

    // Core infrastructure
    QFile m_file;                                           // Output file handle
    QTextStream m_stream;                                   // Output stream
    QString m_fileName;                                     // Base filename for module
    const QVector<GraphicElement *> m_elements;             // Circuit elements to process

    // Variable management
    QHash<QNEPort *, QString> m_varMap;                     // Port to variable mapping
    QSet<QString> m_declaredVariables;                      // Collision prevention
    QVector<MappedSignalVerilog> m_inputMap;                // Input signal mappings
    QVector<MappedSignalVerilog> m_outputMap;               // Output signal mappings
    int m_globalCounter = 1;                                // Global variable counter

    // IC processing state
    IC *m_currentIC = nullptr;                              // Current IC being processed
    int m_icDepth = 0;                                      // Current IC nesting depth
    QVector<IC *> m_icStack;                                // IC processing stack

    // FPGA target configuration
    FPGAFamily m_targetFamily;                              // Target FPGA family
    FPGABoard m_selectedBoard;                              // Selected board specification

    // Generation options
    bool m_debugOutput = false;                             // Enable debug comments
    bool m_optimizeForSpeed = true;                         // Speed vs area optimization
    bool m_generateConstraints = false;                     // Generate constraint files
    bool m_hasCircularLogic = false;                        // Module contains circular/feedback logic
    bool m_circularPragmaOpened = false;                    // Opening pragma has been written

    // Resource tracking
    int m_estimatedLUTs = 0;                                // Estimated LUT usage
    int m_estimatedFlipFlops = 0;                           // Estimated FF usage
    int m_estimatedIOPins = 0;                              // Estimated I/O usage

    // Error handling
    QStringList m_warnings;                                 // Generation warnings
    QStringList m_errors;                                   // Generation errors

    // Circular dependency detection
    QSet<GraphicElement*> m_visitedElements;                // Track visited elements to prevent infinite recursion
};
