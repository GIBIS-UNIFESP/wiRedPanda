// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Nodes/TestConnectionSerialization.h"

#include <memory>

#include <QDataStream>
#include <QTest>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/IO/SerializationContext.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Workspace.h"
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
    // Test loading connection with direct pointer restoration (empty port map)
    // This occurs when loading from older file formats or direct memory restoration
    // With empty portMap, load does raw reinterpret_cast<QNEPort *>(savedPointer)

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
    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(dynamic_cast<QNEOutputPort *>(outputPort));
    conn1->setEndPort(dynamic_cast<QNEInputPort *>(inputPort));

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    conn1->save(saveStream);

    // Create new connection and load with empty port map
    auto conn2 = std::make_unique<QNEConnection>();
    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> emptyPortMap;
    SerializationContext context{emptyPortMap, QVersionNumber(), QString()};

    conn2->load(loadStream, context);

    // With empty portMap, saved pointers are directly restored (reinterpret_cast)
    // Since gate objects are still in scope, raw pointers are still valid
    QCOMPARE(conn2->startPort(), dynamic_cast<QNEOutputPort *>(outputPort));
    QCOMPARE(conn2->endPort(), dynamic_cast<QNEInputPort *>(inputPort));
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

    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(dynamic_cast<QNEOutputPort *>(outputPort1));
    conn1->setEndPort(dynamic_cast<QNEInputPort *>(inputPort1));

    // Save connection
    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    conn1->save(saveStream);

    // Get original port pointers for mapping
    quint64 origOutputPtr = reinterpret_cast<quint64>(outputPort1);
    quint64 origInputPtr = reinterpret_cast<quint64>(inputPort1);

    // Create new elements (simulating new circuit load)
    auto andGate2 = std::make_unique<And>();
    auto orGate2 = std::make_unique<Or>();
    scene->addItem(andGate2.get());
    scene->addItem(orGate2.get());

    auto *outputPort2 = andGate2->outputPort();
    auto *inputPort2 = orGate2->inputPort(0);

    // Create port map (original -> new)
    QMap<quint64, QNEPort *> portMap;
    portMap[origOutputPtr] = outputPort2;
    portMap[origInputPtr] = inputPort2;

    // Create new connection and load with port map
    auto conn2 = std::make_unique<QNEConnection>();
    QDataStream loadStream(data);
    SerializationContext context{portMap, QVersionNumber(), QString()};
    conn2->load(loadStream, context);

    // Verify ports were correctly mapped
    QCOMPARE(conn2->startPort(), dynamic_cast<QNEOutputPort *>(outputPort2));
    QCOMPARE(conn2->endPort(), dynamic_cast<QNEInputPort *>(inputPort2));
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
    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(dynamic_cast<QNEOutputPort *>(outputPort));

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    conn1->save(saveStream);

    // Create port map with missing port (endPort was null, saves as 0)
    QMap<quint64, QNEPort *> portMap;
    portMap[reinterpret_cast<quint64>(outputPort)] = outputPort;
    // Missing the second port on purpose (0 is not in portMap)

    // Load with incomplete port map
    auto conn2 = std::make_unique<QNEConnection>();
    QDataStream loadStream(data);
    SerializationContext context{portMap, QVersionNumber(), QString()};
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
    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(dynamic_cast<QNEOutputPort *>(outputPort));
    conn1->setEndPort(dynamic_cast<QNEInputPort *>(inputPort));

    // Verify initial setup
    QVERIFY(conn1->startPort() == outputPort);
    QVERIFY(conn1->endPort() == inputPort);

    // Save connection
    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    conn1->save(saveStream);

    // Create port map (keep same pointers for this test)
    QMap<quint64, QNEPort *> portMap;
    portMap[reinterpret_cast<quint64>(outputPort)] = outputPort;
    portMap[reinterpret_cast<quint64>(inputPort)] = inputPort;

    // Load connection
    auto conn2 = std::make_unique<QNEConnection>();
    QDataStream loadStream(data);
    SerializationContext context{portMap, QVersionNumber(), QString()};
    conn2->load(loadStream, context);

    // Verify ports are correctly identified despite being loaded from stream
    QCOMPARE(conn2->startPort(), dynamic_cast<QNEOutputPort *>(outputPort));
    QCOMPARE(conn2->endPort(), dynamic_cast<QNEInputPort *>(inputPort));
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
    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(dynamic_cast<QNEOutputPort *>(sw1->outputPort()));
    conn1->setEndPort(dynamic_cast<QNEInputPort *>(andGate->inputPort(0)));

    auto conn2 = std::make_unique<QNEConnection>();
    conn2->setStartPort(dynamic_cast<QNEOutputPort *>(sw2->outputPort()));
    conn2->setEndPort(dynamic_cast<QNEInputPort *>(andGate->inputPort(1)));

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
    QMap<quint64, QNEPort *> portMap;
    portMap[reinterpret_cast<quint64>(sw1->outputPort())] = sw1->outputPort();
    portMap[reinterpret_cast<quint64>(sw2->outputPort())] = sw2->outputPort();
    portMap[reinterpret_cast<quint64>(andGate->inputPort(0))] = andGate->inputPort(0);
    portMap[reinterpret_cast<quint64>(andGate->inputPort(1))] = andGate->inputPort(1);

    // Load both connections
    SerializationContext context{portMap, QVersionNumber(), QString()};

    auto loadedConn1 = std::make_unique<QNEConnection>();
    {
        QDataStream s(data1);
        loadedConn1->load(s, context);
    }

    auto loadedConn2 = std::make_unique<QNEConnection>();
    {
        QDataStream s(data2);
        loadedConn2->load(s, context);
    }

    // Verify connections are on correct ports
    QCOMPARE(loadedConn1->endPort(), dynamic_cast<QNEInputPort *>(andGate->inputPort(0)));
    QCOMPARE(loadedConn2->endPort(), dynamic_cast<QNEInputPort *>(andGate->inputPort(1)));
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
    auto conn1 = std::make_unique<QNEConnection>();
    auto *outputPort = andGate->outputPort();
    auto *inputPort = orGate->inputPort(0);

    conn1->setStartPort(dynamic_cast<QNEOutputPort *>(outputPort));
    conn1->setEndPort(dynamic_cast<QNEInputPort *>(inputPort));

    // Verify initial state
    QCOMPARE(conn1->startPort(), outputPort);
    QCOMPARE(conn1->endPort(), inputPort);
    QVERIFY(outputPort->connections().contains(conn1.get()));
    QVERIFY(inputPort->connections().contains(conn1.get()));

    // Save
    QByteArray data;
    {
        QDataStream s(&data, QIODevice::WriteOnly);
        conn1->save(s);
    }

    // Load with port map
    QMap<quint64, QNEPort *> portMap;
    portMap[reinterpret_cast<quint64>(outputPort)] = outputPort;
    portMap[reinterpret_cast<quint64>(inputPort)] = inputPort;

    SerializationContext context{portMap, QVersionNumber(), QString()};

    auto conn2 = std::make_unique<QNEConnection>();
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
    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(dynamic_cast<QNEOutputPort *>(sw->outputPort()));
    conn1->setEndPort(dynamic_cast<QNEInputPort *>(led->inputPort()));

    // Set initial status
    conn1->setStatus(Status::Active);
    QCOMPARE(conn1->status(), Status::Active);

    // Save
    QByteArray data;
    {
        QDataStream s(&data, QIODevice::WriteOnly);
        conn1->save(s);
    }

    // Load
    QMap<quint64, QNEPort *> portMap;
    portMap[reinterpret_cast<quint64>(sw->outputPort())] = sw->outputPort();
    portMap[reinterpret_cast<quint64>(led->inputPort())] = led->inputPort();

    SerializationContext context{portMap, QVersionNumber(), QString()};

    auto conn2 = std::make_unique<QNEConnection>();
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
    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(dynamic_cast<QNEOutputPort *>(sw->outputPort()));
    conn1->setEndPort(dynamic_cast<QNEInputPort *>(and1->inputPort(0)));

    auto conn2 = std::make_unique<QNEConnection>();
    conn2->setStartPort(dynamic_cast<QNEOutputPort *>(and1->outputPort()));
    conn2->setEndPort(dynamic_cast<QNEInputPort *>(and2->inputPort(0)));

    // Turn on switch to make output Active
    sw->setOn();
    QCOMPARE(sw->outputPort()->status(), Status::Active);

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
    QMap<quint64, QNEPort *> portMap;
    portMap[reinterpret_cast<quint64>(sw->outputPort())] = sw->outputPort();
    portMap[reinterpret_cast<quint64>(and1->inputPort(0))] = and1->inputPort(0);
    portMap[reinterpret_cast<quint64>(and1->outputPort())] = and1->outputPort();
    portMap[reinterpret_cast<quint64>(and2->inputPort(0))] = and2->inputPort(0);

    // Load connections
    SerializationContext context{portMap, QVersionNumber(), QString()};

    auto loadedConn1 = std::make_unique<QNEConnection>();
    {
        QDataStream s(data1);
        loadedConn1->load(s, context);
    }

    auto loadedConn2 = std::make_unique<QNEConnection>();
    {
        QDataStream s(data2);
        loadedConn2->load(s, context);
    }

    // Verify connections are properly established
    QCOMPARE(loadedConn1->startPort(), dynamic_cast<QNEOutputPort *>(sw->outputPort()));
    QCOMPARE(loadedConn1->endPort(), dynamic_cast<QNEInputPort *>(and1->inputPort(0)));
    QCOMPARE(loadedConn2->startPort(), dynamic_cast<QNEOutputPort *>(and1->outputPort()));
    QCOMPARE(loadedConn2->endPort(), dynamic_cast<QNEInputPort *>(and2->inputPort(0)));

    // Verify status propagates: loadedConn1 syncs with sw output, loadedConn2 syncs with and1 output
    QCOMPARE(loadedConn1->status(), sw->outputPort()->status());
    QCOMPARE(loadedConn1->status(), Status::Active);
    QCOMPARE(loadedConn2->status(), and1->outputPort()->status());
}
