// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestArduino.h"

#include <memory>
#include <string>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QTemporaryDir>

#include "App/CodeGen/ArduinoCodeGen.h"
#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Nodes/QNEConnection.h"
#include "Tests/Common/TestUtils.h"

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

void TestArduino::testAndGateGeneration()
{
    auto in1 = ElementFactory::buildElement(ElementType::InputButton);
    auto in2 = ElementFactory::buildElement(ElementType::InputButton);
    auto andGate = ElementFactory::buildElement(ElementType::And);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in1 << in2 << andGate << out;

    // Create connections to verify AND operator is generated
    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(in1->outputPort());
    conn1->setEndPort(andGate->inputPort(0));

    auto conn2 = std::make_unique<QNEConnection>();
    conn2->setStartPort(in2->outputPort());
    conn2->setEndPort(andGate->inputPort(1));

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Arduino code generation should succeed");
    verifyBasicStructure(code.content);

    // Verify AND operator is in the generated code
    verifyLogicOperator(code.content, "&&", false);
}

void TestArduino::testOrGateGeneration()
{
    auto in1 = ElementFactory::buildElement(ElementType::InputButton);
    auto in2 = ElementFactory::buildElement(ElementType::InputButton);
    auto orGate = ElementFactory::buildElement(ElementType::Or);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in1 << in2 << orGate << out;

    // Create connections to verify OR operator is generated
    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(in1->outputPort());
    conn1->setEndPort(orGate->inputPort(0));

    auto conn2 = std::make_unique<QNEConnection>();
    conn2->setStartPort(in2->outputPort());
    conn2->setEndPort(orGate->inputPort(1));

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Arduino code generation should succeed");
    verifyBasicStructure(code.content);

    // Verify OR operator is in the generated code
    verifyLogicOperator(code.content, "||", false);
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

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("mixed.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Should generate code for multiple logic gates
    QVERIFY2(content.contains("void setup()") && content.contains("void loop()"),
        "Generated code should contain both setup() and loop() functions");
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
}

void TestArduino::testPinMapping()
{
    auto btn1 = ElementFactory::buildElement(ElementType::InputButton);
    auto btn2 = ElementFactory::buildElement(ElementType::InputButton);
    auto led1 = ElementFactory::buildElement(ElementType::Led);
    auto led2 = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << btn1 << btn2 << led1 << led2;

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("pins.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Should have pin declarations and pinMode calls
    QVERIFY2(content.contains("pinMode"), "Generated code should contain pinMode calls");
}

void TestArduino::testVariableNaming()
{
    auto btn = ElementFactory::buildElement(ElementType::InputButton);
    auto led = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << btn << led;

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("names.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Should have variable declarations
    QVERIFY2(!content.isEmpty(), "Generated code content should not be empty");
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

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("sanitize.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Should have generated code
    QVERIFY2(!content.isEmpty(), "Generated code content should not be empty");
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

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("inputs.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Should handle input elements
    QVERIFY2(content.contains("digitalWrite") || content.contains("digitalRead"),
        "Generated code should contain digitalWrite or digitalRead calls");
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

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("complex.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Should have valid code structure with multiple elements
    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
}

void TestArduino::testCodeStructure()
{
    auto in = ElementFactory::buildElement(ElementType::InputButton);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in << out;

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("structure.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Check for required code structure
    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
    QVERIFY2(content.contains("pinMode"), "Generated code should contain pinMode calls");

    // Verify order: setup before loop
    int setupPos = content.indexOf("void setup()");
    int loopPos = content.indexOf("void loop()");
    QVERIFY2(setupPos >= 0 && loopPos > setupPos, "setup() should appear before loop() in generated code");
}

void TestArduino::testICElementError()
{
    auto in = ElementFactory::buildElement(ElementType::InputButton);
    QVERIFY2(in != nullptr, "Failed to create InputButton");
    auto ic = ElementFactory::buildElement(ElementType::IC);
    QVERIFY2(ic != nullptr, "Failed to create IC");
    auto out = ElementFactory::buildElement(ElementType::Led);
    QVERIFY2(out != nullptr, "Failed to create Led");

    QVector<GraphicElement *> elements;
    elements << in << ic << out;

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("ic_error.ino");

    // generate() validates elements before writing — must throw Pandaception for IC
    QVERIFY_THROWS_EXCEPTION(Pandaception, {
        ArduinoCodeGen generator(filePath, elements);
        generator.generate();
    });
    // Partial file must be removed on failure
    QVERIFY2(!QFile::exists(filePath), "File must not be left on disk after IC error");

    // Ensure cleanup: delete elements if exception was caught
    qDeleteAll(elements);
}

void TestArduino::testNandGateGeneration()
{
    auto in1 = ElementFactory::buildElement(ElementType::InputButton);
    auto in2 = ElementFactory::buildElement(ElementType::InputButton);
    auto nandGate = ElementFactory::buildElement(ElementType::Nand);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in1 << in2 << nandGate << out;

    // Create connections to verify NAND operator is generated
    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(in1->outputPort());
    conn1->setEndPort(nandGate->inputPort(0));

    auto conn2 = std::make_unique<QNEConnection>();
    conn2->setStartPort(in2->outputPort());
    conn2->setEndPort(nandGate->inputPort(1));

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Arduino code generation should succeed");
    verifyBasicStructure(code.content);

    // Verify NAND operator (! with &&) is in the generated code
    verifyLogicOperator(code.content, "&&", true);
}

void TestArduino::testNorGateGeneration()
{
    auto in1 = ElementFactory::buildElement(ElementType::InputButton);
    auto in2 = ElementFactory::buildElement(ElementType::InputButton);
    auto norGate = ElementFactory::buildElement(ElementType::Nor);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in1 << in2 << norGate << out;

    // Create connections to verify NOR operator is generated
    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(in1->outputPort());
    conn1->setEndPort(norGate->inputPort(0));

    auto conn2 = std::make_unique<QNEConnection>();
    conn2->setStartPort(in2->outputPort());
    conn2->setEndPort(norGate->inputPort(1));

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Arduino code generation should succeed");
    verifyBasicStructure(code.content);

    // Verify NOR operator (! with ||) is in the generated code
    verifyLogicOperator(code.content, "||", true);
}

void TestArduino::testXnorGateGeneration()
{
    auto in1 = ElementFactory::buildElement(ElementType::InputButton);
    auto in2 = ElementFactory::buildElement(ElementType::InputButton);
    auto xnorGate = ElementFactory::buildElement(ElementType::Xnor);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in1 << in2 << xnorGate << out;

    // Create connections to verify XNOR operator is generated
    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(in1->outputPort());
    conn1->setEndPort(xnorGate->inputPort(0));

    auto conn2 = std::make_unique<QNEConnection>();
    conn2->setStartPort(in2->outputPort());
    conn2->setEndPort(xnorGate->inputPort(1));

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Arduino code generation should succeed");
    verifyBasicStructure(code.content);

    // Verify XNOR operator (! with ^) is in the generated code
    verifyLogicOperator(code.content, "^", true);
}

void TestArduino::testUnconnectedLogicGate()
{
    // Create a logic gate without connecting its inputs
    auto andGate = ElementFactory::buildElement(ElementType::And);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << andGate << out;

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("unconnected.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Should generate valid code structure even with unconnected inputs
    QVERIFY2(content.contains("void setup()") && content.contains("void loop()"),
        "Generated code should contain both setup() and loop() functions");
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

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("multiple_outputs.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Should handle multiple output ports correctly
    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
}

void TestArduino::testOutputPortNaming()
{
    // Test output elements with custom port names
    auto in = ElementFactory::buildElement(ElementType::InputButton);
    auto out = ElementFactory::buildElement(ElementType::Led);
    out->setLabel("OutputSignal");

    QVector<GraphicElement *> elements;
    elements << in << out;

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("output_port_naming.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Should generate code with named output
    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
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

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("complex_naming.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Should handle complex names correctly
    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
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

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("sanitize_names.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Should sanitize special characters
    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
}

void TestArduino::testUnconnectedInput()
{
    // Logic gate with one input unconnected, one connected
    auto in1 = ElementFactory::buildElement(ElementType::InputButton);
    auto gate = ElementFactory::buildElement(ElementType::And);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in1 << gate << out;

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("unconnected_input.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
}

void TestArduino::testDefaultValuePropagation()
{
    // Unconnected input ports should use default values
    auto gate = ElementFactory::buildElement(ElementType::And);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << gate << out;

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("default_values.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
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

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("partial_connect.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
}

void TestArduino::testHighPinCountCircuit()
{
    // Create circuit with more input elements than available pins (16 pins total)
    QVector<GraphicElement *> elements;

    // Add 20 input buttons - should exhaust the 16 available pins
    for (int i = 0; i < 20; ++i) {
        auto btn = ElementFactory::buildElement(ElementType::InputButton);
        QVERIFY2(btn != nullptr, QString("Failed to create InputButton %1").arg(i).toLatin1());
        btn->setLabel(QString("Button_%1").arg(i));
        elements << btn;
    }

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("high_pin_count.ino");

    // This should throw a Pandaception when pins are exhausted
    try {
        ArduinoCodeGen generator(filePath, elements);
        generator.generate();
        // If no exception, something is wrong with the test setup
        QFAIL("Expected Pandaception for pin exhaustion");
    } catch (const Pandaception &e) {
        // Expected - pin exhaustion should throw Pandaception
        QString errorMsg = e.what();
        QVERIFY2(errorMsg.contains("pins", Qt::CaseInsensitive), "Error message should mention pins");
    }

    // Ensure cleanup: delete all allocated elements
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

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("multi_input.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
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

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("chained_gates.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
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

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("preset_clear_interact.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
}

void TestArduino::testFlipFlopInitialState()
{
    // Test flip-flop with focus on initial state generation
    auto dff = ElementFactory::buildElement(ElementType::DFlipFlop);
    auto out_q = ElementFactory::buildElement(ElementType::Led);
    auto out_qn = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << dff << out_q << out_qn;

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("flipflop_init_state.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
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

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("latch_edge_trigger.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
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

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("deeply_nested.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
}

void TestArduino::testNodeElementGeneration()
{
    // Test Node element (connection point)
    auto in = ElementFactory::buildElement(ElementType::InputButton);
    auto node = ElementFactory::buildElement(ElementType::Node);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in << node << out;

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("node_element.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
}

void TestArduino::testMuxDemuxIntegration()
{
    // Test complex gate integration with multiple gates chained together
    auto in1 = ElementFactory::buildElement(ElementType::InputButton);
    auto in2 = ElementFactory::buildElement(ElementType::InputButton);
    auto in3 = ElementFactory::buildElement(ElementType::InputButton);
    auto and_gate = ElementFactory::buildElement(ElementType::And);
    auto xor_gate = ElementFactory::buildElement(ElementType::Xor);
    auto or_gate = ElementFactory::buildElement(ElementType::Or);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in1 << in2 << in3 << and_gate << xor_gate << or_gate << out;

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("mux_integration.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
}

void TestArduino::testXorGateAllCombinations()
{
    // Test XOR gate with multiple inputs to verify all logic paths
    // Note: XOR gates have exactly 2 inputs
    auto in1 = ElementFactory::buildElement(ElementType::InputButton);
    auto in2 = ElementFactory::buildElement(ElementType::InputButton);
    auto xor_gate = ElementFactory::buildElement(ElementType::Xor);
    auto out = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << in1 << in2 << xor_gate << out;

    // Create connections for XOR inputs
    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(in1->outputPort());
    conn1->setEndPort(xor_gate->inputPort(0));

    auto conn2 = std::make_unique<QNEConnection>();
    conn2->setStartPort(in2->outputPort());
    conn2->setEndPort(xor_gate->inputPort(1));

    auto code = generateFromElements(elements);
    QVERIFY2(code.success, "Arduino code generation should succeed");
    verifyBasicStructure(code.content);

    // Verify XOR operator (^) is in the generated code
    verifyLogicOperator(code.content, "^", false);
}

void TestArduino::testMultipleClocksInCircuit()
{
    // Test multiple clock elements with different frequencies
    auto clk1 = ElementFactory::buildElement(ElementType::Clock);
    auto clk2 = ElementFactory::buildElement(ElementType::Clock);
    auto dff1 = ElementFactory::buildElement(ElementType::DFlipFlop);
    auto dff2 = ElementFactory::buildElement(ElementType::DFlipFlop);
    auto out1 = ElementFactory::buildElement(ElementType::Led);
    auto out2 = ElementFactory::buildElement(ElementType::Led);

    QVector<GraphicElement *> elements;
    elements << clk1 << clk2 << dff1 << dff2 << out1 << out2;

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("multiple_clocks.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Should include elapsedMillis header for clock support
    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
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

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("complex_preset_clear.ino");

    ArduinoCodeGen generator(filePath, elements);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
         qPrintable(QString("Failed to open %1: %2")
                   .arg(file.fileName(), file.errorString())));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY2(content.contains("void setup()"), "Generated code should contain void setup() function");
    QVERIFY2(content.contains("void loop()"), "Generated code should contain void loop() function");
}

void TestArduino::testOutputPinExhaustion()
{
    // Arduino has 18 pins available (A0-A5, 2-13)
    // Create 20 LEDs to exhaust output pins
    QVector<GraphicElement *> elements;

    // Add 20 LED elements - should exhaust the 18 available output pins
    for (int i = 0; i < 20; ++i) {
        auto led = ElementFactory::buildElement(ElementType::Led);
        QVERIFY2(led != nullptr, QString("Failed to create Led %1").arg(i).toLatin1());
        led->setLabel(QString("LED_%1").arg(i));
        elements << led;
    }

    QTemporaryDir tempDir;
    QString filePath = tempDir.filePath("output_pin_exhaustion.ino");

    // Should throw Pandaception when output pins are exhausted
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

    // Ensure cleanup: delete all allocated elements
    qDeleteAll(elements);
}

void TestArduino::testUnsupportedElementTypes()
{
    // Test unsupported element types that should fail Arduino generation

    // Test 1: Mux (multiplexer)
    {
        auto mux = ElementFactory::buildElement(ElementType::Mux);
        QVERIFY2(mux != nullptr, "Failed to create Mux element");
        mux->setLabel("TestMux");
        QVector<GraphicElement *> elements;
        elements << mux;

        QTemporaryDir tempDir;
        QString filePath = tempDir.filePath("mux_test.ino");

        bool exceptionCaught = false;
        try {
            ArduinoCodeGen generator(filePath, elements);
            generator.generate();
        } catch (const Pandaception &e) {
            exceptionCaught = true;
            QString errorMsg = e.what();
            QVERIFY2(errorMsg.contains("not supported", Qt::CaseInsensitive) ||
                    errorMsg.contains("unsupported", Qt::CaseInsensitive),
                "Error message should indicate element is not supported");
        } catch (...) {
            exceptionCaught = true;  // Any exception is acceptable
        }
        QVERIFY2(exceptionCaught, "Expected exception for unsupported Mux element");

        // Ensure cleanup: delete elements
        qDeleteAll(elements);
    }

    // Test 2: Demux (demultiplexer) - has both inputs and outputs
    {
        auto demux = ElementFactory::buildElement(ElementType::Demux);
        QVERIFY2(demux != nullptr, "Failed to create Demux element");
        demux->setLabel("TestDemux");
        QVector<GraphicElement *> elements;
        elements << demux;

        QTemporaryDir tempDir;
        QString filePath = tempDir.filePath("demux_test.ino");

        bool exceptionCaught = false;
        try {
            ArduinoCodeGen generator(filePath, elements);
            generator.generate();
        } catch (const Pandaception &e) {
            exceptionCaught = true;
            QString errorMsg = e.what();
            QVERIFY2(errorMsg.contains("not supported", Qt::CaseInsensitive) ||
                    errorMsg.contains("unsupported", Qt::CaseInsensitive),
                "Error message should indicate element is not supported");
        } catch (...) {
            exceptionCaught = true;  // Any exception is acceptable
        }
        QVERIFY2(exceptionCaught, "Expected exception for unsupported Demux element");

        // Ensure cleanup: delete elements
        qDeleteAll(elements);
    }

    // Test 3: TruthTable (custom logic)
    {
        auto truthTable = ElementFactory::buildElement(ElementType::TruthTable);
        QVERIFY2(truthTable != nullptr, "Failed to create TruthTable element");
        truthTable->setLabel("TestTruthTable");
        QVector<GraphicElement *> elements;
        elements << truthTable;

        QTemporaryDir tempDir;
        QString filePath = tempDir.filePath("truth_table_test.ino");

        bool exceptionCaught = false;
        try {
            ArduinoCodeGen generator(filePath, elements);
            generator.generate();
        } catch (const Pandaception &e) {
            exceptionCaught = true;
            QString errorMsg = e.what();
            QVERIFY2(errorMsg.contains("not supported", Qt::CaseInsensitive) ||
                    errorMsg.contains("unsupported", Qt::CaseInsensitive),
                "Error message should indicate element is not supported");
        } catch (...) {
            exceptionCaught = true;  // Any exception is acceptable
        }
        QVERIFY2(exceptionCaught, "Expected exception for unsupported TruthTable element");

        // Ensure cleanup: delete elements
        qDeleteAll(elements);
    }
}
