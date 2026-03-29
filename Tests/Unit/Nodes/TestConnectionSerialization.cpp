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
#include "App/Versions.h"
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

    auto andGate = std::make_unique<And>();
    auto orGate = std::make_unique<Or>();
    scene->addItem(andGate.get());
    scene->addItem(orGate.get());

    auto *outputPort = andGate->outputPort();
    auto *inputPort = orGate->inputPort(0);

    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(dynamic_cast<QNEOutputPort *>(outputPort));
    conn1->setEndPort(dynamic_cast<QNEInputPort *>(inputPort));

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    conn1->save(saveStream);

    // Load with empty portMap — ports cannot be resolved, connection not restored
    auto conn2 = std::make_unique<QNEConnection>();
    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> emptyPortMap;
    SerializationContext context{emptyPortMap, AppVersion::current, QString()};

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

    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(dynamic_cast<QNEOutputPort *>(outputPort1));
    conn1->setEndPort(dynamic_cast<QNEInputPort *>(inputPort1));

    // Save connection
    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    conn1->save(saveStream);

    // Calculate serial IDs for fresh ports (Phase 2b serial ID formula)
    // For inputs: (elementId << 16) | portIndex
    // For outputs: (elementId << 16) | (inputCount + portIndex)
    quint64 origOutputSerial = static_cast<quint64>(andGate1->id()) << 16 | static_cast<quint64>(andGate1->inputSize());
    quint64 origInputSerial = static_cast<quint64>(orGate1->id()) << 16 | 0;
    // Assign these serial IDs to the ports before saving
    outputPort1->setSerialId(origOutputSerial);
    inputPort1->setSerialId(origInputSerial);

    // Create new elements (simulating new circuit load)
    auto andGate2 = std::make_unique<And>();
    auto orGate2 = std::make_unique<Or>();
    scene->addItem(andGate2.get());
    scene->addItem(orGate2.get());

    auto *outputPort2 = andGate2->outputPort();
    auto *inputPort2 = orGate2->inputPort(0);

    // Create port map (original -> new, using serial IDs as keys)
    QMap<quint64, QNEPort *> portMap;
    portMap[origOutputSerial] = outputPort2;
    portMap[origInputSerial] = inputPort2;

    // Create new connection and load with port map
    auto conn2 = std::make_unique<QNEConnection>();
    QDataStream loadStream(data);
    SerializationContext context{portMap, AppVersion::current, QString()};
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
    SerializationContext context{portMap, AppVersion::current, QString()};
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

    // Calculate and assign serial IDs
    quint64 outSerial = static_cast<quint64>(sw->id()) << 16 | static_cast<quint64>(sw->inputSize());
    quint64 inSerial = static_cast<quint64>(led->id()) << 16 | 0;
    outputPort->setSerialId(outSerial);
    inputPort->setSerialId(inSerial);

    // Save connection
    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    conn1->save(saveStream);

    // Create port map (using calculated serial IDs)
    QMap<quint64, QNEPort *> portMap;
    portMap[outSerial] = outputPort;
    portMap[inSerial] = inputPort;

    // Load connection
    auto conn2 = std::make_unique<QNEConnection>();
    QDataStream loadStream(data);
    SerializationContext context{portMap, AppVersion::current, QString()};
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
    conn1->setStartPort(sw1->outputPort());
    conn1->setEndPort(andGate->inputPort(0));

    auto conn2 = std::make_unique<QNEConnection>();
    conn2->setStartPort(sw2->outputPort());
    conn2->setEndPort(andGate->inputPort(1));

    // Calculate and assign serial IDs for all ports
    quint64 sw1OutSerial = static_cast<quint64>(sw1->id()) << 16 | static_cast<quint64>(sw1->inputSize());
    quint64 sw2OutSerial = static_cast<quint64>(sw2->id()) << 16 | static_cast<quint64>(sw2->inputSize());
    quint64 andIn0Serial = static_cast<quint64>(andGate->id()) << 16 | 0;
    quint64 andIn1Serial = static_cast<quint64>(andGate->id()) << 16 | 1;
    sw1->outputPort()->setSerialId(sw1OutSerial);
    sw2->outputPort()->setSerialId(sw2OutSerial);
    andGate->inputPort(0)->setSerialId(andIn0Serial);
    andGate->inputPort(1)->setSerialId(andIn1Serial);

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
    QMap<quint64, QNEPort *> portMap;
    portMap[sw1OutSerial] = sw1->outputPort();
    portMap[sw2OutSerial] = sw2->outputPort();
    portMap[andIn0Serial] = andGate->inputPort(0);
    portMap[andIn1Serial] = andGate->inputPort(1);
    SerializationContext context{portMap, AppVersion::current, QString()};

    // Load both connections
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

    // Calculate and assign serial IDs
    quint64 andOutSerial = static_cast<quint64>(andGate->id()) << 16 | static_cast<quint64>(andGate->inputSize());
    quint64 orInSerial = static_cast<quint64>(orGate->id()) << 16 | 0;
    outputPort->setSerialId(andOutSerial);
    inputPort->setSerialId(orInSerial);

    // Save
    QByteArray data;
    {
        QDataStream s(&data, QIODevice::WriteOnly);
        conn1->save(s);
    }

    // Load with port map
    QMap<quint64, QNEPort *> portMap;
    portMap[andOutSerial] = outputPort;
    portMap[orInSerial] = inputPort;
    SerializationContext context{portMap, AppVersion::current, QString()};

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
    conn1->setStartPort(sw->outputPort());
    conn1->setEndPort(led->inputPort());

    // Set initial status
    conn1->setStatus(Status::Active);
    QCOMPARE(conn1->status(), Status::Active);

    // Calculate and assign serial IDs
    quint64 swOutSerial = static_cast<quint64>(sw->id()) << 16 | static_cast<quint64>(sw->inputSize());
    quint64 ledInSerial = static_cast<quint64>(led->id()) << 16 | 0;
    sw->outputPort()->setSerialId(swOutSerial);
    led->inputPort()->setSerialId(ledInSerial);

    // Save
    QByteArray data;
    {
        QDataStream s(&data, QIODevice::WriteOnly);
        conn1->save(s);
    }

    // Load (using calculated serial IDs)
    QMap<quint64, QNEPort *> portMap;
    portMap[swOutSerial] = sw->outputPort();
    portMap[ledInSerial] = led->inputPort();
    SerializationContext context{portMap, AppVersion::current, QString()};

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
    conn1->setStartPort(sw->outputPort());
    conn1->setEndPort(and1->inputPort(0));

    auto conn2 = std::make_unique<QNEConnection>();
    conn2->setStartPort(and1->outputPort());
    conn2->setEndPort(and2->inputPort(0));

    // Turn on switch to make output Active
    sw->setOn();
    QCOMPARE(sw->outputPort()->status(), Status::Active);

    // Calculate and assign serial IDs for all ports
    quint64 swOutSerial = static_cast<quint64>(sw->id()) << 16 | static_cast<quint64>(sw->inputSize());
    quint64 and1In0Serial = static_cast<quint64>(and1->id()) << 16 | 0;
    quint64 and1OutSerial = static_cast<quint64>(and1->id()) << 16 | static_cast<quint64>(and1->inputSize());
    quint64 and2In0Serial = static_cast<quint64>(and2->id()) << 16 | 0;
    sw->outputPort()->setSerialId(swOutSerial);
    and1->inputPort(0)->setSerialId(and1In0Serial);
    and1->outputPort()->setSerialId(and1OutSerial);
    and2->inputPort(0)->setSerialId(and2In0Serial);

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
    QMap<quint64, QNEPort *> portMap;
    portMap[swOutSerial] = sw->outputPort();
    portMap[and1In0Serial] = and1->inputPort(0);
    portMap[and1OutSerial] = and1->outputPort();
    portMap[and2In0Serial] = and2->inputPort(0);
    SerializationContext context{portMap, AppVersion::current, QString()};

    // Load connections
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
    QCOMPARE(loadedConn1->startPort(), sw->outputPort());
    QCOMPARE(loadedConn1->endPort(), and1->inputPort(0));
    QCOMPARE(loadedConn2->startPort(), and1->outputPort());
    QCOMPARE(loadedConn2->endPort(), and2->inputPort(0));

    // Verify status propagates: loadedConn1 syncs with sw output, loadedConn2 syncs with and1 output
    QCOMPARE(loadedConn1->status(), sw->outputPort()->status());
    QCOMPARE(loadedConn1->status(), Status::Active);
    QCOMPARE(loadedConn2->status(), and1->outputPort()->status());
}

