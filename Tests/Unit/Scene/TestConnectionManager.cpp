// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestConnectionManager.h"

#include <algorithm>

#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>

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

void TestConnectionManager::testHoverLabelConstructsForTopAndBottomSides()
{
    // Only Side::Left/Right are exercised elsewhere; Side::Top/Bottom pick different
    // branches in the constructor's switch (bounds centred horizontally, offset vertically).
    PortHoverLabel topLabel(QStringLiteral("T"), PortHoverLabel::Side::Top);
    QVERIFY2(topLabel.boundingRect().bottom() < 0, "A Side::Top chip must sit above the port origin");

    PortHoverLabel bottomLabel(QStringLiteral("B"), PortHoverLabel::Side::Bottom);
    QVERIFY2(bottomLabel.boundingRect().top() > 0, "A Side::Bottom chip must sit below the port origin");
}

void TestConnectionManager::testHoverLabelBoundingRectInflatesBoundsByOnePixel()
{
    PortHoverLabel label(QStringLiteral("Hello"), PortHoverLabel::Side::Right);
    const QRectF rect = label.boundingRect();

    QVERIFY2(!rect.isEmpty(), "boundingRect() must reflect a real, measured chip size");
    QVERIFY2(rect.width() > 0 && rect.height() > 0, "boundingRect() must have positive extent");
    // Side::Right: the chip sits to the right of the origin, offset by kPortGap (8px) before
    // the 1px inflation -- so its left edge stays positive even after being pulled in by 1px.
    QVERIFY2(rect.left() > 0, "A Side::Right chip's bounding rect must sit to the right of the port origin");
}

