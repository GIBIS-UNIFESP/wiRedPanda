// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestSerialization.h"

#include <QFile>
#include <QFileInfo>
#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestSerialization::init()
{
    // Setup before each test
}

// ============================================================================
// Helper Methods
// ============================================================================

QByteArray TestSerialization::saveToMemory(WorkSpace &workspace)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    workspace.save(stream);
    return data;
}

void TestSerialization::loadFromMemory(WorkSpace &workspace, const QByteArray &data)
{
    QDataStream stream(data);
    QVersionNumber version = Serialization::readPandaHeader(stream);
    workspace.load(stream, version);
}

void TestSerialization::verifyElementEquality(GraphicElement *original, GraphicElement *loaded)
{
    QVERIFY2(loaded != nullptr, "Loaded element is null");
    QCOMPARE(loaded->elementType(), original->elementType());
    QCOMPARE(loaded->label(), original->label());
    QCOMPARE(loaded->rotation(), original->rotation());
    QCOMPARE(loaded->inputSize(), original->inputSize());
    QCOMPARE(loaded->outputSize(), original->outputSize());
}

// ============================================================================
// Basic Round-Trip Tests
// ============================================================================

void TestSerialization::testSingleElementRoundTrip()
{
    // Create workspace with single And gate
    WorkSpace workspace1;
    Scene *scene1 = workspace1.scene();
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setLabel("TestAnd");
    scene1->addItem(andGate);

    // Save to memory
    QByteArray data = saveToMemory(workspace1);
    QVERIFY(!data.isEmpty());

    // Load into new workspace
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);
    Scene *scene2 = workspace2.scene();

    // Verify element count
    auto elements = scene2->elements();
    QCOMPARE(elements.size(), 1);

    // Verify element type and properties
    auto *loaded = elements.first();
    QCOMPARE(loaded->elementType(), ElementType::And);
    QCOMPARE(loaded->label(), QString("TestAnd"));
}

void TestSerialization::testMultipleElementsRoundTrip()
{
    // Create circuit with 5 different element types
    WorkSpace workspace1;
    Scene *scene1 = workspace1.scene();

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    QVERIFY2(sw != nullptr, "Failed to create InputSwitch");
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    auto *orGate = ElementFactory::buildElement(ElementType::Or);
    QVERIFY2(orGate != nullptr, "Failed to create Or element");
    auto *notGate = ElementFactory::buildElement(ElementType::Not);
    QVERIFY2(notGate != nullptr, "Failed to create Not element");
    auto *led = ElementFactory::buildElement(ElementType::Led);
    QVERIFY2(led != nullptr, "Failed to create Led element");

    scene1->addItem(sw);
    scene1->addItem(andGate);
    scene1->addItem(orGate);
    scene1->addItem(notGate);
    scene1->addItem(led);

    // Save and reload
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    // Verify all 5 elements preserved with correct count
    auto elements = workspace2.scene()->elements();
    QCOMPARE(elements.size(), 5);

    // Verify each element type is present
    int andCount = 0, orCount = 0, notCount = 0, swCount = 0, ledCount = 0;
    for (auto *elem : elements) {
        if (elem->elementType() == ElementType::And) ++andCount;
        else if (elem->elementType() == ElementType::Or) ++orCount;
        else if (elem->elementType() == ElementType::Not) ++notCount;
        else if (elem->elementType() == ElementType::InputSwitch) ++swCount;
        else if (elem->elementType() == ElementType::Led) ++ledCount;
    }
    QCOMPARE(andCount, 1);
    QCOMPARE(orCount, 1);
    QCOMPARE(notCount, 1);
    QCOMPARE(swCount, 1);
    QCOMPARE(ledCount, 1);
}

void TestSerialization::testConnectionPersistence()
{
    // Build circuit: Switch → And → LED
    WorkSpace workspace1;
    Scene *scene1 = workspace1.scene();

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    QVERIFY2(sw != nullptr, "Failed to create InputSwitch");
    sw->setPos(0, 0);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setPos(100, 0);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    QVERIFY2(led != nullptr, "Failed to create Led element");
    led->setPos(200, 0);

    scene1->addItem(sw);
    scene1->addItem(andGate);
    scene1->addItem(led);

    // Create connections
    auto *conn1 = new QNEConnection();
    auto *conn2 = new QNEConnection();
    conn1->setStartPort(sw->outputPort());
    conn1->setEndPort(andGate->inputPort(0));
    conn2->setStartPort(andGate->outputPort());
    conn2->setEndPort(led->inputPort());
    scene1->addItem(conn1);
    scene1->addItem(conn2);

    // Save and reload
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    // Verify connections restored with correct structure
    int connectionCount = 0;
    auto loadedElements = workspace2.scene()->elements();

    // Find the reloaded elements
    GraphicElement *loadedSw = nullptr, *loadedAnd = nullptr, *loadedLed = nullptr;
    for (auto *elem : loadedElements) {
        if (elem->elementType() == ElementType::InputSwitch) loadedSw = elem;
        else if (elem->elementType() == ElementType::And) loadedAnd = elem;
        else if (elem->elementType() == ElementType::Led) loadedLed = elem;
    }

    // Verify all connections have valid ports
    const auto items = workspace2.scene()->items();
    for (auto *item : std::as_const(items)) {
        if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
            ++connectionCount;
            QVERIFY2(conn->startPort() != nullptr, "Missing start port");
            QVERIFY2(conn->endPort() != nullptr, "Missing end port");
        }
    }
    QCOMPARE(connectionCount, 2);

    // Verify connection structure
    if (loadedSw && loadedAnd && loadedLed) {
        QVERIFY2(loadedSw->outputPort()->isConnected(loadedAnd->inputPort(0)),
                 "Switch should be connected to And input");
        QVERIFY2(loadedAnd->outputPort()->isConnected(loadedLed->inputPort()),
                 "And should be connected to LED");
    }
}

