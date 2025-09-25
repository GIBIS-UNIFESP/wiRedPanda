// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "enums.h"
#include "codegeneratorverilog.h"

#include <QObject>
#include <QtTest>

class GraphicElement;
class IC;
class Scene;

/**
 * @brief Comprehensive Verilog code generation test framework
 *
 * This test class provides extensive validation of the Verilog code generator,
 * following the same comprehensive approach as the Arduino test framework.
 * It includes five phases of testing from basic functionality to complex
 * real-world scenarios and quality assurance.
 *
 * Test Phases:
 * - Phase 1: Foundation (10 tests) - Basic module structure and port declarations
 * - Phase 2: Core Logic (15 tests) - Logic gates, sequential elements, variables
 * - Phase 3: Advanced Elements (20 tests) - Special elements, complex circuits
 * - Phase 4: Complex Features (25 tests) - IC integration, real-world scenarios
 * - Phase 5: Quality Assurance (30 tests) - Syntax compliance, simulation readiness
 */
class TestVerilog : public QObject
{
    Q_OBJECT

public:
    TestVerilog() = default;

private slots:
    void init();
    void cleanup();

    // ============================================================================
    // PHASE 1: FOUNDATION TESTS (10 tests)
    // ============================================================================

    /**
     * @brief Test basic Verilog module generation and structure
     */
    void testBasicVerilogGeneration();

    /**
     * @brief Test FPGA board selection algorithm
     */
    void testFPGABoardSelection();

    /**
     * @brief Test input port declaration and naming
     */
    void testInputPortDeclaration();

    /**
     * @brief Test output port declaration and naming
     */
    void testOutputPortDeclaration();

    /**
     * @brief Test variable naming and collision prevention
     */
    void testVariableNaming();

    /**
     * @brief Test resource usage estimation
     */
    void testResourceEstimation();

    /**
     * @brief Test module header and footer generation
     */
    void testModuleStructure();

    /**
     * @brief Test error handling for invalid circuits
     */
    void testErrorHandling();

    /**
     * @brief Test debug output generation
     */
    void testDebugOutput();

    /**
     * @brief Test file I/O operations
     */
    void testFileOperations();

    // ============================================================================
    // PHASE 2: CORE LOGIC TESTS (15 tests)
    // ============================================================================

    /**
     * @brief Test basic logic gate code generation (AND, OR, NOT)
     */
    void testBasicLogicGates();

    /**
     * @brief Test complex logic gates (NAND, NOR, XOR, XNOR)
     */
    void testComplexLogicGates();

    /**
     * @brief Test multi-input logic gate handling
     */
    void testMultiInputGates();

    /**
     * @brief Test node element routing
     */
    void testNodeElements();

    /**
     * @brief Test VCC and GND constant elements
     */
    void testConstantElements();

    /**
     * @brief Test logic expression optimization
     */
    void testLogicOptimization();

    /**
     * @brief Test topological sorting
     */
    void testTopologicalSorting();

    /**
     * @brief Test circular dependency detection
     */
    void testCircularDependencyDetection();

    /**
     * @brief Test variable mapping consistency
     */
    void testVariableMapping();

    /**
     * @brief Test input/output connection validation
     */
    void testConnectionValidation();

    /**
     * @brief Test multi-element circuit generation
     */
    void testMultiElementCircuits();

    /**
     * @brief Test signal propagation through complex paths
     */
    void testSignalPropagation();

    /**
     * @brief Test continuous assignment generation
     */
    void testContinuousAssignments();

    /**
     * @brief Test wire declaration and management
     */
    void testWireDeclarations();

    /**
     * @brief Test port name sanitization
     */
    void testPortNameSanitization();

    // ============================================================================
    // PHASE 3: ADVANCED ELEMENTS TESTS (20 tests)
    // ============================================================================

    /**
     * @brief Test D flip-flop code generation
     */
    void testDFlipFlop();

    /**
     * @brief Test JK flip-flop code generation
     */
    void testJKFlipFlop();

    /**
     * @brief Test SR flip-flop code generation
     */
    void testSRFlipFlop();

    /**
     * @brief Test T flip-flop code generation
     */
    void testTFlipFlop();

    /**
     * @brief Test D latch code generation
     */
    void testDLatch();

    /**
     * @brief Test SR latch code generation
     */
    void testSRLatch();

    /**
     * @brief Test preset and clear signal handling
     */
    void testPresetClearLogic();

