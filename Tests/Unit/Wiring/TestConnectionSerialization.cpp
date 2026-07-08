// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Wiring/TestConnectionSerialization.h"

#include <memory>

#include <QDataStream>
#include <QTest>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Workspace.h"
#include "App/Versions.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

void TestConnectionSerialization::initTestCase()
{
    // Initialize test environment
}

// ============================================================================
// Port Reference Restoration Tests (5 tests)
// ============================================================================

void TestConnectionSerialization::testLoadWithEmptyPortMapDirectRestore()
{
    // Test loading connection with empty port map yields no connection
    // With the serial ID system, ports must be registered in portMap to be found

    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Create source elements
    auto andGate = std::make_unique<And>();
    auto orGate = std::make_unique<Or>();
    scene->addItem(andGate.get());
    scene->addItem(orGate.get());

    // Get port pointers (these will be saved)
    auto *outputPort = andGate->outputPort();
    auto *inputPort = orGate->inputPort(0);

    // Create and save connection
    auto conn1 = std::make_unique<Connection>();
    conn1->setStartPort(dynamic_cast<OutputPort *>(outputPort));
    conn1->setEndPort(dynamic_cast<InputPort *>(inputPort));

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    conn1->save(saveStream);

    // Create new connection and load with empty port map
    auto conn2 = std::make_unique<Connection>();
    QDataStream loadStream(data);
    QHash<quint64, Port *> emptyPortMap;

    SerializationContext context = {emptyPortMap, FormatRev::current, SerializationPurpose::PortableFile, QString()};
    conn2->load(loadStream, context);

    QVERIFY(conn2->startPort() == nullptr);
    QVERIFY(conn2->endPort() == nullptr);
}

void TestConnectionSerialization::testLoadWithPortMapIndirectRestore()
{
    // Test loading connection with port map (mapping-based restoration)
    // This is the proper way to load connections with new element instances

    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Create original elements and connection
    auto andGate1 = std::make_unique<And>();
    auto orGate1 = std::make_unique<Or>();
    scene->addItem(andGate1.get());
    scene->addItem(orGate1.get());

    auto *outputPort1 = andGate1->outputPort();
    auto *inputPort1 = orGate1->inputPort(0);

    auto conn1 = std::make_unique<Connection>();
    conn1->setStartPort(dynamic_cast<OutputPort *>(outputPort1));
    conn1->setEndPort(dynamic_cast<InputPort *>(inputPort1));

    // Save connection
    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    conn1->save(saveStream);

    // Calculate serial IDs for fresh ports (see Port::makeSerialId())
    // For inputs: (elementId << 16) | portIndex
    // For outputs: (elementId << 16) | (inputCount + portIndex)
    quint64 origOutputSerial = static_cast<quint64>(andGate1->id()) << 16 | static_cast<quint64>(andGate1->inputSize());
    quint64 origInputSerial = static_cast<quint64>(orGate1->id()) << 16 | 0;

    // Create new elements (simulating new circuit load)
    auto andGate2 = std::make_unique<And>();
    auto orGate2 = std::make_unique<Or>();
    scene->addItem(andGate2.get());
    scene->addItem(orGate2.get());

    auto *outputPort2 = andGate2->outputPort();
    auto *inputPort2 = orGate2->inputPort(0);

    // Create port map (original -> new, using serial IDs as keys)
    QHash<quint64, Port *> portMap;
    portMap[origOutputSerial] = outputPort2;
    portMap[origInputSerial] = inputPort2;

    // Create new connection and load with port map
    auto conn2 = std::make_unique<Connection>();
    QDataStream loadStream(data);
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, QString()};
    conn2->load(loadStream, context);

    // Verify ports were correctly mapped
    QCOMPARE(conn2->startPort(), dynamic_cast<OutputPort *>(outputPort2));
    QCOMPARE(conn2->endPort(), dynamic_cast<InputPort *>(inputPort2));
}

