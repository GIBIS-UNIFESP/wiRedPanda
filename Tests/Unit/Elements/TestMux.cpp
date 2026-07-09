// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestMux.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestMux::testMuxInputSize()
{
    WorkSpace workspace;
    auto *mux = new Mux;
    workspace.scene()->addItem(mux);
    QVERIFY(mux->inputSize() > 0);
}

void TestMux::testMuxSelection()
{
    WorkSpace workspace;
    auto *mux = new Mux;
    workspace.scene()->addItem(mux);
    QVERIFY(mux->inputSize() > 0);
}

void TestMux::testMuxPainting()
{
    WorkSpace workspace;
    auto *mux = new Mux;
    workspace.scene()->addItem(mux);

    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    mux->paint(&painter, &option, nullptr);
    painter.end();

    QVERIFY2(TestUtils::pixmapHasInk(pixmap), "Mux paint() must draw visible pixels");
}

void TestMux::testMuxOutOfRangeSelect_data()
{
    QTest::addColumn<int>("totalInputs");
    QTest::addColumn<int>("numDataInputs");
    QTest::addColumn<int>("selectValue");

    // Regression data (F3): select values >= the data-input count are
    // representable whenever the data width is not a power of two.
    // total 4  -> 2 data + 2 select: select 2..3 used to read a *select line*
    //             back as data (silently wrong output).
    // total 7  -> 4 data + 3 select: select 7 used to index past the input
    //             vector entirely (out-of-bounds read).
    // total 8  -> 5 data + 3 select: the smallest UI-offered non-power-of-two
    //             width; select 5..7 has no data input behind it.
    QTest::newRow("total4_select2") << 4 << 2 << 2;
    QTest::newRow("total4_select3") << 4 << 2 << 3;
    QTest::newRow("total7_select7_oob") << 7 << 4 << 7;
    QTest::newRow("total8_select6") << 8 << 5 << 6;
}

void TestMux::testMuxOutOfRangeSelect()
{
    QFETCH(int, totalInputs);
    QFETCH(int, numDataInputs);
    QFETCH(int, selectValue);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    const int numSelectLines = totalInputs - numDataInputs;

    QVector<InputSwitch *> dataIn(numDataInputs);
    for (auto *&sw : dataIn) {
        sw = new InputSwitch();
        builder.add(sw);
    }
    QVector<InputSwitch *> selIn(numSelectLines);
    for (auto *&sw : selIn) {
        sw = new InputSwitch();
        builder.add(sw);
    }

    Mux mux;
    mux.setInputSize(totalInputs);
    builder.add(&mux);

    for (int i = 0; i < numDataInputs; ++i) {
        builder.connect(dataIn[i], 0, &mux, i);
    }
    for (int i = 0; i < numSelectLines; ++i) {
        builder.connect(selIn[i], 0, &mux, numDataInputs + i);
    }

    auto *simulation = builder.initSimulation();

    // Drive every data input high so a wrong read of a data line would be
    // indistinguishable from a wrong read of an active select line; the only
    // correct answer for a select with no data input behind it is Unknown.
    for (auto *sw : dataIn) {
        sw->setOn(true);
    }
    for (int i = 0; i < numSelectLines; ++i) {
        selIn[i]->setOn((selectValue >> i) & 1);
    }
    simulation->update();

    QCOMPARE(mux.outputValue(), Status::Unknown);
}

void TestMux::testMuxBigPivotsAtBoundingRectCenter()
{
    // Direct regression: a big Mux's rotation/flip pivot must be the element's actual
    // footprint centre, not the (0,0)-anchored raw pixmap centre that only happens to
    // coincide with it for small instances.
    WorkSpace workspace;
    auto *mux = new Mux;
    workspace.scene()->addItem(mux);
    mux->setInputSize(11); // max input size -> tallest body

    // Mux's data-port column grows downward from a fixed top, so unlike IC/TruthTable its
    // boundingRect() top-left never goes negative — height is what distinguishes "grew past
    // the base skin" from "still the base skin".
    QVERIFY2(mux->boundingRect().height() > 64,
             "Test Mux isn't actually 'big' — boundingRect() didn't grow past the 64x64 body");
    QCOMPARE(mux->transformOriginPoint(), mux->boundingRect().center());
}

void TestMux::testMuxBigRotationDoesNotDriftInScene()
{
    WorkSpace workspace;
    auto *mux = new Mux;
    workspace.scene()->addItem(mux);
    mux->setInputSize(11);

    const QPointF centerScene = mux->mapToScene(mux->boundingRect().center());

    for (const qreal angle : {90.0, 180.0, 270.0, 0.0}) {
        mux->setRotation(angle);
        QCOMPARE(mux->mapToScene(mux->boundingRect().center()), centerScene);
    }
}

void TestMux::testMuxBigFlipDoesNotDriftInScene()
{
    WorkSpace workspace;
    auto *mux = new Mux;
    workspace.scene()->addItem(mux);
    mux->setInputSize(11);

    const QPointF centerScene = mux->mapToScene(mux->boundingRect().center());

    mux->setFlippedX(true);
    QCOMPARE(mux->mapToScene(mux->boundingRect().center()), centerScene);

    mux->setFlippedY(true);
    QCOMPARE(mux->mapToScene(mux->boundingRect().center()), centerScene);

    mux->setFlippedX(false);
    mux->setFlippedY(false);
    QCOMPARE(mux->mapToScene(mux->boundingRect().center()), centerScene);
}