void TestConnectionManager::testHoverLabelPaintDrawsVisibleChip()
{
    PortHoverLabel label(QStringLiteral("X"), PortHoverLabel::Side::Right);

    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.translate(32, 32); // origin away from the pixmap edge, matching the port-anchored coordinate space
    QStyleOptionGraphicsItem option;
    label.paint(&painter, &option, nullptr);
    painter.end();

    QVERIFY2(TestUtils::pixmapHasInk(pixmap), "paint() must draw a visible chip");
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

void TestConnectionManager::testShowHoverLabelsBiasesLabelForRotatedElementPort()
{
    // sideFor() must pick Top/Bottom (not just Left/Right) once a rotated element's port sits
    // more vertically offset from its own element's centre than horizontally.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    InputSwitch sw;
    And andGate;
    builder.add(&sw, &andGate);
    builder.connect(&sw, 0, &andGate, 0);
    andGate.setRotation(90);
    andGate.inputPort(0)->setName(QStringLiteral("IN"));

    auto *scene = workspace.scene();
    auto *manager = scene->connectionManager();
    manager->showHoverLabels(andGate.inputPort(0));

    PortHoverLabel *ownLabel = nullptr;
    for (auto *item : scene->items()) {
        if (auto *label = qgraphicsitem_cast<PortHoverLabel *>(item); label && label->text() == QStringLiteral("IN")) {
            ownLabel = label;
        }
    }
    QVERIFY(ownLabel != nullptr);

    const QRectF rect = ownLabel->boundingRect();
    QVERIFY2(rect.top() > 0 || rect.bottom() < 0,
             "A rotated element's vertically-offset port must get a Top/Bottom-biased label, not Left/Right");

    manager->releaseHoverPort();
}

void TestConnectionManager::testHoverPortReturnsNullForOutOfRangeStaleIndex()
{
    // decodePort()'s final out-of-range guard: the hover state stores an element ID + port
    // index rather than a raw pointer so it survives undo/redo, but a stale index (e.g. left
    // over after the element's port count shrank) must resolve to nullptr, not a wrong port.
    WorkSpace workspace;
    auto *andGate = new And;
    workspace.scene()->addItem(andGate);
    auto *manager = workspace.scene()->connectionManager();

    manager->setHoverPort(andGate->inputPort(0));
    QVERIFY(manager->hoverPort() != nullptr);

    // Directly corrupt the stored index to something out of range for this element.
    manager->m_hoverPortNumber = andGate->inputSize() + andGate->outputSize() + 5;

    QVERIFY(manager->hoverPort() == nullptr);
}

void TestConnectionManager::testStartFromInputBeginsWireAnchoredAtEnd()
{
    WorkSpace workspace;
    auto *andGate = new And;
    workspace.scene()->addItem(andGate);
    auto *manager = workspace.scene()->connectionManager();

    manager->startFromInput(andGate->inputPort(0));

    QVERIFY(manager->hasEditedConnection());
    auto *connection = manager->editedConnection();
    QCOMPARE(connection->endPort(), andGate->inputPort(0));
    QVERIFY(connection->startPort() == nullptr);
}

void TestConnectionManager::testTryCompleteFromInputCompletesAtOutputPort()
{
    WorkSpace workspace;
    auto *sw = new InputSwitch;
    auto *andGate = new And;
    andGate->setPos(150, 0);
    workspace.scene()->addItem(sw);
    workspace.scene()->addItem(andGate);

    auto *manager = workspace.scene()->connectionManager();
    manager->startFromInput(andGate->inputPort(0));

    manager->tryComplete(sw->outputPort(0)->scenePos());

    QVERIFY(!manager->hasEditedConnection());
    QVERIFY(sw->outputPort(0)->isConnected(andGate->inputPort(0)));
}

void TestConnectionManager::testUpdateEditedEndForInputAnchoredWireMovesStart()
{
    // updatePath()'s own geometry rebuild is a no-op in tests (Application::renderingEnabled
    // is false, since nothing ever paints), so the observable contract here is behavioral: the
    // endPort-anchored branch must be taken (not the "lost both ports" cleanup path) -- the
    // wire must remain in progress, still anchored at the same input.
    WorkSpace workspace;
    auto *andGate = new And;
    workspace.scene()->addItem(andGate);
    auto *manager = workspace.scene()->connectionManager();

    manager->startFromInput(andGate->inputPort(0));
    QVERIFY(manager->hasEditedConnection());

    manager->updateEditedEnd(QPointF(500, 500));

    QVERIFY(manager->hasEditedConnection());
    QCOMPARE(manager->editedConnection()->endPort(), andGate->inputPort(0));
}

void TestConnectionManager::testTryCompleteWithNoPortUnderCursorIsNoOp()
{
    WorkSpace workspace;
    auto *andGate = new And;
    workspace.scene()->addItem(andGate);
    auto *manager = workspace.scene()->connectionManager();

    manager->startFromOutput(andGate->outputPort(0));
    QVERIFY(manager->hasEditedConnection());

    manager->tryComplete(QPointF(-9999, -9999)); // nothing there

    QVERIFY(manager->hasEditedConnection()); // still in progress, unaffected
}

void TestConnectionManager::testTryCompleteAtWrongPortTypeIsNoOp()
{
    WorkSpace workspace;
    auto *and1 = new And;
    auto *and2 = new And;
    and2->setPos(150, 0);
    workspace.scene()->addItem(and1);
    workspace.scene()->addItem(and2);

    auto *manager = workspace.scene()->connectionManager();
    manager->startFromOutput(and1->outputPort(0));

    manager->tryComplete(and2->outputPort(0)->scenePos()); // another output, not an input

    QVERIFY(manager->hasEditedConnection()); // rejected silently, still in progress
}

void TestConnectionManager::testTryCompleteRejectedConnectionDeletesWireAndShowsMessage()
{
    WorkSpace workspace;
    auto *andGate = new And; // 2 inputs, 1 output
    workspace.scene()->addItem(andGate);

    auto *manager = workspace.scene()->connectionManager();
    manager->startFromOutput(andGate->outputPort(0));

    manager->tryComplete(andGate->inputPort(0)->scenePos()); // self-loop: rejected

    QVERIFY(!manager->hasEditedConnection()); // wire deleted, not left dangling
}

void TestConnectionManager::testCancelDeletesInProgressWire()
{
    WorkSpace workspace;
    auto *andGate = new And;
    workspace.scene()->addItem(andGate);
    auto *manager = workspace.scene()->connectionManager();

    manager->startFromOutput(andGate->outputPort(0));
    QVERIFY(manager->hasEditedConnection());

    manager->cancel();

    QVERIFY(!manager->hasEditedConnection());
}

void TestConnectionManager::testDetachRewiresFromSameOutput()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    InputSwitch sw;
    And andGate;
    builder.add(&sw, &andGate);
    builder.connect(&sw, 0, &andGate, 0);

    auto *manager = workspace.scene()->connectionManager();
    QVERIFY(!andGate.inputPort(0)->connections().isEmpty());

    manager->detach(andGate.inputPort(0));

    // Original wire is gone; a new in-progress wire anchored at the same output exists.
    QVERIFY(andGate.inputPort(0)->connections().isEmpty());
    QVERIFY(manager->hasEditedConnection());
    QCOMPARE(manager->editedConnection()->startPort(), sw.outputPort(0));
}