    /**
     * @brief Test clock domain generation
     */
    void testClockGeneration();

    /**
     * @brief Test clock frequency scaling
     */
    void testClockFrequencyScaling();

    /**
     * @brief Test truth table code generation
     */
    void testTruthTableGeneration();

    /**
     * @brief Test complex truth table scenarios
     */
    void testComplexTruthTables();

    /**
     * @brief Test display element code generation (7-segment)
     */
    void testDisplayElements();

    /**
     * @brief Test multi-segment display handling
     */
    void testMultiSegmentDisplays();

    /**
     * @brief Test buzzer/audio element adaptation
     */
    void testAudioElements();

    /**
     * @brief Test multiplexer and demultiplexer elements
     */
    void testMuxDemuxElements();

    /**
     * @brief Test sequential logic timing
     */
    void testSequentialTiming();

    /**
     * @brief Test always block generation
     */
    void testAlwaysBlocks();

    /**
     * @brief Test case statement generation
     */
    void testCaseStatements();

    /**
     * @brief Test sensitivity list generation
     */
    void testSensitivityLists();

    /**
     * @brief Test register vs wire declarations
     */
    void testRegisterWireDeclarations();

    // ============================================================================
    // PHASE 4: COMPLEX FEATURES TESTS (25 tests)
    // ============================================================================

    /**
     * @brief Test simple IC code generation
     */
    void testSimpleICGeneration();

    /**
     * @brief Test nested IC handling
     */
    void testNestedICHandling();

    /**
     * @brief Test IC port mapping
     */
    void testICPortMapping();

    /**
     * @brief Test IC boundary comment generation
     */
    void testICBoundaryComments();

    /**
     * @brief Test IC variable scoping
     */
    void testICVariableScoping();

    /**
     * @brief Test complex hierarchical circuits
     */
    void testHierarchicalCircuits();

    /**
     * @brief Test real-world circuit: binary counter
     */
    void testBinaryCounter();

    /**
     * @brief Test real-world circuit: state machine
     */
    void testStateMachine();

    /**
     * @brief Test real-world circuit: arithmetic logic unit
     */
    void testArithmeticLogicUnit();

    /**
     * @brief Test real-world circuit: memory controller
     */
    void testMemoryController();

    /**
     * @brief Test FPGA-specific optimizations
     */
    void testFPGAOptimizations();

    /**
     * @brief Test Xilinx-specific attributes
     */
    void testXilinxAttributes();

    /**
     * @brief Test Intel/Altera-specific attributes
     */
    void testIntelAttributes();

    /**
     * @brief Test Lattice-specific attributes
     */
    void testLatticeAttributes();

    /**
     * @brief Test clock domain crossing generation
     */
    void testClockDomainCrossing();

    /**
     * @brief Test resource constraint validation
     */
    void testResourceConstraints();

    /**
     * @brief Test large circuit scalability
     */
    void testLargeCircuitScalability();

    /**
     * @brief Test performance with 100+ elements
     */
    void testPerformanceScaling();

    /**
     * @brief Test edge case handling
     */
    void testEdgeCases();

    /**
     * @brief Test disconnected element handling
     */
    void testDisconnectedElements();

    /**
     * @brief Test invalid circuit detection
     */
    void testInvalidCircuits();

    /**
     * @brief Test warning generation and reporting
     */
    void testWarningGeneration();

    /**
     * @brief Test pin allocation and management
     */
    void testPinAllocation();

    /**
     * @brief Test signal name conflict resolution
     */
    void testSignalNameConflicts();

    /**
     * @brief Test multi-clock domain circuits
     */
    void testMultiClockDomains();

    // ============================================================================
    // PHASE 5: QUALITY ASSURANCE TESTS (30 tests)
    // ============================================================================

    /**
     * @brief Test Verilog syntax compliance
     */
    void testVerilogSyntaxCompliance();

    /**
     * @brief Test IEEE 1364 standard compliance
     */
    void testIEEEStandardCompliance();

    /**
     * @brief Test simulation tool compatibility
     */
    void testSimulationCompatibility();

    /**
     * @brief Test synthesis tool compatibility
     */
    void testSynthesisCompatibility();

    /**
     * @brief Test code formatting and style
     */
    void testCodeFormatting();

    /**
     * @brief Test comment generation quality
     */
    void testCommentGeneration();

    /**
     * @brief Test identifier naming conventions
     */
    void testNamingConventions();

    /**
     * @brief Test module parameterization
     */
    void testModuleParameterization();

