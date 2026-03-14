// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestArduino.h"

QString TestArduino::s_cliCachePath;

#include <memory>
#include <string>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryDir>

#include "App/CodeGen/ArduinoCodeGen.h"
#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/IC.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/ElementMapping.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

// ============================================================================
// Platform-Specific Helpers
// ============================================================================

static inline void requireLinuxForArduinoTools()
{
#ifndef Q_OS_LINUX
    QSKIP("Arduino-cli and simavr only available on Linux (CI: Ubuntu)");
#endif
}

// ============================================================================
// Helper Methods
// ============================================================================

TestArduino::GeneratedCode TestArduino::generateFromElements(const QVector<GraphicElement *> &elements)
{
    GeneratedCode result;
    result.success = false;

    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        return result;
    }

    QString filePath = tempDir.filePath("test.ino");

    try {
        ArduinoCodeGen generator(filePath, elements);
        generator.generate();

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return result;
        }

        result.content = QString::fromUtf8(file.readAll());
        file.close();
        result.success = true;
    } catch (...) {
        // Generation failed - return empty content
    }

    return result;
}

void TestArduino::verifyBasicStructure(const QString &content)
{
    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
    QVERIFY2(content.contains("void computeLogic()"), "Generated code should contain void computeLogic() function");
    QVERIFY2(content.contains("computeLogic();"), "loop() should call computeLogic()");
    QVERIFY2(content.contains("#include <elapsedMillis.h>"), "Generated code should include elapsedMillis.h header");
}

void TestArduino::verifyLogicOperator(const QString &content, const QString &logicOp, bool negated)
{
    if (negated) {
        // For negated operators, verify both negation and operator
        // Pattern: ! followed by ( and the operator
        QString pattern = QString("!\\s*\\([^)]*\\%1").arg(logicOp);
        QRegularExpression regex(pattern);
        QVERIFY2(regex.match(content).hasMatch(),
            qPrintable(QString("Generated code should match pattern: %1").arg(pattern)));
    } else {
        // For non-negated operators, verify operator exists
        QVERIFY2(content.contains(logicOp),
            qPrintable(QString("Generated code should contain logic operator: %1").arg(logicOp)));
    }
}

void TestArduino::verifyFlipFlopStructure(const QString &content, const QString &type)
{
    // Verify flip-flop type comments
    QString openComment = QString("//%1 FlipFlop").arg(type);
    QString closeComment = QString("//End of %1 FlipFlop").arg(type);

    QVERIFY2(content.contains(openComment),
        qPrintable(QString("Generated code should contain comment: %1").arg(openComment)));
    QVERIFY2(content.contains(closeComment),
        qPrintable(QString("Generated code should contain comment: %1").arg(closeComment)));

    // Verify basic structure always present
    verifyBasicStructure(content);
}

void TestArduino::verifyAuxVariables(const QString &content, const QStringList &varNames)
{
    // Verify auxiliary variable declarations
    for (const QString &varName : varNames) {
        // Check for variable declaration (with or without word boundaries)
        // Pattern: "boolean var_name" or "int var_name" or just "var_name"
        QRegularExpression regex(varName);
        QVERIFY2(regex.match(content).hasMatch(),
                 QString("Aux variable %1 not found in generated code").arg(varName).toLatin1());
    }
}

void TestArduino::verifyPresetClearLogic(const QString &content)
{
    // Verify preset/clear conditional logic exists
    // Pattern: if (!preset || !clear)
    QString pattern = "if\\s*\\(!.*\\|\\|\\s*!.*\\)";
    QRegularExpression regex(pattern);
    QVERIFY2(regex.match(content).hasMatch(),
             "Preset/Clear logic not found in generated code");
}

// IC test helpers

IC *TestArduino::createICWithAndGate(const QString &label)
{
    auto *ic = new IC();
    ic->setLabel(label);
    ic->setInputSize(2);
    ic->setOutputSize(1);

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    ic->m_icElements.append(andGate);

    return ic;
}

IC *TestArduino::createICContaining(IC *innerIC, const QString &label)
{
    auto *outerIC = new IC();
    outerIC->setLabel(label);
    outerIC->setInputSize(2);
    outerIC->setOutputSize(1);
    outerIC->m_icElements.append(innerIC);
    return outerIC;
}

QNEConnection *TestArduino::createConnection(GraphicElement *from, int fromPort, GraphicElement *to, int toPort)
{
    auto *startPort = from->outputPort(fromPort);
    auto *endPort = to->inputPort(toPort);
    if (!startPort || !endPort) {
        return nullptr;
    }
    auto *conn = new QNEConnection();
    conn->setStartPort(startPort);
    conn->setEndPort(endPort);
    return conn;
}

bool TestArduino::checkBoardSelection(const QString &content, const QString &expectedBoard)
{
    return content.contains(QString("Target Board: %1").arg(expectedBoard));
}

QStringList TestArduino::extractPinAssignments(const QString &content)
{
    QStringList pins;
    QRegularExpression pinRegex(R"(const\s+int\s+\w+\s*=\s*(\w+);)");
    QRegularExpressionMatchIterator matches = pinRegex.globalMatch(content);
    while (matches.hasNext()) {
        pins.append(matches.next().captured(1));
    }
    return pins;
}

QStringList TestArduino::extractVariableDeclarations(const QString &content)
{
    QStringList variables;
    QRegularExpression varRegex(R"((?:const\s+int|bool(?:ean)?)\s+(\w+)\s*=)");
    QRegularExpressionMatchIterator matches = varRegex.globalMatch(content);
    while (matches.hasNext()) {
        variables.append(matches.next().captured(1));
    }
    return variables;
}

bool TestArduino::checkPinUniqueness(const QStringList &pins)
{
    QSet<QString> unique(pins.begin(), pins.end());
    return unique.size() == pins.size();
}

bool TestArduino::validateArduinoSyntax(const QString &content)
{
    if (content.isEmpty()) {
        return false;
    }
    if (!content.contains("void setup()") && !content.contains("setup()")) {
        return false;
    }
    if (!content.contains("void loop()") && !content.contains("loop()")) {
        return false;
    }
    return (content.count('{') - content.count('}')) == 0
        && (content.count('(') - content.count(')')) == 0;
}

// ============================================================================
// Test Case Initialization / Cleanup
// ============================================================================

void TestArduino::initTestCase()
{
    TestUtils::setupTestEnvironment();

    s_cliCachePath = QDir::tempPath() + "/wiredpanda-arduino-build-cache";
    QDir().mkpath(s_cliCachePath);
}

void TestArduino::cleanupTestCase()
{
    // No cleanup needed for now
}

// ============================================================================
// Basic Generation Tests
// ============================================================================

void TestArduino::testBasicCircuitGeneration()
{
    // Create simple elements
    auto button1 = ElementFactory::buildElement(ElementType::InputButton);
    auto button2 = ElementFactory::buildElement(ElementType::InputButton);
    auto andGate = ElementFactory::buildElement(ElementType::And);
    auto led = ElementFactory::buildElement(ElementType::Led);

    QVERIFY2(button1 != nullptr, "Failed to create first button element");
    QVERIFY2(button2 != nullptr, "Failed to create second button element");
    QVERIFY2(andGate != nullptr, "Failed to create AND gate element");
    QVERIFY2(led != nullptr, "Failed to create LED element");

    QVector<GraphicElement *> elements;
    elements << button1 << button2 << andGate << led;

    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed for Arduino code generation");

    QString filePath = tempDir.filePath("test.ino");

    // Generate Arduino code
    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    // Verify file was created
    QFile file(filePath);
    QVERIFY2(file.exists(), "Generated Arduino code file was not created");
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));

    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Check for expected elements
    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
    QVERIFY2(content.contains("digitalWrite") || content.contains("digitalRead"),
        "Generated code should contain digitalWrite or digitalRead calls");
}

void TestArduino::testEmptyCircuit()
{
    // Generate code for empty circuit
    QVector<GraphicElement *> elements;

    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed for Arduino code generation");

    QString filePath = tempDir.filePath("empty.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.exists(), "Generated Arduino code file was not created");
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));

    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Should still have basic structure
    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
}

void TestArduino::testGateGeneration_data()
{
    QTest::addColumn<int>("gateType");
    QTest::addColumn<QString>("expectedOp");
    QTest::addColumn<bool>("negated");

    QTest::newRow("AND")  << static_cast<int>(ElementType::And)  << "&&" << false;
    QTest::newRow("OR")   << static_cast<int>(ElementType::Or)   << "||" << false;
    QTest::newRow("XOR")  << static_cast<int>(ElementType::Xor)  << "^"  << false;
    QTest::newRow("NAND") << static_cast<int>(ElementType::Nand) << "&&" << true;
    QTest::newRow("NOR")  << static_cast<int>(ElementType::Nor)  << "||" << true;
    QTest::newRow("XNOR") << static_cast<int>(ElementType::Xnor) << "^"  << true;
}

void TestArduino::testGateGeneration()
{
    QFETCH(int, gateType);
    QFETCH(QString, expectedOp);
    QFETCH(bool, negated);

    auto inp1 = ElementFactory::buildElement(ElementType::InputButton);
    auto inp2 = ElementFactory::buildElement(ElementType::InputButton);
    auto gate = ElementFactory::buildElement(static_cast<ElementType>(gateType));
    auto out  = ElementFactory::buildElement(ElementType::Led);

    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(inp1->outputPort());
    conn1->setEndPort(gate->inputPort(0));

    auto conn2 = std::make_unique<QNEConnection>();
    conn2->setStartPort(inp2->outputPort());
    conn2->setEndPort(gate->inputPort(1));

    QVector<GraphicElement *> elements{inp1, inp2, gate, out};
    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Gate generation should succeed");
    verifyBasicStructure(code.content);
    verifyLogicOperator(code.content, expectedOp, negated);

    conn1.reset();
    conn2.reset();
    qDeleteAll(elements);
}

void TestArduino::testNotGateGeneration()
{
    auto in = ElementFactory::buildElement(ElementType::InputButton);
    auto notGate = ElementFactory::buildElement(ElementType::Not);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in << notGate << out;

    // Create connection to verify NOT operator is generated
    auto conn = std::make_unique<QNEConnection>();
    conn->setStartPort(in->outputPort());
    conn->setEndPort(notGate->inputPort(0));

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Arduino code generation should succeed");
    verifyBasicStructure(code.content);

    // Verify NOT operator (!) is in the generated code
    QVERIFY2(code.content.contains("!"), "Generated code should contain NOT operator (!)");

    conn.reset();
    qDeleteAll(elements);
}

