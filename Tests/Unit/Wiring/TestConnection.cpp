// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Wiring/TestConnection.h"

#include <memory>

#include "App/Core/Application.h"
#include "App/Core/ThemeManager.h"
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

void TestConnection::testConnectionStatusPenTracksColorAndWidth()
{
    // applyStatusPen() bypasses the item's own setPen() (and the BSP-tree re-index it
    // triggers) whenever the pen width doesn't change, tracking colour via statusPen()
    // instead -- this must still reflect the correct colour and width for every status,
    // including the Error <-> non-Error transitions that exercise the real-setPen() branch.
    const auto &theme = ThemeManager::attributes();
    Connection connection;

    connection.setStatus(Status::Active);
    QCOMPARE(connection.statusPen().color(), theme.m_connectionActive);
    QCOMPARE(connection.statusPen().widthF(), 3.0);

    connection.setStatus(Status::Inactive);
    QCOMPARE(connection.statusPen().color(), theme.m_connectionInactive);
    QCOMPARE(connection.statusPen().widthF(), 3.0);

    // Inactive -> Error: width grows 3 -> 5, the real setPen() must still run.
    connection.setStatus(Status::Error);
    QCOMPARE(connection.statusPen().color(), theme.m_connectionError);
    QCOMPARE(connection.statusPen().widthF(), 5.0);

    // Error -> Unknown: width shrinks back 5 -> 3, exercising the same branch in reverse.
    connection.setStatus(Status::Unknown);
    QCOMPARE(connection.statusPen().color(), theme.m_connectionUnknown);
    QCOMPARE(connection.statusPen().widthF(), 3.0);
}