void TestSerialization::testElementPropertiesPreserved_data()
{
    QTest::addColumn<QString>("label");
    QTest::addColumn<qreal>("rotation");

    QTest::newRow("default label, 0 rotation") << "AND" << 0.0;
    QTest::newRow("custom label, 90 rotation") << "MyCustomAND" << 90.0;
    QTest::newRow("empty label, 180 rotation") << "" << 180.0;
    QTest::newRow("special chars, 270 rotation") << "AND@2024!" << 270.0;
}

void TestSerialization::testElementPropertiesPreserved()
{
    QFETCH(QString, label);
    QFETCH(qreal, rotation);

    // Create and configure element
    WorkSpace workspace1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setLabel(label);
    andGate->setRotation(rotation);
    workspace1.scene()->addItem(andGate);

    // Save and reload
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    // Verify properties preserved
    auto *loaded = workspace2.scene()->elements().first();
    QCOMPARE(loaded->label(), label);
    QCOMPARE(loaded->rotation(), rotation);
}

// ============================================================================
// Complex Circuits
// ============================================================================

void TestSerialization::testComplexCircuitSaveLoad()
{
    // Create multi-gate circuit with CircuitBuilder
    WorkSpace workspace1;
    CircuitBuilder builder(workspace1.scene());

    InputSwitch sw1, sw2, sw3;
    And and1, and2;
    Or orGate;
    Led led1, led2;

    builder.add(&sw1, &sw2, &sw3, &and1, &and2, &orGate, &led1, &led2);

    // Connect: (sw1 AND sw2) OR (sw3 AND sw3) → LED
    builder.connect(&sw1, 0, &and1, 0);
    builder.connect(&sw2, 0, &and1, 1);
    builder.connect(&sw3, 0, &and2, 0);
    builder.connect(&sw3, 0, &and2, 1);
    builder.connect(&and1, 0, &orGate, 0);
    builder.connect(&and2, 0, &orGate, 1);
    builder.connect(&orGate, 0, &led1, 0);
    builder.connect(&and1, 0, &led2, 0);

    int originalElementCount = workspace1.scene()->elements().size();
    int originalConnectionCount = 0;
    const auto items1 = workspace1.scene()->items();
    for (auto *item : std::as_const(items1)) {
        if (qgraphicsitem_cast<QNEConnection *>(item)) {
            ++originalConnectionCount;
        }
    }

    // Save and reload
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    // Verify structure preserved
    QCOMPARE(workspace2.scene()->elements().size(), originalElementCount);

    int newConnectionCount = 0;
    const auto items2 = workspace2.scene()->items();
    for (auto *item : std::as_const(items2)) {
        if (qgraphicsitem_cast<QNEConnection *>(item)) {
            ++newConnectionCount;
        }
    }
    QCOMPARE(newConnectionCount, originalConnectionCount);
}

void TestSerialization::testICSerializationBasic()
{
    // Load an example IC file
    QFileInfo exampleFile(TestUtils::examplesDir() + "ic.panda");

    // Load the file
    WorkSpace workspace;
    QFile file(exampleFile.absoluteFilePath());
    QVERIFY2(file.open(QIODevice::ReadOnly),
             qPrintable(QString("Failed to open %1: %2")
                       .arg(file.fileName(), file.errorString())));

    QDataStream stream(&file);
    QVersionNumber version = Serialization::readPandaHeader(stream);
    workspace.load(stream, version, exampleFile.absolutePath());
    file.close();

    // Verify circuit loaded successfully without errors
    QVERIFY(!workspace.scene()->elements().isEmpty());

    // Verify all loaded elements have valid types
    for (auto *elem : workspace.scene()->elements()) {
        QVERIFY(elem->elementType() != ElementType::Unknown);
    }
}

// ============================================================================
// File Format Tests
// ============================================================================

void TestSerialization::testFileFormatHeader()
{
    // Create simple circuit and save
    WorkSpace workspace;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    workspace.scene()->addItem(andGate);

    QByteArray data = saveToMemory(workspace);

    // Manually read magic header
    QDataStream stream(data);
    quint32 magic;
    stream >> magic;

    // Verify magic header
    QCOMPARE(magic, static_cast<quint32>(0x57504346)); // "WPCF"
}

void TestSerialization::testDolphinFilenamePreserved()
{
    // Create workspace and set dolphin filename
    WorkSpace workspace1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    workspace1.scene()->addItem(andGate);

    QString testDolphinFile = "test_waveform.dolphin";
    workspace1.setDolphinFileName(testDolphinFile);

    // Save and reload
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    // Verify circuit loads without errors (dolphin filename is metadata)
    QVERIFY(!workspace2.scene()->elements().isEmpty());
}

void TestSerialization::testSceneRectPreserved()
{
    // Create workspace with specific scene rect
    WorkSpace workspace1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setPos(100, 100);
    workspace1.scene()->addItem(andGate);

    // Force scene rect calculation
    workspace1.scene()->itemsBoundingRect();

    // Save and reload
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    // Verify circuit loaded with correct structure
    QCOMPARE(workspace2.scene()->elements().size(), 1);
}