void TestConnectionManager::testDetachWithNoWireIsNoOp()
{
    WorkSpace workspace;
    auto *andGate = new And;
    workspace.scene()->addItem(andGate);
    auto *manager = workspace.scene()->connectionManager();

    manager->detach(andGate->inputPort(0)); // no wire attached

    QVERIFY(!manager->hasEditedConnection());
}

void TestConnectionManager::testUpdateEditedEndWithNoConnectionIsNoOp()
{
    WorkSpace workspace;
    auto *manager = workspace.scene()->connectionManager();
    QVERIFY(!manager->hasEditedConnection());

    manager->updateEditedEnd(QPointF(10, 10)); // must not crash

    QVERIFY(!manager->hasEditedConnection());
}

void TestConnectionManager::testUpdateEditedEndWithBothPortsLostCleansUp()
{
    WorkSpace workspace;
    auto *andGate = new And;
    workspace.scene()->addItem(andGate);
    auto *manager = workspace.scene()->connectionManager();

    manager->startFromOutput(andGate->outputPort(0));
    auto *connection = manager->editedConnection();
    connection->setStartPort(nullptr); // simulate the source port disappearing mid-drag

    manager->updateEditedEnd(QPointF(10, 10));

    QVERIFY(!manager->hasEditedConnection());
}

void TestConnectionManager::testUpdateHoverShowsForbiddenCursorForSamePolarityPort()
{
    WorkSpace workspace;
    auto *and1 = new And;
    auto *and2 = new And;
    and2->setPos(150, 0);
    workspace.scene()->addItem(and1);
    workspace.scene()->addItem(and2);

    auto *manager = workspace.scene()->connectionManager();
    manager->startFromOutput(and1->outputPort(0));

    manager->updateHover(and2->outputPort(0)->scenePos()); // same polarity: output

    QCOMPARE(workspace.scene()->view()->viewport()->cursor().shape(), Qt::ForbiddenCursor);
}

void TestConnectionManager::testClearHoverReleasesHoveredPort()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    InputSwitch sw;
    And andGate;
    builder.add(&sw, &andGate);
    builder.connect(&sw, 0, &andGate, 0);

    auto *manager = workspace.scene()->connectionManager();
    auto *hovered = andGate.inputPort(0);
    manager->setHoverPort(hovered);
    QVERIFY(manager->hoverPort() == hovered);

    manager->clearHover();

    QVERIFY(manager->hoverPort() == nullptr);
}

void TestConnectionManager::testConnectionRejectionReasonForAlreadyConnectedPorts()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    InputSwitch sw;
    And andGate;
    builder.add(&sw, &andGate);
    builder.connect(&sw, 0, &andGate, 0);

    const QString reason = ConnectionManager::connectionRejectionReason(sw.outputPort(0), andGate.inputPort(0));

    QVERIFY(!reason.isEmpty());
    QVERIFY(!ConnectionManager::isConnectionAllowed(sw.outputPort(0), andGate.inputPort(0)));
}

void TestConnectionManager::testShowHoverLabelsWithNullPortIsNoOp()
{
    WorkSpace workspace;
    auto *manager = workspace.scene()->connectionManager();

    manager->showHoverLabels(nullptr); // must not crash; no labels spawned

    int labelCount = 0;
    for (auto *item : workspace.scene()->items()) {
        if (item->type() == PortHoverLabel::Type) {
            ++labelCount;
        }
    }
    QCOMPARE(labelCount, 0);
}

void TestConnectionManager::testConnectedPeersWithNullPortReturnsEmpty()
{
    QVERIFY(ConnectionManager::connectedPeers(nullptr).isEmpty());
}
