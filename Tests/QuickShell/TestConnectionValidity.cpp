// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestConnectionValidity.h"

#include <memory>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/ConnectionValidity.h"
#include "App/Wiring/Port.h"

// No Scene is needed here: every behavior under test (status propagation on attach/detach,
// port-owns-connection cascade-delete, the wireless connection guard) lives entirely in
// GraphicElement/Port/Connection/ConnectionValidity -- Scene::addItem() only additionally
// assigns an id and takes ownership, neither of which the assertions below depend on. Exercises
// the Widgets-free isConnectionAllowed()/connectionRejectionReason() in
// App/Wiring/ConnectionValidity.h.

void TestConnectionValidity::testConnectionStatusValid()
{
    // A driven source and a consumer gate
    auto vcc = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputVcc));
    auto and1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));

    auto *outPort = vcc->outputPort(0);
    auto *inPort = and1->inputPort(0);
    QCOMPARE(inPort->status(), Status::Error); // required input, unwired: starts invalid

    // Create a connection (cleaned up transitively when vcc/and1 destroy their ports)
    auto *conn = new Connection();
    conn->setStartPort(outPort);
    conn->setEndPort(inPort);

    // Attaching both ports propagates the driver's definite level onto the wire
    QCOMPARE(conn->status(), Status::Active);
    // ...and the port itself must recover from Error now that it's validly wired --
    // Port::attachConnection()'s updateConnections() call is what actually drives this.
    QCOMPARE(inPort->status(), Status::Active);
}

void TestConnectionValidity::testConnectionStatusInvalid()
{
    // Connection with unset ports should be invalid
    Connection conn;
    QCOMPARE(conn.status(), Status::Unknown);
}

void TestConnectionValidity::testPortDeletionDeletesConnection()
{
    // Create two AND gates
    auto and1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto and2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));

    auto *outPort1 = and1->outputPort(0);
    auto *inPort2 = and2->inputPort(0);
    auto *conn = new Connection();
    conn->setStartPort(outPort1);
    conn->setEndPort(inPort2);

    QCOMPARE(inPort2->connections().size(), 1);
    QCOMPARE(inPort2->status(), Status::Unknown); // driven by an unconnected And, not yet Error

    // When and1 is deleted, its output port is deleted, which automatically
    // deletes the connection (see OutputPort destructor)
    and1.reset();

    // Verify the connection was cleaned up by the port destructor
    QCOMPARE(inPort2->connections().size(), 0);
    // ...and the surviving input port itself reverts to Error now that its sole connection is
    // gone (Port::detachConnection()'s updateConnections() call is what actually drives this).
    QCOMPARE(inPort2->status(), Status::Error);
}

void TestConnectionValidity::testMultipleConnectionsStatus()
{
    // Create a driven chain: vcc -> and2 -> and3
    auto vcc = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputVcc));
    auto and2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto and3 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));

    auto *conn1 = new Connection();
    conn1->setStartPort(vcc->outputPort(0));
    conn1->setEndPort(and2->inputPort(0));

    auto *conn2 = new Connection();
    conn2->setStartPort(and2->outputPort(0));
    conn2->setEndPort(and3->inputPort(0));

    // The wire from the driven source carries its level; the wire from the
    // gate stays Unknown because nothing computes and2's output until the
    // simulation runs
    QCOMPARE(conn1->status(), Status::Active);
    QCOMPARE(conn2->status(), Status::Unknown);
}

void TestConnectionValidity::testPortValidityWithConnections()
{
    auto and1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto and2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));

    auto *outPort = and1->outputPort(0);
    auto *inPort = and2->inputPort(0);

    auto *conn = new Connection();
    conn->setStartPort(outPort);
    conn->setEndPort(inPort);

    QCOMPARE(conn->startPort(), outPort);
    QCOMPARE(conn->endPort(), inPort);
}

void TestConnectionValidity::testInputPortWithMultipleConnections()
{
    // Two driven sources feeding separate inputs of the same gate
    auto vcc = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputVcc));
    auto gnd = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputGnd));
    auto and3 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));

    auto *conn1 = new Connection();
    conn1->setStartPort(vcc->outputPort(0));
    conn1->setEndPort(and3->inputPort(0));

    auto *conn2 = new Connection();
    conn2->setStartPort(gnd->outputPort(0));
    conn2->setEndPort(and3->inputPort(1));

    // Each wire carries its own driver's level
    QCOMPARE(conn1->status(), Status::Active);
    QCOMPARE(conn2->status(), Status::Inactive);
}