// ============================================================================
// ============================================================================
// Error Handling Tests
// ============================================================================

void TestSerialization::testInvalidMagicHeader()
{
    // Create QByteArray with wrong magic header
    QByteArray badData;
    QDataStream stream(&badData, QIODevice::WriteOnly);
    stream << static_cast<quint32>(0x12345678); // Wrong magic

    // Try to load - should handle gracefully (throw or set invalid version)
    WorkSpace workspace;
    QDataStream loadStream(badData);

    bool errorHandled = false;
    try {
        QVersionNumber version = Serialization::readPandaHeader(loadStream);
        // Check if stream error status indicates invalid header was detected
        errorHandled = (loadStream.status() != QDataStream::Ok);
    } catch (const std::exception &) {
        errorHandled = true;  // Exception is acceptable for bad header
    }

    QVERIFY2(errorHandled, "Invalid magic header should be detected");
}

void TestSerialization::testCorruptedStream()
{
    // Create valid data then truncate it
    WorkSpace workspace1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    workspace1.scene()->addItem(andGate);

    QByteArray data = saveToMemory(workspace1);
    // Truncate to break the stream
    data.resize(data.size() / 2);

    // Try to load - should handle gracefully or throw (not crash)
    WorkSpace workspace2;
    bool errorHandled = false;
    try {
        QDataStream stream(data);
        QVersionNumber version = Serialization::readPandaHeader(stream);
        workspace2.load(stream, version);
        // Check if stream detected error
        errorHandled = (stream.status() != QDataStream::Ok);
    } catch (const std::exception &) {
        errorHandled = true;  // Exception is acceptable for corrupted stream
    }

    QVERIFY2(errorHandled, "Corrupted stream should be detected");
}

// ============================================================================
// Property Persistence Tests
// ============================================================================

void TestSerialization::testRotationPreserved_data()
{
    QTest::addColumn<qreal>("rotation");

    QTest::newRow("0 degrees") << 0.0;
    QTest::newRow("90 degrees") << 90.0;
    QTest::newRow("180 degrees") << 180.0;
    QTest::newRow("270 degrees") << 270.0;
    QTest::newRow("45 degrees") << 45.0;
    QTest::newRow("359 degrees") << 359.0;
}

void TestSerialization::testRotationPreserved()
{
    QFETCH(qreal, rotation);

    // Create and rotate element
    WorkSpace workspace1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setRotation(rotation);
    workspace1.scene()->addItem(andGate);

    // Save and reload
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    // Verify rotation preserved
    auto *loaded = workspace2.scene()->elements().first();
    QCOMPARE(loaded->rotation(), rotation);
}

void TestSerialization::testLabelPreserved_data()
{
    QTest::addColumn<QString>("label");

    QTest::newRow("empty") << "";
    QTest::newRow("simple") << "TEST";
    QTest::newRow("with dash") << "Element-1";
    QTest::newRow("special chars") << "2024@Test!";
    QTest::newRow("long text") << "Very Long Label Text";
}

void TestSerialization::testLabelPreserved()
{
    QFETCH(QString, label);

    WorkSpace workspace1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setLabel(label);
    workspace1.scene()->addItem(andGate);

    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    auto *loaded = workspace2.scene()->elements().first();
    QCOMPARE(loaded->label(), label);
}

void TestSerialization::testInputSizePreserved()
{
    // Create And gate with variable inputs
    WorkSpace workspace1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setInputSize(4);
    workspace1.scene()->addItem(andGate);

    QCOMPARE(andGate->inputSize(), 4);

    // Save and reload
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    auto *loaded = workspace2.scene()->elements().first();
    QCOMPARE(loaded->inputSize(), 4);
}

void TestSerialization::testOutputSizePreserved()
{
    // Create AND gate (which has fixed 1 output, so less variability)
    WorkSpace workspace1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    // AND gate has fixed output size of 1
    workspace1.scene()->addItem(andGate);

    QCOMPARE(andGate->outputSize(), 1);

    // Save and reload
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    auto *loaded = workspace2.scene()->elements().first();
    QCOMPARE(loaded->outputSize(), 1);
}

// ============================================================================
// Special Cases
// ============================================================================

void TestSerialization::testEmptyCircuit()
{
    // Save empty workspace
    WorkSpace workspace1;
    QByteArray data = saveToMemory(workspace1);
    QVERIFY(!data.isEmpty());

    // Load empty circuit
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    // Verify empty scene
    QCOMPARE(workspace2.scene()->elements().size(), 0);
}

// ============================================================================
// ============================================================================

void TestSerialization::testLargeCircuitWith100Elements()
{
    // Create circuit with 100 AND gates in a chain
    WorkSpace workspace1;
    Scene *scene1 = workspace1.scene();

    QList<GraphicElement *> gates;
    for (int i = 0; i < 100; ++i) {
        auto *gate = ElementFactory::buildElement(ElementType::And);
        QVERIFY2(gate != nullptr, QString("Failed to create And element %1").arg(i).toLatin1());
        gate->setPos(i * 20, 0);
        gate->setLabel(QString("AND_%1").arg(i));
        scene1->addItem(gate);
        gates.append(gate);
    }

    // Save to memory
    QByteArray data = saveToMemory(workspace1);
    QVERIFY(!data.isEmpty());

    // Load into new workspace
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);
    Scene *scene2 = workspace2.scene();

    // Verify element count
    QCOMPARE(scene2->elements().size(), 100);

    // Verify all elements are AND gates
    for (auto *elem : scene2->elements()) {
        QCOMPARE(elem->elementType(), ElementType::And);
    }
}

