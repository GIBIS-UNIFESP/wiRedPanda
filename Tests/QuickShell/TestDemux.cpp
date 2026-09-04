// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestDemux.h"

#include <QPainter>

#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "Tests/QuickShell/IC/QuickTestUtils.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

void TestDemux::testDemuxOutputSize()
{
    QuickCircuitBuilder builder;
    auto *demux = new Demux;
    builder.addOwnedElement(demux);
    int defaultSize = demux->outputSize();
    QVERIFY(defaultSize > 0);
}

void TestDemux::testDemuxRouting()
{
    // Default Demux: outputSize()==2 -> input 0 = data, input 1 = the single select line.
    // Drive data Active and confirm it actually reaches only the selected output, with the
    // other output tracking Inactive, for both select values.
    QuickCircuitBuilder builder;

    auto *data = new InputSwitch;
    auto *sel = new InputSwitch;
    auto *demux = new Demux;
    builder.addOwned(data, sel, demux);
    builder.connect(data, 0, demux, 0);
    builder.connect(sel, 0, demux, 1);

    auto *simulation = builder.initSimulation();

    data->setOn(true);

    sel->setOn(false); // select=0 -> output 0
    simulation->update();
    QCOMPARE(demux->outputValue(0), Status::Active);
    QCOMPARE(demux->outputValue(1), Status::Inactive);

    sel->setOn(true); // select=1 -> output 1
    simulation->update();
    QCOMPARE(demux->outputValue(0), Status::Inactive);
    QCOMPARE(demux->outputValue(1), Status::Active);
}

void TestDemux::testDemuxPainting()
{
    QuickCircuitBuilder builder;
    auto *demux = new Demux;
    builder.addOwnedElement(demux);

    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    demux->paint(&painter);
    painter.end();

    QVERIFY2(QuickTestUtils::pixmapHasInk(pixmap), "Demux paint() must draw visible pixels");
}

void TestDemux::testDemuxPaintingSelected()
{
    QuickCircuitBuilder builder;
    auto *demux = new Demux;
    builder.addOwnedElement(demux);
    demux->setSelected(true);

    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    demux->paint(&painter);
    painter.end();

    QVERIFY2(QuickTestUtils::pixmapHasInk(pixmap), "Demux paint() must draw the selection highlight when selected");
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
    QuickTestUtils::initElm(demux);
    demux.updateLogic();

    for (int i = 0; i < demux.outputSize(); ++i) {
        QCOMPARE(demux.outputValue(i), Status::Unknown);
    }
}

void TestDemux::testDemuxOutOfRangeSelect()
{
    // Regression test: 5 outputs need 3 select lines, so select values 5..7
    // address no output. Routing is indeterminate — every output must be Unknown.
    QuickCircuitBuilder builder;

    InputSwitch dataIn;
    QVector<InputSwitch *> selIn(3);
    for (auto *&sw : selIn) {
        sw = new InputSwitch();
        builder.addOwnedElement(sw);
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
    QuickCircuitBuilder builder;
    auto *demux = new Demux;
    builder.addOwnedElement(demux);
    demux->setOutputSize(8); // max output size -> tallest body

    QVERIFY2(demux->boundingRect().height() > 64,
             "Test Demux isn't actually 'big' — boundingRect() didn't grow past the 64x64 body");
    QCOMPARE(demux->pixmapCenter(), demux->boundingRect().center());
}

void TestDemux::testDemuxBigRotationDoesNotDriftInScene()
{
    QuickCircuitBuilder builder;
    auto *demux = new Demux;
    builder.addOwnedElement(demux);
    demux->setOutputSize(8);

    const QPointF centerScene = demux->pointToScene(demux->boundingRect().center());

    for (const qreal angle : {90.0, 180.0, 270.0, 0.0}) {
        demux->setRotation(angle);
        QCOMPARE(demux->pointToScene(demux->boundingRect().center()), centerScene);
    }
}

void TestDemux::testDemuxBigFlipDoesNotDriftInScene()
{
    QuickCircuitBuilder builder;
    auto *demux = new Demux;
    builder.addOwnedElement(demux);
    demux->setOutputSize(8);

    const QPointF centerScene = demux->pointToScene(demux->boundingRect().center());

    demux->setFlippedX(true);
    QCOMPARE(demux->pointToScene(demux->boundingRect().center()), centerScene);

    demux->setFlippedY(true);
    QCOMPARE(demux->pointToScene(demux->boundingRect().center()), centerScene);

    demux->setFlippedX(false);
    demux->setFlippedY(false);
    QCOMPARE(demux->pointToScene(demux->boundingRect().center()), centerScene);
}
