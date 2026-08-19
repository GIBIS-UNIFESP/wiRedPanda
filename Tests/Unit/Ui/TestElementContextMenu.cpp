// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestElementContextMenu.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/IC.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Workspace.h"
#include "App/UI/ElementContextMenu.h"
#include "App/UI/SelectionCapabilities.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

void TestElementContextMenu::testRotateRightAction()
{
    // ElementContextMenu::exec()'s "Rotate right"/"Rotate left" actions dispatch
    // RotateCommand(elements, +90.0, scene) / RotateCommand(elements, -90.0, scene)
    // respectively (App/UI/ElementContextMenu.cpp) — exec() itself can't be driven from a
    // headless unit test (it blocks on a real QMenu::exec() popup), so pin the sign
    // convention those two menu entries rely on directly through RotateCommand.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{andGate}, scene));
    const int initialRotation = static_cast<int>(andGate->rotation());

    scene->receiveCommand(new RotateCommand({andGate}, 90.0, scene)); // "Rotate right"
    QCOMPARE(static_cast<int>(andGate->rotation()), (initialRotation + 90) % 360);

    scene->undoStack()->undo();
    scene->receiveCommand(new RotateCommand({andGate}, -90.0, scene)); // "Rotate left"
    // ElementOrientation::setRotation normalizes via std::fmod, which is sign-preserving
    // (unlike a positive-wrapping modulo), so a negative angle stays negative here.
    QCOMPARE(static_cast<int>(andGate->rotation()), initialRotation - 90);
}

void TestElementContextMenu::testMorphMenuAction()
{
    WorkSpace workspace;
    And andGate;
    workspace.scene()->addItem(&andGate);

    SelectionCapabilities caps = computeCapabilities({&andGate});
    QVERIFY(caps.canMorph);
}

void TestElementContextMenu::testICSubcircuitAction()
{
    // The "Edit sub-circuit"/"Embed sub-circuit"/"Extract to file" menu entries are gated by
    // caps.isEmbedded/caps.isFileBacked (App/UI/ElementContextMenu.cpp), computed from
    // IC::isEmbedded() (App/UI/SelectionCapabilities.cpp) — exercise the actual IC states
    // those flags are derived from, not just the generic "no selection" case.
    QVERIFY(!computeCapabilities({}).hasElements);

    WorkSpace workspace;
    And andGate;
    workspace.scene()->addItem(&andGate);
    const SelectionCapabilities nonIcCaps = computeCapabilities({&andGate});
    QVERIFY(!nonIcCaps.isEmbedded);
    QVERIFY(!nonIcCaps.isFileBacked);

    IC fileBackedIc;
    workspace.scene()->addItem(&fileBackedIc);
    const SelectionCapabilities fileBackedCaps = computeCapabilities({&fileBackedIc});
    QVERIFY(!fileBackedCaps.isEmbedded);
    QVERIFY(fileBackedCaps.isFileBacked);

    IC embeddedIc;
    embeddedIc.setBlobName("test_blob");
    workspace.scene()->addItem(&embeddedIc);
    const SelectionCapabilities embeddedCaps = computeCapabilities({&embeddedIc});
    QVERIFY(embeddedCaps.isEmbedded);
    QVERIFY(!embeddedCaps.isFileBacked);
}

void TestElementContextMenu::testPairPortsByOrderPairsPortsInOrder()
{
    // ElementContextMenu::exec()'s "Connect ports (...) > By port order" action
    // (App/UI/ElementContextMenu.cpp) pairs a source's output ports against a
    // destination's free input ports, in index order -- exec() itself can't be driven
    // from a headless unit test (see testRotateRightAction's comment above), so exercise
    // the extracted helpers it dispatches to directly.
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *demux = new Demux();
    demux->setOutputSize(4);
    auto *display = new Display14();
    scene->receiveCommand(new AddItemsCommand({demux, display}, scene));

    const auto outputs = ElementContextMenu::allOutputPorts(demux);
    QCOMPARE(outputs.size(), 4);
    for (int i = 0; i < outputs.size(); ++i) {
        QCOMPARE(outputs[i], demux->outputPort(i));
    }

    const auto freeInputs = ElementContextMenu::freeInputPorts(display);
    QCOMPARE(freeInputs.size(), 15); // nothing connected yet

    const auto pairs = ElementContextMenu::pairPortsByOrder(outputs, freeInputs);
    QCOMPARE(pairs.size(), 4); // truncated to the shorter (outputs) list

    for (int i = 0; i < pairs.size(); ++i) {
        QCOMPARE(pairs[i].first, demux->outputPort(i));
        QCOMPARE(pairs[i].second, display->inputPort(i));
    }

    const auto connections = ElementContextMenu::buildConnections(pairs);
    QCOMPARE(connections.size(), 4);
    for (int i = 0; i < connections.size(); ++i) {
        auto *conn = qgraphicsitem_cast<Connection *>(connections[i]);
        QVERIFY(conn);
        QCOMPARE(conn->startPort(), demux->outputPort(i));
        QCOMPARE(conn->endPort(), display->inputPort(i));
        QVERIFY(!conn->scene()); // not yet added -- caller wraps in AddItemsCommand
    }

    qDeleteAll(connections);
}