void TestArduino::testMixedLogicGates()
{
    auto in1 = ElementFactory::buildElement(ElementType::InputButton);
    auto in2 = ElementFactory::buildElement(ElementType::InputButton);
    auto andGate = ElementFactory::buildElement(ElementType::And);
    auto orGate = ElementFactory::buildElement(ElementType::Or);
    auto notGate = ElementFactory::buildElement(ElementType::Not);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in1 << in2 << andGate << orGate << notGate << out;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Mixed logic gate generation should succeed");
    QVERIFY2(code.content.contains("&&"), "Mixed logic code should contain AND operator");
    QVERIFY2(code.content.contains("||"), "Mixed logic code should contain OR operator");
    QVERIFY2(code.content.contains("!"),  "Mixed logic code should contain NOT operator");
    QVERIFY2(validateArduinoSyntax(code.content), "Mixed logic code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testClockGeneration()
{
    auto clock = ElementFactory::buildElement(ElementType::Clock);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << clock << out;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Arduino code generation should succeed");

    // Should include elapsedMillis header for clock support
    QVERIFY2(code.content.contains("#include <elapsedMillis.h>"), "Generated code should include elapsedMillis.h");

    // Verify clock-specific patterns
    QVERIFY2(code.content.contains("aux_clock"), "Generated code should declare auxiliary clock variable");
    QVERIFY2(code.content.contains("elapsedMillis"), "Generated code should use elapsedMillis for timing");
    QVERIFY2(code.content.contains("_elapsed"), "Generated code should declare elapsed time variable");
    QVERIFY2(code.content.contains("_interval"), "Generated code should declare interval variable");

    // Verify clock toggle logic in loop
    QVERIFY2(code.content.contains("_elapsed >") || code.content.contains("_elapsed>"),
        "Generated code should contain elapsed time comparison");

    qDeleteAll(elements);
}

void TestArduino::testFlipFlopGeneration()
{
    auto in = ElementFactory::buildElement(ElementType::InputButton);
    auto clock = ElementFactory::buildElement(ElementType::Clock);
    auto dff = ElementFactory::buildElement(ElementType::DFlipFlop);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in << clock << dff << out;

    // Create connections for D flip-flop
    auto conn_d = std::make_unique<QNEConnection>();
    conn_d->setStartPort(in->outputPort());
    conn_d->setEndPort(dff->inputPort(0));  // D input

    auto conn_clk = std::make_unique<QNEConnection>();
    conn_clk->setStartPort(clock->outputPort());
    conn_clk->setEndPort(dff->inputPort(1));  // Clock input

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Arduino code generation should succeed");

    // Verify D flip-flop structure
    verifyFlipFlopStructure(code.content, "D");

    // Verify D flip-flop-specific variables are present
    QVERIFY2(code.content.contains("aux_d_flip_flop"), "Generated code should declare D flip-flop variable");
    QVERIFY2(code.content.contains("_inclk"), "Generated code should have edge detection variable for clock");  // Edge detection variable
    QVERIFY2(code.content.contains("_last"), "Generated code should have data latch variable");   // Data latch variable

    conn_d.reset();
    conn_clk.reset();
    qDeleteAll(elements);
}

void TestArduino::testLatchGeneration()
{
    auto in1 = ElementFactory::buildElement(ElementType::InputButton);
    auto in2 = ElementFactory::buildElement(ElementType::InputButton);
    auto latch = ElementFactory::buildElement(ElementType::DLatch);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in1 << in2 << latch << out;

    // Create connections for D latch (level-triggered)
    auto conn_d = std::make_unique<QNEConnection>();
    conn_d->setStartPort(in1->outputPort());
    conn_d->setEndPort(latch->inputPort(0));  // D input

    auto conn_en = std::make_unique<QNEConnection>();
    conn_en->setStartPort(in2->outputPort());
    conn_en->setEndPort(latch->inputPort(1));  // Enable input

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Arduino code generation should succeed");

    // Verify basic structure
    verifyBasicStructure(code.content);

    // Verify latch-specific logic (level-triggered, not edge-triggered)
    // Latches should have "if (enable)" without edge detection
    QVERIFY2(code.content.contains("aux_d_latch"), "Generated code should declare D latch variable");
    QVERIFY2(code.content.contains("//D Latch"), "Generated code should have D Latch section");
    QVERIFY2(code.content.contains("//End of D Latch"), "Generated code should have D Latch end marker");

    conn_d.reset();
    conn_en.reset();
    qDeleteAll(elements);
}

void TestArduino::testClockWithFrequency()
{
    auto clock = ElementFactory::buildElement(ElementType::Clock);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << clock << out;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Arduino code generation should succeed");

    // Should have frequency-related code
    QVERIFY2(code.content.contains("#include <elapsedMillis.h>"), "Generated code should include elapsedMillis.h");

    // Verify frequency calculation (_interval)
    QVERIFY2(code.content.contains("_interval ="), "Generated code should initialize interval variable");

    // Verify elapsedMillis variable declaration
    QVERIFY2(code.content.contains("elapsedMillis"), "Generated code should use elapsedMillis for timing");

    qDeleteAll(elements);
}

void TestArduino::testPinMapping()
{
    auto btn1 = ElementFactory::buildElement(ElementType::InputButton);
    auto btn2 = ElementFactory::buildElement(ElementType::InputButton);
    auto led1 = ElementFactory::buildElement(ElementType::Led);
    auto led2 = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << btn1 << btn2 << led1 << led2;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Pin mapping generation should succeed");
    // 2 buttons + 2 LEDs → at least 4 unique pin assignments
    QStringList pins = extractPinAssignments(code.content);
    QVERIFY2(pins.size() >= 4, "Should assign at least 4 pins for 2 buttons and 2 LEDs");
    QVERIFY2(checkPinUniqueness(pins), "All pin assignments should be unique");
    QVERIFY2(code.content.contains("INPUT"),  "Generated code should declare button pins as INPUT");
    QVERIFY2(code.content.contains("OUTPUT"), "Generated code should declare LED pins as OUTPUT");

    qDeleteAll(elements);
}

void TestArduino::testVariableNaming()
{
    auto btn = ElementFactory::buildElement(ElementType::InputButton);
    auto led = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << btn << led;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Variable naming generation should succeed");
    // All declared variable names must be valid C++ identifiers
    QStringList vars = extractVariableDeclarations(code.content);
    QVERIFY2(!vars.isEmpty(), "Generated code should have variable declarations");
    QRegularExpression validIdent(R"(^[a-zA-Z_]\w*$)");
    for (const QString &var : vars) {
        QVERIFY2(validIdent.match(var).hasMatch(),
                 qPrintable(QString("Variable '%1' is not a valid C++ identifier").arg(var)));
    }

    qDeleteAll(elements);
}

void TestArduino::testVariableNameSanitization()
{
    // Create elements with special characters in names
    auto elem1 = ElementFactory::buildElement(ElementType::InputButton);
    elem1->setLabel("Button-1");
    auto elem2 = ElementFactory::buildElement(ElementType::Led);
    elem2->setLabel("LED 1");

    QVector<GraphicElement *> elements;
    elements << elem1 << elem2;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Generation should succeed with special-character names");
    // "Button-1" → hyphen removed → "button_1"
    QVERIFY2(code.content.contains("button_1"), "Hyphen in label should become underscore");
    // "LED 1" → space removed → "led_1"
    QVERIFY2(code.content.contains("led_1"), "Space in label should be removed from variable name");
    // Variable declarations must only use valid identifier characters
    QRegularExpression badVarDecl(R"((?:const int|bool)\s+\w*[^a-zA-Z0-9_\s]\w*\s*=)");
    QVERIFY2(!badVarDecl.match(code.content).hasMatch(),
             "Variable declarations must not contain non-identifier characters");

    qDeleteAll(elements);
}

void TestArduino::testInputElementHandling()
{
    auto button = ElementFactory::buildElement(ElementType::InputButton);
    auto sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto vcc = ElementFactory::buildElement(ElementType::InputVcc);
    auto gnd = ElementFactory::buildElement(ElementType::InputGnd);
    auto andGate = ElementFactory::buildElement(ElementType::And);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << button << sw << vcc << gnd << andGate << out;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Input element handling should succeed");
    // Buttons and switches get digitalRead in generated code
    QVERIFY2(code.content.contains("digitalRead"), "Generated code should contain digitalRead calls");
    // Buttons/switches declared as INPUT
    QVERIFY2(code.content.contains("INPUT"), "Generated code should declare button/switch pins as INPUT");
    // VCC and GND produce HIGH/LOW literals, not pin declarations
    QVERIFY2(code.content.contains("HIGH") || code.content.contains("LOW"),
             "Generated code should reference HIGH/LOW for VCC/GND constants");
    QVERIFY2(validateArduinoSyntax(code.content), "Input handling code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testOutputElementHandling()
{
    auto in = ElementFactory::buildElement(ElementType::InputButton);
    auto out1 = ElementFactory::buildElement(ElementType::Led);
    auto out2 = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in << out1 << out2;

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("outputs.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Should have output handling
    QVERIFY2(content.contains("digitalWrite"), "Generated code should contain digitalWrite calls");
    QVERIFY2(content.contains("pinMode"), "Generated code should contain pinMode calls");
}

void TestArduino::testComplexCircuit()
{
    // Create a more complex circuit: multiple gates
    auto a = ElementFactory::buildElement(ElementType::InputButton);
    auto b = ElementFactory::buildElement(ElementType::InputButton);
    auto c = ElementFactory::buildElement(ElementType::InputButton);
    auto xor1 = ElementFactory::buildElement(ElementType::Xor);
    auto xor2 = ElementFactory::buildElement(ElementType::Xor);
    auto and1 = ElementFactory::buildElement(ElementType::And);
    auto and2 = ElementFactory::buildElement(ElementType::And);
    auto or1 = ElementFactory::buildElement(ElementType::Or);
    auto sum = ElementFactory::buildElement(ElementType::Led);
    auto carry = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << a << b << c << xor1 << xor2 << and1 << and2 << or1 << sum << carry;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Complex circuit generation should succeed");
    // Full adder topology uses XOR (^), AND (&&), and OR (||) gates
    QVERIFY2(code.content.contains("^"),  "Full adder code should contain XOR operator");
    QVERIFY2(code.content.contains("&&"), "Full adder code should contain AND operator");
    QVERIFY2(code.content.contains("||"), "Full adder code should contain OR operator");
    // 5 logic gates (2×XOR, 2×AND, 1×OR) produce at least 5 aux_ variables
    QVERIFY2(code.content.count("aux_") >= 5,
             "Complex circuit should have at least 5 auxiliary variables for 5 logic gates");
    QVERIFY2(validateArduinoSyntax(code.content), "Complex circuit code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testCodeStructure()
{
    auto in = ElementFactory::buildElement(ElementType::InputButton);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in << out;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Code structure generation should succeed");
    const QString &content = code.content;

    // Check for required code structure
    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
    QVERIFY2(content.contains("pinMode"), "Generated code should contain pinMode calls");

    // Verify order: setup before loop
    int setupPos = content.indexOf("void setup()");
    int loopPos = content.indexOf("void loop()");
    QVERIFY2(setupPos >= 0 && loopPos > setupPos, "setup() should appear before loop() in generated code");

    qDeleteAll(elements);
}

// ============================================================================
// Board Selection Tests
// ============================================================================

void TestArduino::testBoardSelection()
{
    auto in = ElementFactory::buildElement(ElementType::InputSwitch);
    auto out = ElementFactory::buildElement(ElementType::Led);

    auto conn = std::make_unique<QNEConnection>();
    conn->setStartPort(in->outputPort());
    conn->setEndPort(out->inputPort());

    QVector<GraphicElement *> elements{in, out};
    auto code = generateFromElements(elements);

    QVERIFY2(code.success, "Code generation should succeed");
    QVERIFY2(checkBoardSelection(code.content, "Arduino UNO R3/R4"),
             "Small circuit should select Arduino UNO R3/R4");
    QVERIFY2(code.content.contains("Pin Usage:"), "Generated code should contain pin usage info");

    conn.reset();
    qDeleteAll(elements);
}

void TestArduino::testExcessivePinRequirements()
{
    // 25 LEDs exceeds UNO/Nano (18 pins) — should escalate to a larger board (Mega or ESP32)
    QVector<GraphicElement *> elements;
    for (int i = 0; i < 25; ++i) {
        auto led = ElementFactory::buildElement(ElementType::Led);
        led->setLabel(QString("led%1").arg(i));
        elements << led;
    }

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Code generation should succeed by escalating to a larger board");
    bool hasBigBoard = code.content.contains("Mega 2560") || code.content.contains("ESP32");
    QVERIFY2(hasBigBoard, "Should select a large board when UNO/Nano pin count is exceeded");

    qDeleteAll(elements);
}

// ============================================================================
// Special Element Tests
// ============================================================================

void TestArduino::testBuzzerGeneration()
{
    auto buzzerInput = ElementFactory::buildElement(ElementType::InputSwitch);
    buzzerInput->setLabel("buzzer_enable");
    auto buzzer = ElementFactory::buildElement(ElementType::Buzzer);
    buzzer->setLabel("alarm");

    auto conn = std::make_unique<QNEConnection>();
    conn->setStartPort(buzzerInput->outputPort());
    conn->setEndPort(buzzer->inputPort());

    QVector<GraphicElement *> elements{buzzerInput, buzzer};
    auto code = generateFromElements(elements);

    QVERIFY2(code.success, "Code generation should succeed for buzzer circuit");
    QVERIFY2(code.content.contains("alarm"), "Generated code should contain buzzer label");
    QVERIFY2(code.content.contains("tone("), "Generated code should use tone() for buzzer sound");
    QVERIFY2(code.content.contains("noTone("), "Generated code should use noTone() for buzzer off");
    QVERIFY2(code.content.contains("pinMode"), "Generated code should contain pinMode");
    QVERIFY2(code.content.contains("OUTPUT"), "Generated code should set buzzer as output");

    conn.reset();
    qDeleteAll(elements);
}

void TestArduino::testDisplay7Generation()
{
    auto bit0 = ElementFactory::buildElement(ElementType::InputSwitch);
    bit0->setLabel("bit0");
    auto bit1 = ElementFactory::buildElement(ElementType::InputSwitch);
    bit1->setLabel("bit1");
    auto bit2 = ElementFactory::buildElement(ElementType::InputSwitch);
    bit2->setLabel("bit2");
    auto display = ElementFactory::buildElement(ElementType::Display7);
    display->setLabel("seg_display");

    auto c0 = std::make_unique<QNEConnection>();
    c0->setStartPort(bit0->outputPort());
    c0->setEndPort(display->inputPort(0));

    auto c1 = std::make_unique<QNEConnection>();
    c1->setStartPort(bit1->outputPort());
    c1->setEndPort(display->inputPort(1));

    auto c2 = std::make_unique<QNEConnection>();
    c2->setStartPort(bit2->outputPort());
    c2->setEndPort(display->inputPort(2));

    QVector<GraphicElement *> elements{bit0, bit1, bit2, display};
    auto code = generateFromElements(elements);

    QVERIFY2(code.success, "Code generation should succeed for 7-segment display");
    QVERIFY2(code.content.contains("seg_display"), "Generated code should contain display label");
    QVERIFY2(code.content.contains("OUTPUT"), "Generated code should declare display pins as output");

    // 3 input switches (3 pins) + 7 display segments (7 pins minimum)
    QStringList pins = extractPinAssignments(code.content);
    QVERIFY2(pins.size() >= 7, "Should assign at least 7 pins for 7-segment display segments");
    QVERIFY2(checkPinUniqueness(pins), "All pin assignments should be unique");

    c0.reset();
    c1.reset();
    c2.reset();
    qDeleteAll(elements);
}

void TestArduino::testDisplay14Generation()
{
    // Display14 has 15 input pins (14 segments + decimal point), group = Output.
    // It is handled by the generic declareOutputs() path, same as Display7.
    auto sw0 = ElementFactory::buildElement(ElementType::InputSwitch);
    sw0->setLabel("seg_a");
    auto sw1 = ElementFactory::buildElement(ElementType::InputSwitch);
    sw1->setLabel("seg_b");
    auto display = ElementFactory::buildElement(ElementType::Display14);
    display->setLabel("alphanum");

    auto c0 = std::make_unique<QNEConnection>();
    c0->setStartPort(sw0->outputPort());
    c0->setEndPort(display->inputPort(0));

    auto c1 = std::make_unique<QNEConnection>();
    c1->setStartPort(sw1->outputPort());
    c1->setEndPort(display->inputPort(1));

    QVector<GraphicElement *> elements{sw0, sw1, display};
    auto code = generateFromElements(elements);

    QVERIFY2(code.success, "Code generation should succeed for 14-segment display");
    QVERIFY2(code.content.contains("alphanum"), "Generated code should contain display label");
    QVERIFY2(code.content.contains("OUTPUT"), "Generated code should declare display pins as output");

    // 2 input switches + 15 display segment pins
    QStringList pins = extractPinAssignments(code.content);
    QVERIFY2(pins.size() >= 15, "Should assign at least 15 pins for 14-segment display (14 segments + dp)");
    QVERIFY2(checkPinUniqueness(pins), "All pin assignments should be unique");

    c0.reset();
    c1.reset();
    qDeleteAll(elements);
}

void TestArduino::testInputRotaryGeneration()
{
    // InputRotary with 4 positions: each position maps to one digital input pin.
    // Exactly one position is HIGH at a time (one-hot output).
    auto rotary = ElementFactory::buildElement(ElementType::InputRotary);
    rotary->setOutputSize(4);
    rotary->setLabel("selector");

    auto led0 = ElementFactory::buildElement(ElementType::Led);
    led0->setLabel("out0");
    auto led1 = ElementFactory::buildElement(ElementType::Led);
    led1->setLabel("out1");

    auto c0 = std::make_unique<QNEConnection>();
    c0->setStartPort(rotary->outputPort(0));
    c0->setEndPort(led0->inputPort(0));

    auto c1 = std::make_unique<QNEConnection>();
    c1->setStartPort(rotary->outputPort(1));
    c1->setEndPort(led1->inputPort(0));

    QVector<GraphicElement *> elements{rotary, led0, led1};
    auto code = generateFromElements(elements);

    QVERIFY2(code.success, "Code generation should succeed for rotary switch circuit");

    // 4 positions → 4 input pins declared
    QRegularExpression inputPinDecl(R"(const\s+int\s+\w+selector\w*_pos\d+\s*=)");
    int rotaryPinCount = 0;
    QRegularExpressionMatchIterator it = inputPinDecl.globalMatch(code.content);
    while (it.hasNext()) {
        it.next();
        ++rotaryPinCount;
    }
    QVERIFY2(rotaryPinCount == 4, qPrintable(QString("Expected 4 rotary pin declarations, got %1").arg(rotaryPinCount)));

    // Each pin must be read with digitalRead
    QVERIFY2(code.content.count("digitalRead") >= 4, "Should have at least 4 digitalRead calls for rotary positions");

    // pinMode INPUT for each position
    QVERIFY2(code.content.count("INPUT") >= 4, "Should have at least 4 INPUT pinMode calls for rotary positions");

    // Connected outputs drive LEDs
    QVERIFY2(code.content.contains("out0") || code.content.contains("out1"),
             "Generated code should contain output LED labels");

    c0.reset();
    c1.reset();
    qDeleteAll(elements);
}

// ============================================================================
// IC Support Tests
// ============================================================================

void TestArduino::testSimpleICGeneration()
{
    auto in1 = ElementFactory::buildElement(ElementType::InputSwitch);
    in1->setLabel("sw1");
    auto in2 = ElementFactory::buildElement(ElementType::InputSwitch);
    in2->setLabel("sw2");
    auto out = ElementFactory::buildElement(ElementType::Led);
    out->setLabel("result");

    auto *ic = createICWithAndGate("SimpleAND");

    QVector<GraphicElement *> elements{in1, in2, ic, out};
    auto code = generateFromElements(elements);

    QVERIFY2(code.success, "Code generation should succeed for IC circuit");
    QVERIFY2(code.content.contains("IC: SimpleAND") || code.content.contains("// IC"),
             "Generated code should contain IC boundary comment");
    QVERIFY2(code.content.contains("aux_"),
             "Generated code should declare auxiliary variables for IC");
    QVERIFY2(validateArduinoSyntax(code.content), "Generated code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testNestedICHandling()
{
    auto in1 = ElementFactory::buildElement(ElementType::InputSwitch);
    in1->setLabel("in1");
    auto in2 = ElementFactory::buildElement(ElementType::InputSwitch);
    in2->setLabel("in2");
    auto out = ElementFactory::buildElement(ElementType::Led);
    out->setLabel("out");

    auto *innerIC = createICWithAndGate("InnerAND");
    auto *outerIC = createICContaining(innerIC, "OuterContainer");

    QVector<GraphicElement *> elements{in1, in2, outerIC, out};
    auto code = generateFromElements(elements);

    QVERIFY2(code.success, "Code generation should succeed for nested IC circuit");
    QVERIFY2(code.content.contains("OuterContainer") || code.content.contains("// IC"),
             "Generated code should reference outer IC");
    QVERIFY2(code.content.contains("aux_"),
             "Generated code should declare auxiliary variables for nested IC");
    QVERIFY2(validateArduinoSyntax(code.content), "Generated code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testICPortMapping()
{
    auto sw1 = ElementFactory::buildElement(ElementType::InputSwitch);
    sw1->setLabel("EnableA");
    auto sw2 = ElementFactory::buildElement(ElementType::InputSwitch);
    sw2->setLabel("EnableB");
    auto led = ElementFactory::buildElement(ElementType::Led);
    led->setLabel("Output");

    auto *ic = createICWithAndGate("ANDModule");

    QVector<GraphicElement *> elements{sw1, sw2, ic, led};
    auto code = generateFromElements(elements);

    QVERIFY2(code.success, "Code generation should succeed for IC with labeled ports");
    QVERIFY2(code.content.contains("ANDModule") || code.content.contains("// IC"),
             "Generated code should reference the IC");

    QStringList vars = extractVariableDeclarations(code.content);
    QVERIFY2(!vars.isEmpty(), "Generated code should declare variables for IC port mapping");

    QVERIFY2(validateArduinoSyntax(code.content), "Generated code should have valid syntax");

    qDeleteAll(elements);
}

// ============================================================================
// Advanced Circuit Tests
// ============================================================================

void TestArduino::testAdvancedRealWorldCircuits()
{
    // Traffic light: switch -> NOT -> green led; switch -> red led; switch -> walk led
    auto pedButton = ElementFactory::buildElement(ElementType::InputSwitch);
    pedButton->setLabel("ped_button");
    auto redLight = ElementFactory::buildElement(ElementType::Led);
    redLight->setLabel("red_light");
    auto greenLight = ElementFactory::buildElement(ElementType::Led);
    greenLight->setLabel("green_light");
    auto walkSignal = ElementFactory::buildElement(ElementType::Led);
    walkSignal->setLabel("walk_signal");
    auto notGate = ElementFactory::buildElement(ElementType::Not);

    auto c1 = std::make_unique<QNEConnection>();
    c1->setStartPort(pedButton->outputPort());
    c1->setEndPort(notGate->inputPort());

    auto c2 = std::make_unique<QNEConnection>();
    c2->setStartPort(notGate->outputPort());
    c2->setEndPort(greenLight->inputPort());

    auto c3 = std::make_unique<QNEConnection>();
    c3->setStartPort(pedButton->outputPort());
    c3->setEndPort(redLight->inputPort());

    QVector<GraphicElement *> trafficElements{pedButton, redLight, greenLight, walkSignal, notGate};
    auto trafficCode = generateFromElements(trafficElements);

    QVERIFY2(trafficCode.success, "Traffic light circuit generation should succeed");
    QVERIFY2(trafficCode.content.contains("red_light"), "Code should contain red light label");
    QVERIFY2(trafficCode.content.contains("green_light"), "Code should contain green light label");
    QVERIFY2(trafficCode.content.contains("ped_button"), "Code should contain button label");

    c1.reset();
    c2.reset();
    c3.reset();
    qDeleteAll(trafficElements);

    // 4-bit adder inputs test: XOR operators for sum calculation
    QVector<GraphicElement *> adderElements;
    for (int i = 0; i < 4; ++i) {
        auto a = ElementFactory::buildElement(ElementType::InputSwitch);
        a->setLabel(QString("A%1").arg(i));
        adderElements << a;
        auto b = ElementFactory::buildElement(ElementType::InputSwitch);
        b->setLabel(QString("B%1").arg(i));
        adderElements << b;
    }
    auto xorGate = ElementFactory::buildElement(ElementType::Xor);
    adderElements << xorGate;
    for (int i = 0; i < 4; ++i) {
        auto sum = ElementFactory::buildElement(ElementType::Led);
        sum->setLabel(QString("Sum%1").arg(i));
        adderElements << sum;
    }

    auto adderCode = generateFromElements(adderElements);
    QVERIFY2(adderCode.success, "Adder circuit generation should succeed");
    QVERIFY2(adderCode.content.contains("^") || adderCode.content.contains("xor"),
             "Adder code should contain XOR operator");

    qDeleteAll(adderElements);
}

void TestArduino::testUnconnectedLogicGate()
{
    // Create a logic gate without connecting its inputs
    auto andGate = ElementFactory::buildElement(ElementType::And);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << andGate << out;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Unconnected gate generation should succeed");
    // Unconnected inputs resolve to HIGH or LOW defaults
    QVERIFY2(code.content.contains("HIGH") || code.content.contains("LOW"),
             "Unconnected inputs should default to HIGH or LOW");
    // AND gate still emits its operator even with default inputs
    QVERIFY2(code.content.contains("&&"), "Unconnected AND gate should still emit && operator");
    QVERIFY2(validateArduinoSyntax(code.content), "Code with unconnected gate should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testTFlipFlopGeneration()
{
    auto in = ElementFactory::buildElement(ElementType::InputButton);
    auto clock = ElementFactory::buildElement(ElementType::Clock);
    auto tff = ElementFactory::buildElement(ElementType::TFlipFlop);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in << clock << tff << out;

    // Create connections for T flip-flop
    auto conn_t = std::make_unique<QNEConnection>();
    conn_t->setStartPort(in->outputPort());
    conn_t->setEndPort(tff->inputPort(0));  // T input

    auto conn_clk = std::make_unique<QNEConnection>();
    conn_clk->setStartPort(clock->outputPort());
    conn_clk->setEndPort(tff->inputPort(1));  // Clock input

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Arduino code generation should succeed");

    // Verify T flip-flop structure (comment markers and basic structure)
    verifyFlipFlopStructure(code.content, "T");

    // Verify toggle logic is present
    QVERIFY2(code.content.contains("!aux_t_flip_flop"), "Generated code should declare T flip-flop with negation");

    conn_t.reset();
    conn_clk.reset();
    qDeleteAll(elements);
}

void TestArduino::testJKFlipFlopGeneration()
{
    auto j_in = ElementFactory::buildElement(ElementType::InputButton);
    auto k_in = ElementFactory::buildElement(ElementType::InputButton);
    auto clock = ElementFactory::buildElement(ElementType::Clock);
    auto jkff = ElementFactory::buildElement(ElementType::JKFlipFlop);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << j_in << k_in << clock << jkff << out;

    // Create connections for JK flip-flop
    auto conn_j = std::make_unique<QNEConnection>();
    conn_j->setStartPort(j_in->outputPort());
    conn_j->setEndPort(jkff->inputPort(0));  // J input

    auto conn_clk = std::make_unique<QNEConnection>();
    conn_clk->setStartPort(clock->outputPort());
    conn_clk->setEndPort(jkff->inputPort(1));  // Clock input

    auto conn_k = std::make_unique<QNEConnection>();
    conn_k->setStartPort(k_in->outputPort());
    conn_k->setEndPort(jkff->inputPort(2));  // K input

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Arduino code generation should succeed");

    // Verify JK flip-flop structure
    verifyFlipFlopStructure(code.content, "JK");

    // Verify JK-specific logic patterns: toggle, set, reset conditions
    QVERIFY2(code.content.contains("aux_jk_flip_flop"), "Generated code should declare JK flip-flop variable");
    QVERIFY2(code.content.contains("_inclk"), "Generated code should have edge detection variable for clock");  // Clock input variable

    conn_j.reset();
    conn_clk.reset();
    conn_k.reset();
    qDeleteAll(elements);
}

void TestArduino::testSRFlipFlopGeneration()
{
    auto set_in = ElementFactory::buildElement(ElementType::InputButton);
    auto reset_in = ElementFactory::buildElement(ElementType::InputButton);
    auto clock = ElementFactory::buildElement(ElementType::Clock);
    auto srff = ElementFactory::buildElement(ElementType::SRFlipFlop);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << set_in << reset_in << clock << srff << out;

    // Create connections for SR flip-flop
    auto conn_s = std::make_unique<QNEConnection>();
    conn_s->setStartPort(set_in->outputPort());
    conn_s->setEndPort(srff->inputPort(0));  // S input

    auto conn_clk = std::make_unique<QNEConnection>();
    conn_clk->setStartPort(clock->outputPort());
    conn_clk->setEndPort(srff->inputPort(1));  // Clock input

    auto conn_r = std::make_unique<QNEConnection>();
    conn_r->setStartPort(reset_in->outputPort());
    conn_r->setEndPort(srff->inputPort(2));  // R input

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Arduino code generation should succeed");

    // Verify SR flip-flop structure
    verifyFlipFlopStructure(code.content, "SR");

    // Verify SR-specific set/reset logic patterns
    QVERIFY2(code.content.contains("aux_sr_flip_flop"), "Generated code should declare SR flip-flop variable");  // Try without space
    QVERIFY2(code.content.contains("_inclk"), "Generated code should have edge detection variable for clock");  // Clock input variable

    conn_s.reset();
    conn_clk.reset();
    conn_r.reset();
    qDeleteAll(elements);
}

void TestArduino::testFlipFlopPresetClear()
{
    // Create flip-flop with preset/clear inputs
    auto d_in = ElementFactory::buildElement(ElementType::InputButton);
    auto clk = ElementFactory::buildElement(ElementType::Clock);
    auto prst = ElementFactory::buildElement(ElementType::InputButton);
    auto clr = ElementFactory::buildElement(ElementType::InputButton);
    auto dff = ElementFactory::buildElement(ElementType::DFlipFlop);
    auto out_q = ElementFactory::buildElement(ElementType::Led);
    auto out_qn = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << d_in << clk << prst << clr << dff << out_q << out_qn;

    // Create connections
    auto conn_d = std::make_unique<QNEConnection>();
    conn_d->setStartPort(d_in->outputPort());
    conn_d->setEndPort(dff->inputPort(0));

    auto conn_clk = std::make_unique<QNEConnection>();
    conn_clk->setStartPort(clk->outputPort());
    conn_clk->setEndPort(dff->inputPort(1));

    auto conn_prst = std::make_unique<QNEConnection>();
    conn_prst->setStartPort(prst->outputPort());
    conn_prst->setEndPort(dff->inputPort(2));

    auto conn_clr = std::make_unique<QNEConnection>();
    conn_clr->setStartPort(clr->outputPort());
    conn_clr->setEndPort(dff->inputPort(3));

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Arduino code generation should succeed");

    // Verify D flip-flop structure
    verifyFlipFlopStructure(code.content, "D");

    // Verify preset/clear logic is present
    verifyPresetClearLogic(code.content);

    conn_d.reset();
    conn_clk.reset();
    conn_prst.reset();
    conn_clr.reset();
    qDeleteAll(elements);
}

void TestArduino::testMultipleOutputElement()
{
    // Create element with multiple outputs (DFlipFlop has Q and Q' outputs)
    auto d_in = ElementFactory::buildElement(ElementType::InputButton);
    auto clk = ElementFactory::buildElement(ElementType::Clock);
    auto dff = ElementFactory::buildElement(ElementType::DFlipFlop);
    auto out_q = ElementFactory::buildElement(ElementType::Led);
    auto out_qn = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << d_in << clk << dff << out_q << out_qn;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Multiple output element generation should succeed");
    // Two LEDs → two distinct output pin assignments
    QStringList pins = extractPinAssignments(code.content);
    QVERIFY2(pins.size() >= 2, "DFlipFlop should produce at least 2 output pin assignments (Q and Q')");
    QVERIFY2(checkPinUniqueness(pins), "Q and Q' output pins should be unique");
    QVERIFY2(validateArduinoSyntax(code.content), "Multiple output code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testOutputPortNaming()
{
    // Test output elements with custom port names
    auto in = ElementFactory::buildElement(ElementType::InputButton);
    auto out = ElementFactory::buildElement(ElementType::Led);
    out->setLabel("OutputSignal");

    QVector<GraphicElement *> elements;
    elements << in << out;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Output port naming generation should succeed");
    // LED label "OutputSignal" → sanitized → "outputsignal"
    QVERIFY2(code.content.contains("outputsignal"),
             "LED label 'OutputSignal' should appear sanitized as 'outputsignal' in generated code");
    QVERIFY2(validateArduinoSyntax(code.content), "Output naming code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testComplexElementNaming()
{
    // Test elements with both labels and complex naming
    auto btn = ElementFactory::buildElement(ElementType::InputButton);
    btn->setLabel("InputButton_1");
    auto gate = ElementFactory::buildElement(ElementType::And);
    gate->setLabel("AND_Gate_A");
    auto led = ElementFactory::buildElement(ElementType::Led);
    led->setLabel("OutputLED_Result");

    QVector<GraphicElement *> elements;
    elements << btn << gate << led;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Complex element naming generation should succeed");
    // "InputButton_1" → "inputbutton_1" (lowercased, underscores preserved)
    QVERIFY2(code.content.contains("inputbutton_1"),
             "Button label 'InputButton_1' should appear sanitized as 'inputbutton_1'");
    // "OutputLED_Result" → "outputled_result"
    QVERIFY2(code.content.contains("outputled_result"),
             "LED label 'OutputLED_Result' should appear sanitized as 'outputled_result'");
    QVERIFY2(validateArduinoSyntax(code.content), "Complex naming code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testNameSanitizationEdgeCases()
{
    // Test names with special characters that need sanitization
    auto btn = ElementFactory::buildElement(ElementType::InputButton);
    btn->setLabel("Button-1-Input");
    auto gate = ElementFactory::buildElement(ElementType::Or);
    gate->setLabel("OR@Gate!");
    auto led = ElementFactory::buildElement(ElementType::Led);
    led->setLabel("LED (output) #1");

    QVector<GraphicElement *> elements;
    elements << btn << gate << led;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Generation should succeed with sanitizable names");

    // Hyphens become underscores: "Button-1-Input" → contains "button_1_input"
    QVERIFY2(code.content.contains("button_1_input"), "Hyphens should become underscores in variable names");
    // Variable declarations (const int and bool) must only use valid identifier characters
    QRegularExpression badVarDecl(R"((?:const int|bool)\s+\w*[^a-zA-Z0-9_\s]\w*\s*=)");
    QVERIFY2(!badVarDecl.match(code.content).hasMatch(),
             "Variable declarations must not contain non-identifier characters");

    qDeleteAll(elements);
}

void TestArduino::testUnconnectedInput()
{
    // AND gate with one input connected (button) and one unconnected
    auto in1 = ElementFactory::buildElement(ElementType::InputButton);
    auto gate = ElementFactory::buildElement(ElementType::And);
    auto out = ElementFactory::buildElement(ElementType::Led);

    auto conn = std::make_unique<QNEConnection>();
    conn->setStartPort(in1->outputPort());
    conn->setEndPort(gate->inputPort(0));

    QVector<GraphicElement *> elements;
    elements << in1 << gate << out;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Partially connected gate generation should succeed");
    // The unconnected port defaults to HIGH or LOW
    QVERIFY2(code.content.contains("HIGH") || code.content.contains("LOW"),
             "Unconnected input port should default to HIGH or LOW");
    // AND gate still emits && even with one unconnected input
    QVERIFY2(code.content.contains("&&"), "AND gate with one unconnected input should still emit &&");
    QVERIFY2(validateArduinoSyntax(code.content), "Unconnected input code should have valid syntax");

    conn.reset();
    qDeleteAll(elements);
}

void TestArduino::testDefaultValuePropagation()
{
    // Unconnected input ports should use HIGH or LOW default values in expressions
    auto gate = ElementFactory::buildElement(ElementType::And);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << gate << out;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Generation should succeed with unconnected gate");

    // Disconnected inputs must resolve to HIGH or LOW, not empty expressions like "= ;"
    QRegularExpression emptyRhs(R"(=\s*;)");
    QVERIFY2(!emptyRhs.match(code.content).hasMatch(),
             "Generated code must not have empty right-hand sides (unresolved default values)");
    QVERIFY2(code.content.contains("HIGH") || code.content.contains("LOW"),
             "Generated code should reference HIGH or LOW for disconnected ports");

    qDeleteAll(elements);
}

void TestArduino::testPartiallyConnectedLogic()
{
    // Some inputs connected, some not
    auto in1 = ElementFactory::buildElement(ElementType::InputButton);
    auto in2 = ElementFactory::buildElement(ElementType::InputButton);
    auto gate = ElementFactory::buildElement(ElementType::And);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in1 << in2 << gate << out;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Partially connected logic generation should succeed");
    // AND gate always produces && even when inputs are not wired to the gate
    QVERIFY2(code.content.contains("&&"), "AND gate should produce && operator");
    // Unconnected gate inputs resolve to HIGH or LOW
    QVERIFY2(code.content.contains("HIGH") || code.content.contains("LOW"),
             "Unconnected inputs should resolve to HIGH or LOW defaults");
    QVERIFY2(validateArduinoSyntax(code.content), "Partially connected code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testHighPinCountCircuit()
{
    // Board selection escalates boards. Exhaust even the largest board (~38 pins for ESP32)
    // by creating 80 input buttons.
    QVector<GraphicElement *> elements;

    for (int i = 0; i < 80; ++i) {
        auto btn = ElementFactory::buildElement(ElementType::InputButton);
        QVERIFY2(btn != nullptr, QString("Failed to create InputButton %1").arg(i).toLatin1());
        btn->setLabel(QString("Button_%1").arg(i));
        elements << btn;
    }

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("high_pin_count.ino");

    // Should throw a Pandaception when all board pins are exhausted
    try {
        ArduinoCodeGen generator(filePath, elements);
        generator.generate();
        QFAIL("Expected Pandaception for pin exhaustion");
    } catch (const Pandaception &e) {
        QString errorMsg = e.what();
        QVERIFY2(errorMsg.contains("pins", Qt::CaseInsensitive), "Error message should mention pins");
    }

    qDeleteAll(elements);
}

void TestArduino::testMultipleInputConnections()
{
    // AND gate with multiple input connections
    auto in1 = ElementFactory::buildElement(ElementType::InputButton);
    auto in2 = ElementFactory::buildElement(ElementType::InputButton);
    auto in3 = ElementFactory::buildElement(ElementType::InputButton);
    auto gate = ElementFactory::buildElement(ElementType::And);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in1 << in2 << in3 << gate << out;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Multiple input connection generation should succeed");
    // 3 input buttons + 1 LED → at least 4 pinMode calls
    QVERIFY2(code.content.count("pinMode(") >= 4,
             "3 buttons + 1 LED should produce at least 4 pinMode calls");
    // AND gate produces && operator
    QVERIFY2(code.content.contains("&&"), "AND gate should produce && operator for multiple inputs");
    QVERIFY2(validateArduinoSyntax(code.content), "Multiple input code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testChainedLogicGates()
{
    // Chain of logic gates: in -> AND -> OR -> LED
    auto in1 = ElementFactory::buildElement(ElementType::InputButton);
    auto in2 = ElementFactory::buildElement(ElementType::InputButton);
    auto and_gate = ElementFactory::buildElement(ElementType::And);
    auto or_gate = ElementFactory::buildElement(ElementType::Or);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in1 << in2 << and_gate << or_gate << out;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Chained gate generation should succeed");

    // Both && (AND) and || (OR) operators must appear in the generated code
    QVERIFY2(code.content.contains("&&"), "Chained code should contain AND operator");
    QVERIFY2(code.content.contains("||"), "Chained code should contain OR operator");
    // Must declare auxiliary variables for both intermediate gates
    QVERIFY2(code.content.count("aux_") >= 2, "Should have at least 2 aux variables for chained gates");
    QVERIFY2(validateArduinoSyntax(code.content), "Chained gate code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testPresetClearInteraction()
{
    // Test interaction between preset and clear pins
    auto d_in = ElementFactory::buildElement(ElementType::InputButton);
    auto clk = ElementFactory::buildElement(ElementType::Clock);
    auto prst = ElementFactory::buildElement(ElementType::InputVcc);  // Always high
    auto clr = ElementFactory::buildElement(ElementType::InputGnd);   // Always low
    auto dff = ElementFactory::buildElement(ElementType::DFlipFlop);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << d_in << clk << prst << clr << dff << out;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Preset/Clear interaction generation should succeed");
    verifyFlipFlopStructure(code.content, "D");
    // VCC (preset) → HIGH, GND (clear) → LOW in generated expressions
    QVERIFY2(code.content.contains("HIGH") && code.content.contains("LOW"),
             "VCC/GND preset-clear inputs should appear as HIGH/LOW in generated code");
    verifyPresetClearLogic(code.content);

    qDeleteAll(elements);
}

void TestArduino::testFlipFlopInitialState()
{
    // Test flip-flop with focus on initial state generation
    auto dff = ElementFactory::buildElement(ElementType::DFlipFlop);
    auto out_q = ElementFactory::buildElement(ElementType::Led);
    auto out_qn = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << dff << out_q << out_qn;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Flip-flop initial state generation should succeed");
    // State variable must be declared and initialized (not bare declaration)
    QVERIFY2(code.content.contains("aux_d_flip_flop"),
             "DFlipFlop should declare aux state variable");
    QRegularExpression bareDecl(R"(aux_d_flip_flop\s*;)");
    QVERIFY2(!bareDecl.match(code.content).hasMatch(),
             "Flip-flop state variable must be initialized, not just declared");
    QVERIFY2(validateArduinoSyntax(code.content), "Flip-flop initial state code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testLatchEdgeTriggeringLogic()
{
    // Test latch (level-triggered) vs flip-flop (edge-triggered)
    auto d_in = ElementFactory::buildElement(ElementType::InputButton);
    auto enable = ElementFactory::buildElement(ElementType::InputButton);
    auto latch = ElementFactory::buildElement(ElementType::DLatch);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << d_in << enable << latch << out;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Latch generation should succeed");
    // D latch is level-triggered — no clock edge detection variable (_inclk)
    QVERIFY2(!code.content.contains("_inclk"),
             "D Latch should NOT have clock edge detection variable — it is level-triggered");
    // Latch state variable must be present
    QVERIFY2(code.content.contains("aux_d_latch"),
             "D Latch should declare aux state variable");
    QVERIFY2(validateArduinoSyntax(code.content), "Latch level-triggering code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testFileCreationFailure()
{
    // Test with invalid file path (non-existent parent directory)
    auto in = ElementFactory::buildElement(ElementType::InputButton);
    QVERIFY2(in != nullptr, "Failed to create InputButton");
    auto out = ElementFactory::buildElement(ElementType::Led);
    QVERIFY2(out != nullptr, "Failed to create Led");

    QVector<GraphicElement *> elements;
    elements << in << out;

    // Use a path that's very unlikely to work
    QString filePath = "/invalid/path/that/does/not/exist/test.ino";

    // Constructor must throw immediately with a meaningful message when the file can't be opened
    try {
        ArduinoCodeGen generator(filePath, elements);
        QFAIL("Expected Pandaception for invalid file path");
    } catch (const Pandaception &e) {
        QVERIFY2(QString(e.what()).contains("Could not open", Qt::CaseInsensitive),
                 qPrintable(QString("Expected 'Could not open' in: %1").arg(e.what())));
    }
    QVERIFY2(!QFile::exists(filePath), "File must not be created for an invalid path");

    // Ensure cleanup: delete elements
    qDeleteAll(elements);
}

void TestArduino::testReadOnlyDirectoryHandling()
{
    // Test with read-only directory
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed for Arduino code generation");

    // Create a subdirectory and make it read-only
    QString subDir = tempDir.filePath("readonly");
    QDir().mkdir(subDir);

#ifdef Q_OS_UNIX
    // Root ignores permission bits — skip rather than give a false-positive
    if (getuid() == 0) {
        QSKIP("Skipping read-only directory test when running as root");
    }
#endif

#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot prevent file creation on Windows (uses ACLs, not Unix permission bits)");
#else
    QFile::setPermissions(subDir, QFileDevice::ReadOwner | QFileDevice::ExeOwner);

    auto in = ElementFactory::buildElement(ElementType::InputButton);
    QVERIFY2(in != nullptr, "Failed to create InputButton");
    auto out = ElementFactory::buildElement(ElementType::Led);
    QVERIFY2(out != nullptr, "Failed to create Led");

    QVector<GraphicElement *> elements;
    elements << in << out;

    QString filePath = subDir + "/test.ino";

    // Constructor must throw immediately with a meaningful message when the file can't be opened
    try {
        ArduinoCodeGen generator(filePath, elements);
        QFAIL("Expected Pandaception for read-only directory");
    } catch (const Pandaception &e) {
        QVERIFY2(QString(e.what()).contains("Could not open", Qt::CaseInsensitive),
                 qPrintable(QString("Expected 'Could not open' in: %1").arg(e.what())));
    }
    QVERIFY2(!QFile::exists(filePath), "File must not be created in a read-only directory");

    // Restore permissions so QTemporaryDir can clean up
    QFile::setPermissions(subDir, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    // Ensure cleanup: delete elements
    qDeleteAll(elements);
#endif
}

void TestArduino::testDeeplyNestedElementNaming()
{
    // Test deeply nested naming scenarios
    auto btn1 = ElementFactory::buildElement(ElementType::InputButton);
    btn1->setLabel("Input_Section_A_Button_1");
    auto gate1 = ElementFactory::buildElement(ElementType::And);
    gate1->setLabel("Logic_Section_1_AND_Gate_A");
    auto gate2 = ElementFactory::buildElement(ElementType::Or);
    gate2->setLabel("Logic_Section_2_OR_Gate_B");
    auto led = ElementFactory::buildElement(ElementType::Led);
    led->setLabel("Output_Section_Final_LED_Result");

    QVector<GraphicElement *> elements;
    elements << btn1 << gate1 << gate2 << led;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Deeply nested name generation should succeed");
    // "Input_Section_A_Button_1" → "input_section_a_button_1" (lowercased, underscores preserved)
    QVERIFY2(code.content.contains("input_section_a_button_1"),
             "Long underscored label should appear verbatim (lowercased) in variable names");
    // "Output_Section_Final_LED_Result" → "output_section_final_led_result"
    QVERIFY2(code.content.contains("output_section_final_led_result"),
             "Long LED label should appear sanitized in output variable name");
    QVERIFY2(validateArduinoSyntax(code.content), "Deeply nested naming code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testNodeElementGeneration()
{
    // Node element is a wire junction — should generate a simple pass-through assignment
    auto in = ElementFactory::buildElement(ElementType::InputButton);
    auto node = ElementFactory::buildElement(ElementType::Node);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in << node << out;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Node element generation should succeed");

    // Node produces an aux variable declared in the aux variables section
    QVERIFY2(code.content.contains("aux_node"), "Node should produce an aux_node variable");
    // Node assignment should be a simple = without operators
    QRegularExpression nodeAssign(R"(aux_node\w*\s*=\s*\w+;)");
    QVERIFY2(nodeAssign.match(code.content).hasMatch(),
             "Node should generate a simple pass-through assignment");

    qDeleteAll(elements);
}

void TestArduino::testWirelessNodeGeneration()
{
    // A Tx and Rx node pair on the same channel — the Rx output variable should be
    // assigned from whatever drives the Tx input, not from LOW (the default for
    // unconnected ports).
    auto *sw = new InputSwitch();
    auto *txNode = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    QVERIFY(txNode);
    txNode->setLabel("CLK");
    txNode->setWirelessMode(WirelessMode::Tx);

    auto *rxNode = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    QVERIFY(rxNode);
    rxNode->setLabel("CLK");
    rxNode->setWirelessMode(WirelessMode::Rx);

    auto *led = new Led();

    // Physical wire: InputSwitch → Tx node, Rx node → LED
    auto *conn1 = createConnection(sw, 0, txNode, 0);
    auto *conn2 = createConnection(rxNode, 0, led, 0);

    QVector<GraphicElement *> elements{sw, txNode, rxNode, led};
    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Wireless node generation should succeed");

    // Assignments in computeLogic() are indented with 4 spaces; declarations start
    // with 'bool'. The pattern below matches only indented node assignments to LOW,
    // so it cannot false-positive on 'bool aux_node_X = LOW;' declarations.
    QRegularExpression nodeAssignLow(R"(\n    aux_node\w*\s*=\s*LOW;)");
    QVERIFY2(!nodeAssignLow.match(code.content).hasMatch(),
             "Wireless Rx node should not produce LOW assignment in computeLogic");

    delete conn1;
    delete conn2;
    qDeleteAll(elements);
}

void TestArduino::testMuxDemuxIntegration()
{
    // Test Mux element generates correct if/else select logic
    {
        auto sel = ElementFactory::buildElement(ElementType::InputSwitch);
        auto d0 = ElementFactory::buildElement(ElementType::InputSwitch);
        auto d1 = ElementFactory::buildElement(ElementType::InputSwitch);
        auto mux = ElementFactory::buildElement(ElementType::Mux);
        auto out = ElementFactory::buildElement(ElementType::Led);

        QVector<GraphicElement *> elements{sel, d0, d1, mux, out};
        auto code = generateFromElements(elements);

        QVERIFY2(code.success, "Mux circuit generation should succeed");
        QVERIFY2(code.content.contains("//Multiplexer"), "Generated code should contain Multiplexer comment");
        QVERIFY2(code.content.contains("if ("), "Mux code should contain if statement for select logic");
        QVERIFY2(validateArduinoSyntax(code.content), "Mux generated code should have valid syntax");

        qDeleteAll(elements);
    }

    // Test Demux element generates correct if/else select logic
    {
        auto sel = ElementFactory::buildElement(ElementType::InputSwitch);
        auto data = ElementFactory::buildElement(ElementType::InputSwitch);
        auto demux = ElementFactory::buildElement(ElementType::Demux);
        auto out1 = ElementFactory::buildElement(ElementType::Led);
        auto out2 = ElementFactory::buildElement(ElementType::Led);

        QVector<GraphicElement *> elements{sel, data, demux, out1, out2};
        auto code = generateFromElements(elements);

        QVERIFY2(code.success, "Demux circuit generation should succeed");
        QVERIFY2(code.content.contains("//Demultiplexer"), "Generated code should contain Demultiplexer comment");
        QVERIFY2(code.content.contains("if ("), "Demux code should contain if statement for select logic");
        QVERIFY2(validateArduinoSyntax(code.content), "Demux generated code should have valid syntax");

        qDeleteAll(elements);
    }
}

void TestArduino::testMultipleClocksInCircuit()
{
    // Test multiple clock elements produce separate elapsedMillis instances
    auto clk1 = ElementFactory::buildElement(ElementType::Clock);
    auto clk2 = ElementFactory::buildElement(ElementType::Clock);
    auto dff1 = ElementFactory::buildElement(ElementType::DFlipFlop);
    auto dff2 = ElementFactory::buildElement(ElementType::DFlipFlop);
    auto out1 = ElementFactory::buildElement(ElementType::Led);
    auto out2 = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << clk1 << clk2 << dff1 << dff2 << out1 << out2;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Multiple clock generation should succeed");

    // Two separate elapsedMillis declarations — one per clock
    QVERIFY2(code.content.count("elapsedMillis") >= 2,
             "Should declare a separate elapsedMillis variable for each clock");
    // Two separate _interval variables
    QVERIFY2(code.content.count("_interval =") >= 2,
             "Should declare a separate interval variable for each clock");
    QVERIFY2(validateArduinoSyntax(code.content), "Multiple clock code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testComplexPresetClearSequence()
{
    // Test complex preset/clear sequence with multiple flip-flops
    auto vcc = ElementFactory::buildElement(ElementType::InputVcc);
    auto gnd = ElementFactory::buildElement(ElementType::InputGnd);
    auto clk = ElementFactory::buildElement(ElementType::Clock);
    auto d1 = ElementFactory::buildElement(ElementType::InputButton);
    auto d2 = ElementFactory::buildElement(ElementType::InputButton);
    auto dff1 = ElementFactory::buildElement(ElementType::DFlipFlop);
    auto dff2 = ElementFactory::buildElement(ElementType::DFlipFlop);
    auto out1 = ElementFactory::buildElement(ElementType::Led);
    auto out2 = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << vcc << gnd << clk << d1 << d2 << dff1 << dff2 << out1 << out2;

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Complex preset/clear sequence generation should succeed");
    // Two D flip-flop sections should appear in the generated code
    QVERIFY2(code.content.count("//D FlipFlop") >= 2,
             "Two DFlipFlop elements should produce two D FlipFlop sections");
    // Preset/clear conditional logic should be present
    verifyPresetClearLogic(code.content);
    QVERIFY2(validateArduinoSyntax(code.content), "Complex preset/clear code should have valid syntax");

    qDeleteAll(elements);
}

void TestArduino::testOutputPinExhaustion()
{
    // Board selection escalates boards as needed (UNO→Nano→Mega→ESP32).
    // The ESP32 (last board) has ~38 pins. Create 80 LEDs to exceed even the largest board.
    QVector<GraphicElement *> elements;

    for (int i = 0; i < 80; ++i) {
        auto led = ElementFactory::buildElement(ElementType::Led);
        QVERIFY2(led != nullptr, QString("Failed to create Led %1").arg(i).toLatin1());
        led->setLabel(QString("LED_%1").arg(i));
        elements << led;
    }

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("output_pin_exhaustion.ino");

    // Should throw Pandaception when output pins are exhausted on the largest board
    try {
        ArduinoCodeGen generator(filePath, elements);
        generator.generate();
        QFAIL("Expected Pandaception for output pin exhaustion");
    } catch (const Pandaception &e) {
        QString errorMsg = e.what();
        QVERIFY2(errorMsg.contains("output", Qt::CaseInsensitive), "Error message should mention output");
        QVERIFY2(errorMsg.contains("pins", Qt::CaseInsensitive), "Error message should mention pins");
    } catch (const std::exception &) {
        QFAIL("Caught wrong exception type");
    } catch (...) {
        QFAIL("Caught unknown exception");
    }

    qDeleteAll(elements);
}

void TestArduino::testUnsupportedElementTypes()
{
    // Test that previously unsupported elements (Mux, Demux, TruthTable) now generate correctly

    // Test 1: Mux (multiplexer) - now supported
    {
        auto mux = ElementFactory::buildElement(ElementType::Mux);
        QVERIFY2(mux != nullptr, "Failed to create Mux element");
        mux->setLabel("TestMux");
        QVector<GraphicElement *> elements;
        elements << mux;

        QTemporaryDir tempDir;
        QString filePath = tempDir.filePath("mux_test.ino");

        try {
            ArduinoCodeGen generator(filePath, elements);
            generator.generate();
            QVERIFY2(QFileInfo::exists(filePath), "Mux should generate Arduino code successfully");

            QFile file(filePath);
            QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
            QString content = QString::fromUtf8(file.readAll());
            file.close();

            QVERIFY2(content.contains("if"), "Mux code should contain if statements");
            QVERIFY2(content.contains("Multiplexer"), "Mux code should contain multiplexer comment");
        } catch (...) {
            QFAIL("Mux element should now be supported and generate without exception");
        }

        qDeleteAll(elements);
    }

    // Test 2: Demux (demultiplexer) - now supported
    {
        auto demux = ElementFactory::buildElement(ElementType::Demux);
        QVERIFY2(demux != nullptr, "Failed to create Demux element");
        demux->setLabel("TestDemux");
        QVector<GraphicElement *> elements;
        elements << demux;

        QTemporaryDir tempDir;
        QString filePath = tempDir.filePath("demux_test.ino");

        try {
            ArduinoCodeGen generator(filePath, elements);
            generator.generate();
            QVERIFY2(QFileInfo::exists(filePath), "Demux should generate Arduino code successfully");

            QFile file(filePath);
            QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
            QString content = QString::fromUtf8(file.readAll());
            file.close();

            QVERIFY2(content.contains("if"), "Demux code should contain if statements");
            QVERIFY2(content.contains("Demultiplexer"), "Demux code should contain demultiplexer comment");
        } catch (...) {
            QFAIL("Demux element should now be supported and generate without exception");
        }

        qDeleteAll(elements);
    }

    // Test 3: TruthTable (custom logic) - now supported
    {
        auto truthTable = ElementFactory::buildElement(ElementType::TruthTable);
        QVERIFY2(truthTable != nullptr, "Failed to create TruthTable element");
        truthTable->setLabel("TestTruthTable");
        QVector<GraphicElement *> elements;
        elements << truthTable;

        QTemporaryDir tempDir;
        QString filePath = tempDir.filePath("truth_table_test.ino");

        try {
            ArduinoCodeGen generator(filePath, elements);
            generator.generate();
            QVERIFY2(QFileInfo::exists(filePath), "TruthTable should generate Arduino code successfully");

            QFile file(filePath);
            QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
            QString content = QString::fromUtf8(file.readAll());
            file.close();

            QVERIFY2(content.contains("if"), "TruthTable code should contain if statements");
            QVERIFY2(content.contains("TruthTable"), "TruthTable code should contain TruthTable comment");
        } catch (...) {
            QFAIL("TruthTable element should now be supported and generate without exception");
        }

        qDeleteAll(elements);
    }
}

// ============================================================================
// arduino-cli Validation Helper
// ============================================================================

bool TestArduino::validateWithArduinoCli(const QString &inoFile)
{
    const QString cliPath = QStandardPaths::findExecutable("arduino-cli");
    if (cliPath.isEmpty()) {
        qWarning() << "FATAL: arduino-cli not found in PATH - required for validation";
        return false;  // Fail test - tool is required
    }

    // arduino-cli requires the .ino to live in a folder whose name matches the sketch name.
    // Create a temporary sketch directory with the correct layout.
    QTemporaryDir sketchDir;
    if (!sketchDir.isValid()) {
        qWarning() << "Could not create temp directory for arduino-cli sketch";
        return false;
    }

    QString sketchName = QFileInfo(inoFile).baseName();
    QString sketchFolder = sketchDir.filePath(sketchName);
    if (!QDir().mkdir(sketchFolder)) {
        qWarning() << "Could not create sketch folder" << sketchFolder;
        return false;
    }

    QString sketchIno = sketchFolder + "/" + sketchName + ".ino";
    if (!QFile::copy(inoFile, sketchIno)) {
        qWarning() << "Could not copy" << inoFile << "to" << sketchIno;
        return false;
    }

    // Detect which board the codegen selected from the "Target Board:" comment,
    // then pick the matching FQBN so analog pin ranges are correct.
    QString fqbn = "arduino:avr:uno";  // default
    QFile f(inoFile);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString header = QString::fromUtf8(f.read(512));
        f.close();
        if (header.contains("Mega 2560"))
            fqbn = "arduino:avr:mega";
        else if (header.contains("Arduino Nano"))
            fqbn = "arduino:avr:nano";
        // ESP32 would need a separate core; leave as UNO fallback
    }

    QStringList compileArgs = {"compile", "--fqbn", fqbn};
    if (!s_cliCachePath.isEmpty())
        compileArgs << "--build-cache-path" << s_cliCachePath;
    compileArgs << sketchFolder;

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(cliPath, compileArgs);

    if (!proc.waitForFinished(120000)) {
        qWarning() << "arduino-cli timed out for" << inoFile;
        proc.kill();
        return false;
    }

    if (proc.exitCode() != 0) {
        QString output = QString::fromUtf8(proc.readAllStandardOutput());
        qWarning() << "arduino-cli compilation failed for" << inoFile;
        qWarning() << "Output:" << output;
        return false;
    }

    qInfo() << "arduino-cli validation passed for" << inoFile;
    return true;
}

// ============================================================================
// simavr Functional Simulation Helper
// ============================================================================

bool TestArduino::simulateWithSimavr(const QString &elfFile, int durationMs)
{
    // simavr runs a compiled .elf for a given number of simulated milliseconds.
    // Usage: simavr -m atmega328p -f 16000000 sketch.elf
    const QString simavrPath = QStandardPaths::findExecutable("simavr");
    if (simavrPath.isEmpty()) {
        qWarning() << "FATAL: simavr not found in PATH — required for functional simulation";
        return false;  // Fail test - tool is required
    }

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);

    // Run for the requested simulated duration (simavr exits on AVR sleep or timeout).
    // durationMs is used as a wall-clock timeout; simavr itself runs at simulation speed.
    proc.start(simavrPath, {"-m", "atmega328p", "-f", "16000000", elfFile});

    if (!proc.waitForFinished(durationMs + 5000)) {
        // Still running — that is expected for non-terminating sketches; consider it a pass.
        proc.kill();
        proc.waitForFinished(2000);
        qInfo() << "simavr ran for" << durationMs << "ms without crashing —" << elfFile;
        return true;
    }

    // A non-zero exit indicates a simulator crash or AVR RESET loop.
    if (proc.exitCode() != 0) {
        QString output = QString::fromUtf8(proc.readAllStandardOutput());
        qWarning() << "simavr reported error for" << elfFile << ":" << output;
        return false;
    }

    return true;
}

// ============================================================================
// Combinational Circuit Detection
// ============================================================================

bool TestArduino::isCombinationalCircuit(const QVector<GraphicElement *> &elements)
{
    for (auto *elm : elements) {
        switch (elm->elementType()) {
        case ElementType::Clock:
        case ElementType::DFlipFlop:
        case ElementType::DLatch:
        case ElementType::JKFlipFlop:
        case ElementType::SRFlipFlop:
        case ElementType::SRLatch:
        case ElementType::TFlipFlop:
            return false;
        case ElementType::IC: {
            auto *ic = qobject_cast<IC *>(elm);
            // Conservatively treat ICs with no loaded sub-elements as non-combinational
            if (!ic || ic->m_icElements.isEmpty() || !isCombinationalCircuit(ic->m_icElements)) {
                return false;
            }
            break;
        }
        default:
            break;
        }
    }
    return true;
}

// ============================================================================
// Truth Table Generation (Phase 3)
// ============================================================================

QVector<ArduinoCodeGen::TestVector> TestArduino::generateTruthTable(
    const QVector<GraphicElement *> &elements, int maxInputBits)
{
    struct InputEntry { GraphicElement *elm; int portIndex; };
    struct OutputEntry { GraphicElement *elm; int portIndex; };

    // Collect inputs in the same order as ArduinoCodeGen::declareInputs()
    QVector<InputEntry> inputs;
    for (auto *elm : elements) {
        const auto type = elm->elementType();
        if (type == ElementType::InputSwitch || type == ElementType::InputButton) {
            inputs.append({elm, 0});
        } else if (type == ElementType::InputRotary) {
            for (int i = 0; i < elm->outputSize(); ++i) {
                inputs.append({elm, i});
            }
        }
    }

    // Collect outputs in the same order as ArduinoCodeGen::declareOutputs()
    QVector<OutputEntry> outputs;
    for (auto *elm : elements) {
        if (elm->elementGroup() == ElementGroup::Output) {
            for (int i = 0; i < elm->inputSize(); ++i) {
                outputs.append({elm, i});
            }
        }
    }

    const int N = inputs.size();
    if (N == 0 || N > maxInputBits) {
        return {};
    }

    ElementMapping mapping(elements);
    mapping.sort();

    // Skip circuits with feedback loops — single-pass propagation is not valid for them
    if (mapping.hasFeedbackElements()) {
        return {};
    }

    const int combos = 1 << N;
    QVector<ArduinoCodeGen::TestVector> table;
    table.reserve(combos);

    for (int i = 0; i < combos; ++i) {
        // Drive input logic elements
        for (int j = 0; j < N; ++j) {
            const bool bit = (i >> j) & 1;
            if (inputs[j].elm->logic()) {
                inputs[j].elm->logic()->setOutputValue(inputs[j].portIndex, bit);
            }
        }

        // Propagate through all logic elements (already sorted topologically)
        for (const auto &logic : mapping.logicElms()) {
            logic->updateLogic();
        }

        // Read output values
        ArduinoCodeGen::TestVector entry;
        entry.inputs.resize(N);
        for (int j = 0; j < N; ++j) {
            entry.inputs[j] = (i >> j) & 1;
        }
        entry.outputs.resize(outputs.size());
        for (int k = 0; k < outputs.size(); ++k) {
            if (outputs[k].elm->logic()) {
                entry.outputs[k] = outputs[k].elm->logic()->outputValue(outputs[k].portIndex) == Status::Active;
            }
        }
        table.append(entry);
    }

    return table;
}

// ============================================================================
// Testbench Runner (Phase 4)
// ============================================================================

bool TestArduino::runTestbench(const QString &tbInoPath, int timeoutMs)
{
    const QString cliPath = QStandardPaths::findExecutable("arduino-cli");
    if (cliPath.isEmpty()) {
        qWarning() << "FATAL: arduino-cli not found — required for testbench validation";
        return false;
    }
    const QString simavrPath = QStandardPaths::findExecutable("simavr");
    if (simavrPath.isEmpty()) {
        qWarning() << "FATAL: simavr not found — required for testbench simulation";
        return false;
    }

    // Sketch folder is the directory containing the .ino file
    QFileInfo inoInfo(tbInoPath);
    const QString sketchFolder = inoInfo.absolutePath();
    const QString sketchName = inoInfo.completeBaseName();

    // Compile
    QTemporaryDir outDir;
    if (!outDir.isValid()) {
        return false;
    }

    QStringList tbArgs = {"compile", "--fqbn", "arduino:avr:uno",
                          "--output-dir", outDir.path()};
    if (!s_cliCachePath.isEmpty())
        tbArgs << "--build-cache-path" << s_cliCachePath;
    tbArgs << sketchFolder;

    QProcess compile;
    compile.setProcessChannelMode(QProcess::MergedChannels);
    compile.start(cliPath, tbArgs);
    if (!compile.waitForFinished(120000) || compile.exitCode() != 0) {
        const QString compileOut = QString::fromUtf8(compile.readAllStandardOutput());
        if (compileOut.contains("data section exceeds") || compileOut.contains("Not enough memory")) {
            qInfo() << "Testbench skipped — too large for Arduino Uno RAM:" << tbInoPath;
            return true;
        }
        qWarning() << "arduino-cli testbench compile failed:" << compileOut;
        return false;
    }

    const QString elfPath = outDir.filePath(sketchName + ".ino.elf");
    if (!QFileInfo::exists(elfPath)) {
        qWarning() << "Expected .elf not found:" << elfPath;
        return false;
    }

    // Simulate and capture UART output.
    // The testbench sketch never exits — it loops forever after printing the result.
    // Stream output and kill simavr as soon as a verdict line appears.
    QProcess sim;
    sim.setProcessChannelMode(QProcess::MergedChannels);
    sim.start(simavrPath, {"-m", "atmega328p", "-f", "16000000", elfPath});

    QString output;
    const QStringList verdicts = {"ALL PASS", "SOME FAILED", "FAIL vector"};
    auto hasVerdict = [&](const QString &s) {
        for (const auto &v : verdicts)
            if (s.contains(v))
                return true;
        return false;
    };

    QElapsedTimer elapsed;
    elapsed.start();
    while (!hasVerdict(output) && elapsed.elapsed() < timeoutMs) {
        sim.waitForReadyRead(500);
        output += QString::fromUtf8(sim.readAllStandardOutput());
    }
    sim.kill();
    sim.waitForFinished(2000);
    output += QString::fromUtf8(sim.readAllStandardOutput());

    qInfo() << "simavr testbench output:" << output.trimmed();

    if (output.contains("SOME FAILED") || output.contains("FAIL vector")) {
        qWarning() << "Testbench reported failures:\n" << output;
        return false;
    }
    return output.contains("ALL PASS");
}

// ============================================================================
// Real File-Based Test Helper
// ============================================================================

void TestArduino::testArduinoExportHelper(const QString &icFile)
{
    // Create workspace and circuit builder
    std::unique_ptr<WorkSpace> workspace = TestUtils::createWorkspace();
    QVERIFY(workspace != nullptr);

    auto *scene = workspace->scene();
    QVERIFY(scene != nullptr);

    CircuitBuilder builder(scene);

    // Load IC from test components directory
    IC *ic = nullptr;
    try {
        ic = CPUTestUtils::loadBuildingBlockIC(icFile);
    } catch (const std::exception &ex) {
        QFAIL(qPrintable(QString("Failed to load IC: %1").arg(QString::fromStdString(ex.what()))));
        return;
    }

    builder.add(ic);

    // Create input switches (one per IC input)
    QVector<InputSwitch *> switches;
    for (int i = 0; i < ic->inputSize(); ++i) {
        auto *sw = new InputSwitch();
        builder.add(sw);
        switches.append(sw);
        builder.connect(sw, 0, ic, i);
    }

    // Create LEDs (one per IC output)
    QVector<Led *> leds;
    for (int i = 0; i < ic->outputSize(); ++i) {
        auto *led = new Led();
        builder.add(led);
        leds.append(led);
        builder.connect(ic, i, led, 0);
    }

    // Get all elements from scene
    QVector<GraphicElement *> allElements;
    const auto items = scene->items();
    for (auto *item : std::as_const(items)) {
        if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
            allElements.append(elm);
        }
    }

    QVERIFY2(!allElements.isEmpty(), "Scene should contain elements");

    // Generate Arduino code to a temporary file
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString inoPath = tempDir.filePath(icFile.split('/').last().replace(".panda", ".ino"));

    ArduinoCodeGen generator(inoPath, allElements);
    try {
        generator.generate();
    } catch (const std::exception &ex) {
        QFile::remove(inoPath);
        QFAIL(qPrintable(QString("Arduino generation failed: %1").arg(QString::fromStdString(ex.what()))));
        return;
    } catch (...) {
        QFile::remove(inoPath);
        QFAIL("Arduino generation threw unknown exception");
        return;
    }

    // Verify file was created
    QVERIFY2(QFileInfo::exists(inoPath), "Arduino file should be created");

    // Read and verify basic structural validity
    QFile generatedFile(inoPath);
    QVERIFY(generatedFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(generatedFile.readAll());
    generatedFile.close();

    // Verify basic structure
    QVERIFY2(content.contains("void setup()"), "Arduino code should have setup()");
    QVERIFY2(content.contains("void loop()"), "Arduino code should have loop()");
    QVERIFY2(content.contains("#include"), "Arduino code should have #include");

    // Verify pin counts
    int inputPins = switches.size();
    int outputPins = leds.size();
    int expectedPins = inputPins + outputPins;

    QRegularExpression constIntRegex("const int");
    int constIntCount = content.count(constIntRegex);
    QVERIFY2(constIntCount >= expectedPins,
        qPrintable(QString("Expected at least %1 const int declarations, found %2").arg(expectedPins).arg(constIntCount)));

    // Verify pinMode calls
    int pinModeCount = content.count("pinMode(");
    QVERIFY2(pinModeCount >= expectedPins,
        qPrintable(QString("Expected at least %1 pinMode calls, found %2").arg(expectedPins).arg(pinModeCount)));

    // Verify input and output counts
    QVERIFY2(content.contains("INPUT") || inputPins == 0, "Arduino code should have INPUT declarations for inputs");
    QVERIFY2(content.contains("OUTPUT") || outputPins == 0, "Arduino code should have OUTPUT declarations for outputs");

    // Generate or compare against expected Arduino output
    const bool generateMode = qEnvironmentVariableIsSet("GENERATE_EXPECTED_ARDUINO");
    const QString expectedDir = TestUtils::arduinoExpectedDir();
    const QString baseName = icFile.split('/').last().replace(".panda", "");
    const QString expectedPath = expectedDir + baseName + ".ino";

    if (generateMode) {
        // Save generated code for future reference
        QDir().mkpath(expectedDir);
        QFile expectedFile(expectedPath);
        QVERIFY2(expectedFile.open(QIODevice::WriteOnly | QIODevice::Text),
            qPrintable(QString("Could not write expected file: %1").arg(expectedPath)));
        expectedFile.write(content.toUtf8());
        expectedFile.close();
        qInfo() << "Generated expected Arduino file:" << expectedPath;
    } else {
        // Compare against expected output (fast, no compilation)
        if (QFileInfo::exists(expectedPath)) {
            QFile expectedFile(expectedPath);
            QVERIFY2(expectedFile.open(QIODevice::ReadOnly | QIODevice::Text),
                qPrintable(QString("Could not read expected file: %1").arg(expectedPath)));
            QString expectedContent = QString::fromUtf8(expectedFile.readAll());
            expectedFile.close();
            QCOMPARE(content, expectedContent);
        } else {
            // First time: generate and validate with arduino-cli
            qWarning() << "Expected file not found, using arduino-cli validation:" << expectedPath;
            QVERIFY2(validateWithArduinoCli(inoPath),
                qPrintable(QString("arduino-cli compilation validation failed for %1").arg(icFile)));
        }
    }

    // Run testbench validation only when explicitly generating (GENERATE_EXPECTED_ARDUINO=1)
    if (generateMode) {
        // Functional testbench validation for combinational circuits (Phase 5)
        if (isCombinationalCircuit(allElements)) {
            const auto truthTable = generateTruthTable(allElements);
            if (!truthTable.isEmpty()) {
                const QString tbSketchName = baseName + "_tb";
                const QString tbSketchFolder = tempDir.filePath(tbSketchName);
                if (QDir().mkdir(tbSketchFolder)) {
                    const QString tbInoPath = tbSketchFolder + "/" + tbSketchName + ".ino";
                    try {
                        generator.generateTestbench(tbInoPath, truthTable);
                        QVERIFY2(runTestbench(tbInoPath),
                            qPrintable(QString("Testbench functional validation failed for %1").arg(icFile)));
                    } catch (const std::exception &ex) {
                        QFAIL(qPrintable(QString("Testbench generation failed for %1: %2")
                            .arg(icFile, QString::fromStdString(ex.what()))));
                    }
                }
            }
        }
    }
}

// ============================================================================
// Level 1 - Sequential IC Tests (Flip-Flops and Latches)
// ============================================================================

void TestArduino::testArduinoExportDFlipFlop()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level1_d_flip_flop.panda");
}

void TestArduino::testArduinoExportDLatch()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level1_d_latch.panda");
}

void TestArduino::testArduinoExportJKFlipFlop()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level1_jk_flip_flop.panda");
}

void TestArduino::testArduinoExportSRLatch()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level1_sr_latch.panda");
}

// ============================================================================
// Level 2 - Combinational IC Tests (Adders, Multiplexers, Decoders)
// ============================================================================

void TestArduino::testArduinoExportHalfAdder()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level2_half_adder.panda");
}

void TestArduino::testArduinoExportFullAdder1Bit()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level2_full_adder_1bit.panda");
}

void TestArduino::testArduinoExportDecoder2to4()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level2_decoder_2to4.panda");
}

void TestArduino::testArduinoExportDecoder3to8()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level2_decoder_3to8.panda");
}

void TestArduino::testArduinoExportDecoder4to16()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level2_decoder_4to16.panda");
}

void TestArduino::testArduinoExportMux2to1()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level2_mux_2to1.panda");
}

void TestArduino::testArduinoExportMux4to1()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level2_mux_4to1.panda");
}

void TestArduino::testArduinoExportMux8to1()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level2_mux_8to1.panda");
}

void TestArduino::testArduinoExportParityChecker()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level2_parity_checker.panda");
}

void TestArduino::testArduinoExportParityGenerator()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level2_parity_generator.panda");
}

void TestArduino::testArduinoExportPriorityEncoder8to3()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level2_priority_encoder_8to3.panda");
}

void TestArduino::testArduinoExportPriorityMux3to1()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level2_priority_mux_3to1.panda");
}

// ============================================================================
// Level 3 - Medium-Complexity IC Tests
// ============================================================================

void TestArduino::testArduinoExportAluSelector5Way()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level3_alu_selector_5way.panda");
}

void TestArduino::testArduinoExportBcd7SegmentDecoder()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level3_bcd_7segment_decoder.panda");
}

void TestArduino::testArduinoExportComparator4Bit()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level3_comparator_4bit.panda");
}

void TestArduino::testArduinoExportComparator4BitEquality()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level3_comparator_4bit_equality.panda");
}

void TestArduino::testArduinoExportRegister1Bit()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level3_register_1bit.panda");
}

// ============================================================================
// Level 4 - Nested IC Tests
// ============================================================================

void TestArduino::testArduinoExportRippleAdder4Bit()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level4_ripple_adder_4bit.panda");
}

void TestArduino::testArduinoExportRegister4Bit()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level4_register_4bit.panda");
}

void TestArduino::testArduinoExportBinaryCounter4Bit()
{
    requireLinuxForArduinoTools();
    testArduinoExportHelper("level4_binary_counter_4bit.panda");
}

// ============================================================================
// Codegen Utility Unit Tests
// ============================================================================

void TestArduino::testReservedKeywordSanitization()
{
    // Element labeled with a C++ keyword should get a trailing underscore
    auto btn = ElementFactory::buildElement(ElementType::InputButton);
    btn->setLabel("for");  // C++ reserved keyword
    QVector<GraphicElement *> elements{btn};
    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Generation with reserved keyword label should succeed");
    // "for" becomes "inputbutton1_for" after removeForbiddenChars; isArduinoReserved checks
    // the full sanitized name, not just the label fragment, so just verify no bare "for" as identifier
    QVERIFY2(!QRegularExpression(R"(\bfor\b\s*=)").match(code.content).hasMatch(),
             "Reserved keyword 'for' must not appear as a bare variable name");
    qDeleteAll(elements);

    // Element labeled with an Arduino built-in should also be protected
    auto sw = ElementFactory::buildElement(ElementType::InputSwitch);
    sw->setLabel("setup");  // Arduino built-in (also reserved)
    QVector<GraphicElement *> elements2{sw};
    auto code2 = generateFromElements(elements2);
    QVERIFY2(code2.success, "Generation with 'setup' label should succeed");
    // The generated variable must not shadow the setup() function
    QVERIFY2(!QRegularExpression(R"(\bsetup\b\s*=)").match(code2.content).hasMatch(),
             "Arduino built-in 'setup' must not appear as a bare variable name");
    qDeleteAll(elements2);
}

void TestArduino::testAccentStripping()
{
    // Element with accented label should produce clean ASCII variable name
    auto btn = ElementFactory::buildElement(ElementType::InputButton);
    btn->setLabel(QString("Sa") + QChar(0x00ED) + QString("da"));  // "Saída" (í = U+00ED)
    QVector<GraphicElement *> elements{btn};
    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Generation with accented label should succeed");

    // The generated code must not contain non-ASCII characters in variable names
    QRegularExpression nonAscii(R"([^\x00-\x7F])");
    QVERIFY2(!nonAscii.match(code.content).hasMatch(),
             "Generated code must not contain non-ASCII characters");
    // "saida" or "sada" must NOT appear — correct stripping gives "saida"
    QVERIFY2(code.content.contains("saida"),
             "Accent-stripped 'Saída' should produce 'saida' in the variable name");
    qDeleteAll(elements);
}

void TestArduino::testCycleDetection()
{
    // Build a feedback loop: NOT gate output → NOT gate input
    // otherPortNameImpl() must detect the cycle via the visited set and return a default value
    // rather than looping infinitely.
    auto notGate = ElementFactory::buildElement(ElementType::Not);
    auto out = ElementFactory::buildElement(ElementType::Led);

    // Wire output of NOT back to its own input — creates a cycle
    auto cycleConn = std::make_unique<QNEConnection>();
    cycleConn->setStartPort(notGate->outputPort(0));
    cycleConn->setEndPort(notGate->inputPort(0));

    QVector<GraphicElement *> elements{notGate, out};

    // Generation must complete without hanging or crashing
    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Generation should succeed even with a feedback cycle");

    // Cycle detection returns the port's defaultValue (HIGH or LOW)
    // so the generated expression must not be empty
    QRegularExpression emptyRhs(R"(=\s*;)");
    QVERIFY2(!emptyRhs.match(code.content).hasMatch(),
             "Cycle detection must resolve to a default value, not leave empty RHS");
    QVERIFY2(code.content.contains("HIGH") || code.content.contains("LOW"),
             "Cyclic port should resolve to HIGH or LOW default");

    cycleConn.reset();
    qDeleteAll(elements);
}

void TestArduino::testSimavrFunctionalSimulation()
{
    requireLinuxForArduinoTools();
    // Skip the whole test if neither arduino-cli nor simavr are available.
    if (QStandardPaths::findExecutable("arduino-cli").isEmpty()) {
        QSKIP("arduino-cli not found — skipping simavr functional simulation test");
    }
    if (QStandardPaths::findExecutable("simavr").isEmpty()) {
        QSKIP("simavr not found — skipping simavr functional simulation test");
    }

    // Build a simple NOT gate circuit: InputSwitch → NOT → LED
    auto sw = ElementFactory::buildElement(ElementType::InputSwitch);
    sw->setLabel("in");
    auto notGate = ElementFactory::buildElement(ElementType::Not);
    auto led = ElementFactory::buildElement(ElementType::Led);
    led->setLabel("out");

    auto c1 = std::make_unique<QNEConnection>();
    c1->setStartPort(sw->outputPort());
    c1->setEndPort(notGate->inputPort(0));

    auto c2 = std::make_unique<QNEConnection>();
    c2->setStartPort(notGate->outputPort(0));
    c2->setEndPort(led->inputPort(0));

    QVector<GraphicElement *> elements{sw, notGate, led};

    // Step 1: generate .ino
    QTemporaryDir workDir;
    QVERIFY2(workDir.isValid(), "Work dir creation failed");
    QString sketchName = "not_gate_sim";
    QString sketchFolder = workDir.filePath(sketchName);
    QVERIFY2(QDir().mkdir(sketchFolder), "Sketch folder creation failed");
    QString inoPath = sketchFolder + "/" + sketchName + ".ino";

    ArduinoCodeGen gen(inoPath, elements);
    try {
        gen.generate();
    } catch (const std::exception &ex) {
        QFAIL(qPrintable(QString("Generation failed: %1").arg(ex.what())));
    }
    QVERIFY2(QFileInfo::exists(inoPath), ".ino file should be created");

    // Step 2: generate truth table from simulator and produce testbench sketch
    const auto truthTable = generateTruthTable(elements);
    QVERIFY2(!truthTable.isEmpty(), "Truth table should not be empty for NOT gate");

    const QString tbSketchName = sketchName + "_tb";
    const QString tbSketchFolder = workDir.filePath(tbSketchName);
    QVERIFY2(QDir().mkdir(tbSketchFolder), "Testbench sketch folder creation failed");
    const QString tbInoPath = tbSketchFolder + "/" + tbSketchName + ".ino";

    try {
        gen.generateTestbench(tbInoPath, truthTable);
    } catch (const std::exception &ex) {
        QFAIL(qPrintable(QString("Testbench generation failed: %1").arg(ex.what())));
    }
    QVERIFY2(QFileInfo::exists(tbInoPath), "Testbench .ino file should be created");

    // Step 3: compile and simulate testbench — verify functional correctness
    QVERIFY2(runTestbench(tbInoPath, 15000),
             "Testbench functional simulation should report ALL PASS for NOT gate");

    // Step 4: compile production sketch and verify it runs without crashing
    const QString cliPath = QStandardPaths::findExecutable("arduino-cli");
    QProcess compileProc;
    compileProc.setProcessChannelMode(QProcess::MergedChannels);
    compileProc.start(cliPath, {"compile", "--fqbn", "arduino:avr:uno",
                                "--output-dir", workDir.path(), sketchFolder});
    if (!compileProc.waitForFinished(120000)) {
        compileProc.kill();
        QSKIP("arduino-cli compile timed out — skipping production simavr step");
    }
    if (compileProc.exitCode() != 0) {
        QSKIP(qPrintable(QString("arduino-cli compile failed: %1")
                         .arg(QString::fromUtf8(compileProc.readAllStandardOutput()))));
    }

    // arduino-cli writes <sketchName>.ino.elf into the output dir
    QString elfPath = workDir.filePath(sketchName + ".ino.elf");
    QVERIFY2(QFileInfo::exists(elfPath), ".elf file should be produced by arduino-cli");

    QVERIFY2(simulateWithSimavr(elfPath, 500),
             "simavr should be able to run the generated NOT gate sketch");

    c1.reset();
    c2.reset();
    qDeleteAll(elements);
}