void TestConnectionSerialization::testLoadInvalidPortReferencesHandled()
{
    // Test loading connection with missing port references
    // When portMap doesn't contain a saved port pointer, load() returns early
    // without setting any ports, resulting in a connection with both ports null

    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto andGate = std::make_unique<And>();
    scene->addItem(andGate.get());

    auto *outputPort = andGate->outputPort();

    // Create connection with one valid port
    auto conn1 = std::make_unique<Connection>();
    conn1->setStartPort(dynamic_cast<OutputPort *>(outputPort));

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    conn1->save(saveStream);

    // Create port map with missing port (endPort was null, saves as 0)
    QHash<quint64, Port *> portMap;
    portMap[reinterpret_cast<quint64>(outputPort)] = outputPort;
    // Missing the second port on purpose (0 is not in portMap)

    // Load with incomplete port map
    auto conn2 = std::make_unique<Connection>();
    QDataStream loadStream(data);
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, QString()};
    conn2->load(loadStream, context);

    // With missing port reference, load() returns early, both ports remain null
    // Verify connection is in valid state (no partial connections)
    QVERIFY(conn2->startPort() == nullptr);
    QVERIFY(conn2->endPort() == nullptr);
}

void TestConnectionSerialization::testLoadPortTypeResolution()
{
    // Test that load() correctly determines port types (input vs output)
    // even if ports are presented in either order

    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto sw = std::make_unique<InputSwitch>();
    auto led = std::make_unique<Led>();
    scene->addItem(sw.get());
    scene->addItem(led.get());

    auto *outputPort = sw->outputPort();
    auto *inputPort = led->inputPort();

    // Create connection
    auto conn1 = std::make_unique<Connection>();
    conn1->setStartPort(dynamic_cast<OutputPort *>(outputPort));
    conn1->setEndPort(dynamic_cast<InputPort *>(inputPort));

    // Verify initial setup
    QVERIFY(conn1->startPort() == outputPort);
    QVERIFY(conn1->endPort() == inputPort);

    // Calculate serial IDs (see Port::makeSerialId())
    quint64 outSerial = static_cast<quint64>(sw->id()) << 16 | static_cast<quint64>(sw->inputSize());
    quint64 inSerial = static_cast<quint64>(led->id()) << 16 | 0;

    // Save connection
    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    conn1->save(saveStream);

    // Create port map (using calculated serial IDs)
    QHash<quint64, Port *> portMap;
    portMap[outSerial] = outputPort;
    portMap[inSerial] = inputPort;

    // Load connection
    auto conn2 = std::make_unique<Connection>();
    QDataStream loadStream(data);
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, QString()};
    conn2->load(loadStream, context);

    // Verify ports are correctly identified despite being loaded from stream
    QCOMPARE(conn2->startPort(), dynamic_cast<OutputPort *>(outputPort));
    QCOMPARE(conn2->endPort(), dynamic_cast<InputPort *>(inputPort));
}

