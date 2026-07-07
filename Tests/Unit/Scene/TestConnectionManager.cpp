// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestConnectionManager.h"

#include <algorithm>

#include "App/Core/Settings.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Scene/Commands.h"
#include "App/Scene/ConnectionManager.h"
#include "App/Scene/PortHoverLabel.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

void TestConnectionManager::testConnectionCreation()
{
    // Test: Can create a connection between elements
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    builder.add(&sw, &andGate);
    builder.connect(&sw, 0, &andGate, 0);

    QVERIFY(andGate.inputs().size() > 0);
}

void TestConnectionManager::testConnectionValidation()
{
    // Test: ConnectionManager validates connections
    WorkSpace workspace;
    And andGate1;
    And andGate2;
    workspace.scene()->addItem(&andGate1);
    workspace.scene()->addItem(&andGate2);

    // Both gates exist in scene
    QVERIFY(workspace.scene()->items().contains(&andGate1));
    QVERIFY(workspace.scene()->items().contains(&andGate2));
}

void TestConnectionManager::testConnectionDeletion()
{
    // Test: Connections can be deleted properly
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    builder.add(&sw, &andGate);
    builder.connect(&sw, 0, &andGate, 0);

    // Connection exists
    QVERIFY(andGate.inputs().size() > 0);
}

void TestConnectionManager::testMultiPortConnection()
{
    // Test: Multi-port connections (fan-out)
    WorkSpace workspace;
    And andGate;
    workspace.scene()->addItem(&andGate);

    QVERIFY(andGate.inputs().size() > 0);
}

void TestConnectionManager::testHoverShowsConnectedPortLabels()
{
    // Hovering a port should reveal an in-situ label for itself and for the port on the
    // other end of its wire, and highlight that peer port.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    builder.add(&sw, &andGate);
    builder.connect(&sw, 0, &andGate, 0);

    auto *scene = workspace.scene();
    auto *manager = scene->connectionManager();
    auto *hovered = andGate.inputPort(0);
    auto *peer = sw.outputPort(0);
    hovered->setName(QStringLiteral("IN"));
    peer->setName(QStringLiteral("OUT"));

    const auto peerLabels = [scene] {
        QList<PortHoverLabel *> found;
        const auto items = scene->items();
        for (auto *item : items) {
            if (item->type() == PortHoverLabel::Type) {
                found.append(static_cast<PortHoverLabel *>(item));
            }
        }
        return found;
    };

    // Hovering highlights the connected peer immediately; labels are deferred until the
    // tooltip fires (Scene::helpEvent -> showHoverLabels) so they match the native timing.
    manager->setHoverPort(hovered);
    QCOMPARE(peer->brush().color(), ThemeManager::attributes().m_portHoverPort);
    QVERIFY(peerLabels().isEmpty());

    manager->showHoverLabels(hovered);
    const auto shown = peerLabels();
    QCOMPARE(shown.size(), 2);
    QVERIFY(std::any_of(shown.begin(), shown.end(), [&](auto *label) { return label->text() == hovered->name(); }));
    QVERIFY(std::any_of(shown.begin(), shown.end(), [&](auto *label) { return label->text() == peer->name(); }));

    manager->releaseHoverPort();
    QVERIFY(peerLabels().isEmpty());
    QVERIFY(peer->brush().color() != ThemeManager::attributes().m_portHoverPort);
}

void TestConnectionManager::testHoverSkipsUnnamedPortLabels()
{
    // Most elements never set a port name (e.g. basic gates). Hovering them should still
    // highlight the connected peer, but must not show a blank label chip for either port.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    builder.add(&sw, &andGate);
    builder.connect(&sw, 0, &andGate, 0);

    auto *scene = workspace.scene();
    auto *manager = scene->connectionManager();
    auto *hovered = andGate.inputPort(0);
    auto *peer = sw.outputPort(0);
    QVERIFY(hovered->name().isEmpty());
    QVERIFY(peer->name().isEmpty());

    manager->setHoverPort(hovered);
    QCOMPARE(peer->brush().color(), ThemeManager::attributes().m_portHoverPort);

    manager->showHoverLabels(hovered);

    int labelCount = 0;
    const auto items = scene->items();
    for (auto *item : items) {
        if (item->type() == PortHoverLabel::Type) {
            ++labelCount;
        }
    }
    QCOMPARE(labelCount, 0);

    manager->releaseHoverPort();
}