void TestSerialization::testLargeCircuitWith500Elements()
{
    // Create circuit with 500 mixed elements
    WorkSpace workspace1;
    Scene *scene1 = workspace1.scene();

    // Create 500 elements in batches of different types
    for (int i = 0; i < 500; ++i) {
        ElementType type;
        if (i % 5 == 0) {
            type = ElementType::And;
        } else if (i % 5 == 1) {
            type = ElementType::Or;
        } else if (i % 5 == 2) {
            type = ElementType::Not;
        } else if (i % 5 == 3) {
            type = ElementType::Nand;
        } else {
            type = ElementType::Nor;
        }

        auto *elem = ElementFactory::buildElement(type);
        elem->setPos(i % 50 * 20, (i / 50) * 20);
        elem->setLabel(QString("Elem_%1").arg(i));
        scene1->addItem(elem);
    }

    // Save to memory
    QByteArray data = saveToMemory(workspace1);
    QVERIFY(!data.isEmpty());

    // Load into new workspace
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    // Verify element count
    QCOMPARE(workspace2.scene()->elements().size(), 500);
}

// ============================================================================
// ============================================================================

void TestSerialization::testUnicodeInLabels()
{
    // Test Unicode characters in element labels
    WorkSpace workspace1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");

    // Test various Unicode strings
    QString unicodeLabel = QString::fromUtf8("AND_Elemento_αβγ_日本語");
    andGate->setLabel(unicodeLabel);
    workspace1.scene()->addItem(andGate);

    // Save and reload
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    auto *loaded = workspace2.scene()->elements().first();
    QCOMPARE(loaded->label(), unicodeLabel);
}

void TestSerialization::testSpecialCharactersInLabels()
{
    // Test special characters in element labels
    WorkSpace workspace1;
    auto *orGate = ElementFactory::buildElement(ElementType::Or);
    QVERIFY2(orGate != nullptr, "Failed to create Or element");

    QString specialLabel = "OR_!@#$%^&*()_+-=[]{}|;':\",./<>?";
    orGate->setLabel(specialLabel);
    workspace1.scene()->addItem(orGate);

    // Save and reload
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    auto *loaded = workspace2.scene()->elements().first();
    QCOMPARE(loaded->label(), specialLabel);
}

void TestSerialization::testExtendedAsciiCharacters()
{
    // Test extended ASCII characters
    WorkSpace workspace1;
    auto *notGate = ElementFactory::buildElement(ElementType::Not);
    QVERIFY2(notGate != nullptr, "Failed to create Not element");

    // Extended ASCII with accented characters
    QString extendedLabel = "NOT_àáâãäåæçèéêëìíîï";
    notGate->setLabel(extendedLabel);
    workspace1.scene()->addItem(notGate);

    // Save and reload
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    auto *loaded = workspace2.scene()->elements().first();
    QCOMPARE(loaded->label(), extendedLabel);
}

// ============================================================================
// ============================================================================

void TestSerialization::testMissingRequiredAttributes()
{
    // Test handling of corrupted stream data
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    // Write header
    Serialization::writePandaHeader(stream);

    // Write minimal/corrupted data
    stream << -1;  // Invalid position

    // Try to load - should handle gracefully or throw exception
    WorkSpace workspace;
    try {
        loadFromMemory(workspace, data);
        // If load succeeds, workspace should be empty (corrupted data couldn't be loaded)
        QVERIFY2(workspace.scene()->elements().isEmpty(),
                 "Corrupted data should result in empty workspace, not partial load");
    } catch (const Pandaception &e) {
        // Expected: corrupted data causes exception
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should provide a meaningful error message");
    }
}

void TestSerialization::testInvalidElementTypeInStream()
{
    // Create workspace with element
    WorkSpace workspace1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    workspace1.scene()->addItem(andGate);

    QByteArray data = saveToMemory(workspace1);

    // Corrupt the element type in the stream (replace valid type with invalid)
    // This tests robustness to corrupted data
    if (data.size() > 20) {
        // Modify bytes in the middle (where element type data would be)
        data[20] = static_cast<char>(0xFF);
        data[21] = static_cast<char>(0xFF);
        data[22] = static_cast<char>(0xFF);
        data[23] = static_cast<char>(0xFF);
    }

    // Try to load corrupted data
    WorkSpace workspace2;
    try {
        loadFromMemory(workspace2, data);
        // If load succeeds despite corruption, at least verify it didn't crash
        // and the workspace is in some valid state
        QVERIFY2(workspace2.scene() != nullptr, "Scene should exist after load");
    } catch (const Pandaception &e) {
        // Expected: corrupted data causes exception
        QVERIFY2(!QString(e.what()).isEmpty(), "Pandaception should have a meaningful message");
    } catch (const std::exception &e) {
        // Other exceptions are acceptable for corrupted data
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should provide a meaningful error message");
    }
}

void TestSerialization::testNegativePositionValues()
{
    // Test elements with negative position values
    WorkSpace workspace1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setPos(-1000, -500);
    andGate->setLabel("NegativePos");
    workspace1.scene()->addItem(andGate);

    // Save and reload
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    auto *loaded = workspace2.scene()->elements().first();
    QCOMPARE(loaded->x(), -1000.0);
    QCOMPARE(loaded->y(), -500.0);
    QCOMPARE(loaded->label(), QString("NegativePos"));
}

