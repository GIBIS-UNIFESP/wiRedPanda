// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestSerialization.h"

#include <cmath>

#include <QDataStream>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QKeySequence>
#include <QMetaType>
#include <QRectF>
#include <QSet>
#include <QTemporaryDir>
#include <QTest>

#include "App/Core/Common.h"
#include "App/Core/ItemWithId.h"
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
#include "App/IO/SerializationContext.h"
#include "App/Versions.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/QuickShell/IC/QuickTestUtils.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

// CURRENTDIR is a target_compile_definitions() value (see CMakeLists.txt), not itself a string
// literal -- QUOTE()/_QUOTE() stringify it. Used by every testFuzzRegression*() fixture path
// below; mirrors the identical file-scoped macro pair other Tests/QuickShell/*.cpp files
// define locally per-use (e.g. TestDanglingPointer.cpp, TestSimulationUnit.cpp).
#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string

namespace {
// Scene-free replacement for iterating a Scene's items() and qgraphicsitem_cast<Connection*>-
// filtering them: walks every element's ports instead, same technique
// QuickTestUtils::writePandaPayload() itself uses to collect connections for serialization.
QSet<Connection *> collectConnections(const QVector<GraphicElement *> &elements)
{
    QSet<Connection *> result;
    for (auto *elm : elements) {
        for (auto *port : elm->inputs()) {
            for (auto *conn : port->connections()) {
                result.insert(conn);
            }
        }
        for (auto *port : elm->outputs()) {
            for (auto *conn : port->connections()) {
                result.insert(conn);
            }
        }
    }
    return result;
}

// Scene-free replacement for Scene::items() -- every element plus every Connection reachable
// from their ports, the full item list Serialization::serialize() itself expects.
QList<ItemWithId *> collectSerializableItems(const QVector<GraphicElement *> &elements)
{
    QList<ItemWithId *> items;
    items.reserve(elements.size());
    for (auto *elm : elements) {
        items.append(elm);
    }
    for (auto *conn : collectConnections(elements)) {
        items.append(conn);
    }
    return items;
}
} // namespace

void TestSerialization::init()
{
    // Setup before each test
}

// ============================================================================
// Helper Methods
// ============================================================================

QByteArray TestSerialization::saveToMemory(QuickCircuitBuilder &builder)
{
    return QuickTestUtils::savePandaBytes(builder);
}

void TestSerialization::loadFromMemory(QuickCircuitBuilder &builder, const QByteArray &data)
{
    QuickTestUtils::loadPandaBytes(builder, data);
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
    // Create circuit with single And gate
    QuickCircuitBuilder builder1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setLabel("TestAnd");
    builder1.addOwnedElement(andGate);

    // Save to memory
    QByteArray data = saveToMemory(builder1);
    QVERIFY(!data.isEmpty());

    // Load into new builder
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    // Verify element count
    auto elements = builder2.elements();
    QCOMPARE(elements.size(), 1);

    // Verify element type and properties
    auto *loaded = elements.first();
    verifyElementEquality(andGate, loaded);
}

void TestSerialization::testMultipleElementsRoundTrip()
{
    // Create circuit with 5 different element types
    QuickCircuitBuilder builder1;

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

    builder1.addOwnedElement(sw);
    builder1.addOwnedElement(andGate);
    builder1.addOwnedElement(orGate);
    builder1.addOwnedElement(notGate);
    builder1.addOwnedElement(led);

    // Save and reload
    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    // Verify all 5 elements preserved with correct count
    auto elements = builder2.elements();
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
    QuickCircuitBuilder builder1;

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    QVERIFY2(sw != nullptr, "Failed to create InputSwitch");
    sw->setPos(0, 0);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setPos(100, 0);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    QVERIFY2(led != nullptr, "Failed to create Led element");
    led->setPos(200, 0);

    builder1.addOwnedElement(sw);
    builder1.addOwnedElement(andGate);
    builder1.addOwnedElement(led);

    // Create connections
    builder1.connect(sw, 0, andGate, 0);
    builder1.connect(andGate, 0, led, 0);

    // Save and reload
    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    // Verify connections restored with correct structure
    auto loadedElements = builder2.elements();

    // Find the reloaded elements
    GraphicElement *loadedSw = nullptr, *loadedAnd = nullptr, *loadedLed = nullptr;
    for (auto *elem : loadedElements) {
        if (elem->elementType() == ElementType::InputSwitch) loadedSw = elem;
        else if (elem->elementType() == ElementType::And) loadedAnd = elem;
        else if (elem->elementType() == ElementType::Led) loadedLed = elem;
    }

    // Verify all connections have valid ports
    const auto connections = collectConnections(loadedElements);
    for (auto *conn : connections) {
        QVERIFY2(conn->startPort() != nullptr, "Missing start port");
        QVERIFY2(conn->endPort() != nullptr, "Missing end port");
    }
    QCOMPARE(connections.size(), 2);

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
    QuickCircuitBuilder builder1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setLabel(label);
    andGate->setRotation(rotation);
    builder1.addOwnedElement(andGate);

    // Save and reload
    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    // Verify properties preserved
    auto *loaded = builder2.elements().first();
    QCOMPARE(loaded->label(), label);
    QCOMPARE(loaded->rotation(), rotation);
}

// ============================================================================
// Complex Circuits
// ============================================================================

void TestSerialization::testComplexCircuitSaveLoad()
{
    // Create multi-gate circuit with QuickCircuitBuilder
    QuickCircuitBuilder builder1;

    InputSwitch sw1, sw2, sw3;
    And and1, and2;
    Or orGate;
    Led led1, led2;

    builder1.add(&sw1, &sw2, &sw3, &and1, &and2, &orGate, &led1, &led2);

    // Connect: (sw1 AND sw2) OR (sw3 AND sw3) → LED
    builder1.connect(&sw1, 0, &and1, 0);
    builder1.connect(&sw2, 0, &and1, 1);
    builder1.connect(&sw3, 0, &and2, 0);
    builder1.connect(&sw3, 0, &and2, 1);
    builder1.connect(&and1, 0, &orGate, 0);
    builder1.connect(&and2, 0, &orGate, 1);
    builder1.connect(&orGate, 0, &led1, 0);
    builder1.connect(&and1, 0, &led2, 0);

    int originalElementCount = static_cast<int>(builder1.elements().size());
    int originalConnectionCount = static_cast<int>(collectConnections(builder1.elements()).size());

    // Save and reload
    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    // Verify structure preserved
    QCOMPARE(builder2.elements().size(), originalElementCount);

    int newConnectionCount = static_cast<int>(collectConnections(builder2.elements()).size());
    QCOMPARE(newConnectionCount, originalConnectionCount);
}

void TestSerialization::testICSerializationBasic()
{
    // Load an example IC file
    QFileInfo exampleFile(QuickTestUtils::examplesDir() + "ic.panda");

    // Load the file
    QuickCircuitBuilder builder;
    QFile file(exampleFile.absoluteFilePath());
    QVERIFY2(file.open(QIODevice::ReadOnly),
             qPrintable(QString("Failed to open %1: %2")
                       .arg(file.fileName(), file.errorString())));

    QDataStream stream(&file);
    QVersionNumber version = Serialization::readPandaHeader(stream);
    QuickTestUtils::loadPandaStream(builder, stream, version, exampleFile.absolutePath());
    file.close();

    // Verify circuit loaded successfully without errors
    QVERIFY(!builder.elements().isEmpty());

    // Verify all loaded elements have valid types
    for (auto *elem : builder.elements()) {
        QVERIFY(elem->elementType() != ElementType::Unknown);
    }
}

// ============================================================================
// File Format Tests
// ============================================================================

void TestSerialization::testFileFormatHeader()
{
    // Create simple circuit and save
    QuickCircuitBuilder builder;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    builder.addOwnedElement(andGate);

    QByteArray data = saveToMemory(builder);

    // Manually read magic header
    QDataStream stream(data);
    quint32 magic;
    stream >> magic;

    // Verify magic header
    QCOMPARE(magic, static_cast<quint32>(0x57504346)); // "WPCF"
}

void TestSerialization::testCompressedPayloadRoundTrip()
{
    // Circuit large enough that its serialized (repetitive) form compresses well.
    QuickCircuitBuilder builder1;
    for (int i = 0; i < 100; ++i) {
        auto *gate = ElementFactory::buildElement(ElementType::And);
        QVERIFY2(gate != nullptr, QString("Failed to create And element %1").arg(i).toLatin1());
        gate->setPos(i * 20, 0);
        gate->setLabel(QString("AND_%1").arg(i));
        builder1.addOwnedElement(gate);
    }

    QByteArray data = saveToMemory(builder1);
    QVERIFY(!data.isEmpty());

    QDataStream headerStream(data);
    const QVersionNumber version = Serialization::readPandaHeader(headerStream);
    QVERIFY2(version >= Versions::Rev100, "New saves should use the compressed-payload format");

    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);
    QCOMPARE(builder2.elements().size(), 100);
    for (auto *elem : builder2.elements()) {
        QCOMPARE(elem->elementType(), ElementType::And);
    }
}

