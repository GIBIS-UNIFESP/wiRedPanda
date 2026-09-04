// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestConnection.h"

#include <memory>

#include <QImage>
#include <QPainter>

#include "App/Core/Application.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

void TestConnection::testConnectionPathUpdate()
{
    // Path construction is gated on Application::renderingEnabled, not interactiveMode --
    // interactiveMode is false in both MCP modes, but the --mcp-gui window and headless
    // export_image still need visible wire paths since paint() draws path().
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
    // applyStatusPen() bypasses the item's own setPen() whenever the pen width doesn't change,
    // tracking colour via statusPen() instead -- this must still reflect the correct colour and
    // width for every status, including the Error <-> non-Error transitions that exercise the
    // real setPen() branch.
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
    // shape() is cached -- the cache must follow path geometry changes and real pen-width
    // changes.
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

void TestConnection::testPaintDrawsHighlightHalo()
{
    const bool prevRendering = Application::renderingEnabled;
    Application::renderingEnabled = true;

    Connection connection;
    connection.setStartPos({0, 0});
    connection.setEndPos({100, 0});
    connection.updatePath();

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
        connection.paint(&painter);
    }
    const QColor highlightedColor = highlighted.pixelColor(samplePoint);
    QVERIFY2(highlightedColor.blue() > highlightedColor.red(), "highlighted wire must paint a blue halo off its centreline");

    QImage plain(120, 40, QImage::Format_ARGB32);
    plain.fill(Qt::white);
    {
        QPainter painter(&plain);
        painter.translate(10, 20);
        connection.setHighLight(false);
        connection.paint(&painter);
    }
    QCOMPARE(plain.pixelColor(samplePoint), QColor(Qt::white));

    Application::renderingEnabled = prevRendering;
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
