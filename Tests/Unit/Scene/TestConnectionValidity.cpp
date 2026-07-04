// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestConnectionValidity.h"

#include <memory>

#include <QTemporaryDir>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Scene/Scene.h"
#include "App/Wiring/Connection.h"
#include "Tests/Common/TestUtils.h"

void TestConnectionValidity::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
}

void TestConnectionValidity::testConnectionStatusValid()
{
    Scene scene;

    // A driven source and a consumer gate
    auto *vcc = ElementFactory::buildElement(ElementType::InputVcc);
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(vcc);
    scene.addItem(and1);

    // Get their ports
    auto *outPort = vcc->outputPort(0);
    auto *inPort = and1->inputPort(0);

    // Create a connection
    auto *conn = new Connection();
    conn->setStartPort(outPort);
    conn->setEndPort(inPort);
    scene.addItem(conn);

    // Attaching both ports propagates the driver's definite level onto the wire
    QCOMPARE(conn->status(), Status::Active);
}

void TestConnectionValidity::testConnectionStatusInvalid()
{
    Scene scene;

    // Create a connection with no ports set
    auto *conn = new Connection();
    scene.addItem(conn);

    // Connection with unset ports should be invalid
    QCOMPARE(conn->status(), Status::Unknown);
}

void TestConnectionValidity::testPortDeletionDeletesConnection()
{
    Scene scene;

    // Create two AND gates
    auto and1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(and1.get());
    scene.addItem(and2);

    // Create a connection
    auto *conn = new Connection();
    auto *outPort1 = and1->outputPort(0);
    auto *inPort2 = and2->inputPort(0);
    conn->setStartPort(outPort1);
    conn->setEndPort(inPort2);
    scene.addItem(conn);

    QCOMPARE(TestUtils::countConnections(&scene), 1);

    // When and1 is deleted, its output port is deleted, which automatically
    // deletes the connection (see OutputPort destructor)
    scene.removeItem(and1.get());
    and1.reset();

    // Verify the connection was cleaned up by the port destructor
    QCOMPARE(TestUtils::countConnections(&scene), 0);
}

void TestConnectionValidity::testMultipleConnectionsStatus()
{
    Scene scene;

    // Create a driven chain: vcc -> and2 -> and3
    auto *vcc = ElementFactory::buildElement(ElementType::InputVcc);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    auto *and3 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(vcc);
    scene.addItem(and2);
    scene.addItem(and3);

    // Create connections: vcc->and2->and3
    auto *conn1 = new Connection();
    conn1->setStartPort(vcc->outputPort(0));
    conn1->setEndPort(and2->inputPort(0));
    scene.addItem(conn1);

    auto *conn2 = new Connection();
    conn2->setStartPort(and2->outputPort(0));
    conn2->setEndPort(and3->inputPort(0));
    scene.addItem(conn2);

    // The wire from the driven source carries its level; the wire from the
    // gate stays Unknown because nothing computes and2's output until the
    // simulation runs
    QCOMPARE(conn1->status(), Status::Active);
    QCOMPARE(conn2->status(), Status::Unknown);
}

void TestConnectionValidity::testPortValidityWithConnections()
{
    Scene scene;

    // Create two AND gates
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(and1);
    scene.addItem(and2);

    // Connect them
    auto *outPort = and1->outputPort(0);
    auto *inPort = and2->inputPort(0);

    auto *conn = new Connection();
    conn->setStartPort(outPort);
    conn->setEndPort(inPort);
    scene.addItem(conn);

    // Verify ports match the connection
    QCOMPARE(conn->startPort(), outPort);
    QCOMPARE(conn->endPort(), inPort);
}

void TestConnectionValidity::testConnectionWithDisconnectedPorts()
{
    Scene scene;

    // Create elements but don't add to scene
    auto and1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto and2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto *outPort = and1->outputPort(0);
    auto *inPort = and2->inputPort(0);

    // Create connection with ports not in scene
    auto conn = std::unique_ptr<Connection>(new Connection());
    conn->setStartPort(outPort);
    conn->setEndPort(inPort);

    // Connection exists but element is not in scene
    QCOMPARE(conn->startPort(), outPort);
    QCOMPARE(conn->endPort(), inPort);
}

void TestConnectionValidity::testInputPortWithMultipleConnections()
{
    Scene scene;

    // Two driven sources feeding separate inputs of the same gate
    auto *vcc = ElementFactory::buildElement(ElementType::InputVcc);
    auto *gnd = ElementFactory::buildElement(ElementType::InputGnd);
    auto *and3 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(vcc);
    scene.addItem(gnd);
    scene.addItem(and3);

    // Connect vcc and gnd outputs to separate and3 inputs
    auto *conn1 = new Connection();
    conn1->setStartPort(vcc->outputPort(0));
    conn1->setEndPort(and3->inputPort(0));
    scene.addItem(conn1);

    auto *conn2 = new Connection();
    conn2->setStartPort(gnd->outputPort(0));
    conn2->setEndPort(and3->inputPort(1));
    scene.addItem(conn2);

    // Each wire carries its own driver's level
    QCOMPARE(conn1->status(), Status::Active);
    QCOMPARE(conn2->status(), Status::Inactive);
}

