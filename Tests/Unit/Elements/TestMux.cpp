// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestMux.h"

#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestMux::testMuxInputSize()
{
    // F28: was three near-identical constructor checks; this one asserts the
    // structural contract and the others assert behavior
    WorkSpace workspace;
    auto *mux = new Mux;
    workspace.scene()->addItem(mux);

    QCOMPARE(mux->elementType(), ElementType::Mux);
    QCOMPARE(mux->inputSize(), 3); // In0 + In1 + S0
    QCOMPARE(mux->outputSize(), 1);
    QCOMPARE(mux->minInputSize(), 3);
    QCOMPARE(mux->maxInputSize(), 11);
}

void TestMux::testMuxSelection()
{
    // In-range selection routes the chosen data input to the output
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *in0 = new InputSwitch();
    auto *in1 = new InputSwitch();
    auto *sel = new InputSwitch();
    auto *mux = new Mux();
    auto *led = new Led();
    builder.add(in0, in1, sel, mux, led);

    builder.connect(in0, 0, mux, "In0");
    builder.connect(in1, 0, mux, "In1");
    builder.connect(sel, 0, mux, "S0");
    builder.connect(mux, 0, led, 0);

    auto *sim = builder.initSimulation();

    in0->setOn(false);
    in1->setOn(true);

    sel->setOn(false); // S0 = 0 selects In0 (off)
    sim->update();
    QVERIFY(!TestUtils::getInputStatus(led));

    sel->setOn(true); // S0 = 1 selects In1 (on)
    sim->update();
    QVERIFY(TestUtils::getInputStatus(led));
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

    QVERIFY(TestUtils::pixmapHasInk(pixmap));
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