// ============================================================================
// Orthogonal Wire Tests
// ============================================================================

void TestConnectionSerialization::testOrthogonalWireModeDefaultIsBezier()
{
    auto conn = std::make_unique<QNEConnection>();
    QCOMPARE(conn->wireMode(), WireMode::Bezier);
    QVERIFY(conn->waypoints().isEmpty());
}

void TestConnectionSerialization::testOrthogonalUpdatePathLineSegments()
{
    auto conn = std::make_unique<QNEConnection>();
    conn->setWireMode(WireMode::Orthogonal);
    conn->setStartPos(QPointF(0, 0));
    conn->setEndPos(QPointF(64, 32));
    conn->setWaypoints({{64, 0}});
    conn->updatePath();

    const QPainterPath path = conn->path();
    QCOMPARE(path.elementCount(), 3); // moveTo + 2 lineTo
    QCOMPARE(path.elementAt(0).type, QPainterPath::MoveToElement);
    QCOMPARE(path.elementAt(1).type, QPainterPath::LineToElement);
    QCOMPARE(path.elementAt(2).type, QPainterPath::LineToElement);

    // Verify coordinates
    QCOMPARE(QPointF(path.elementAt(0).x, path.elementAt(0).y), QPointF(0, 0));
    QCOMPARE(QPointF(path.elementAt(1).x, path.elementAt(1).y), QPointF(64, 0));
    QCOMPARE(QPointF(path.elementAt(2).x, path.elementAt(2).y), QPointF(64, 32));
}