    /**
     * @brief Test port width handling
     */
    void testPortWidthHandling();

    /**
     * @brief Test bus signal support
     */
    void testBusSignals();

    /**
     * @brief Test timing constraint generation
     */
    void testTimingConstraints();

    /**
     * @brief Test synthesis attribute validation
     */
    void testSynthesisAttributes();

    /**
     * @brief Test simulation directive handling
     */
    void testSimulationDirectives();

    /**
     * @brief Test testbench generation capability
     */
    void testTestbenchGeneration();

    /**
     * @brief Test constraint file generation
     */
    void testConstraintFileGeneration();

    /**
     * @brief Test code optimization effectiveness
     */
    void testCodeOptimization();

    /**
     * @brief Test memory usage efficiency
     */
    void testMemoryUsageEfficiency();

    /**
     * @brief Test generation speed performance
     */
    void testGenerationSpeed();

    /**
     * @brief Test output file size optimization
     */
    void testFileSizeOptimization();

    /**
     * @brief Test error recovery mechanisms
     */
    void testErrorRecovery();

    /**
     * @brief Test partial generation handling
     */
    void testPartialGeneration();

    /**
     * @brief Test concurrent generation safety
     */
    void testConcurrentGeneration();

    /**
     * @brief Test platform compatibility
     */
    void testPlatformCompatibility();

    /**
     * @brief Test internationalization support
     */
    void testInternationalizationSupport();

    /**
     * @brief Test accessibility features
     */
    void testAccessibilityFeatures();

    /**
     * @brief Test documentation generation
     */
    void testDocumentationGeneration();

    /**
     * @brief Test version compatibility
     */
    void testVersionCompatibility();

    /**
     * @brief Test backwards compatibility
     */
    void testBackwardsCompatibility();

    /**
     * @brief Test future extensibility
     */
    void testFutureExtensibility();

    /**
     * @brief Test comprehensive integration
     */
    void testComprehensiveIntegration();

private:
    // ============================================================================
    // HELPER METHODS
    // ============================================================================

    /**
     * @brief Create a test scene with specified elements
     * @param elements Vector of elements to add to scene
     * @return Configured test scene
     */
    Scene *createTestScene(const QVector<GraphicElement *> &elements);

    /**
     * @brief Create a logic gate element of specified type
     * @param type Logic gate type
     * @return Created logic gate element
     */
    GraphicElement *createLogicGate(ElementType type);

    /**
     * @brief Create a sequential element of specified type
     * @param type Sequential element type
     * @return Created sequential element
     */
    GraphicElement *createSequentialElement(ElementType type);

    /**
     * @brief Create an input element
     * @param type Input element type (Button, Switch, Clock)
     * @return Created input element
     */
    GraphicElement *createInputElement(ElementType type);

    /**
     * @brief Create an output element
     * @param type Output element type (LED, Display, etc.)
     * @return Created output element
     */
    GraphicElement *createOutputElement(ElementType type);

    /**
     * @brief Create a special element (VCC, GND, Node, TruthTable, etc.)
     * @param type Special element type
     * @return Created special element
     */
    GraphicElement *createSpecialElement(ElementType type);

    /**
     * @brief Create an IC element with specified configuration
     * @param numInputs Number of input ports
     * @param numOutputs Number of output ports
     * @param label IC label
     * @return Created IC element
     */
    IC *createTestIC(int numInputs, int numOutputs, const QString &label = "TestIC");

    /**
     * @brief Connect two elements via their ports
     * @param source Source element
     * @param sourcePort Source port index
     * @param dest Destination element
     * @param destPort Destination port index
     */
    void connectElements(GraphicElement *source, int sourcePort, GraphicElement *dest, int destPort);

    /**
     * @brief Generate Verilog code for test elements
     * @param elements Elements to generate code for
     * @param targetFamily FPGA family target
     * @return Generated Verilog code
     */
    QString generateTestVerilog(const QVector<GraphicElement *> &elements, FPGAFamily targetFamily = FPGAFamily::Generic);

    /**
     * @brief Validate generated Verilog syntax
     * @param code Verilog code to validate
     * @return True if syntax is valid
     */
    bool validateVerilogSyntax(const QString &code);

    /**
     * @brief Extract module declaration from Verilog code
     * @param code Verilog code
     * @return Module declaration string
     */
    QString extractModuleDeclaration(const QString &code);

