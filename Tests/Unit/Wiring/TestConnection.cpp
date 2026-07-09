// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Wiring/TestConnection.h"

#include <memory>

#include "App/Core/Application.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

void TestConnection::testConnectionPathUpdate()
{
    // Regression test (F5): path construction was gated on
    // Application::interactiveMode — false in both MCP modes — so the
    // --mcp-gui window and headless export_image painted wire-less circuits
    // (paint() draws path()). Geometry is gated on its own rendering flag
    // now; non-interactive sessions still build wire paths.
    const bool prevRendering = Application::renderingEnabled;
    const bool prevInteractive = Application::interactiveMode;

    Application::renderingEnabled = true;
    Application::interactiveMode = false; // both MCP modes run non-interactive

    Connection connection;
    connection.setStartPos({0, 0});
    connection.setEndPos({100, 100});
    connection.updatePath();
    QVERIFY(!connection.path().isEmpty());

    // Tests/fuzzers opt out of the geometry work entirely.
    Application::renderingEnabled = false;
    Connection skipped;
    skipped.setStartPos({0, 0});
    skipped.setEndPos({100, 100});
    skipped.updatePath();
    QVERIFY(skipped.path().isEmpty());

    Application::renderingEnabled = prevRendering;
    Application::interactiveMode = prevInteractive;
}

void TestConnection::testConnectionHoverEffect()
{
    Connection connection;
    connection.setHighLight(true);
    QVERIFY(connection.highLight());
    connection.setHighLight(false);
    QVERIFY(!connection.highLight());
}

void TestConnection::testConnectionSelection()
{
    Connection connection;
    connection.setStatus(Status::Active);
    QCOMPARE(connection.status(), Status::Active);
    connection.setStatus(Status::Inactive);
    QCOMPARE(connection.status(), Status::Inactive);
}

void TestConnection::testConnectionDestruction()
{
    // Destroying a connection must detach it from both of its ports, leaving no dangling
    // pointer behind in the ports' connection lists.
    auto inputSwitch = std::make_unique<InputSwitch>();
    auto andGate = std::make_unique<And>();

    OutputPort *outPort = inputSwitch->outputPort(0);
    InputPort *inPort = andGate->inputPort(0);
    QVERIFY(outPort && inPort);

    {
        Connection connection;
        connection.setStartPort(outPort);
        connection.setEndPort(inPort);
        QVERIFY(outPort->connections().contains(&connection));
        QVERIFY(inPort->connections().contains(&connection));
    } // connection destroyed here

    QVERIFY(outPort->connections().isEmpty());
    QVERIFY(inPort->connections().isEmpty());
}