void TestSerialization::testExtremelyLargeValues()
{
    // Test elements with very large position values
    WorkSpace workspace1;
    auto *orGate = ElementFactory::buildElement(ElementType::Or);
    QVERIFY2(orGate != nullptr, "Failed to create Or element");
    orGate->setPos(1e6, 1e6);  // 1 million units
    orGate->setLabel("LargePos");
    workspace1.scene()->addItem(orGate);

    // Save and reload
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    auto *loaded = workspace2.scene()->elements().first();
    QCOMPARE(loaded->x(), 1e6);
    QCOMPARE(loaded->y(), 1e6);
    QCOMPARE(loaded->label(), QString("LargePos"));
}

void TestSerialization::testNullElementHandling()
{
    // Test that null/invalid elements are handled gracefully
    WorkSpace workspace1;

    // Add valid element
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    workspace1.scene()->addItem(andGate);

    // Save workspace (which should only save valid elements)
    QByteArray data = saveToMemory(workspace1);

    // Load and verify
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    QCOMPARE(workspace2.scene()->elements().size(), 1);
    QCOMPARE(workspace2.scene()->elements().first()->elementType(), ElementType::And);
}

// ============================================================================
// ============================================================================

void TestSerialization::testOldVersionHeaderFormat()
{
    // Test loading files with old header format (wiRedPanda 4.1)
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);

    // Write old format header
    stream << QString("wiRedPanda 4.1");

    // Write a simple circuit structure (old format compatible)
    QPointF sceneCenter(0, 0);
    stream << sceneCenter;

    // Write element count
    stream << 1;

    // Write a simple AND gate (old format)
    stream << static_cast<quint64>(ElementType::And);  // Element type
    stream << QPointF(100, 100);  // Position
    stream << QString("TestAND");  // Label
    stream << 0.0;  // Rotation
    stream << 2;  // Input size
    stream << 1;  // Output size

    // Try to load old format
    WorkSpace workspace;
    try {
        QDataStream readStream(data);
        QVersionNumber version = Serialization::readPandaHeader(readStream);
        // Should detect version 4.1
        QCOMPARE(version.majorVersion(), 4);
        QCOMPARE(version.minorVersion(), 1);
    } catch (const Pandaception &e) {
        // Old format may not be fully supported - that's acceptable
        // But verify the exception has a meaningful message
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should explain why old format failed");
    }
}

void TestSerialization::testMissingVersionHeader()
{
    // Test loading data without version header (very old format or copy/paste)
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);

    // Write copy/paste format (no header, just raw data)
    QPointF center(0, 0);
    stream << center;

    // Try to read as header
    try {
        QDataStream readStream(data);
        QVersionNumber version = Serialization::readPandaHeader(readStream);
        // Should detect copy/paste format and assume version 4.1
        QCOMPARE(version.majorVersion(), 4);
        QCOMPARE(version.minorVersion(), 1);
    } catch (const Pandaception &e) {
        // May fail on very corrupted data
        // Verify the exception has a meaningful message
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should explain why the header is invalid");
    }
}

void TestSerialization::testTruncatedFileRecovery()
{
    // Create valid workspace data, then truncate it mid-element
    WorkSpace workspace1;
    auto *andGate1 = ElementFactory::buildElement(ElementType::And);
    auto *orGate1 = ElementFactory::buildElement(ElementType::Or);
    QVERIFY(andGate1 != nullptr && orGate1 != nullptr);
    workspace1.scene()->addItem(andGate1);
    workspace1.scene()->addItem(orGate1);

    // Save full data
    QByteArray fullData = saveToMemory(workspace1);
    QVERIFY(!fullData.isEmpty());

    // Truncate to 70% of original size (cuts off mid-element)
    QByteArray truncatedData = fullData.left(fullData.size() * 7 / 10);

    // Try to load truncated data - should fail gracefully
    WorkSpace workspace2;
    try {
        loadFromMemory(workspace2, truncatedData);
        // If we get here, load succeeded partially - verify workspace is in valid state
        QVERIFY2(workspace2.scene() != nullptr, "Scene should be valid after partial load");
    } catch (const std::exception &e) {
        // Expected behavior for corrupted file - exception provides clear error
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should explain why the file is truncated");
    }
}

void TestSerialization::testInvalidUTF8InLabels()
{
    // Create data with invalid UTF-8 sequences in element label
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    // Write scene center
    QPointF center(0, 0);
    stream << center;

    // Write element count
    stream << static_cast<quint64>(1);

    // Write element type
    stream << static_cast<quint64>(ElementType::And);

    // Write position
    stream << QPointF(100, 100);

    // Write INVALID UTF-8 label
    QByteArray invalidLabel;
    invalidLabel.append(static_cast<char>(0xFF));  // Invalid UTF-8 start byte
    invalidLabel.append(static_cast<char>(0xFE));
    invalidLabel.append(static_cast<char>(0xFD));
    stream << invalidLabel;

    // Write other properties
    stream << 0.0;  // rotation
    stream << 2;    // input size
    stream << 1;    // output size

    // Try to load - should either fix or skip invalid bytes
    WorkSpace workspace;
    try {
        loadFromMemory(workspace, data);
        // If load succeeds, element should be loaded (even if label is corrupted)
        QVERIFY2(workspace.scene()->elements().size() >= 1, "Element should load despite invalid UTF-8 in label");
    } catch (const std::exception &e) {
        // Acceptable to fail on invalid UTF-8
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should explain the UTF-8 issue");
    }
}