    /**
     * @brief Extract port declarations from Verilog code
     * @param code Verilog code
     * @return List of port declarations
     */
    QStringList extractPortDeclarations(const QString &code);

    /**
     * @brief Extract wire/reg declarations from Verilog code
     * @param code Verilog code
     * @return List of variable declarations
     */
    QStringList extractVariableDeclarations(const QString &code);

    /**
     * @brief Extract always blocks from Verilog code
     * @param code Verilog code
     * @return List of always blocks
     */
    QStringList extractAlwaysBlocks(const QString &code);

    /**
     * @brief Extract continuous assignments from Verilog code
     * @param code Verilog code
     * @return List of assign statements
     */
    QStringList extractAssignStatements(const QString &code);

    /**
     * @brief Count specific patterns in Verilog code
     * @param code Verilog code
     * @param pattern Pattern to count
     * @return Pattern count
     */
    int countPattern(const QString &code, const QString &pattern);

    /**
     * @brief Check if code contains specific FPGA attributes
     * @param code Verilog code
     * @param family Target FPGA family
     * @return True if attributes are present
     */
    bool containsFPGAAttributes(const QString &code, FPGAFamily family);

    /**
     * @brief Validate code structure and organization
     * @param code Verilog code
     * @return True if structure is valid
     */
    bool validateCodeStructure(const QString &code);

    /**
     * @brief Extract comments from Verilog code
     * @param code Verilog code
     * @return List of comments
     */
    QStringList extractComments(const QString &code);

    /**
     * @brief Measure code complexity metrics
     * @param code Verilog code
     * @return Complexity score
     */
    int measureCodeComplexity(const QString &code);

    /**
     * @brief Test code compilation with external tools (if available)
     * @param code Verilog code
     * @return True if code compiles successfully
     */
    bool testCodeCompilation(const QString &code);

    // ============================================================================
    // VALIDATION HELPER METHODS (Added for test quality improvement)
    // ============================================================================

    /**
     * @brief Validate basic Verilog code structure (module, endmodule, syntax)
     * @param code Generated Verilog code
     * @param testName Name of test for error messages
     */
    void validateBasicVerilogStructure(const QString &code, const QString &testName);

    /**
     * @brief Validate that code contains expected logic patterns
     * @param code Generated Verilog code
     * @param expectedPatterns List of patterns that should be present
     * @param testName Name of test for error messages
     * @param requireAll If true, all patterns must be present; if false, at least one
     */
    void validateLogicPatterns(const QString &code, const QStringList &expectedPatterns,
                              const QString &testName, bool requireAll = false);

    /**
     * @brief Count and validate minimum/maximum occurrences of patterns
     * @param code Generated Verilog code
     * @param pattern Pattern to count
     * @param minCount Minimum expected count (use 0 for no minimum)
     * @param maxCount Maximum expected count (use -1 for no maximum)
     * @param context Description for error messages
     */
    void validatePatternCount(const QString &code, const QString &pattern,
                             int minCount, int maxCount, const QString &context);

    // ============================================================================
    // SEMANTIC CONSTANTS - Replace magic numbers with meaningful thresholds
    // ============================================================================

    // Logic complexity thresholds
    static constexpr int MIN_LOGIC_DEPTH = 3;           // Minimum levels for multi-level logic
    static constexpr int MIN_BASIC_ASSIGNS = 3;         // Basic logic gate assignments
    static constexpr int MIN_COMPLEX_ASSIGNS = 5;       // Complex circuit assignments
    static constexpr int MIN_PROPAGATION_LEVELS = 5;    // Signal propagation chain depth

    // Port and connection thresholds
    static constexpr int MIN_MULTI_ELEMENT_PORTS = 10;  // Multi-element circuit ports
    static constexpr int MIN_PORT_DECLARATIONS = 5;     // Basic port declaration count
    static constexpr int MIN_CONNECTION_PAIRS = 2;      // Valid connection pairs

    // Sequential logic thresholds
    static constexpr int MIN_ALWAYS_BLOCKS = 2;         // Sequential circuit complexity
    static constexpr int MIN_CLOCK_DOMAINS = 2;         // Multi-clock designs

    // Code structure thresholds
    static constexpr int MIN_CODE_LENGTH = 20;          // Non-trivial code generation
    static constexpr int MIN_VARIABLE_DECLARATIONS = 4; // Internal signal complexity

    // Test data management
    Scene *m_scene = nullptr;
    QVector<GraphicElement *> m_testElements;
    QString m_tempDir;
};
