// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestWaveformSimulator.h"

#include "App/BeWavedDolphin/WaveformSimulator.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

void TestWaveformSimulator::testCaptureAndRestoreSinglePortInputs()
{
    InputSwitch sw1;
    sw1.setOn(true);
    InputSwitch sw2;
    sw2.setOn(false);
    QVector<GraphicElementInput *> inputs{&sw1, &sw2};

    const auto saved = WaveformSimulator::captureInputs(inputs, 2);
    QCOMPARE(saved.size(), 2);

    // Flip both away from their captured state before restoring.
    sw1.setOn(false);
    sw2.setOn(true);

    WaveformSimulator::restoreInputs(inputs, saved);

    QCOMPARE(sw1.outputPort(0)->status(), Status::Active);
    QCOMPARE(sw2.outputPort(0)->status(), Status::Inactive);
}

void TestWaveformSimulator::testCaptureAndRestoreMultiPortInput()
{
    // InputRotary exposes multiple output ports, exercising restoreInputs()'s
    // per-port setOn(value, port) path (single-port inputs only ever use port 0).
    InputRotary rotary;
    QVERIFY2(rotary.outputSize() > 1, "InputRotary should have more than one output port by default");
    QVector<GraphicElementInput *> inputs{&rotary};

    const auto saved = WaveformSimulator::captureInputs(inputs, rotary.outputSize());
    QCOMPARE(saved.size(), rotary.outputSize());

    // Flip every port away from its captured state.
    for (int port = 0; port < rotary.outputSize(); ++port) {
        rotary.setOn(rotary.outputPort(port)->status() != Status::Active, port);
    }

    WaveformSimulator::restoreInputs(inputs, saved);

    for (int port = 0; port < rotary.outputSize(); ++port) {
        QCOMPARE(rotary.outputPort(port)->status(), saved.at(port));
    }
}

void TestWaveformSimulator::testRestorePreservesNonDefiniteStatus()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    builder.add(sw);

    // A port that has never been driven reads Unknown -- the state captureInputs() would see for
    // an input added but not yet simulated.
    sw->outputPort(0)->setStatus(Status::Unknown);

    const QVector<GraphicElementInput *> inputs{sw};
    const auto saved = WaveformSimulator::captureInputs(inputs, 1);
    QCOMPARE(saved.size(), 1);
    QCOMPARE(saved.at(0), Status::Unknown);

    // Whatever the sweep left behind:
    sw->setOn(true);
    QCOMPARE(sw->outputPort(0)->status(), Status::Active);

    WaveformSimulator::restoreInputs(inputs, saved);
    QVERIFY2(sw->outputPort(0)->status() == Status::Unknown,
             "an undefined input port must be restored undefined, not driven HIGH");
}
