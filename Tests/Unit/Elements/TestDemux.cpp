// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestDemux.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestDemux::testDemuxOutputSize()
{
    WorkSpace workspace;
    auto *demux = ElementFactory::buildElement(ElementType::Demux);
    workspace.scene()->addItem(demux);
    int defaultSize = demux->outputSize();
    QVERIFY(defaultSize > 0);
}

void TestDemux::testDemuxRouting()
{
    // Default Demux: outputSize()==2 -> input 0 = data, input 1 = the single select line.
    // Drive data Active and confirm it actually reaches only the selected output, with the
    // other output tracking Inactive, for both select values.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *data = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *sel = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *demux = ElementFactory::buildElement(ElementType::Demux);
    builder.add(data, sel, demux);
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
    WorkSpace workspace;
    auto *demux = ElementFactory::buildElement(ElementType::Demux);
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
    auto *demux = ElementFactory::buildElement(ElementType::Demux);
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
    // A Demux's inputs follow its outputs (1 data + log2(outputs) select lines,
    // ElementTypeDescriptor::inputsForOutputs), so setInputSize() refuses even a size within
    // the type's input range, which a generic caller (the port-resize command, a morph) can ask for.
    Element demux{ElementType::Demux};
    const int before = demux.inputSize();
    QVERIFY(before + 1 <= demux.maxInputSize());

    demux.setInputSize(before + 1);

    QCOMPARE(demux.inputSize(), before);
}

void TestDemux::testDemuxDisconnectedInputsAreUnknown()
{
    Element demux{ElementType::Demux};
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

    Element dataIn{ElementType::InputSwitch};
    QVector<Element *> selIn(3);
    for (auto *&sw : selIn) {
        sw = ElementFactory::buildElement(ElementType::InputSwitch);
        builder.add(sw);
    }

    Element demux{ElementType::Demux};
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
    auto *demux = ElementFactory::buildElement(ElementType::Demux);
    workspace.scene()->addItem(demux);
    demux->setOutputSize(8); // max output size -> tallest body

    QVERIFY2(demux->boundingRect().height() > 64,
             "Test Demux isn't actually 'big' — boundingRect() didn't grow past the 64x64 body");
    QCOMPARE(demux->transformOriginPoint(), demux->boundingRect().center());
}

void TestDemux::testDemuxBigRotationDoesNotDriftInScene()
{
    WorkSpace workspace;
    auto *demux = ElementFactory::buildElement(ElementType::Demux);
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
    auto *demux = ElementFactory::buildElement(ElementType::Demux);
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