void TestElementContextMenu::testPairPortsByOrderSkipsOccupiedInputs()
{
    // An input port already driven by an unrelated wire must not be reused by the bulk
    // connect action -- it's excluded from freeInputPorts(), so pairing lands on the next
    // actually-free input instead of silently adding a second, ineffective driver.
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *source = new Demux();
    source->setOutputSize(2);
    auto *display = new Display14();
    auto *filler = new And();
    scene->receiveCommand(new AddItemsCommand({source, display, filler}, scene));

    auto *fillerConn = new Connection();
    fillerConn->setStartPort(filler->outputPort(0));
    fillerConn->setEndPort(display->inputPort(0));
    scene->receiveCommand(new AddItemsCommand({fillerConn}, scene));

    const auto freeInputs = ElementContextMenu::freeInputPorts(display);
    QCOMPARE(freeInputs.size(), 14);
    QVERIFY(!freeInputs.contains(display->inputPort(0)));
    QCOMPARE(freeInputs.constFirst(), display->inputPort(1));

    const auto pairs = ElementContextMenu::pairPortsByOrder(ElementContextMenu::allOutputPorts(source), freeInputs);
    QCOMPARE(pairs.size(), 2);
    QCOMPARE(pairs[0].second, display->inputPort(1));
    QCOMPARE(pairs[1].second, display->inputPort(2));
}

void TestElementContextMenu::testConnectPortsByOrderUndoRemovesAllAsOneStep()
{
    // The menu action wraps every built Connection in a single AddItemsCommand, so undo
    // removes the whole bulk-connect as one step, not one wire at a time.
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *source = new Demux();
    source->setOutputSize(3);
    auto *display = new Display14();
    scene->receiveCommand(new AddItemsCommand({source, display}, scene));

    const int stackIndexBefore = scene->undoStack()->index();

    const auto pairs = ElementContextMenu::pairPortsByOrder(
        ElementContextMenu::allOutputPorts(source), ElementContextMenu::freeInputPorts(display));
    QCOMPARE(pairs.size(), 3);
    const auto connections = ElementContextMenu::buildConnections(pairs);

    scene->receiveCommand(new AddItemsCommand(connections, scene));
    QCOMPARE(scene->undoStack()->index(), stackIndexBefore + 1);
    for (int i = 0; i < 3; ++i) {
        QCOMPARE(display->inputPort(i)->connections().size(), 1);
    }

    scene->undoStack()->undo();
    QCOMPARE(scene->undoStack()->index(), stackIndexBefore);
    for (int i = 0; i < 3; ++i) {
        QVERIFY(display->inputPort(i)->connections().isEmpty());
    }
}

void TestElementContextMenu::testPairPortsByNameMatchesExactCaseInsensitiveIgnoringOrder()
{
    // "By matching name" pairs ports whose names are exactly equal (case-insensitive),
    // regardless of their index position on either side.
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *source = new Demux();
    source->setOutputSize(3);
    auto *display = new Display14();
    scene->receiveCommand(new AddItemsCommand({source, display}, scene));

    source->outputPort(0)->setName("clk");
    source->outputPort(1)->setName("DATA");
    source->outputPort(2)->setName("Unmatched");
    // Deliberately out of index order relative to the source, and mixed case.
    display->inputPort(5)->setName("Data");
    display->inputPort(2)->setName("CLK");

    const auto pairs = ElementContextMenu::pairPortsByName(
        ElementContextMenu::allOutputPorts(source), ElementContextMenu::freeInputPorts(display));

    QCOMPARE(pairs.size(), 2);
    QVERIFY(pairs.contains(ElementContextMenu::PortPair{source->outputPort(0), display->inputPort(2)}));
    QVERIFY(pairs.contains(ElementContextMenu::PortPair{source->outputPort(1), display->inputPort(5)}));
}

void TestElementContextMenu::testPairPortsByNameFirstMatchingOutputWinsOnDuplicateNames()
{
    // If two outputs share a name, the lower-index one is used -- deterministic, not
    // an arbitrary/duplicate connection.
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *source = new Demux();
    source->setOutputSize(2);
    auto *display = new Display14();
    scene->receiveCommand(new AddItemsCommand({source, display}, scene));

    source->outputPort(0)->setName("A");
    source->outputPort(1)->setName("A");
    display->inputPort(0)->setName("A");

    const auto pairs = ElementContextMenu::pairPortsByName(
        ElementContextMenu::allOutputPorts(source), ElementContextMenu::freeInputPorts(display));

    QCOMPARE(pairs.size(), 1);
    QCOMPARE(pairs.first().first, source->outputPort(0));
    QCOMPARE(pairs.first().second, display->inputPort(0));
}

void TestElementContextMenu::testPairPortsByNameNoMatchesReturnsEmpty()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *source = new Demux();
    source->setOutputSize(2);
    auto *display = new Display14();
    scene->receiveCommand(new AddItemsCommand({source, display}, scene));

    source->outputPort(0)->setName("X");
    source->outputPort(1)->setName("Y");
    // Display14's default port names are verbose labels like "A (top)" -- none equal "X"/"Y".

    const auto pairs = ElementContextMenu::pairPortsByName(
        ElementContextMenu::allOutputPorts(source), ElementContextMenu::freeInputPorts(display));

    QVERIFY(pairs.isEmpty());
}
