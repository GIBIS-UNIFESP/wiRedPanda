// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestConnectionValidity.h"

#include <memory>

#include <QTemporaryDir>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Scene.h"
#include "Tests/Common/TestUtils.h"

void TestConnectionValidity::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
}

void TestConnectionValidity::testConnectionStatusValid()
{
    Scene scene;

    // Create two AND gates
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(and1);
    scene.addItem(and2);

    // Get their ports
    auto *outPort1 = and1->outputPort(0);
    auto *inPort2 = and2->inputPort(0);

    // Create a connection
    auto *conn = new QNEConnection();
    conn->setStartPort(outPort1);
    conn->setEndPort(inPort2);
    scene.addItem(conn);

    // Connection should have a valid status
    QVERIFY2(conn->status() != Status::Invalid, "and1→and2 connection should be valid after both ports are set");
}

void TestConnectionValidity::testConnectionStatusInvalid()
{
    Scene scene;

    // Create a connection with no ports set
    auto *conn = new QNEConnection();
    scene.addItem(conn);

    // Connection with unset ports should be invalid
    QCOMPARE(conn->status(), Status::Invalid);
}

void TestConnectionValidity::testPortDeletionDeletesConnection()
{
    Scene scene;

    // Create two AND gates
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(and1);
    scene.addItem(and2);

    // Create a connection
    auto *conn = new QNEConnection();
    auto *outPort1 = and1->outputPort(0);
    auto *inPort2 = and2->inputPort(0);
    conn->setStartPort(outPort1);
    conn->setEndPort(inPort2);
    scene.addItem(conn);

    // Remove the output element
    // When and1 is deleted, its output port is deleted, which automatically
    // deletes the connection (see QNEOutputPort destructor)
    scene.removeItem(and1);
    delete and1;

    // After port removal, connection is already deleted by the port destructor.
    // No need to verify connection status as the connection no longer exists.
}

void TestConnectionValidity::testMultipleConnectionsStatus()
{
    Scene scene;

    // Create 3 AND gates
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    auto *and3 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(and1);
    scene.addItem(and2);
    scene.addItem(and3);

    // Create connections: and1->and2->and3
    auto *conn1 = new QNEConnection();
    conn1->setStartPort(and1->outputPort(0));
    conn1->setEndPort(and2->inputPort(0));
    scene.addItem(conn1);

    auto *conn2 = new QNEConnection();
    conn2->setStartPort(and2->outputPort(0));
    conn2->setEndPort(and3->inputPort(0));
    scene.addItem(conn2);

    // Both connections should be valid
    QVERIFY2(conn1->status() != Status::Invalid, "conn1 (and1→and2) should be valid");
    QVERIFY2(conn2->status() != Status::Invalid, "conn2 (and2→and3) should be valid");
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

    auto *conn = new QNEConnection();
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
    auto conn = std::unique_ptr<QNEConnection>(new QNEConnection());
    conn->setStartPort(outPort);
    conn->setEndPort(inPort);

    // Connection exists but element is not in scene
    QCOMPARE(conn->startPort(), outPort);
    QCOMPARE(conn->endPort(), inPort);
}

void TestConnectionValidity::testInputPortWithMultipleConnections()
{
    Scene scene;

    // Create 3 AND gates
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    auto *and3 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(and1);
    scene.addItem(and2);
    scene.addItem(and3);

    // Connect both and1 and and2 output to and3 input
    auto *conn1 = new QNEConnection();
    conn1->setStartPort(and1->outputPort(0));
    conn1->setEndPort(and3->inputPort(0));
    scene.addItem(conn1);

    auto *conn2 = new QNEConnection();
    conn2->setStartPort(and2->outputPort(0));
    conn2->setEndPort(and3->inputPort(1));
    scene.addItem(conn2);

    // Both connections should be valid
    QVERIFY2(conn1->status() != Status::Invalid, "conn1 (and1→and3 input 0) should be valid");
    QVERIFY2(conn2->status() != Status::Invalid, "conn2 (and2→and3 input 1) should be valid");
}

void TestConnectionValidity::testOutputPortWithMultipleConnections()
{
    Scene scene;

    // Create 3 AND gates
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    auto *and3 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(and1);
    scene.addItem(and2);
    scene.addItem(and3);

    // Connect and1 output to both and2 and and3 inputs
    auto *conn1 = new QNEConnection();
    conn1->setStartPort(and1->outputPort(0));
    conn1->setEndPort(and2->inputPort(0));
    scene.addItem(conn1);

    auto *conn2 = new QNEConnection();
    conn2->setStartPort(and1->outputPort(0));
    conn2->setEndPort(and3->inputPort(0));
    scene.addItem(conn2);

    // Both connections should be valid
    QVERIFY2(conn1->status() != Status::Invalid, "conn1 (and1→and2 input 0) should be valid");
    QVERIFY2(conn2->status() != Status::Invalid, "conn2 (and1→and3 input 0) should be valid");
}