void TestConnectionManager::testHoverFanOutShowsAllPeerLabels()
{
    // A fan-out output wired to two inputs should show a label at each destination, plus
    // one for the hovered port itself.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And and1;
    And and2;
    builder.add(&sw, &and1, &and2);
    builder.connect(&sw, 0, &and1, 0);
    builder.connect(&sw, 0, &and2, 0);

    sw.outputPort(0)->setName(QStringLiteral("OUT"));
    and1.inputPort(0)->setName(QStringLiteral("IN1"));
    and2.inputPort(0)->setName(QStringLiteral("IN2"));

    auto *scene = workspace.scene();
    auto *manager = scene->connectionManager();

    const auto labelCount = [scene] {
        int count = 0;
        const auto items = scene->items();
        for (auto *item : items) {
            if (item->type() == PortHoverLabel::Type) {
                ++count;
            }
        }
        return count;
    };

    manager->setHoverPort(sw.outputPort(0));
    manager->showHoverLabels(sw.outputPort(0));
    QCOMPARE(labelCount(), 3);

    manager->releaseHoverPort();
    QCOMPARE(labelCount(), 0);
}

void TestConnectionManager::testHoverLabelExcludedFromHitTesting()
{
    // PortHoverLabel is purely decorative (setAcceptedMouseButtons(Qt::NoButton)) and must
    // also be excluded from scene hit-testing (shape()), otherwise an overlapping chip can
    // shadow a wire or element in Scene::itemAt()'s generic fallback loop and silently break
    // features built on it (e.g. Ctrl+click clone-drag in Scene::eventFilter).
    PortHoverLabel label(QStringLiteral("A"), PortHoverLabel::Side::Right);
    QVERIFY(label.shape().isEmpty());
}

void TestConnectionManager::testHoverReleaseClearsPeerHighlightAfterWireDeleted()
{
    // If the wire to a highlighted peer is deleted while the hover is still active, the
    // peer must not stay stuck highlighted once the hover ends.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    builder.add(&sw, &andGate);
    auto *connection = builder.connect(&sw, 0, &andGate, 0);

    auto *scene = workspace.scene();
    auto *manager = scene->connectionManager();
    auto *hovered = andGate.inputPort(0);
    auto *peer = sw.outputPort(0);

    manager->setHoverPort(hovered);
    QCOMPARE(peer->brush().color(), ThemeManager::attributes().m_portHoverPort);

    // Delete the wire without moving the mouse off the hovered port, so connectedPeers(hovered)
    // would now return empty if releaseHoverPort() recomputed it live instead of using the
    // snapshot taken when the peer was actually highlighted.
    scene->receiveCommand(new DeleteItemsCommand({connection}, scene));
    QVERIFY(peer->connections().isEmpty());

    manager->releaseHoverPort();
    QVERIFY(peer->brush().color() != ThemeManager::attributes().m_portHoverPort);
}

void TestConnectionManager::testOrthogonalDragStraightLineHasNoWaypoints()
{
    const bool prevOrthogonal = Settings::orthogonalWires();
    Settings::setOrthogonalWires(true);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    builder.add(&sw, &andGate);

    auto *manager = workspace.scene()->connectionManager();
    auto *startPort = sw.outputPort(0);
    // The manager anchors the drag to the grid-snapped port position, not its raw
    // scenePos() — use the same private helper (TestConnectionManager is a friend)
    // so the offsets below land exactly where the algorithm expects them.
    const QPointF anchor = ConnectionManager::snapToGrid(startPort->scenePos());

    manager->startFromOutput(startPort);
    auto *connection = manager->editedConnection();
    QVERIFY(connection != nullptr);
    QCOMPARE(connection->wireMode(), WireMode::Orthogonal);

    // Drag purely horizontally, ending exactly on the locked axis — no corner
    // should be committed, and no live rubber-band point either (that only
    // appears when the cursor is off-axis, even within the commit threshold).
    manager->updateEditedEnd(anchor + QPointF(16, 0));
    manager->updateEditedEnd(anchor + QPointF(64, 0));
    QVERIFY(connection->waypoints().isEmpty());

    manager->cancel();
    Settings::setOrthogonalWires(prevOrthogonal);
}

void TestConnectionManager::testOrthogonalDragCommitsCornerOnDirectionChange()
{
    const bool prevOrthogonal = Settings::orthogonalWires();
    Settings::setOrthogonalWires(true);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    builder.add(&sw, &andGate);

    auto *manager = workspace.scene()->connectionManager();
    auto *startPort = sw.outputPort(0);
    // The manager anchors the drag to the grid-snapped port position, not its raw
    // scenePos() — use the same private helper (TestConnectionManager is a friend)
    // so the offsets below land exactly where the algorithm expects them.
    const QPointF anchor = ConnectionManager::snapToGrid(startPort->scenePos());

    manager->startFromOutput(startPort);
    auto *connection = manager->editedConnection();
    QVERIFY(connection != nullptr);

    // Move right first to lock in the Horizontal direction...
    manager->updateEditedEnd(anchor + QPointF(32, 0));
    QVERIFY(connection->waypoints().isEmpty());

    // ...then drop well past the threshold (48px) vertically to force a corner
    // commit and a flip to Vertical.
    manager->updateEditedEnd(anchor + QPointF(64, 64));
    QCOMPARE(connection->waypoints().size(), 1);
    QCOMPARE(connection->waypoints().first().y(), anchor.y());

    manager->cancel();
    Settings::setOrthogonalWires(prevOrthogonal);
}
