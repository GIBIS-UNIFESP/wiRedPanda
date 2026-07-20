// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestDemux.h"

#include <QPainter>

#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"
#include "Tests/QuickShell/QuickCpuTestUtils.h"

void TestDemux::testDemuxOutputSize()
{
    QuickCircuitBuilder builder;
    Demux demux;
    builder.add(&demux);
    int defaultSize = demux.outputSize();
    QVERIFY(defaultSize > 0);
}

void TestDemux::testDemuxRouting()
{
    QuickCircuitBuilder builder;
    Demux demux;
    builder.add(&demux);
    QVERIFY(demux.inputSize() > 0);
}

void TestDemux::testDemuxPainting()
{
    QuickCircuitBuilder builder;
    Demux demux;
    builder.add(&demux);

    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    demux.paint(&painter);
    painter.end();

    QVERIFY2(TestUtils::pixmapHasInk(pixmap), "Demux paint() must draw visible pixels");
}

void TestDemux::testDemuxOutOfRangeSelect()
{
    // Regression test (F3 alignment): 5 outputs need 3 select lines, so
    // select values 5..7 address no output. Routing is indeterminate — every
    // output must be Unknown (previously they were silently all-Inactive,
    // i.e. the data vanished without any indication).
    QuickCircuitBuilder builder;

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