void TestConnectionValidity::testOutputPortWithMultipleConnections()
{
    // One driven source fanning out to two gates
    auto vcc = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputVcc));
    auto and2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto and3 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));

    auto *conn1 = new Connection();
    conn1->setStartPort(vcc->outputPort(0));
    conn1->setEndPort(and2->inputPort(0));

    auto *conn2 = new Connection();
    conn2->setStartPort(vcc->outputPort(0));
    conn2->setEndPort(and3->inputPort(0));

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
    auto src = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputSwitch));
    auto rxElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));

    auto *node = qobject_cast<Node *>(rxElm.get());
    QVERIFY(node != nullptr);
    node->setWirelessMode(WirelessMode::Rx);

    QVERIFY(!isConnectionAllowed(src->outputPort(0), node->inputPort(0)));
}

void TestConnectionValidity::testIsConnectionAllowedRejectsTxOutputPort()
{
    // A Tx node's output port is a dead-end transmitter (tunnel convention).
    // No physical wire may originate from it.
    auto txElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto dstElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputSwitch));

    auto *txNode = qobject_cast<Node *>(txElm.get());
    QVERIFY(txNode != nullptr);
    txNode->setWirelessMode(WirelessMode::Tx);

    QVERIFY(!isConnectionAllowed(txNode->outputPort(0), dstElm->inputPort(0)));
}

void TestConnectionValidity::testIsConnectionAllowedPermitsNonePort()
{
    // Physical wires to/from a plain (None-mode) Node must still be allowed.
    auto src = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputSwitch));
    auto noneElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto dst = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));

    auto *noneNode = qobject_cast<Node *>(noneElm.get());
    QVERIFY(noneNode != nullptr);

    QVERIFY(isConnectionAllowed(src->outputPort(0), noneNode->inputPort(0)));
    QVERIFY(isConnectionAllowed(noneNode->outputPort(0), dst->inputPort(0)));
}

void TestConnectionValidity::testIsConnectionAllowedPermitsTxInputPort()
{
    // A Tx node's input port must still accept physical wires — that is how
    // the signal source is connected.  Only the output port is blocked.
    auto src = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputSwitch));
    auto txElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));

    auto *txNode = qobject_cast<Node *>(txElm.get());
    QVERIFY(txNode != nullptr);
    txNode->setWirelessMode(WirelessMode::Tx);

    QVERIFY(isConnectionAllowed(src->outputPort(0), txNode->inputPort(0)));
}

void TestConnectionValidity::testIsConnectionAllowedPermitsRxOutputPort()
{
    // An Rx node's output port must accept downstream wires — that is how the
    // wirelessly received signal is forwarded to the rest of the circuit.
    auto rxElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto dst = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Led));

    auto *rxNode = qobject_cast<Node *>(rxElm.get());
    QVERIFY(rxNode != nullptr);
    rxNode->setWirelessMode(WirelessMode::Rx);

    QVERIFY(isConnectionAllowed(rxNode->outputPort(0), dst->inputPort(0)));
}

void TestConnectionValidity::testConnectionRejectionReasonMessages()
{
    // Every rejected connection now carries a user-facing explanation (shown in the status
    // bar instead of the wire silently vanishing); an allowed one carries none.
    auto src = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputSwitch));
    auto andElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto rxElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto txElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));

    auto *rx = qobject_cast<Node *>(rxElm.get());
    QVERIFY(rx != nullptr);
    rx->setWirelessMode(WirelessMode::Rx);
    auto *tx = qobject_cast<Node *>(txElm.get());
    QVERIFY(tx != nullptr);
    tx->setWirelessMode(WirelessMode::Tx);

    // Allowed: no message.
    QVERIFY(connectionRejectionReason(src->outputPort(0), andElm->inputPort(0)).isEmpty());

    // Self-loop (same element) is rejected with a message.
    QVERIFY(!connectionRejectionReason(andElm->outputPort(0), andElm->inputPort(0)).isEmpty());

    // The wireless cases spell out why no wire is needed.
    const QString rxReason = connectionRejectionReason(src->outputPort(0), rx->inputPort(0));
    QVERIFY2(rxReason.contains("wirelessly"), qPrintable(rxReason));
    const QString txReason = connectionRejectionReason(tx->outputPort(0), andElm->inputPort(0));
    QVERIFY2(txReason.contains("wirelessly"), qPrintable(txReason));

    // The reason and the bool verdict stay in agreement.
    QVERIFY(!isConnectionAllowed(andElm->outputPort(0), andElm->inputPort(0)));
    QVERIFY(isConnectionAllowed(src->outputPort(0), andElm->inputPort(0)));
}