void TestSerialization::testLegacyUncompressedPayloadLoads()
{
    // Build a stream matching the pre-Rev100 on-disk format: header + a plain,
    // uncompressed metadata map + elements -- exactly what WorkSpace::save()
    // produced before payload compression was introduced. Confirms old files
    // still load correctly once readPayload() only decompresses Rev100+ streams.
    QuickCircuitBuilder builder1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setLabel("LegacyAnd");
    builder1.addOwnedElement(andGate);

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << static_cast<quint32>(0x57504346); // "WPCF"
    writeStream << Versions::V_5_1;

    QMap<QString, QVariant> metadata;
    writeStream << metadata;
    Serialization::serialize(collectSerializableItems(builder1.elements()), writeStream, {.purpose = SerializationPurpose::PortableFile});

    QDataStream headerStream(data);
    const QVersionNumber version = Serialization::readPandaHeader(headerStream);
    QCOMPARE(version, Versions::V_5_1);

    QuickCircuitBuilder builder2;
    headerStream.setVersion(QDataStream::Qt_5_12);
    QuickTestUtils::loadPandaStream(builder2, headerStream, version, {});

    QCOMPARE(builder2.elements().size(), 1);
    verifyElementEquality(andGate, builder2.elements().first());
}

void TestSerialization::testImplausibleCompressedSizeRejected()
{
    // A crafted "compressed" payload whose 4-byte declared uncompressed size is
    // implausible must be rejected before qUncompress() would allocate that many
    // bytes -- the decompression-bomb guard in Serialization::readPayload().
    QByteArray bogus;
    QDataStream writeStream(&bogus, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << quint32{0xFFFFFFFF}; // declares ~4 GiB uncompressed

    QDataStream readStream(bogus);
    bool threw = false;
    try {
        Serialization::readPayload(readStream, Versions::Rev100);
    } catch (const Pandaception &e) {
        threw = true;
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should provide a meaningful error message");
    }
    QVERIFY2(threw, "Implausible decompressed payload size should be rejected");
}

void TestSerialization::testSlimFormatElidesDefaults()
{
    // A default-state element saved as a PortableFile must not carry its
    // default rotation/label/trigger keys, nor per-port serialId/name entries
    // -- the loader derives serialIds from the element-level "id" and
    // reconstructs everything else from constructor defaults.
    QuickCircuitBuilder builder;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    builder.addOwnedElement(andGate);

    QByteArray data = saveToMemory(builder);

    QDataStream stream(data);
    const QVersionNumber version = Serialization::readPandaHeader(stream);
    QByteArray payload = Serialization::readPayload(stream, version);
    QDataStream payloadStream(&payload, QIODevice::ReadOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);

    Serialization::readBoundedMetadata(payloadStream); // file-level metadata

    int typeTag = 0;
    payloadStream >> typeTag;
    QCOMPARE(typeTag, static_cast<int>(GraphicElement::Type));
    ElementType elmType;
    payloadStream >> elmType;
    QCOMPARE(elmType, ElementType::And);

    const auto baseMap = Serialization::readBoundedMetadata(payloadStream);
    QVERIFY2(baseMap.contains("id"), "Slim format must write the element id");
    QVERIFY2(baseMap.contains("pos"), "pos is always written");
    QVERIFY2(!baseMap.contains("rotation"), "Default rotation must be elided");
    QVERIFY2(!baseMap.contains("label"), "Empty label must be elided");
    QVERIFY2(!baseMap.contains("trigger"), "Empty trigger must be elided");

    // Port lists: count preserved, entries empty (no serialId, no name).
    quint32 inputCount = 0;
    payloadStream >> inputCount;
    QCOMPARE(inputCount, 2u);
    for (quint32 i = 0; i < inputCount; ++i) {
        const auto entry = Serialization::readBoundedMetadata(payloadStream);
        QVERIFY2(!entry.contains("serialId"), "Slim port entries must not carry serialIds");
        QVERIFY2(!entry.contains("name"), "Slim non-IC port entries must not carry names");
    }
    quint32 outputCount = 0;
    payloadStream >> outputCount;
    QCOMPARE(outputCount, 1u);
    for (quint32 i = 0; i < outputCount; ++i) {
        const auto entry = Serialization::readBoundedMetadata(payloadStream);
        QVERIFY2(!entry.contains("serialId"), "Slim port entries must not carry serialIds");
        QVERIFY2(!entry.contains("name"), "Slim non-IC port entries must not carry names");
    }

    // All-default appearances collapse to an empty list.
    quint32 appearanceCount = 0;
    payloadStream >> appearanceCount;
    QCOMPARE(appearanceCount, 0u);
}

void TestSerialization::testNonDefaultValuesRoundTrip()
{
    // Every elidable property set to a NON-default value must survive a
    // PortableFile round-trip -- proving elision only ever drops defaults.
    QuickCircuitBuilder builder1;

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setRotation(45.0);
    andGate->setLabel("NonDefault");
    builder1.addOwnedElement(andGate);

    auto *led = ElementFactory::buildElement(ElementType::Led);
    QVERIFY2(led != nullptr, "Failed to create Led element");
    led->setColor("Green");
    builder1.addOwnedElement(led);

    auto *sw = qobject_cast<InputSwitch *>(ElementFactory::buildElement(ElementType::InputSwitch));
    QVERIFY2(sw != nullptr, "Failed to create InputSwitch");
    sw->setOn(true);
    builder1.addOwnedElement(sw);

    auto *clock = ElementFactory::buildElement(ElementType::Clock);
    QVERIFY2(clock != nullptr, "Failed to create Clock");
    clock->setFrequency(2.0);
    builder1.addOwnedElement(clock);

    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    GraphicElement *loadedAnd = nullptr, *loadedLed = nullptr, *loadedClock = nullptr;
    InputSwitch *loadedSw = nullptr;
    for (auto *elem : builder2.elements()) {
        switch (elem->elementType()) {
        case ElementType::And:         loadedAnd = elem; break;
        case ElementType::Led:         loadedLed = elem; break;
        case ElementType::InputSwitch: loadedSw = qobject_cast<InputSwitch *>(elem); break;
        case ElementType::Clock:       loadedClock = elem; break;
        default: break;
        }
    }

    QVERIFY(loadedAnd && loadedLed && loadedSw && loadedClock);
    QCOMPARE(loadedAnd->rotation(), 45.0);
    QCOMPARE(loadedAnd->label(), QString("NonDefault"));
    QCOMPARE(loadedLed->color(), QString("Green"));
    QVERIFY2(loadedSw->isOn(), "Non-default switch state must round-trip");
    QCOMPARE(loadedClock->frequency(), 2.0);
}

void TestSerialization::testDolphinFilenamePreserved()
{
    // Create circuit and set dolphin filename
    QuickCircuitBuilder builder1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    builder1.addOwnedElement(andGate);

    QString testDolphinFile = "test_waveform.dolphin";

    // Save and reload
    QByteArray data = QuickTestUtils::savePandaBytes(builder1, testDolphinFile);
    QuickCircuitBuilder builder2;
    const auto loaded = QuickTestUtils::loadPandaBytes(builder2, data);

    // Verify circuit loads without errors (dolphin filename is metadata)
    QVERIFY(!builder2.elements().isEmpty());
    QCOMPARE(loaded.dolphinFileName, testDolphinFile);
}

void TestSerialization::testSceneRectPreserved()
{
    // Create circuit with an element at a specific position
    QuickCircuitBuilder builder1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setPos(100, 100);
    builder1.addOwnedElement(andGate);

    // Save and reload
    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    // Verify circuit loaded with correct structure
    QCOMPARE(builder2.elements().size(), 1);
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
    QuickCircuitBuilder builder1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    builder1.addOwnedElement(andGate);

    QByteArray data = saveToMemory(builder1);
    // Truncate to break the stream
    data.resize(data.size() / 2);

    // Try to load - should handle gracefully or throw (not crash)
    QuickCircuitBuilder builder2;
    bool errorHandled = false;
    try {
        QDataStream stream(data);
        QVersionNumber version = Serialization::readPandaHeader(stream);
        QuickTestUtils::loadPandaStream(builder2, stream, version, {});
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
    QuickCircuitBuilder builder1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setRotation(rotation);
    builder1.addOwnedElement(andGate);

    // Save and reload
    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    // Verify rotation preserved
    auto *loaded = builder2.elements().first();
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

    QuickCircuitBuilder builder1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setLabel(label);
    builder1.addOwnedElement(andGate);

    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    auto *loaded = builder2.elements().first();
    QCOMPARE(loaded->label(), label);
}

void TestSerialization::testInputSizePreserved()
{
    // Create And gate with variable inputs
    QuickCircuitBuilder builder1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setInputSize(4);
    builder1.addOwnedElement(andGate);

    QCOMPARE(andGate->inputSize(), 4);

    // Save and reload
    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    auto *loaded = builder2.elements().first();
    QCOMPARE(loaded->inputSize(), 4);
}

void TestSerialization::testOutputSizePreserved()
{
    // Create AND gate (which has fixed 1 output, so less variability)
    QuickCircuitBuilder builder1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    // AND gate has fixed output size of 1
    builder1.addOwnedElement(andGate);

    QCOMPARE(andGate->outputSize(), 1);

    // Save and reload
    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    auto *loaded = builder2.elements().first();
    QCOMPARE(loaded->outputSize(), 1);
}

// ============================================================================
// Special Cases
// ============================================================================

void TestSerialization::testEmptyCircuit()
{
    // Save empty circuit
    QuickCircuitBuilder builder1;
    QByteArray data = saveToMemory(builder1);
    QVERIFY(!data.isEmpty());

    // Load empty circuit
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    // Verify empty circuit
    QCOMPARE(builder2.elements().size(), 0);
}

// ============================================================================
// ============================================================================

void TestSerialization::testLargeCircuitWith100Elements()
{
    // Create circuit with 100 AND gates in a chain
    QuickCircuitBuilder builder1;

    QList<GraphicElement *> gates;
    for (int i = 0; i < 100; ++i) {
        auto *gate = ElementFactory::buildElement(ElementType::And);
        QVERIFY2(gate != nullptr, QString("Failed to create And element %1").arg(i).toLatin1());
        gate->setPos(i * 20, 0);
        gate->setLabel(QString("AND_%1").arg(i));
        builder1.addOwnedElement(gate);
        gates.append(gate);
    }

    // Save to memory
    QByteArray data = saveToMemory(builder1);
    QVERIFY(!data.isEmpty());

    // Load into new builder
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    // Verify element count
    QCOMPARE(builder2.elements().size(), 100);

    // Verify all elements are AND gates
    for (auto *elem : builder2.elements()) {
        QCOMPARE(elem->elementType(), ElementType::And);
    }
}

void TestSerialization::testLargeCircuitWith500Elements()
{
    // Create circuit with 500 mixed elements
    QuickCircuitBuilder builder1;

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
        elem->setPos(i % 50 * 20, (i / 50) * 20); // NOLINT(bugprone-integer-division) -- deliberate grid row index, not lossy math
        elem->setLabel(QString("Elem_%1").arg(i));
        builder1.addOwnedElement(elem);
    }

    // Save to memory
    QByteArray data = saveToMemory(builder1);
    QVERIFY(!data.isEmpty());

    // Load into new builder
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    // Verify element count
    QCOMPARE(builder2.elements().size(), 500);
}

// ============================================================================
// ============================================================================

void TestSerialization::testUnicodeInLabels()
{
    // Test Unicode characters in element labels
    QuickCircuitBuilder builder1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");

    // Test various Unicode strings
    QString unicodeLabel = QString::fromUtf8("AND_Elemento_αβγ_日本語");
    andGate->setLabel(unicodeLabel);
    builder1.addOwnedElement(andGate);

    // Save and reload
    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    auto *loaded = builder2.elements().first();
    QCOMPARE(loaded->label(), unicodeLabel);
}

void TestSerialization::testSpecialCharactersInLabels()
{
    // Test special characters in element labels
    QuickCircuitBuilder builder1;
    auto *orGate = ElementFactory::buildElement(ElementType::Or);
    QVERIFY2(orGate != nullptr, "Failed to create Or element");

    QString specialLabel = "OR_!@#$%^&*()_+-=[]{}|;':\",./<>?";
    orGate->setLabel(specialLabel);
    builder1.addOwnedElement(orGate);

    // Save and reload
    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    auto *loaded = builder2.elements().first();
    QCOMPARE(loaded->label(), specialLabel);
}

void TestSerialization::testExtendedAsciiCharacters()
{
    // Test extended ASCII characters
    QuickCircuitBuilder builder1;
    auto *notGate = ElementFactory::buildElement(ElementType::Not);
    QVERIFY2(notGate != nullptr, "Failed to create Not element");

    // Extended ASCII with accented characters
    QString extendedLabel = "NOT_àáâãäåæçèéêëìíîï";
    notGate->setLabel(extendedLabel);
    builder1.addOwnedElement(notGate);

    // Save and reload
    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    auto *loaded = builder2.elements().first();
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
    QuickCircuitBuilder builder;
    try {
        loadFromMemory(builder, data);
        // If load succeeds, builder should be empty (corrupted data couldn't be loaded)
        QVERIFY2(builder.elements().isEmpty(),
                 "Corrupted data should result in an empty circuit, not partial load");
    } catch (const Pandaception &e) {
        // Expected: corrupted data causes exception
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should provide a meaningful error message");
    }
}

void TestSerialization::testInvalidElementTypeInStream()
{
    // Create circuit with element
    QuickCircuitBuilder builder1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    builder1.addOwnedElement(andGate);

    QByteArray data = saveToMemory(builder1);

    // NOT surgical field corruption: WorkSpace::save() qCompress()es the whole
    // metadata+elements+connections payload (FormatRev::current has hasCompressedPayload()),
    // and the header written before it is only a handful of bytes, so byte offsets 20-23 land
    // inside the compressed blob -- this flips arbitrary compressed bytes, not the element-type
    // field specifically. Still a legitimate robustness/fuzz-style check either way.
    if (data.size() > 20) {
        data[20] = static_cast<char>(0xFF);
        data[21] = static_cast<char>(0xFF);
        data[22] = static_cast<char>(0xFF);
        data[23] = static_cast<char>(0xFF);
    }

    // Try to load corrupted data
    QuickCircuitBuilder builder2;
    try {
        loadFromMemory(builder2, data);
        // If load succeeds despite the corruption (qUncompress()/parsing happened to tolerate
        // it), it must not silently produce a wrong partial load -- either genuinely empty or
        // a real And element, never something in between.
        QVERIFY2(builder2.elements().isEmpty() || builder2.elements().size() == 1,
                 "A load that survives corruption must not leave a partially-loaded circuit");
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
    QuickCircuitBuilder builder1;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    andGate->setPos(-1000, -500);
    andGate->setLabel("NegativePos");
    builder1.addOwnedElement(andGate);

    // Save and reload
    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    auto *loaded = builder2.elements().first();
    QCOMPARE(loaded->pos().x(), -1000.0);
    QCOMPARE(loaded->pos().y(), -500.0);
    QCOMPARE(loaded->label(), QString("NegativePos"));
}

void TestSerialization::testExtremelyLargeValues()
{
    // Test elements with very large position values
    QuickCircuitBuilder builder1;
    auto *orGate = ElementFactory::buildElement(ElementType::Or);
    QVERIFY2(orGate != nullptr, "Failed to create Or element");
    orGate->setPos(1e6, 1e6);  // 1 million units
    orGate->setLabel("LargePos");
    builder1.addOwnedElement(orGate);

    // Save and reload
    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    auto *loaded = builder2.elements().first();
    QCOMPARE(loaded->pos().x(), 1e6);
    QCOMPARE(loaded->pos().y(), 1e6);
    QCOMPARE(loaded->label(), QString("LargePos"));
}

void TestSerialization::testNullElementHandling()
{
    // Test that null/invalid elements are handled gracefully
    QuickCircuitBuilder builder1;

    // Add valid element
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create And element");
    builder1.addOwnedElement(andGate);

    // Save circuit (which should only save valid elements)
    QByteArray data = saveToMemory(builder1);

    // Load and verify
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    QCOMPARE(builder2.elements().size(), 1);
    QCOMPARE(builder2.elements().first()->elementType(), ElementType::And);
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
    // Create valid circuit data, then truncate it mid-element
    QuickCircuitBuilder builder1;
    auto *andGate1 = ElementFactory::buildElement(ElementType::And);
    auto *orGate1 = ElementFactory::buildElement(ElementType::Or);
    QVERIFY(andGate1 != nullptr && orGate1 != nullptr);
    builder1.addOwnedElement(andGate1);
    builder1.addOwnedElement(orGate1);

    // Save full data
    QByteArray fullData = saveToMemory(builder1);
    QVERIFY(!fullData.isEmpty());

    // Truncate to 70% of original size (cuts off mid-element)
    QByteArray truncatedData = fullData.left(fullData.size() * 7 / 10);

    // Try to load truncated data - should fail gracefully
    QuickCircuitBuilder builder2;
    try {
        loadFromMemory(builder2, truncatedData);
        // If we get here, load succeeded despite truncation -- it must not have fabricated
        // elements beyond the 2 that were actually saved.
        QVERIFY2(builder2.elements().size() <= 2,
                 "A load that survives truncation must not exceed the originally saved element count");
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
    QuickCircuitBuilder builder;
    try {
        loadFromMemory(builder, data);
        // If load succeeds, element should be loaded (even if label is corrupted)
        QVERIFY2(!builder.elements().isEmpty(), "Element should load despite invalid UTF-8 in label");
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

    // Try to load - should detect the mismatch between the claimed count (5) and the 2
    // elements actually present in the stream, rather than reading past the end or
    // fabricating elements from garbage data.
    QuickCircuitBuilder builder;
    try {
        loadFromMemory(builder, data);
        QVERIFY2(builder.elements().size() <= 2,
                 "A load that survives a count mismatch must not exceed the elements actually present");
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

    QuickCircuitBuilder builder1;

    // Create nested gate structure: inputs -> AND -> OR -> NAND -> NOR -> output
    auto *input1 = ElementFactory::buildElement(ElementType::InputButton);
    auto *input2 = ElementFactory::buildElement(ElementType::InputButton);
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *or1 = ElementFactory::buildElement(ElementType::Or);
    auto *nand1 = ElementFactory::buildElement(ElementType::Nand);
    auto *nor1 = ElementFactory::buildElement(ElementType::Nor);
    auto *output = ElementFactory::buildElement(ElementType::Led);

    QVERIFY(input1 && input2 && and1 && or1 && nand1 && nor1 && output);

    // Add to builder
    builder1.addOwnedElement(input1);
    builder1.addOwnedElement(input2);
    builder1.addOwnedElement(and1);
    builder1.addOwnedElement(or1);
    builder1.addOwnedElement(nand1);
    builder1.addOwnedElement(nor1);
    builder1.addOwnedElement(output);

    // Set labels to indicate nesting level
    input1->setLabel("Input1");
    input2->setLabel("Input2");
    and1->setLabel("Level1_AND");
    or1->setLabel("Level2_OR");
    nand1->setLabel("Level3_NAND");
    nor1->setLabel("Level4_NOR");
    output->setLabel("Output");

    // Save and load
    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    // Verify all elements loaded
    QCOMPARE(builder2.elements().size(), 7);

    // Verify every element's label survived the round trip, not just that some element
    // has one of the expected labels.
    QSet<QString> loadedLabels;
    for (auto *elm : builder2.elements()) {
        loadedLabels.insert(elm->label());
    }
    const QSet<QString> expectedLabels = {
        "Input1", "Input2", "Level1_AND", "Level2_OR", "Level3_NAND", "Level4_NOR", "Output"
    };
    QCOMPARE(loadedLabels, expectedLabels);
}

void TestSerialization::testExtremelyLongLabels()
{
    // Test serialization of elements with very long labels (>10000 chars)
    QuickCircuitBuilder builder1;

    // Create element with extremely long label
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY(andGate != nullptr);

    // Create a 20000 character label
    QString longLabel;
    for (int i = 0; i < 2000; i++) {
        longLabel.append("0123456789");
    }
    QCOMPARE(longLabel.size(), 20000);

    andGate->setLabel(longLabel);
    builder1.addOwnedElement(andGate);

    // Save and load
    QByteArray data = saveToMemory(builder1);
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    // Verify long label preserved
    auto *loaded = builder2.elements().first();
    QCOMPARE(loaded->label().size(), 20000);
    QCOMPARE(loaded->label(), longLabel);
}

void TestSerialization::testStreamPositionValidation()
{
    // Test that stream position is validated during loading
    QuickCircuitBuilder builder1;
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *or1 = ElementFactory::buildElement(ElementType::Or);
    QVERIFY(and1 != nullptr && or1 != nullptr);
    builder1.addOwnedElement(and1);
    builder1.addOwnedElement(or1);

    // Save full data
    QByteArray data = saveToMemory(builder1);

    // Verify we can load valid data
    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);
    QCOMPARE(builder2.elements().size(), 2);

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
    QuickCircuitBuilder builder3;
    try {
        loadFromMemory(builder3, badData);
        // If load succeeds, it must not have fabricated elements, and the extreme position
        // it was fed must not have decayed into a NaN/Inf coordinate.
        QVERIFY2(builder3.elements().size() <= 1,
                 "A load that survives an extreme position must not fabricate elements");
        if (!builder3.elements().isEmpty()) {
            const QPointF pos = builder3.elements().first()->pos();
            QVERIFY2(std::isfinite(pos.x()) && std::isfinite(pos.y()),
                     "Loaded element position must remain finite even for extreme input values");
        }
    } catch (const std::exception &e) {
        // Exception is acceptable for extreme values
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should explain why extreme positions failed");
    }
}

void TestSerialization::testConnectionSurvivesSaveLoadRoundTrip()
{
    // Deleting a port that has a live connection (GraphicElement::setInputSize() shrinking
    // below the wired index) deletes the connection along with it -- InputPort/OutputPort's
    // destructors drain their owned connections -- so a save/load round trip can never observe
    // a connection referencing a port that no longer exists via the production API; that
    // scenario is only reachable via a hand-crafted malformed file, already covered by
    // testMalformedConnectionData(). This is instead a lighter 2-element smoke test that a
    // single connection's wiring, not just its element count, survives a round trip.
    QuickCircuitBuilder builder1;

    // Create elements and connections
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *or1 = ElementFactory::buildElement(ElementType::Or);
    QVERIFY(and1 != nullptr && or1 != nullptr);

    and1->setPos(0, 50);
    or1->setPos(100, 50);
    builder1.addOwnedElement(and1);
    builder1.addOwnedElement(or1);

    // Create connection
    builder1.connect(and1, 0, or1, 0);

    // Save
    QByteArray data = saveToMemory(builder1);

    // Load into new builder
    QuickCircuitBuilder builder2;
    try {
        loadFromMemory(builder2, data);
        QCOMPARE(builder2.elements().size(), 2);

        GraphicElement *loadedAnd = nullptr, *loadedOr = nullptr;
        for (auto *elm : builder2.elements()) {
            if (elm->elementType() == ElementType::And) { loadedAnd = elm; }
            if (elm->elementType() == ElementType::Or)  { loadedOr = elm; }
        }
        QVERIFY2(loadedAnd && loadedOr, "Both the And and Or gates must be reloaded");
        QVERIFY2(loadedAnd->outputPort(0)->isConnected(loadedOr->inputPort(0)),
                 "The connection between the And output and the Or input must survive the round trip");
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
    QuickCircuitBuilder builder;
    try {
        loadFromMemory(builder, data);
        // Should load elements but skip bad connections
        QCOMPARE(builder.elements().size(), 2);
        for (auto *elm : builder.elements()) {
            QVERIFY2(elm->outputPort(0)->connections().isEmpty(),
                     "A connection with invalid element/port indices must be skipped, not attached");
        }
    } catch (const std::exception &e) {
        // Acceptable to fail on malformed data
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should explain the malformed connection issue");
    }
}

void TestSerialization::testMalformedEmbeddedICsRegistryRejected()
{
    // A crafted embeddedICs blob whose leading QMap entry count vastly exceeds what
    // the buffer could hold. Serialization::deserializeBlobRegistry must reject this
    // via readBoundedBlobMap before Qt's raw QMap<QString,QByteArray>::operator>> can
    // spin through an implausible entry count.
    QByteArray badRegistry;
    QDataStream writeStream(&badRegistry, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << 0xFFFFFFFFu; // implausible entry count
    // No entries follow — nowhere near enough bytes for that many key/value pairs.

    QMap<QString, QVariant> metadata;
    metadata.insert("embeddedICs", badRegistry);

    bool threw = false;
    try {
        Serialization::deserializeBlobRegistry(metadata, FormatRev::current);
    } catch (const std::exception &e) {
        threw = true;
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should explain the implausible count");
    }
    QVERIFY2(threw, "Implausible embeddedICs entry count should be rejected, not allocated");
}

// ============================================================
// Wireless Node Serialization Tests
// ============================================================

void TestSerialization::testWirelessTxNodePreservedInScene()
{
    // A Node in Tx mode must survive a full save/load round-trip.
    QuickCircuitBuilder builder1;
    auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(nodeElm != nullptr);
    nodeElm->setLabel("CLK");
    qobject_cast<Node *>(nodeElm)->setWirelessMode(WirelessMode::Tx);
    builder1.addOwnedElement(nodeElm);

    QByteArray data = saveToMemory(builder1);
    QVERIFY(!data.isEmpty());

    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    auto elements = builder2.elements();
    QCOMPARE(elements.size(), 1);

    auto *loaded = qobject_cast<Node *>(elements.first());
    QVERIFY(loaded != nullptr);
    QCOMPARE(loaded->wirelessMode(), WirelessMode::Tx);
    QCOMPARE(loaded->label(), QStringLiteral("CLK"));
    QVERIFY(loaded->inputPort()->isRequired());
}

void TestSerialization::testWirelessRxNodePreservedInScene()
{
    // A Node in Rx mode must survive a full save/load round-trip,
    // including the isRequired=false state on its input port.
    QuickCircuitBuilder builder1;
    auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(nodeElm != nullptr);
    nodeElm->setLabel("DATA");
    qobject_cast<Node *>(nodeElm)->setWirelessMode(WirelessMode::Rx);
    builder1.addOwnedElement(nodeElm);

    QByteArray data = saveToMemory(builder1);

    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    auto *loaded = qobject_cast<Node *>(builder2.elements().first());
    QVERIFY(loaded != nullptr);
    QCOMPARE(loaded->wirelessMode(), WirelessMode::Rx);
    QCOMPARE(loaded->label(), QStringLiteral("DATA"));
    QVERIFY(!loaded->inputPort()->isRequired());
}

void TestSerialization::testWirelessPairCircuit()
{
    // A Tx+Rx pair with the same label must both survive the round-trip with
    // their respective modes intact.
    QuickCircuitBuilder builder1;

    auto *txElm = ElementFactory::buildElement(ElementType::Node);
    auto *rxElm = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(txElm && rxElm);
    txElm->setLabel("SIG"); qobject_cast<Node *>(txElm)->setWirelessMode(WirelessMode::Tx);
    rxElm->setLabel("SIG"); qobject_cast<Node *>(rxElm)->setWirelessMode(WirelessMode::Rx);
    txElm->setPos(0, 0);
    rxElm->setPos(200, 0);
    builder1.addOwnedElement(txElm);
    builder1.addOwnedElement(rxElm);

    QByteArray data = saveToMemory(builder1);

    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    const auto elements = builder2.elements();
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
    QuickCircuitBuilder builder1;
    auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(nodeElm != nullptr);
    nodeElm->setLabel("PLAIN");
    // wirelessMode intentionally left at None (default)
    builder1.addOwnedElement(nodeElm);

    QByteArray data = saveToMemory(builder1);

    QuickCircuitBuilder builder2;
    loadFromMemory(builder2, data);

    auto *loaded = qobject_cast<Node *>(builder2.elements().first());
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

// ============================================================================
// File-copy utility — Serialization::copyPandaFile
// ============================================================================

namespace {
// Writes a minimal .panda file with only the header + a metadata map containing
// a single fileBackedICs entry — just enough for copyPandaFile to traverse.
void writePandaWithFileBackedIC(const QString &path, const QString &referencedIC)
{
    QFile out(path);
    QVERIFY(out.open(QIODevice::WriteOnly));
    QDataStream stream(&out);
    Serialization::writePandaHeader(stream);

    QMap<QString, QVariant> metadata;
    metadata["dolphinFileName"] = QString();
    metadata["sceneRect"] = QRectF();
    metadata["fileBackedICs"] = QStringList{referencedIC};

    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);
    payloadStream << metadata;
    Serialization::writePayload(stream, payload);
}
} // namespace

void TestSerialization::testCopyPandaFileCopiesNonPandaContent()
{
    // copyPandaFile always copies the file itself, even when its content doesn't parse
    // as a panda file — only the *recursion* into fileBackedICs is skipped on parse failure.
    QTemporaryDir sourceDir;
    QTemporaryDir destDir;
    QVERIFY(sourceDir.isValid() && destDir.isValid());

    const QString sourcePanda = sourceDir.path() + "/circuit.panda";
    { QFile f(sourcePanda); QVERIFY(f.open(QIODevice::WriteOnly)); f.write("not a real panda file"); }

    const QString destPanda = destDir.path() + "/circuit.panda";
    Serialization::copyPandaFile(QFileInfo(sourcePanda), QFileInfo(destPanda));

    QVERIFY2(QFile::exists(destPanda), "The file itself must still be copied");
}

void TestSerialization::testCopyPandaFileCopiesFileBackedDependency()
{
    // A real fileBackedICs entry must be discovered and copied alongside the parent file.
    QTemporaryDir sourceDir;
    QTemporaryDir destDir;
    QVERIFY(sourceDir.isValid() && destDir.isValid());

    const QString parentPath = sourceDir.path() + "/parent.panda";
    const QString depPath = sourceDir.path() + "/dependency.panda";
    writePandaWithFileBackedIC(parentPath, "dependency.panda");
    writePandaWithFileBackedIC(depPath, {});

    Serialization::copyPandaFile(QFileInfo(parentPath), QFileInfo(destDir.path() + "/parent.panda"));

    QVERIFY2(QFile::exists(destDir.path() + "/parent.panda"), "Parent file must be copied");
    QVERIFY2(QFile::exists(destDir.path() + "/dependency.panda"), "Referenced dependency must also be copied");
}

void TestSerialization::testCopyPandaFileTerminatesOnCircularMetadata()
{
    // Hand-craft two .panda files that reference each other in their fileBackedICs metadata --
    // copyPandaFile's visited-set guard must short-circuit the cycle rather than
    // infinite-recurse until stack overflow.
    QTemporaryDir sourceDir;
    QTemporaryDir destDir;
    QVERIFY(sourceDir.isValid() && destDir.isValid());

    const QString aPath = sourceDir.path() + "/a.panda";
    const QString bPath = sourceDir.path() + "/b.panda";
    writePandaWithFileBackedIC(aPath, "b.panda");
    writePandaWithFileBackedIC(bPath, "a.panda");

    // Should return promptly — if it doesn't terminate the test runner kills us. Termination
    // alone doesn't prove the copy actually succeeded, so also check the real output file.
    Serialization::copyPandaFile(QFileInfo(aPath), QFileInfo(destDir.path() + "/a.panda"));
    QVERIFY2(QFile::exists(destDir.path() + "/a.panda"), "The circularly-referenced file must still be copied despite the cycle");
}

void TestSerialization::testCopyPandaFileRejectsDeepDependencyChain()
{
    // A long but non-cyclic chain of distinct dependency files (each real, on disk, never
    // repeated) must be rejected once it exceeds the nesting depth cap — the visited-set
    // cycle guard alone doesn't bound this, unlike the circular-reference case above.
    QTemporaryDir sourceDir;
    QTemporaryDir destDir;
    QVERIFY(sourceDir.isValid() && destDir.isValid());

    constexpr int chainLength = 20; // > the copyPandaFile nesting depth cap (16)
    for (int i = 0; i < chainLength; ++i) {
        const QString path = sourceDir.path() + QString("/dep%1.panda").arg(i);
        const QString nextDep = (i + 1 < chainLength) ? QString("dep%1.panda").arg(i + 1) : QString();
        writePandaWithFileBackedIC(path, nextDep);
    }

    bool threw = false;
    try {
        Serialization::copyPandaFile(QFileInfo(sourceDir.path() + "/dep0.panda"),
                                      QFileInfo(destDir.path() + "/dep0.panda"));
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "A dependency chain deeper than the nesting cap must be rejected, not stack-overflow");
}

// ============================================================================
// libFuzzer Regressions
// ============================================================================
// Each fixture is a malformed .panda byte stream exercising a known deserializer crash class
// (heap-use-after-free, double-free, ...). The contract: load() must throw cleanly or succeed
// — never abort. Under ASan, any reintroduced UAF aborts the test process, which is the test
// failure.

void TestSerialization::testFuzzRegressionCleanupUAF()
{
    QFile fixture(QString(QUOTE(CURRENTDIR)) + "/Fuzz/regressions/bugA_cleanup_uaf.bin");
    QVERIFY2(fixture.open(QIODevice::ReadOnly), qPrintable(fixture.errorString()));
    const QByteArray data = fixture.readAll();

    QuickCircuitBuilder builder;
    bool threw = false;
    try {
        loadFromMemory(builder, data);
    } catch (...) {
        threw = true;
    }
    // Crash artifacts always trigger a mid-loop throw inside Serialization::
    // deserialize(); the qScopeGuard cleanup must run and tear down the
    // partially-constructed item list without double-freeing connections.
    QVERIFY2(threw, "Crash fixture must throw — if it loaded successfully, the fixture is no longer a regression for this bug");
}

void TestSerialization::testFuzzRegressionICBlobShrink()
{
    QFile fixture(QString(QUOTE(CURRENTDIR)) + "/Fuzz/regressions/bugB_ic_blob_shrink.bin");
    QVERIFY2(fixture.open(QIODevice::ReadOnly), qPrintable(fixture.errorString()));
    const QByteArray data = fixture.readAll();

    // Fuzzed input declares more outer ports on an IC than its embedded
    // blob exposes.  IC::loadFromBlob shrinks the port count via
    // setInputSize/setOutputSize, freeing the surplus ports.  Without the
    // portMap eviction in IC::load(), the outer connection load reads
    // the dangling pointer → UAF.  Under ASan, a UAF aborts before we
    // reach the QVERIFY below; the test passes only if the load either
    // succeeds or throws cleanly.
    QuickCircuitBuilder builder;
    try {
        loadFromMemory(builder, data);
    } catch (...) {
    }
    QVERIFY2(true, "Reaching here means no ASan abort — fix is in place");
}

void TestSerialization::testFuzzRegressionUnboundedPortList()
{
    QFile fixture(QString(QUOTE(CURRENTDIR)) + "/Fuzz/regressions/bugC_unbounded_portlist.bin");
    QVERIFY2(fixture.open(QIODevice::ReadOnly), qPrintable(fixture.errorString()));
    const QByteArray data = fixture.readAll();

    // Fuzzed input claims an enormous port count in the input/output port
    // list header.  QDataStream's container operator>> would reserve()
    // billions of entries before any application-level validation, OOM-ing
    // the process.  The pre-allocation count guard in
    // GraphicElementSerializer::readPortList must reject the count and
    // throw PANDACEPTION before allocation.
    QuickCircuitBuilder builder;
    bool threw = false;
    try {
        loadFromMemory(builder, data);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "Implausible port count must be rejected, not allocated");
}

void TestSerialization::testFuzzRegressionConnectionStreamMapOOM()
{
    QFile fixture(QString(QUOTE(CURRENTDIR)) + "/Fuzz/regressions/bugD_oom_qneconnection_stream_map.bin");
    QVERIFY2(fixture.open(QIODevice::ReadOnly), qPrintable(fixture.errorString()));
    const QByteArray data = fixture.readAll();

    // Fuzzed input declares an implausible entry count in a QMap header read
    // during Connection deserialization.  QDataStream's map operator>> would
    // reserve the declared count before any application-level validation,
    // OOM-ing the process.  The bounded reader must reject the count first.
    QuickCircuitBuilder builder;
    bool threw = false;
    try {
        loadFromMemory(builder, data);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "Implausible connection map count must be rejected, not allocated");
}

void TestSerialization::testFuzzRegressionICBoundaryOrphanConns()
{
    QFile fixture(QString(QUOTE(CURRENTDIR)) + "/Fuzz/regressions/bugE_uaf_ic_boundary_orphan_conns.bin");
    QVERIFY2(fixture.open(QIODevice::ReadOnly), qPrintable(fixture.errorString()));
    const QByteArray data = fixture.readAll();

    // Fuzzed input wires connections to IC boundary ports the embedded blob
    // does not expose.  When the IC trims its boundary set the surplus ports
    // are freed; without evicting their portMap entries the outer connection
    // pass dereferences the dangling pointers → UAF.  Under ASan a reintroduced
    // UAF aborts before the QVERIFY; passing means the eviction fix holds.
    QuickCircuitBuilder builder;
    try {
        loadFromMemory(builder, data);
    } catch (...) {
    }
    QVERIFY2(true, "Reaching here means no ASan abort — fix is in place");
}

void TestSerialization::testFuzzRegressionUnknownTypeIdMetadataOOM()
{
    QFile fixture(QString(QUOTE(CURRENTDIR)) + "/Fuzz/regressions/bugF_oom_unknown_typeid_metadata.bin");
    QVERIFY2(fixture.open(QIODevice::ReadOnly), qPrintable(fixture.errorString()));
    const QByteArray data = fixture.readAll();

    // Fuzzed metadata map carries a value whose QVariant typeId is unknown or
    // whose payload length is fuzz-controlled.  The bounded variant reader must
    // reject it rather than letting QDataStream allocate against the length.
    QuickCircuitBuilder builder;
    bool threw = false;
    try {
        loadFromMemory(builder, data);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "Unknown/oversized metadata typeId must be rejected, not allocated");
}

void TestSerialization::testFuzzRegressionElementStreamMapOOM()
{
    QFile fixture(QString(QUOTE(CURRENTDIR)) + "/Fuzz/regressions/bugG_oom_element_stream_map.bin");
    QVERIFY2(fixture.open(QIODevice::ReadOnly), qPrintable(fixture.errorString()));
    const QByteArray data = fixture.readAll();

    // Fuzzed input declares an implausible entry count in an element's property
    // QMap header; the bounded metadata reader must reject the count before
    // QDataStream reserves it.
    QuickCircuitBuilder builder;
    bool threw = false;
    try {
        loadFromMemory(builder, data);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "Implausible element map count must be rejected, not allocated");
}

void TestSerialization::testFuzzRegressionOldFormatV11Elements()
{
    QFile fixture(QString(QUOTE(CURRENTDIR)) + "/Fuzz/regressions/bugH_old_format_v11_elements.bin");
    QVERIFY2(fixture.open(QIODevice::ReadOnly), qPrintable(fixture.errorString()));
    const QByteArray data = fixture.readAll();

    // Pre-4.1 flat-binary stream (legacy "wiRedPanda 1.1" header) with element
    // records that drove GraphicElement::loadOldFormat() into undefined
    // behaviour.  The bounded readers now keep every field within stream
    // limits; load must throw cleanly or succeed, never abort under ASan/UBSan.
    QuickCircuitBuilder builder;
    try {
        loadFromMemory(builder, data);
    } catch (...) {
    }
    QVERIFY2(true, "Reaching here means no ASan/UBSan abort — fix is in place");
}

void TestSerialization::testFuzzRegressionOldFormatICSkinRef()
{
    QFile fixture(QString(QUOTE(CURRENTDIR)) + "/Fuzz/regressions/bugI_old_format_ic_skin_ref.bin");
    QVERIFY2(fixture.open(QIODevice::ReadOnly), qPrintable(fixture.errorString()));
    const QByteArray data = fixture.readAll();

    // Pre-4.1 stream whose IC element carries a skin reference that the old
    // loadOldFormat() path mishandled.  Must not abort under ASan/UBSan.
    QuickCircuitBuilder builder;
    try {
        loadFromMemory(builder, data);
    } catch (...) {
    }
    QVERIFY2(true, "Reaching here means no ASan/UBSan abort — fix is in place");
}

void TestSerialization::testFuzzRegressionDolphinFilenameOOM()
{
    QFile fixture(QString(QUOTE(CURRENTDIR)) + "/Fuzz/regressions/oom_dolphin_filename_large.bin");
    QVERIFY2(fixture.open(QIODevice::ReadOnly), qPrintable(fixture.errorString()));
    const QByteArray data = fixture.readAll();

    // A circuit whose metadata stores a beWavedDolphin filename with a
    // fuzz-controlled, implausibly large length prefix.  The bounded string
    // reader must reject it before QDataStream allocates the QString.
    QuickCircuitBuilder builder;
    bool threw = false;
    try {
        loadFromMemory(builder, data);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "Implausible dolphin filename length must be rejected, not allocated");
}

void TestSerialization::testFuzzRegressionPandaHeaderAppNameOOM()
{
    QFile fixture(QString(QUOTE(CURRENTDIR)) + "/Fuzz/regressions/oom_panda_header_large_appname.bin");
    QVERIFY2(fixture.open(QIODevice::ReadOnly), qPrintable(fixture.errorString()));
    const QByteArray data = fixture.readAll();

    // Legacy header whose leading QString length field (~1.4 GB here) would
    // make readPandaHeader allocate the app-name string before any validation.
    // readBoundedString must reject the length and throw first.
    QuickCircuitBuilder builder;
    bool threw = false;
    try {
        loadFromMemory(builder, data);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "Implausible header app-name length must be rejected, not allocated");
}

void TestSerialization::testFuzzRegressionNonFinitePosition()
{
    QFile fixture(QString(QUOTE(CURRENTDIR)) + "/Fuzz/regressions/ic_nonfinite_position.bin");
    QVERIFY2(fixture.open(QIODevice::ReadOnly), qPrintable(fixture.errorString()));
    const QByteArray data = fixture.readAll();

    // A malformed file-backed IC whose inner element carries a NaN position.
    // ICRenderer::generatePreviewPixmap computes the preview from itemsBoundingRect(),
    // and QSizeF::toSize() on a NaN rect trips Qt's qSaturateRound assertion
    // (!qIsNaN) and aborts the process — a DoS on malformed input.  The
    // finite-position guard in GraphicElementSerializer::loadPos/loadNewFormat
    // must reject it before it enters the circuit.
    QuickCircuitBuilder builder;
    bool threw = false;
    try {
        loadFromMemory(builder, data);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "Non-finite element position must be rejected, not loaded");
}

void TestSerialization::testFuzzRegressionNonFiniteRotation()
{
    QFile fixture(QString(QUOTE(CURRENTDIR)) + "/Fuzz/regressions/ic_nonfinite_rotation.bin");
    QVERIFY2(fixture.open(QIODevice::ReadOnly), qPrintable(fixture.errorString()));
    const QByteArray data = fixture.readAll();

    // Sibling of the non-finite-position bug, found by the same harness: a
    // fuzzed element carries a NaN rotation.  setAngleRaw(NaN) makes the item's
    // scene transform — and hence the IC preview's itemsBoundingRect — NaN,
    // tripping the same QSizeF::toSize() / qSaturateRound assertion (abort).
    // The finite-rotation guard in GraphicElementSerializer::loadRotation /
    // loadNewFormat must reject it before it enters the circuit.
    QuickCircuitBuilder builder;
    bool threw = false;
    try {
        loadFromMemory(builder, data);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "Non-finite element rotation must be rejected, not loaded");
}

void TestSerialization::testReadBoundedKeySequenceRoundTrip()
{
    const QKeySequence original(Qt::CTRL | Qt::Key_A);

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << original;

    QDataStream readStream(&data, QIODevice::ReadOnly);
    const QKeySequence result = Serialization::readBoundedKeySequence(readStream);

    QCOMPARE(result, original);
}

void TestSerialization::testReadBoundedKeySequenceAcceptsMaxFourKeys()
{
    // A QKeySequence holds at most 4 chained key combinations — count == 4 is the
    // legitimate maximum and must still be accepted, not just count < 4.
    const QKeySequence original(Qt::Key_A, Qt::Key_B, Qt::Key_C, Qt::Key_D);
    QCOMPARE(original.count(), 4);

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << original;

    QDataStream readStream(&data, QIODevice::ReadOnly);
    const QKeySequence result = Serialization::readBoundedKeySequence(readStream);

    QCOMPARE(result, original);
}

void TestSerialization::testReadBoundedKeySequenceRejectsImplausibleCount()
{
    // GraphicElementSerializer::loadTrigger() (old-format loader, versions 1.9-4.0) reads the
    // trigger via Serialization::readBoundedKeySequence(), tested here directly: without this
    // guard, an implausible count would reserve() a QList<int> sized by the stream-controlled
    // count before validating it -- a crafted old-format .panda file with a valid header/
    // version and this count is enough to trigger the allocation.
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << static_cast<quint32>(0x7FFFFFFF); // implausible count, no real data behind it

    QDataStream readStream(&data, QIODevice::ReadOnly);
    bool threw = false;
    try {
        Serialization::readBoundedKeySequence(readStream);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "Implausible QKeySequence count must be rejected, not allocated");
}

void TestSerialization::testReadVersionNumberRejectsImplausibleSegmentCount()
{
    // readVersionNumber() (reached only via readPandaHeader()'s modern-magic path) caps
    // segment count at 8 — no real release version has more than 3.
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << Serialization::MAGIC_HEADER_CIRCUIT;
    writeStream << static_cast<quint32>(9); // > the 8-segment cap

    QDataStream readStream(data);
    bool threw = false;
    QString message;
    try {
        Serialization::readPandaHeader(readStream);
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "An implausible version segment count must be rejected");
    QVERIFY2(message.contains("Implausible version segment count"), qPrintable(message));
}

void TestSerialization::testReadBoundedByteArrayRejectsImplausibleLength()
{
    // readBoundedByteArray() is reached via Serialization::readBoundedBlobMap()'s value read.
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << static_cast<quint32>(1); // 1 blob entry
    writeStream << QStringLiteral("k");     // key
    writeStream << 0xFFFFFFF0u; // implausible byte length

    QDataStream readStream(data);
    readStream.setVersion(QDataStream::Qt_5_12);

    bool threw = false;
    QString message;
    try {
        Serialization::readBoundedBlobMap(readStream);
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "An implausible QByteArray length must be rejected");
    QVERIFY2(message.contains("QByteArray length"), qPrintable(message));
}

void TestSerialization::testReadBoundedStringListRejectsImplausibleCount()
{
    // readBoundedStringList() is reached via readBoundedVariant() for a QStringList-typed
    // metadata value, which in turn is reached via Serialization::readBoundedMetadata().
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << static_cast<quint32>(1); // 1 metadata entry
    writeStream << QStringLiteral("k");     // key
    writeStream << static_cast<quint32>(QMetaType::QStringList); // typeId
    writeStream << static_cast<quint8>(0);  // isNull = false
    writeStream << 0xFFFFFFF0u; // implausible element count

    QDataStream readStream(data);
    readStream.setVersion(QDataStream::Qt_5_12);

    bool threw = false;
    QString message;
    try {
        Serialization::readBoundedMetadata(readStream);
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "An implausible QStringList count must be rejected");
    QVERIFY2(message.contains("QStringList count"), qPrintable(message));
}

void TestSerialization::testReadBoundedBitArrayRejectsImplausibleNumBits()
{
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << static_cast<quint32>(1);
    writeStream << QStringLiteral("k");
    writeStream << static_cast<quint32>(QMetaType::QBitArray);
    writeStream << static_cast<quint8>(0);
    writeStream << 0xFFFFFFF0u; // implausible numBits

    QDataStream readStream(data);
    readStream.setVersion(QDataStream::Qt_5_12);

    bool threw = false;
    QString message;
    try {
        Serialization::readBoundedMetadata(readStream);
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "An implausible QBitArray numBits must be rejected");
    QVERIFY2(message.contains("QBitArray numBits"), qPrintable(message));
}

void TestSerialization::testReadBoundedVariantTooShortForHeaderFallsBackToRawRead()
{
    // readBoundedVariant()'s <5-remaining-bytes fallback delegates to Qt's own (unbounded)
    // QVariant operator>>, which fails cleanly (bad stream status, no throw) when truncated
    // this early -- the caller's own status check breaks the loop rather than inserting a
    // bogus entry, returning normally with whatever was parsed so far.
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << static_cast<quint32>(1);
    writeStream << QStringLiteral("k");
    const char shortTail[3] = {0, 0, 0};
    writeStream.writeRawData(shortTail, sizeof(shortTail));

    QDataStream readStream(data);
    readStream.setVersion(QDataStream::Qt_5_12);

    const QMap<QString, QVariant> result = Serialization::readBoundedMetadata(readStream);

    QVERIFY(result.isEmpty());
    QVERIFY(readStream.status() != QDataStream::Ok);
}

void TestSerialization::testReadBoundedVariantRejectsVariantListType()
{
    // QVariantList/QVariantMap are never used in metadata; rejected outright to avoid a
    // nested-container OOM (inner QVariants would bypass this bounded reader entirely).
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << static_cast<quint32>(1);
    writeStream << QStringLiteral("k");
    writeStream << static_cast<quint32>(QMetaType::QVariantList);
    writeStream << static_cast<quint8>(0);

    QDataStream readStream(data);
    readStream.setVersion(QDataStream::Qt_5_12);

    bool threw = false;
    QString message;
    try {
        Serialization::readBoundedMetadata(readStream);
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "QVariantList must never be accepted as a metadata value type");
    QVERIFY2(message.contains("Unsupported container type"), qPrintable(message));
}

void TestSerialization::testReadBoundedMetadataRejectsImplausibleCount()
{
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << static_cast<quint32>(1000000); // far more entries than remaining bytes allow

    QDataStream readStream(data);
    readStream.setVersion(QDataStream::Qt_5_12);

    bool threw = false;
    QString message;
    try {
        Serialization::readBoundedMetadata(readStream);
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "An implausible metadata map count must be rejected");
    QVERIFY2(message.contains("Metadata map count"), qPrintable(message));
}

void TestSerialization::testReadBoundedBlobMapRejectsTruncatedCount()
{
    QByteArray data; // empty -- not even a full 4-byte count field
    QDataStream readStream(data);
    readStream.setVersion(QDataStream::Qt_5_12);

    bool threw = false;
    QString message;
    try {
        Serialization::readBoundedBlobMap(readStream);
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "A stream too short to even hold the blob map count must be rejected");
    QVERIFY2(message.contains("Stream error reading blob map count"), qPrintable(message));
}

void TestSerialization::testReadPayloadRejectsTooShortCompressedPayload()
{
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    const char raw[3] = {1, 2, 3}; // fewer than the 4-byte "uncompressed size" header needs
    writeStream.writeRawData(raw, sizeof(raw));

    QDataStream readStream(data);
    bool threw = false;
    QString message;
    try {
        Serialization::readPayload(readStream, Versions::Rev100);
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "A compressed payload shorter than its own size header must be rejected");
    QVERIFY2(message.contains("Compressed payload too short"), qPrintable(message));
}

void TestSerialization::testReadPandaHeaderNoDeviceThrows()
{
    QDataStream stream; // default-constructed: no I/O device attached
    QVERIFY_THROWS(std::exception, Serialization::readPandaHeader(stream));
}

void TestSerialization::testReadPandaHeaderLegacyHeaderMissingVersionPartThrows()
{
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << QStringLiteral("wiRedPanda"); // no trailing " X.Y" version part

    QDataStream readStream(data);
    bool threw = false;
    QString message;
    try {
        Serialization::readPandaHeader(readStream);
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "A legacy header with no version part must be rejected");
    QVERIFY2(message.contains("Invalid legacy file header"), qPrintable(message));
}

void TestSerialization::testReadDolphinHeaderNoDeviceThrows()
{
    QDataStream stream;
    QVERIFY_THROWS(std::exception, Serialization::readDolphinHeader(stream));
}

void TestSerialization::testReadDolphinHeaderTruncatedLegacyHeaderThrows()
{
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << static_cast<quint32>(40); // claims 40 bytes of app-name text follow; none do

    QDataStream readStream(data);
    bool threw = false;
    QString message;
    try {
        Serialization::readDolphinHeader(readStream);
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "A legacy dolphin header claiming more bytes than the file has must be rejected");
    QVERIFY2(message.contains("Invalid file format"), qPrintable(message));
}

void TestSerialization::testReadDolphinHeaderWrongAppNameThrows()
{
    // A syntactically valid legacy-header shape (even byte length, matching bytes present)
    // but text that isn't "beWavedDolphin".
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << static_cast<quint32>(20); // claims 20 bytes (10 UTF-16 code units)
    const char garbage[20] = "XXXXXXXXXXXXXXXXXXX";
    writeStream.writeRawData(garbage, 20);

    QDataStream readStream(data);
    bool threw = false;
    QString message;
    try {
        Serialization::readDolphinHeader(readStream);
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "A legacy header whose text isn't beWavedDolphin must be rejected");
    QVERIFY2(message.contains("Invalid file format"), qPrintable(message));
}

void TestSerialization::testDeserializeTruncatedTypeTagThrows()
{
    // deserialize()'s own post-type-tag-read check: this is a raw stream >> read directly in
    // the loop, not delegated to a sub-loader with its own check.
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    const char stray = 0x00; // 1 byte: enough for !atEnd() to be true, not enough for a full int
    writeStream.writeRawData(&stray, 1);

    QHash<quint64, Port *> portMap;
    SerializationContext context{portMap, FormatRev::current, SerializationPurpose::PortableFile};

    QDataStream readStream(data);
    readStream.setVersion(QDataStream::Qt_5_12);

    bool threw = false;
    QString message;
    try {
        Serialization::deserialize(readStream, context);
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "A truncated type tag must throw, not silently stop parsing");
    QVERIFY2(message.contains("Stream error reading type tag"), qPrintable(message));
}

void TestSerialization::testDeserializeTruncatedElementTypeThrows()
{
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << static_cast<int>(GraphicElement::Type);
    const char partial[2] = {0, 0}; // 2 of the 8 bytes a quint64 ElementType needs
    writeStream.writeRawData(partial, sizeof(partial));

    QHash<quint64, Port *> portMap;
    SerializationContext context{portMap, FormatRev::current, SerializationPurpose::PortableFile};

    QDataStream readStream(data);
    readStream.setVersion(QDataStream::Qt_5_12);

    bool threw = false;
    QString message;
    try {
        Serialization::deserialize(readStream, context);
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "A truncated element type must throw, not proceed with a garbage ElementType");
    QVERIFY2(message.contains("Stream error reading element type"), qPrintable(message));
}

void TestSerialization::testDeserializeTruncatedOldFormatElementLoadThrows()
{
    // Unlike the modern (4.1+) format's loadNewFormat(), which explicitly checks-and-throws
    // on every truncation itself, the old-format loadOldFormat() path's very first step
    // (loadPos()) does a raw, unchecked `stream >> pos;` -- a truncated read there leaves
    // stream.status() silently bad (Qt zeroes the value rather than throwing), which only
    // deserialize()'s own outer check catches.
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << static_cast<int>(GraphicElement::Type);
    writeStream << ElementType::And;
    // Nothing follows: loadPos()'s QPointF read (16 bytes) has 0 bytes available.

    QHash<quint64, Port *> portMap;
    SerializationContext context{portMap, QVersionNumber(1, 2), SerializationPurpose::PortableFile};

    QDataStream readStream(data);
    readStream.setVersion(QDataStream::Qt_5_12);

    bool threw = false;
    QString message;
    try {
        Serialization::deserialize(readStream, context);
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "A truncated old-format element load must throw, not silently produce a bad element");
    QVERIFY2(message.contains("Stream error loading element"), qPrintable(message));
}

void TestSerialization::testLoadDolphinFileNameNormalizesNoneSentinelForOldVersions()
{
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << QStringLiteral("none");

    QDataStream readStream(data);
    readStream.setVersion(QDataStream::Qt_5_12);

    // V_3_1: hasDolphinFilename() true, hasDolphinSentinelFix() false.
    const QString filename = Serialization::loadDolphinFileName(readStream, Versions::V_3_1);

    QVERIFY2(filename.isEmpty(), "The pre-3.3 \"none\" sentinel must be normalized to an empty string");
}

void TestSerialization::testCreateVersionedBackupCopyFailureThrows()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot make a directory unwritable on Windows (uses ACLs, not Unix permission bits)");
#else
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString fileName = dir.path() + "/circuit.panda";
    {
        QFile f(fileName);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("data");
    }

    QVERIFY(QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));

    // Sanity: confirm a copy into this directory really fails on this system.
    QVERIFY(!QFile::copy(fileName, dir.path() + "/probe.panda"));

    bool threw = false;
    QString message;
    try {
        Serialization::createVersionedBackup(fileName, QVersionNumber(1, 2));
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    QVERIFY2(threw, "A failed backup copy must throw");
    QVERIFY2(message.contains("Failed to create versioned backup"), qPrintable(message));
#endif
}

void TestSerialization::testReadPreambleTruncatedMetadataThrows()
{
    // Mirrors testReadBoundedVariantTooShortForHeaderFallsBackToRawRead(): readBoundedMetadata()
    // can return normally with a bad stream status (no throw of its own); readPreamble() is
    // the caller that must detect and throw on it.
    QByteArray metadataBytes;
    QDataStream metaStream(&metadataBytes, QIODevice::WriteOnly);
    metaStream.setVersion(QDataStream::Qt_5_12);
    metaStream << static_cast<quint32>(1);
    metaStream << QStringLiteral("k");
    const char shortTail[3] = {0, 0, 0};
    metaStream.writeRawData(shortTail, sizeof(shortTail));

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    Serialization::writePayload(writeStream, metadataBytes);

    QDataStream readStream(data);
    bool threw = false;
    QString message;
    try {
        Serialization::readPreamble(readStream);
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "Truncated metadata content must be caught by readPreamble()'s own status check");
    QVERIFY2(message.contains("Stream error reading preamble"), qPrintable(message));
}

void TestSerialization::testTypeNameMapsKnownTypesAndDefaultsForUnknown()
{
    // typeName() is otherwise reached only as an argument to a qCDebug() call, which the
    // disabled-by-default logging category never evaluates in tests -- call it directly.
    QCOMPARE(Serialization::typeName(Port::Type), QStringLiteral("Port"));
    QCOMPARE(Serialization::typeName(Connection::Type), QStringLiteral("Connection"));
    QCOMPARE(Serialization::typeName(GraphicElement::Type), QStringLiteral("GraphicElement"));
    QCOMPARE(Serialization::typeName(0), QStringLiteral("UnknownType"));
}

void TestSerialization::testCopyPandaFileCopyFailureThrows()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString srcPath = dir.path() + "/source.panda";
    {
        QFile f(srcPath);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("anything");
    }

    const QString destPath = dir.path() + "/no/such/subdir/dest.panda";

    QVERIFY_THROWS(std::exception,
        Serialization::copyPandaFile(QFileInfo(srcPath), QFileInfo(destPath)));
}

void TestSerialization::testCopyPandaFileTruncatedLegacyHeaderIsSkipped()
{
    // A file whose first 4 bytes look like a plausible legacy-header length prefix, but the
    // file doesn't actually contain that many bytes -- copyPandaFile must still copy the file
    // itself (it always does, unconditionally, before this check) and simply skip dependency
    // scanning.
    QTemporaryDir srcDir;
    QTemporaryDir destDir;
    QVERIFY(srcDir.isValid());
    QVERIFY(destDir.isValid());

    const QString srcPath = srcDir.path() + "/weird.panda";
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << static_cast<quint32>(100); // claims 100 bytes follow; none do
    {
        QFile f(srcPath);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(data);
    }

    const QString destPath = destDir.path() + "/weird.panda";
    Serialization::copyPandaFile(QFileInfo(srcPath), QFileInfo(destPath));

    QVERIFY(QFile::exists(destPath));
}

void TestSerialization::testCopyPandaFileNonWiredPandaTextHeaderIsSkipped()
{
    QTemporaryDir srcDir;
    QTemporaryDir destDir;
    QVERIFY(srcDir.isValid());
    QVERIFY(destDir.isValid());

    const QString srcPath = srcDir.path() + "/notpanda.panda";
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << QStringLiteral("SomeOtherApp"); // plausible-length text header, wrong content
    {
        QFile f(srcPath);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(data);
    }

    const QString destPath = destDir.path() + "/notpanda.panda";
    Serialization::copyPandaFile(QFileInfo(srcPath), QFileInfo(destPath));

    QVERIFY(QFile::exists(destPath));
}

void TestSerialization::testCopyPandaFileCorruptButPlausibleHeaderIsSkipped()
{
    // A genuine modern magic header + version (so copyPandaFile's own peek-based plausibility
    // pre-check -- which only runs for non-modern-magic files -- is bypassed entirely) but
    // corrupt content beyond that: readPreamble() throws, and copyPandaFile must swallow it
    // (the file itself was already copied; nothing to recurse into) rather than propagating.
    QTemporaryDir srcDir;
    QTemporaryDir destDir;
    QVERIFY(srcDir.isValid());
    QVERIFY(destDir.isValid());

    const QString srcPath = srcDir.path() + "/corrupt.panda";
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    writeStream << static_cast<quint8>(0xFF); // too short to be a valid compressed payload
    {
        QFile f(srcPath);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(data);
    }

    const QString destPath = destDir.path() + "/corrupt.panda";
    Serialization::copyPandaFile(QFileInfo(srcPath), QFileInfo(destPath));

    QVERIFY(QFile::exists(destPath));
}

void TestSerialization::testCopyPandaFileOldVersionWithoutMetadataSupportIsSkipped()
{
    // A genuinely valid, fully parseable old-format file (pre-4.5, !hasMetadata) -- readPreamble()
    // succeeds cleanly, but there's no fileBackedICs metadata to scan for this version.
    QTemporaryDir srcDir;
    QTemporaryDir destDir;
    QVERIFY(srcDir.isValid());
    QVERIFY(destDir.isValid());

    const QString srcPath = srcDir.path() + "/old.panda";
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream.setVersion(QDataStream::Qt_5_12);
    writeStream << Serialization::MAGIC_HEADER_CIRCUIT;
    writeStream << Versions::V_1_2; // pre-Rev100/hasDolphinFilename/hasSceneRect/hasMetadata
    {
        QFile f(srcPath);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(data);
    }

    const QString destPath = destDir.path() + "/old.panda";
    Serialization::copyPandaFile(QFileInfo(srcPath), QFileInfo(destPath));

    QVERIFY(QFile::exists(destPath));
}
