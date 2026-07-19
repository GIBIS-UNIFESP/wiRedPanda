// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Wiring/TestConnection.h"

#include <memory>

#include <QGraphicsScene>

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
    //
    // Added to a real QGraphicsScene: applyStatusPen() now skips this entirely for a
    // sceneless connection -- real, always-wasted cost otherwise for CanvasItem's
    // Quick-rendered wires, which are never scene-attached and never painted via
    // QGraphicsView (see project memory project_quick_hotspot_fixes_2_landed.md). A scene
    // here matches the precondition production always satisfies before this runs.
    const auto &theme = ThemeManager::attributes();
    QGraphicsScene scene;
    Connection connection;
    scene.addItem(&connection);

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

void TestConnection::testShapeFollowsPathAndPenWidth()
{
    // shape() is cached (the QGraphicsPathItem default re-strokes the Bézier on every
    // call) -- the cache must follow path geometry changes and real pen-width changes.
    // Needs a real QGraphicsScene: the Error-pen-width assertion below depends on
    // applyStatusPen()'s real setPen() call, which now short-circuits for a sceneless
    // connection (see testConnectionStatusPenTracksColorAndWidth()'s doc comment).
    const bool prevRendering = Application::renderingEnabled;
    Application::renderingEnabled = true; // updatePath() builds geometry only when rendering

    QGraphicsScene scene;
    Connection connection;
    scene.addItem(&connection);
    connection.setStartPos({0, 0});
    connection.setEndPos({100, 0});
    connection.updatePath();

    // A point on the wire's midline is inside the stroke; a far-away point is not.
    QVERIFY(connection.shape().contains(QPointF(50, 0)));
    QVERIFY(!connection.shape().contains(QPointF(50, 30)));

    // Moving an endpoint must invalidate the cache: the old midline no longer hits.
    connection.setEndPos({100, 100});
    connection.updatePath();
    QVERIFY(!connection.shape().contains(QPointF(50, 0)));

    // The default 3-wide stroke (~1.5 half-width) misses a point 2 px off-axis at the
    // horizontal wire's start; the Error pen (width 5, half-width 2.5) must reach it.
    connection.setEndPos({100, 0});
    connection.updatePath();
    QVERIFY(!connection.shape().contains(QPointF(2, 2)));
    connection.setStatus(Status::Error);
    QVERIFY(connection.shape().contains(QPointF(2, 2)));

    Application::renderingEnabled = prevRendering;
}

void TestConnection::testConnectionSkipsStatusPenWhenSceneless()
{
    // The optimization the two tests above's scene setup exists to accommodate: a
    // connection that is NOT in any QGraphicsScene (CanvasItem's real, permanent case --
    // its wires are never scene-attached; wire colour is read directly from
    // startPort()->status() instead) must skip applyStatusPen() entirely on a status
    // change, not just happen to still work. statusPen() must stay at its default
    // (never computed), proving the early-return actually ran.
    Connection connection;
    QVERIFY(!connection.scene());
    const QPen before = connection.statusPen();

    connection.setStatus(Status::Active);
    connection.setStatus(Status::Error);
    connection.setStatus(Status::Unknown);

    QCOMPARE(connection.statusPen(), before);
}