void TestConnectionSerialization::testOrthogonalSaveLoadRoundTrip()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto sw = std::make_unique<InputSwitch>();
    auto led = std::make_unique<Led>();
    scene->addItem(sw.get());
    scene->addItem(led.get());

    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(sw->outputPort());
    conn1->setEndPort(led->inputPort());
    conn1->setWireMode(WireMode::Orthogonal);
    conn1->setWaypoints({{100, 50}, {100, 80}, {200, 80}});

    // Calculate serial IDs
    quint64 swOutSerial = static_cast<quint64>(sw->id()) << 16 | static_cast<quint64>(sw->inputSize());
    quint64 ledInSerial = static_cast<quint64>(led->id()) << 16 | 0;

    // Save
    QByteArray data;
    {
        QDataStream s(&data, QIODevice::WriteOnly);
        conn1->save(s);
    }

    // Load
    QMap<quint64, QNEPort *> portMap;
    portMap[swOutSerial] = sw->outputPort();
    portMap[ledInSerial] = led->inputPort();
    SerializationContext context{portMap, AppVersion::current, QString()};

    auto conn2 = std::make_unique<QNEConnection>();
    {
        QDataStream s(data);
        conn2->load(s, context);
    }

    QCOMPARE(conn2->wireMode(), WireMode::Orthogonal);
    QCOMPARE(conn2->waypoints().size(), 3);
    QCOMPARE(conn2->waypoints().at(0), QPointF(100, 50));
    QCOMPARE(conn2->waypoints().at(1), QPointF(100, 80));
    QCOMPARE(conn2->waypoints().at(2), QPointF(200, 80));
}

void TestConnectionSerialization::testOrthogonalSaveLoadEmptyWaypoints()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto sw = std::make_unique<InputSwitch>();
    auto led = std::make_unique<Led>();
    scene->addItem(sw.get());
    scene->addItem(led.get());

    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(sw->outputPort());
    conn1->setEndPort(led->inputPort());
    conn1->setWireMode(WireMode::Orthogonal);
    // No waypoints set

    quint64 swOutSerial = static_cast<quint64>(sw->id()) << 16 | static_cast<quint64>(sw->inputSize());
    quint64 ledInSerial = static_cast<quint64>(led->id()) << 16 | 0;

    QByteArray data;
    {
        QDataStream s(&data, QIODevice::WriteOnly);
        conn1->save(s);
    }

    QMap<quint64, QNEPort *> portMap;
    portMap[swOutSerial] = sw->outputPort();
    portMap[ledInSerial] = led->inputPort();
    SerializationContext context{portMap, AppVersion::current, QString()};

    auto conn2 = std::make_unique<QNEConnection>();
    {
        QDataStream s(data);
        conn2->load(s, context);
    }

    QCOMPARE(conn2->wireMode(), WireMode::Orthogonal);
    QVERIFY(conn2->waypoints().isEmpty());
}

