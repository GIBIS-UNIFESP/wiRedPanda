// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Wiring/TestConnection.h"

#include <memory>

#include <QGraphicsSceneMouseEvent>
#include <QImage>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "App/Core/Application.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Scene/Workspace.h"
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

void TestConnection::testShapeFollowsPathAndPenWidth()
{
    // shape() is cached (the QGraphicsPathItem default re-strokes the Bézier on every
    // call) -- the cache must follow path geometry changes and real pen-width changes.
    const bool prevRendering = Application::renderingEnabled;
    Application::renderingEnabled = true; // updatePath() builds geometry only when rendering

    Connection connection;
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

void TestConnection::testAngleReturnsZeroWithoutBothPorts()
{
    Connection connection;
    QCOMPARE(connection.angle(), 0.0);
}

void TestConnection::testPaintDisablesAntialiasingWhenSceneWireAaDisabled()
{
    const bool prevRendering = Application::renderingEnabled;
    Application::renderingEnabled = true;

    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *connection = new Connection();
    connection->setStartPos({0, 0});
    connection->setEndPos({100, 0});
    connection->updatePath();
    scene->addItem(connection);

    QImage image(120, 40, QImage::Format_ARGB32);
    QPainter painter(&image);
    QStyleOptionGraphicsItem option;

    QVERIFY(scene->wireAntialiasingEnabled());
    painter.setRenderHint(QPainter::Antialiasing, true);
    connection->paint(&painter, &option, nullptr);
    QVERIFY2(painter.testRenderHint(QPainter::Antialiasing), "full-quality wires must not touch the painter's hint");

    // Degrade: two consecutive slow (60ms) passes trip the debounce -- same recipe as
    // TestScene's Scene::recordWirePaintPass() coverage.
    scene->recordWirePaintPass(60'000'000);
    scene->recordWirePaintPass(60'000'000);
    QVERIFY(!scene->wireAntialiasingEnabled());

    painter.setRenderHint(QPainter::Antialiasing, true);
    connection->paint(&painter, &option, nullptr);
    QVERIFY2(!painter.testRenderHint(QPainter::Antialiasing), "degraded wires must drop the painter's antialiasing hint");

    painter.end();
    Application::renderingEnabled = prevRendering;
}

void TestConnection::testPaintDrawsHighlightHalo()
{
    const bool prevRendering = Application::renderingEnabled;
    Application::renderingEnabled = true;

    Connection connection;
    connection.setStartPos({0, 0});
    connection.setEndPos({100, 0});
    connection.updatePath();

    QStyleOptionGraphicsItem option;
    // 4px off the horizontal wire's centreline: outside the default 3-wide status pen's
    // stroke (half-width 1.5) but inside the 10-wide highlight halo (half-width 5) -- pure
    // halo blue with nothing else drawn there.
    const QPoint samplePoint(60, 24);

    QImage highlighted(120, 40, QImage::Format_ARGB32);
    highlighted.fill(Qt::white);
    {
        QPainter painter(&highlighted);
        painter.translate(10, 20);
        connection.setHighLight(true);
        connection.paint(&painter, &option, nullptr);
    }
    const QColor highlightedColor = highlighted.pixelColor(samplePoint);
    QVERIFY2(highlightedColor.blue() > highlightedColor.red(), "highlighted wire must paint a blue halo off its centreline");

    QImage plain(120, 40, QImage::Format_ARGB32);
    plain.fill(Qt::white);
    {
        QPainter painter(&plain);
        painter.translate(10, 20);
        connection.setHighLight(false);
        connection.paint(&painter, &option, nullptr);
    }
    QCOMPARE(plain.pixelColor(samplePoint), QColor(Qt::white));

    Application::renderingEnabled = prevRendering;
}

void TestConnection::testSceneEventSwallowsCtrlClick()
{
    Connection connection;

    QGraphicsSceneMouseEvent ctrlEvent(QEvent::GraphicsSceneMousePress);
    ctrlEvent.setModifiers(Qt::ControlModifier);
    QVERIFY2(connection.sceneEvent(&ctrlEvent), "Ctrl+click must be swallowed so the scene can use it for multi-selection");
}

void TestConnection::testPortAttachConnectionIgnoresNull()
{
    auto inputSwitch = std::make_unique<InputSwitch>();
    OutputPort *outPort = inputSwitch->outputPort();
    const qsizetype before = outPort->connections().size();

    outPort->attachConnection(nullptr);

    QCOMPARE(outPort->connections().size(), before);
}

void TestConnection::testPortConstGraphicElementAccessor()
{
    auto andGate = std::make_unique<And>();
    const Port *constPort = andGate->outputPort();

    QCOMPARE(constPort->graphicElement(), andGate.get());
}