void TestConnectionSerialization::testLoadMultipleConnectionsOnSamePorts()
{
    // Test that port map correctly handles multiple connections on same ports
    // (Only one connection per input port allowed, but output can have multiple)

    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto sw1 = std::make_unique<InputSwitch>();
    auto sw2 = std::make_unique<InputSwitch>();
    auto andGate = std::make_unique<And>();
    scene->addItem(sw1.get());
    scene->addItem(sw2.get());
    scene->addItem(andGate.get());

    // Connect both switches to AND gate inputs
    auto conn1 = std::make_unique<Connection>();
    conn1->setStartPort(sw1->outputPort());
    conn1->setEndPort(andGate->inputPort(0));

    auto conn2 = std::make_unique<Connection>();
    conn2->setStartPort(sw2->outputPort());
    conn2->setEndPort(andGate->inputPort(1));

    // Calculate serial IDs for all ports (see Port::makeSerialId())
    quint64 sw1OutSerial = static_cast<quint64>(sw1->id()) << 16 | static_cast<quint64>(sw1->inputSize());
    quint64 sw2OutSerial = static_cast<quint64>(sw2->id()) << 16 | static_cast<quint64>(sw2->inputSize());
    quint64 andIn0Serial = static_cast<quint64>(andGate->id()) << 16 | 0;
    quint64 andIn1Serial = static_cast<quint64>(andGate->id()) << 16 | 1;

    // Save both connections
    QByteArray data1, data2;
    {
        QDataStream s(&data1, QIODevice::WriteOnly);
        conn1->save(s);
    }
    {
        QDataStream s(&data2, QIODevice::WriteOnly);
        conn2->save(s);
    }

    // Create port map (using calculated serial IDs)
    QHash<quint64, Port *> portMap;
    portMap[sw1OutSerial] = sw1->outputPort();
    portMap[sw2OutSerial] = sw2->outputPort();
    portMap[andIn0Serial] = andGate->inputPort(0);
    portMap[andIn1Serial] = andGate->inputPort(1);

    // Load both connections
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, QString()};

    auto loadedConn1 = std::make_unique<Connection>();
    {
        QDataStream s(data1);
        loadedConn1->load(s, context);
    }

    auto loadedConn2 = std::make_unique<Connection>();
    {
        QDataStream s(data2);
        loadedConn2->load(s, context);
    }

    // Verify connections are on correct ports
    QCOMPARE(loadedConn1->endPort(), andGate->inputPort(0));
    QCOMPARE(loadedConn2->endPort(), andGate->inputPort(1));
}

// ============================================================================
// Serialization Round-Trip Tests (3 tests)
// ============================================================================

void TestConnectionSerialization::testSaveLoadRoundTripPreservesPorts()
{
    // Test that saving and loading a connection preserves port connectivity

    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto andGate = std::make_unique<And>();
    auto orGate = std::make_unique<Or>();
    scene->addItem(andGate.get());
    scene->addItem(orGate.get());

    // Create connection
    auto conn1 = std::make_unique<Connection>();
    auto *outputPort = andGate->outputPort();
    auto *inputPort = orGate->inputPort(0);

    conn1->setStartPort(dynamic_cast<OutputPort *>(outputPort));
    conn1->setEndPort(dynamic_cast<InputPort *>(inputPort));

    // Verify initial state
    QCOMPARE(conn1->startPort(), outputPort);
    QCOMPARE(conn1->endPort(), inputPort);
    QVERIFY(outputPort->connections().contains(conn1.get()));
    QVERIFY(inputPort->connections().contains(conn1.get()));

    // Calculate serial IDs (see Port::makeSerialId())
    quint64 andOutSerial = static_cast<quint64>(andGate->id()) << 16 | static_cast<quint64>(andGate->inputSize());
    quint64 orInSerial = static_cast<quint64>(orGate->id()) << 16 | 0;

    // Save
    QByteArray data;
    {
        QDataStream s(&data, QIODevice::WriteOnly);
        conn1->save(s);
    }

    // Load with port map
    QHash<quint64, Port *> portMap;
    portMap[andOutSerial] = outputPort;
    portMap[orInSerial] = inputPort;

    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, QString()};

    auto conn2 = std::make_unique<Connection>();
    {
        QDataStream s(data);
        conn2->load(s, context);
    }

    // Verify ports match
    QCOMPARE(conn2->startPort(), conn1->startPort());
    QCOMPARE(conn2->endPort(), conn1->endPort());
}