void TestConnectionSerialization::testBezierPathProducesCurves()
{
    auto conn = std::make_unique<QNEConnection>();
    conn->setWireMode(WireMode::Bezier);
    conn->setStartPos(QPointF(0, 0));
    conn->setEndPos(QPointF(64, 32));
    conn->updatePath();

    const QPainterPath path = conn->path();
    // Bezier path: moveTo + cubicTo (cubicTo produces 3 CurveToElement/CurveToDataElement entries)
    QCOMPARE(path.elementAt(0).type, QPainterPath::MoveToElement);
    QCOMPARE(path.elementAt(1).type, QPainterPath::CurveToElement);
}

void TestConnectionSerialization::testOldFormatLoadsAsBezier()
{
    // Simulate old format: two raw quint64 port IDs, no QMap
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto sw = std::make_unique<InputSwitch>();
    auto led = std::make_unique<Led>();
    scene->addItem(sw.get());
    scene->addItem(led.get());

    quint64 swOutSerial = static_cast<quint64>(sw->id()) << 16 | static_cast<quint64>(sw->inputSize());
    quint64 ledInSerial = static_cast<quint64>(led->id()) << 16 | 0;

    // Write old format (raw quint64 pairs)
    QByteArray data;
    {
        QDataStream s(&data, QIODevice::WriteOnly);
        s << swOutSerial;
        s << ledInSerial;
    }

    QMap<quint64, QNEPort *> portMap;
    portMap[swOutSerial] = sw->outputPort();
    portMap[ledInSerial] = led->inputPort();
    // Use pre-4.7 version to trigger old format path
    SerializationContext context{portMap, Versions::V_4_6, QString()};

    auto conn = std::make_unique<QNEConnection>();
    {
        QDataStream s(data);
        conn->load(s, context);
    }

    QCOMPARE(conn->wireMode(), WireMode::Bezier);
    QVERIFY(conn->waypoints().isEmpty());
    QCOMPARE(conn->startPort(), sw->outputPort());
    QCOMPARE(conn->endPort(), led->inputPort());
}

void TestConnectionSerialization::testMixedModesSerializeCorrectly()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto sw1 = std::make_unique<InputSwitch>();
    auto sw2 = std::make_unique<InputSwitch>();
    auto andGate = std::make_unique<And>();
    scene->addItem(sw1.get());
    scene->addItem(sw2.get());
    scene->addItem(andGate.get());

    // Bezier connection
    auto bezierConn = std::make_unique<QNEConnection>();
    bezierConn->setStartPort(sw1->outputPort());
    bezierConn->setEndPort(andGate->inputPort(0));
    bezierConn->setWireMode(WireMode::Bezier);

    // Orthogonal connection with waypoints
    auto orthoConn = std::make_unique<QNEConnection>();
    orthoConn->setStartPort(sw2->outputPort());
    orthoConn->setEndPort(andGate->inputPort(1));
    orthoConn->setWireMode(WireMode::Orthogonal);
    orthoConn->setWaypoints({{50, 100}, {50, 200}});

    quint64 sw1OutSerial = static_cast<quint64>(sw1->id()) << 16 | static_cast<quint64>(sw1->inputSize());
    quint64 sw2OutSerial = static_cast<quint64>(sw2->id()) << 16 | static_cast<quint64>(sw2->inputSize());
    quint64 andIn0Serial = static_cast<quint64>(andGate->id()) << 16 | 0;
    quint64 andIn1Serial = static_cast<quint64>(andGate->id()) << 16 | 1;

    // Save both sequentially
    QByteArray data;
    {
        QDataStream s(&data, QIODevice::WriteOnly);
        bezierConn->save(s);
        orthoConn->save(s);
    }

    // Load both
    QMap<quint64, QNEPort *> portMap;
    portMap[sw1OutSerial] = sw1->outputPort();
    portMap[sw2OutSerial] = sw2->outputPort();
    portMap[andIn0Serial] = andGate->inputPort(0);
    portMap[andIn1Serial] = andGate->inputPort(1);
    SerializationContext context{portMap, AppVersion::current, QString()};

    auto loadedBezier = std::make_unique<QNEConnection>();
    auto loadedOrtho = std::make_unique<QNEConnection>();
    {
        QDataStream s(data);
        loadedBezier->load(s, context);
        loadedOrtho->load(s, context);
    }

    QCOMPARE(loadedBezier->wireMode(), WireMode::Bezier);
    QVERIFY(loadedBezier->waypoints().isEmpty());

    QCOMPARE(loadedOrtho->wireMode(), WireMode::Orthogonal);
    QCOMPARE(loadedOrtho->waypoints().size(), 2);
    QCOMPARE(loadedOrtho->waypoints().at(0), QPointF(50, 100));
    QCOMPARE(loadedOrtho->waypoints().at(1), QPointF(50, 200));
}