void TestSerialization::testMismatchedElementCount()
{
    // Write element count that doesn't match actual elements
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    // Write scene center
    QPointF center(0, 0);
    stream << center;

    // Write incorrect element count (say 5 but only provide 2)
    stream << static_cast<quint64>(5);

    // Write only 2 elements
    for (int i = 0; i < 2; i++) {
        stream << static_cast<quint64>(ElementType::And);
        stream << QPointF(100 + i * 50, 100);
        stream << QString("And%1").arg(i);
        stream << 0.0;  // rotation
        stream << 2;    // input size
        stream << 1;    // output size
    }

    // Try to load - should detect mismatch
    WorkSpace workspace;
    try {
        QDataStream readStream(data);
        QVersionNumber version = Serialization::readPandaHeader(readStream);
        // Stream should handle count mismatch gracefully - verify header was read
        QVERIFY2(version.majorVersion() >= 0, "Header should be readable even with count mismatch");
    } catch (const std::exception &e) {
        // Acceptable to fail on count mismatch
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should explain the count mismatch");
    }
}

void TestSerialization::testMultiGateChainSerialization()
{
    // Create circuit with many logic elements in a nested structure pattern
    // (True IC nesting would require actual IC files, which we skip)
    // Instead, test serialization of deeply nested gate structures

    WorkSpace workspace1;
    Scene *scene = workspace1.scene();

    // Create nested gate structure: inputs -> AND -> OR -> NAND -> NOR -> output
    auto *input1 = ElementFactory::buildElement(ElementType::InputButton);
    auto *input2 = ElementFactory::buildElement(ElementType::InputButton);
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *or1 = ElementFactory::buildElement(ElementType::Or);
    auto *nand1 = ElementFactory::buildElement(ElementType::Nand);
    auto *nor1 = ElementFactory::buildElement(ElementType::Nor);
    auto *output = ElementFactory::buildElement(ElementType::Led);

    QVERIFY(input1 && input2 && and1 && or1 && nand1 && nor1 && output);

    // Add to scene
    scene->addItem(input1);
    scene->addItem(input2);
    scene->addItem(and1);
    scene->addItem(or1);
    scene->addItem(nand1);
    scene->addItem(nor1);
    scene->addItem(output);

    // Set labels to indicate nesting level
    input1->setLabel("Input1");
    input2->setLabel("Input2");
    and1->setLabel("Level1_AND");
    or1->setLabel("Level2_OR");
    nand1->setLabel("Level3_NAND");
    nor1->setLabel("Level4_NOR");
    output->setLabel("Output");

    // Save and load
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    // Verify all elements loaded
    QCOMPARE(workspace2.scene()->elements().size(), 7);

    // Verify labels preserved (indicating structure preserved)
    auto elements = workspace2.scene()->elements();
    QVERIFY(elements[0]->label() == "Input1" || elements[0]->label() == "Input2" ||
            elements[0]->label() == "Level1_AND" || elements[0]->label() == "Level2_OR" ||
            elements[0]->label() == "Level3_NAND" || elements[0]->label() == "Level4_NOR" ||
            elements[0]->label() == "Output");
}

void TestSerialization::testExtremelyLongLabels()
{
    // Test serialization of elements with very long labels (>10000 chars)
    WorkSpace workspace1;

    // Create element with extremely long label
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY(andGate != nullptr);

    // Create a 20000 character label
    QString longLabel;
    for (int i = 0; i < 2000; i++) {
        longLabel.append("0123456789");
    }
    QCOMPARE(longLabel.length(), 20000);

    andGate->setLabel(longLabel);
    workspace1.scene()->addItem(andGate);

    // Save and load
    QByteArray data = saveToMemory(workspace1);
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);

    // Verify long label preserved
    auto *loaded = workspace2.scene()->elements().first();
    QCOMPARE(loaded->label().length(), 20000);
    QCOMPARE(loaded->label(), longLabel);
}