void TestConnectionSerialization::testSaveLoadPreservesConnectionStatus()
{
    // Test that connection status is preserved through save/load cycle

    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto sw = std::make_unique<InputSwitch>();
    auto led = std::make_unique<Led>();
    scene->addItem(sw.get());
    scene->addItem(led.get());

    // Create valid connection
    auto conn1 = std::make_unique<Connection>();
    conn1->setStartPort(sw->outputPort());
    conn1->setEndPort(led->inputPort());

    // Set initial status
    conn1->setStatus(Status::Active);
    QCOMPARE(conn1->status(), Status::Active);

    // Calculate serial IDs (see Port::makeSerialId())
    quint64 swOutSerial = static_cast<quint64>(sw->id()) << 16 | static_cast<quint64>(sw->inputSize());
    quint64 ledInSerial = static_cast<quint64>(led->id()) << 16 | 0;

    // Save
    QByteArray data;
    {
        QDataStream s(&data, QIODevice::WriteOnly);
        conn1->save(s);
    }

    // Load
    QHash<quint64, Port *> portMap;
    portMap[swOutSerial] = sw->outputPort();
    portMap[ledInSerial] = led->inputPort();

    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, QString()};

    auto conn2 = std::make_unique<Connection>();
    {
        QDataStream s(data);
        conn2->load(s, context);
    }

    // Status should be synced from start port
    QCOMPARE(conn2->status(), sw->outputPort()->status());
}

void TestConnectionSerialization::testSaveLoadWithStatusPropagation()
{
    // Test that connection status propagates through a chain on load()
    // Chain: sw -> and1 -> and2

    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto sw = std::make_unique<InputSwitch>();
    auto and1 = std::make_unique<And>();
    auto and2 = std::make_unique<And>();
    scene->addItem(sw.get());
    scene->addItem(and1.get());
    scene->addItem(and2.get());

    // sw -> and1 -> and2 chain
    auto conn1 = std::make_unique<Connection>();
    conn1->setStartPort(sw->outputPort());
    conn1->setEndPort(and1->inputPort(0));

    auto conn2 = std::make_unique<Connection>();
    conn2->setStartPort(and1->outputPort());
    conn2->setEndPort(and2->inputPort(0));

    // Turn on switch to make output Active
    sw->setOn();
    QCOMPARE(sw->outputPort()->status(), Status::Active);

    // Calculate serial IDs for all ports (see Port::makeSerialId())
    quint64 swOutSerial = static_cast<quint64>(sw->id()) << 16 | static_cast<quint64>(sw->inputSize());
    quint64 and1In0Serial = static_cast<quint64>(and1->id()) << 16 | 0;
    quint64 and1OutSerial = static_cast<quint64>(and1->id()) << 16 | static_cast<quint64>(and1->inputSize());
    quint64 and2In0Serial = static_cast<quint64>(and2->id()) << 16 | 0;

    // Save both connections
    QByteArray data1, data2;
    {
        QDataStream s(&data1, QIODevice::WriteOnly);
        conn1->save(s);
    }
    {
        QDataStream s(&data2, QIODevice::WriteOnly);
        conn2->save(s);
    }

    // Create port map
    QHash<quint64, Port *> portMap;
    portMap[swOutSerial] = sw->outputPort();
    portMap[and1In0Serial] = and1->inputPort(0);
    portMap[and1OutSerial] = and1->outputPort();
    portMap[and2In0Serial] = and2->inputPort(0);

    // Load connections
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, QString()};

    auto loadedConn1 = std::make_unique<Connection>();
    {
        QDataStream s(data1);
        loadedConn1->load(s, context);
    }

    auto loadedConn2 = std::make_unique<Connection>();
    {
        QDataStream s(data2);
        loadedConn2->load(s, context);
    }

    // Verify connections are properly established
    QCOMPARE(loadedConn1->startPort(), sw->outputPort());
    QCOMPARE(loadedConn1->endPort(), and1->inputPort(0));
    QCOMPARE(loadedConn2->startPort(), and1->outputPort());
    QCOMPARE(loadedConn2->endPort(), and2->inputPort(0));

    // Verify status propagates: loadedConn1 syncs with sw output, loadedConn2 syncs with and1 output
    QCOMPARE(loadedConn1->status(), sw->outputPort()->status());
    QCOMPARE(loadedConn1->status(), Status::Active);
    QCOMPARE(loadedConn2->status(), and1->outputPort()->status());
}