void TestConnectionSerialization::testWaypointsPreservedOnPortUpdate()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto sw = std::make_unique<InputSwitch>();
    auto led = std::make_unique<Led>();
    scene->addItem(sw.get());
    scene->addItem(led.get());

    auto conn = std::make_unique<QNEConnection>();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(led->inputPort());
    conn->setWireMode(WireMode::Orthogonal);

    const QVector<QPointF> waypoints = {{100, 50}, {100, 80}, {200, 80}};
    conn->setWaypoints(waypoints);

    // Simulate element movement: updatePosFromPorts re-syncs endpoints
    sw->setPos(sw->pos() + QPointF(16, 0));
    conn->updatePosFromPorts();

    // Waypoints must remain unchanged (only endpoints adjust)
    QCOMPARE(conn->waypoints(), waypoints);
    QCOMPARE(conn->wireMode(), WireMode::Orthogonal);
}

void TestConnectionSerialization::testClearWaypoints()
{
    auto conn = std::make_unique<QNEConnection>();
    conn->setWaypoints({{10, 20}, {30, 40}});
    QCOMPARE(conn->waypoints().size(), 2);

    conn->clearWaypoints();
    QVERIFY(conn->waypoints().isEmpty());
}

void TestConnectionSerialization::testBezierModeIgnoresWaypoints()
{
    // Even if waypoints are set, Bezier mode should produce a curve, not line segments
    auto conn = std::make_unique<QNEConnection>();
    conn->setWireMode(WireMode::Bezier);
    conn->setStartPos(QPointF(0, 0));
    conn->setEndPos(QPointF(64, 32));
    conn->setWaypoints({{32, 0}, {32, 32}});
    conn->updatePath();

    const QPainterPath path = conn->path();
    QCOMPARE(path.elementAt(0).type, QPainterPath::MoveToElement);
    QCOMPARE(path.elementAt(1).type, QPainterPath::CurveToElement);
}

void TestConnectionSerialization::testInvalidWireModeDefaultsToBezier()
{
    // Simulate a corrupt wireMode value in the QMap
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto sw = std::make_unique<InputSwitch>();
    auto led = std::make_unique<Led>();
    scene->addItem(sw.get());
    scene->addItem(led.get());

    quint64 swOutSerial = static_cast<quint64>(sw->id()) << 16 | static_cast<quint64>(sw->inputSize());
    quint64 ledInSerial = static_cast<quint64>(led->id()) << 16 | 0;

    // Manually build a QMap with an invalid wireMode value
    QByteArray data;
    {
        QDataStream s(&data, QIODevice::WriteOnly);
        QMap<QString, QVariant> map;
        map.insert("startPortId", swOutSerial);
        map.insert("endPortId", ledInSerial);
        map.insert("wireMode", static_cast<quint8>(99)); // invalid
        s << map;
    }

    QMap<quint64, QNEPort *> portMap;
    portMap[swOutSerial] = sw->outputPort();
    portMap[ledInSerial] = led->inputPort();
    SerializationContext context{portMap, AppVersion::current, QString()};

    auto conn = std::make_unique<QNEConnection>();
    {
        QDataStream s(data);
        conn->load(s, context);
    }

    // Invalid value gets cast to WireMode(99), but the connection should still
    // load and resolve ports. updatePath will take the Bezier branch since
    // only WireMode::Orthogonal triggers line segments.
    QVERIFY(conn->startPort() != nullptr);
    QVERIFY(conn->endPort() != nullptr);

    conn->updatePath();
    const QPainterPath path = conn->path();
    QCOMPARE(path.elementAt(1).type, QPainterPath::CurveToElement);
}