void TestConnectionValidity::testOutputPortWithMultipleConnections()
{
    Scene scene;

    // One driven source fanning out to two gates
    auto *vcc = ElementFactory::buildElement(ElementType::InputVcc);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    auto *and3 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(vcc);
    scene.addItem(and2);
    scene.addItem(and3);

    // Connect vcc output to both and2 and and3 inputs
    auto *conn1 = new Connection();
    conn1->setStartPort(vcc->outputPort(0));
    conn1->setEndPort(and2->inputPort(0));
    scene.addItem(conn1);

    auto *conn2 = new Connection();
    conn2->setStartPort(vcc->outputPort(0));
    conn2->setEndPort(and3->inputPort(0));
    scene.addItem(conn2);

    // Fan-out: every wire leaving the output carries the same driven level
    QCOMPARE(conn1->status(), Status::Active);
    QCOMPARE(conn2->status(), Status::Active);
}

void TestConnectionValidity::testPortValidityStyleAtCreation()
{
    // Required-but-unconnected inputs must show Error from birth — before any
    // scene interaction runs updateConnections() — so missing connections are
    // visible in freshly created and freshly loaded circuits (both position
    // elements before they enter the scene).
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QCOMPARE(andGate->inputPort(0)->status(), Status::Error);
    QCOMPARE(andGate->inputPort(1)->status(), Status::Error);

    // Ports marked optional after construction recover from the Error override:
    // no configured default reads as undriven, an explicit default as its level.
    auto sr = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::SRFlipFlop));
    QCOMPARE(sr->inputPort(0)->status(), Status::Unknown); // S: optional, no default
    QCOMPARE(sr->inputPort(1)->status(), Status::Error);   // Clock: required
    QCOMPARE(sr->inputPort(3)->status(), Status::Active);  // ~Preset: optional, defaults HIGH
}

void TestConnectionValidity::testIsConnectionAllowedRejectsRxPort()
{
    // A physical wire to an Rx wireless node's input must be rejected because the
    // simulation would silently ignore it in favour of the wireless signal.
    Scene scene;

    auto *src  = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *rxElm = ElementFactory::buildElement(ElementType::Node);
    scene.addItem(src);
    scene.addItem(rxElm);

    auto *node = qobject_cast<Node *>(rxElm);
    QVERIFY(node != nullptr);
    node->setWirelessMode(WirelessMode::Rx);

    QVERIFY(!Scene::isConnectionAllowed(src->outputPort(0), node->inputPort(0)));
}

void TestConnectionValidity::testIsConnectionAllowedRejectsTxOutputPort()
{
    // A Tx node's output port is a dead-end transmitter (tunnel convention).
    // No physical wire may originate from it.
    Scene scene;

    auto *txElm  = ElementFactory::buildElement(ElementType::Node);
    auto *dstElm = ElementFactory::buildElement(ElementType::InputSwitch);
    scene.addItem(txElm);
    scene.addItem(dstElm);

    auto *txNode = qobject_cast<Node *>(txElm);
    QVERIFY(txNode != nullptr);
    txNode->setWirelessMode(WirelessMode::Tx);

    QVERIFY(!Scene::isConnectionAllowed(txNode->outputPort(0), dstElm->inputPort(0)));
}

void TestConnectionValidity::testIsConnectionAllowedPermitsNonePort()
{
    // Physical wires to/from a plain (None-mode) Node must still be allowed.
    Scene scene;

    auto *src     = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *noneElm = ElementFactory::buildElement(ElementType::Node);
    auto *dst     = ElementFactory::buildElement(ElementType::And);
    scene.addItem(src);
    scene.addItem(noneElm);
    scene.addItem(dst);

    auto *noneNode = qobject_cast<Node *>(noneElm);
    QVERIFY(noneNode != nullptr);

    QVERIFY(Scene::isConnectionAllowed(src->outputPort(0), noneNode->inputPort(0)));
    QVERIFY(Scene::isConnectionAllowed(noneNode->outputPort(0), dst->inputPort(0)));
}

void TestConnectionValidity::testIsConnectionAllowedPermitsTxInputPort()
{
    // A Tx node's input port must still accept physical wires — that is how
    // the signal source is connected.  Only the output port is blocked.
    Scene scene;

    auto *src   = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *txElm = ElementFactory::buildElement(ElementType::Node);
    scene.addItem(src);
    scene.addItem(txElm);

    auto *txNode = qobject_cast<Node *>(txElm);
    QVERIFY(txNode != nullptr);
    txNode->setWirelessMode(WirelessMode::Tx);

    QVERIFY(Scene::isConnectionAllowed(src->outputPort(0), txNode->inputPort(0)));
}

void TestConnectionValidity::testIsConnectionAllowedPermitsRxOutputPort()
{
    // An Rx node's output port must accept downstream wires — that is how the
    // wirelessly received signal is forwarded to the rest of the circuit.
    Scene scene;

    auto *rxElm = ElementFactory::buildElement(ElementType::Node);
    auto *dst   = ElementFactory::buildElement(ElementType::Led);
    scene.addItem(rxElm);
    scene.addItem(dst);

    auto *rxNode = qobject_cast<Node *>(rxElm);
    QVERIFY(rxNode != nullptr);
    rxNode->setWirelessMode(WirelessMode::Rx);

    QVERIFY(Scene::isConnectionAllowed(rxNode->outputPort(0), dst->inputPort(0)));
}