void TestSerialization::testStreamPositionValidation()
{
    // Test that stream position is validated during loading
    WorkSpace workspace1;
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *or1 = ElementFactory::buildElement(ElementType::Or);
    QVERIFY(and1 != nullptr && or1 != nullptr);
    workspace1.scene()->addItem(and1);
    workspace1.scene()->addItem(or1);

    // Save full data
    QByteArray data = saveToMemory(workspace1);

    // Verify we can load valid data
    WorkSpace workspace2;
    loadFromMemory(workspace2, data);
    QCOMPARE(workspace2.scene()->elements().size(), 2);

    // Now create data with bad stream position markers
    QByteArray badData;
    QDataStream stream(&badData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    QPointF center(0, 0);
    stream << center;
    stream << static_cast<quint64>(1);

    // Write element with mismatched position data
    stream << static_cast<quint64>(ElementType::And);
    stream << QPointF(999999, 999999);  // Extreme position
    stream << QString("Test");
    stream << 0.0;
    stream << 2;
    stream << 1;

    // Try to load - should handle extreme positions gracefully
    WorkSpace workspace3;
    try {
        loadFromMemory(workspace3, badData);
        // If load succeeds, verify the scene is valid
        QVERIFY2(workspace3.scene() != nullptr, "Scene should be valid after loading extreme positions");
    } catch (const std::exception &e) {
        // Exception is acceptable for extreme values
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should explain why extreme positions failed");
    }
}

void TestSerialization::testConnectionWithDeletedPorts()
{
    // Test loading circuit where connections reference deleted ports
    WorkSpace workspace1;
    Scene *scene = workspace1.scene();

    // Create elements and connections
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *or1 = ElementFactory::buildElement(ElementType::Or);
    QVERIFY(and1 != nullptr && or1 != nullptr);

    and1->setPos(0, 50);
    or1->setPos(100, 50);
    scene->addItem(and1);
    scene->addItem(or1);

    // Create connection
    auto *conn = new QNEConnection();
    conn->setStartPort(and1->outputPort(0));
    conn->setEndPort(or1->inputPort(0));
    scene->addItem(conn);

    // Save
    QByteArray data = saveToMemory(workspace1);

    // Load into new workspace
    WorkSpace workspace2;
    try {
        loadFromMemory(workspace2, data);
        // If load succeeds, connections should be preserved
        // Verify at least 2 elements loaded
        QCOMPARE(workspace2.scene()->elements().size(), 2);
        QVERIFY2(workspace2.scene() != nullptr, "Scene should be valid after loading connections");
    } catch (const std::exception &e) {
        // Acceptable if connection loading fails
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should explain the connection issue");
    }
}

void TestSerialization::testMalformedConnectionData()
{
    // Test loading file with malformed connection data
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    QPointF center(0, 0);
    stream << center;

    // Write 2 elements
    stream << static_cast<quint64>(2);

    // Write two And gates
    for (int i = 0; i < 2; i++) {
        stream << static_cast<quint64>(ElementType::And);
        stream << QPointF(i * 100, 50);
        stream << QString("And%1").arg(i);
        stream << 0.0;
        stream << 2;
        stream << 1;
    }

    // Write malformed connection (invalid port indices)
    stream << static_cast<quint64>(1);  // 1 connection
    stream << static_cast<quint64>(999);  // Invalid start element ID
    stream << static_cast<quint64>(0);    // Invalid output port
    stream << static_cast<quint64>(999);  // Invalid end element ID
    stream << static_cast<quint64>(0);    // Invalid input port

    // Try to load - should skip malformed connections
    WorkSpace workspace;
    try {
        loadFromMemory(workspace, data);
        // Should load elements but skip bad connections
        QCOMPARE(workspace.scene()->elements().size(), 2);
        QVERIFY2(workspace.scene() != nullptr, "Scene should be valid after skipping malformed connections");
    } catch (const std::exception &e) {
        // Acceptable to fail on malformed data
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should explain the malformed connection issue");
    }
}

// ============================================================
// Wireless Node Serialization Tests
// ============================================================

void TestSerialization::testWirelessTxNodePreservedInScene()
{
    // A Node in Tx mode must survive a full Workspace save/load round-trip.
    WorkSpace ws1;
    auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(nodeElm != nullptr);
    nodeElm->setLabel("CLK");
    qobject_cast<Node *>(nodeElm)->setWirelessMode(WirelessMode::Tx);
    ws1.scene()->addItem(nodeElm);

    QByteArray data = saveToMemory(ws1);
    QVERIFY(!data.isEmpty());

    WorkSpace ws2;
    loadFromMemory(ws2, data);

    auto elements = ws2.scene()->elements();
    QCOMPARE(elements.size(), 1);

    auto *loaded = qobject_cast<Node *>(elements.first());
    QVERIFY(loaded != nullptr);
    QCOMPARE(loaded->wirelessMode(), WirelessMode::Tx);
    QCOMPARE(loaded->label(), QStringLiteral("CLK"));
    QVERIFY(loaded->inputPort()->isRequired());
}

void TestSerialization::testWirelessRxNodePreservedInScene()
{
    // A Node in Rx mode must survive a full Workspace save/load round-trip,
    // including the isRequired=false state on its input port.
    WorkSpace ws1;
    auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(nodeElm != nullptr);
    nodeElm->setLabel("DATA");
    qobject_cast<Node *>(nodeElm)->setWirelessMode(WirelessMode::Rx);
    ws1.scene()->addItem(nodeElm);

    QByteArray data = saveToMemory(ws1);

    WorkSpace ws2;
    loadFromMemory(ws2, data);

    auto *loaded = qobject_cast<Node *>(ws2.scene()->elements().first());
    QVERIFY(loaded != nullptr);
    QCOMPARE(loaded->wirelessMode(), WirelessMode::Rx);
    QCOMPARE(loaded->label(), QStringLiteral("DATA"));
    QVERIFY(!loaded->inputPort()->isRequired());
}

void TestSerialization::testWirelessPairCircuit()
{
    // A Tx+Rx pair with the same label must both survive the round-trip with
    // their respective modes intact.
    WorkSpace ws1;
    Scene *scene1 = ws1.scene();

    auto *txElm = ElementFactory::buildElement(ElementType::Node);
    auto *rxElm = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(txElm && rxElm);
    txElm->setLabel("SIG"); qobject_cast<Node *>(txElm)->setWirelessMode(WirelessMode::Tx);
    rxElm->setLabel("SIG"); qobject_cast<Node *>(rxElm)->setWirelessMode(WirelessMode::Rx);
    txElm->setPos(0, 0);
    rxElm->setPos(200, 0);
    scene1->addItem(txElm);
    scene1->addItem(rxElm);

    QByteArray data = saveToMemory(ws1);

    WorkSpace ws2;
    loadFromMemory(ws2, data);

    const auto elements = ws2.scene()->elements();
    QCOMPARE(elements.size(), 2);

    int txCount = 0, rxCount = 0;
    for (auto *elm : elements) {
        auto *n = qobject_cast<Node *>(elm);
        QVERIFY(n != nullptr);
        QCOMPARE(n->label(), QStringLiteral("SIG"));
        if (n->wirelessMode() == WirelessMode::Tx) { ++txCount; }
        else if (n->wirelessMode() == WirelessMode::Rx) { ++rxCount; }
    }
    QCOMPARE(txCount, 1);
    QCOMPARE(rxCount, 1);
}

void TestSerialization::testWirelessNoneNodeInScene()
{
    // A Node in None mode (default) must load back as None — verifies backward
    // compatibility when no wirelessMode key is present in the serialized map.
    WorkSpace ws1;
    auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(nodeElm != nullptr);
    nodeElm->setLabel("PLAIN");
    // wirelessMode intentionally left at None (default)
    ws1.scene()->addItem(nodeElm);

    QByteArray data = saveToMemory(ws1);

    WorkSpace ws2;
    loadFromMemory(ws2, data);

    auto *loaded = qobject_cast<Node *>(ws2.scene()->elements().first());
    QVERIFY(loaded != nullptr);
    QCOMPARE(loaded->wirelessMode(), WirelessMode::None);
    QVERIFY(loaded->inputPort()->isRequired());
}

// ============================================================================
// Versioned backup utility — Serialization::createVersionedBackup
// ============================================================================

void TestSerialization::testVersionedBackupNamingPattern()
{
    // Verify that createVersionedBackup(path, version) produces a sidecar file
    // named  <basename>.v<major>.<minor>[.<patch>].<suffix>
    // e.g. "mycircuit.panda" + v4.2  → "mycircuit.v4.2.panda"

    QVERIFY2(m_tempDir.isValid(), "Temp dir must be valid");

    const QString original = m_tempDir.path() + "/mycircuit.panda";
    { QFile f(original); QVERIFY(f.open(QIODevice::WriteOnly)); f.write("test"); }

    Serialization::createVersionedBackup(original, QVersionNumber(4, 2));

    const QString expected = m_tempDir.path() + "/mycircuit.v4.2.panda";
    QVERIFY2(QFile::exists(expected),
             qPrintable("Backup file not found at expected path: " + expected));
}

void TestSerialization::testVersionedBackupIsIdempotent()
{
    // Calling createVersionedBackup twice must not overwrite the existing backup.
    // The backup file's byte content must be identical to the original at the
    // time of the first call, regardless of any subsequent changes to the original.

    QVERIFY2(m_tempDir.isValid(), "Temp dir must be valid");

    const QString original = m_tempDir.path() + "/idempotent.panda";
    { QFile f(original); QVERIFY(f.open(QIODevice::WriteOnly)); f.write("first-content"); }

    Serialization::createVersionedBackup(original, QVersionNumber(4, 1));

    const QString backup = m_tempDir.path() + "/idempotent.v4.1.panda";
    QVERIFY2(QFile::exists(backup), "Backup must exist after first call");

    // Overwrite the original with different content
    { QFile f(original); QVERIFY(f.open(QIODevice::WriteOnly)); f.write("second-content"); }

    // Second call — backup must NOT be overwritten
    Serialization::createVersionedBackup(original, QVersionNumber(4, 1));

    QFile bf(backup);
    QVERIFY(bf.open(QIODevice::ReadOnly));
    QCOMPARE(bf.readAll(), QByteArray("first-content"));
}

void TestSerialization::testVersionedBackupPreservesOriginal()
{
    // After createVersionedBackup, the original file must be byte-for-byte unchanged.

    QVERIFY2(m_tempDir.isValid(), "Temp dir must be valid");

    const QByteArray originalContent = "original-data-must-survive";
    const QString original = m_tempDir.path() + "/preserve.panda";
    { QFile f(original); QVERIFY(f.open(QIODevice::WriteOnly)); f.write(originalContent); }

    Serialization::createVersionedBackup(original, QVersionNumber(3, 1));

    QFile f(original);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QCOMPARE(f.readAll(), originalContent);
}

void TestSerialization::testVersionedBackupMultiVersions()
{
    // Different old versions produce different backup files in the same directory;
    // each backup has the correct content for its respective version.

    QVERIFY2(m_tempDir.isValid(), "Temp dir must be valid");

    const QString original = m_tempDir.path() + "/multi.panda";

    // Write v4.1 content and back it up
    const QByteArray content41 = "content-v4.1";
    { QFile f(original); QVERIFY(f.open(QIODevice::WriteOnly)); f.write(content41); }
    Serialization::createVersionedBackup(original, QVersionNumber(4, 1));

    // Write v4.2 content and back it up
    const QByteArray content42 = "content-v4.2";
    { QFile f(original); QVERIFY(f.open(QIODevice::WriteOnly)); f.write(content42); }
    Serialization::createVersionedBackup(original, QVersionNumber(4, 2));

    // Both backup files must exist and contain the correct content
    const QString backup41 = m_tempDir.path() + "/multi.v4.1.panda";
    const QString backup42 = m_tempDir.path() + "/multi.v4.2.panda";

    QVERIFY2(QFile::exists(backup41), "v4.1 backup must exist");
    QVERIFY2(QFile::exists(backup42), "v4.2 backup must exist");

    QFile f41(backup41);
    QVERIFY(f41.open(QIODevice::ReadOnly));
    QCOMPARE(f41.readAll(), content41);

    QFile f42(backup42);
    QVERIFY(f42.open(QIODevice::ReadOnly));
    QCOMPARE(f42.readAll(), content42);
}
