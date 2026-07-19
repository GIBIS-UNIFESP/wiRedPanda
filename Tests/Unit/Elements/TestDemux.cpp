// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestDemux.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestDemux::testDemuxOutputSize()
{
    WorkSpace workspace;
    auto *demux = new Demux;
    workspace.scene()->addItem(demux);
    int defaultSize = demux->outputSize();
    QVERIFY(defaultSize > 0);
}

void TestDemux::testDemuxRouting()
{
    WorkSpace workspace;
    auto *demux = new Demux;
    workspace.scene()->addItem(demux);
    QVERIFY(demux->inputSize() > 0);
}

void TestDemux::testDemuxPainting()
{
    WorkSpace workspace;
    auto *demux = new Demux;
    workspace.scene()->addItem(demux);

    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    demux->paint(&painter, &option, nullptr);
    painter.end();

    QVERIFY2(TestUtils::pixmapHasInk(pixmap), "Demux paint() must draw visible pixels");
}

void TestDemux::testDemuxPaintingSelected()
{
    WorkSpace workspace;
    auto *demux = new Demux;
    workspace.scene()->addItem(demux);
    demux->setSelected(true);

    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    demux->paint(&painter, &option, nullptr);
    painter.end();

    QVERIFY2(TestUtils::pixmapHasInk(pixmap), "Demux paint() must draw the selection highlight when selected");
}

void TestDemux::testDemuxSetInputSizeIsNoOp()
{
    // Demux derives its input count from setOutputSize() (1 data + log2(outputs) select
    // lines); setInputSize() is overridden as a deliberate no-op so a caller can't desync
    // that invariant. The override is reachable via generic GraphicElement* callers (e.g.
    // Commands.cpp's port-resize command), not just direct Demux:: calls -- exercise it
    // that way so the virtual dispatch itself is what's under test.
    Demux demux;
    const int before = demux.inputSize();

    auto *elm = static_cast<GraphicElement *>(&demux);
    elm->setInputSize(before + 5);

    QCOMPARE(demux.inputSize(), before);
}

void TestDemux::testDemuxDisconnectedInputsAreUnknown()
{
    Demux demux;
    TestUtils::initElm(demux);
    demux.updateLogic();

    for (int i = 0; i < demux.outputSize(); ++i) {
        QCOMPARE(demux.outputValue(i), Status::Unknown);
    }
}

void TestDemux::testDemuxOutOfRangeSelect()
{
    // Regression test (F3 alignment): 5 outputs need 3 select lines, so
    // select values 5..7 address no output. Routing is indeterminate — every
    // output must be Unknown (previously they were silently all-Inactive,
    // i.e. the data vanished without any indication).
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch dataIn;
    QVector<InputSwitch *> selIn(3);
    for (auto *&sw : selIn) {
        sw = new InputSwitch();
        builder.add(sw);
    }

    Demux demux;
    demux.setOutputSize(5);
    builder.add(&dataIn, &demux);

    builder.connect(&dataIn, 0, &demux, 0);
    for (int i = 0; i < 3; ++i) {
        builder.connect(selIn[i], 0, &demux, 1 + i);
    }

    auto *simulation = builder.initSimulation();

    dataIn.setOn(true);
    const int selectValue = 7;
    for (int i = 0; i < 3; ++i) {
        selIn[i]->setOn((selectValue >> i) & 1);
    }
    simulation->update();

    for (int i = 0; i < demux.outputSize(); ++i) {
        QCOMPARE(demux.outputValue(i), Status::Unknown);
    }
}

void TestDemux::testDemuxBigPivotsAtBoundingRectCenter()
{
    // Direct regression: a big Demux's rotation/flip pivot must be the element's actual
    // footprint centre, not the (0,0)-anchored raw pixmap centre that only happens to
    // coincide with it for small instances.
    WorkSpace workspace;
    auto *demux = new Demux;
    workspace.scene()->addItem(demux);
    demux->setOutputSize(8); // max output size -> tallest body

    QVERIFY2(demux->boundingRect().height() > 64,
             "Test Demux isn't actually 'big' — boundingRect() didn't grow past the 64x64 body");
    QCOMPARE(demux->transformOriginPoint(), demux->boundingRect().center());
}

void TestDemux::testDemuxBigRotationDoesNotDriftInScene()
{
    WorkSpace workspace;
    auto *demux = new Demux;
    workspace.scene()->addItem(demux);
    demux->setOutputSize(8);

    const QPointF centerScene = demux->mapToScene(demux->boundingRect().center());

    for (const qreal angle : {90.0, 180.0, 270.0, 0.0}) {
        demux->setRotation(angle);
        QCOMPARE(demux->mapToScene(demux->boundingRect().center()), centerScene);
    }
}

void TestDemux::testDemuxBigFlipDoesNotDriftInScene()
{
    WorkSpace workspace;
    auto *demux = new Demux;
    workspace.scene()->addItem(demux);
    demux->setOutputSize(8);

    const QPointF centerScene = demux->mapToScene(demux->boundingRect().center());

    demux->setFlippedX(true);
    QCOMPARE(demux->mapToScene(demux->boundingRect().center()), centerScene);

    demux->setFlippedY(true);
    QCOMPARE(demux->mapToScene(demux->boundingRect().center()), centerScene);

    demux->setFlippedX(false);
    demux->setFlippedY(false);
    QCOMPARE(demux->mapToScene(demux->boundingRect().center()), centerScene);
}
