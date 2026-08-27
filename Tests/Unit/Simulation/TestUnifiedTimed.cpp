// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestUnifiedTimed.h"

#include <algorithm>
#include <functional>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QtTest>

#include "App/Core/Enums.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "Tests/Common/TestUtils.h"

namespace {

/// Rows for every tick-window-parameterised test below. The window is the parameter four of this
/// branch's landed defects hid behind -- each was found only at a window the fixed `1` never
/// reached -- so a timing test that pins one window is testing one point of a space, not the
/// behaviour. Row 0 is functional mode, where delays do not apply at all, which makes each of
/// these a mode-agreement check as well.
///
/// The delay is a column too, and not for symmetry. A row whose window is at least as wide as
/// the delay lands past the deadline on its FIRST tick, so it can never observe the output
/// early -- measured: with the delay forced to 0, the 1 and 3 ns rows failed and the 7, 20 and
/// 1000 ns rows passed. Those coarse rows are still worth keeping (a wide window is exactly what
/// E16 mis-handled) but in a test that watches for an early transition they check the settled
/// value only, so each is paired with a row whose
/// delay is a multiple of the window and which therefore does observe the wait.
void addDelayAndWindowRows()
{
    QTest::addColumn<quint64>("nsPerTick");
    QTest::addColumn<quint64>("delayScale"); // delay = delayScale * the test's base delay

    QTest::newRow("functional (0 ns/tick)")        << quint64{0}    << quint64{1};
    QTest::newRow("1 ns/tick")                     << quint64{1}    << quint64{1};
    QTest::newRow("3 ns/tick")                     << quint64{3}    << quint64{1};
    QTest::newRow("7 ns/tick (window > delay)")      << quint64{7}    << quint64{1};
    QTest::newRow("20 ns/tick (window > delay)")     << quint64{20}   << quint64{1};
    QTest::newRow("1000 ns/tick (window > delay)")   << quint64{1000} << quint64{1};
    // Window still coarse, delay scaled past it, so the wait itself is observable again.
    QTest::newRow("7 ns/tick, delay x7")           << quint64{7}    << quint64{7};
    QTest::newRow("20 ns/tick, delay x20")         << quint64{20}   << quint64{20};
    QTest::newRow("1000 ns/tick, delay x1000")     << quint64{1000} << quint64{1000};
}

/// Ticks \a sim until sim time reaches \a deadline, asserting \a stillUnchanged holds at every
/// instant strictly before it. Expresses the temporal contract without assuming how many ticks
/// that takes: at a window wider than the delay the loop simply observes fewer instants.
/// Returns false (with the failure already reported) if the deadline is not reached.
bool advanceToDeadline(Simulation *sim, const SimTime deadline, const std::function<bool()> &stillUnchanged,
                       const char *tooEarly)
{
    int guard = 0;
    while (sim->currentTime() < deadline) {
        if (!stillUnchanged()) {
            QTest::qFail(tooEarly, __FILE__, __LINE__);
            return false;
        }
        sim->update();
        if (++guard > 10000) {
            QTest::qFail("the deadline was never reached", __FILE__, __LINE__);
            return false;
        }
    }
    return true;
}

} // namespace

void TestUnifiedTimed::testGateDelay_data()
{
    addDelayAndWindowRows();
}

void TestUnifiedTimed::testGateDelay()
{
    QFETCH(quint64, nsPerTick);
    QFETCH(quint64, delayScale);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *notGate = new Not();
    builder.add(sw, notGate);
    builder.connect(sw, 0, notGate, 0);
    Simulation *sim = builder.initSimulation();

    // Functional baseline (zero delay): NOT(0) settles to 1 immediately.
    sw->setOn(false);
    sim->update();
    QCOMPARE(notGate->outputValue(0), Status::Active);

    const SimTime kDelay = 5 * static_cast<SimTime>(delayScale);
    sim->setElementDelay(notGate, kDelay);
    sim->setTimePerTick(static_cast<SimTime>(nsPerTick));

    const SimTime t0 = sim->currentTime();
    sw->setOn(true); // input rises; NOT must NOT fall until t0 + kDelay

    if (nsPerTick == 0) {
        // Functional mode: the delay does not apply, so one tick settles it and sim time
        // does not advance at all.
        sim->update();
        QCOMPARE(notGate->outputValue(0), Status::Inactive);
        QCOMPARE(sim->currentTime(), t0);
        return;
    }

    if (!advanceToDeadline(sim, t0 + kDelay,
                           [notGate] { return notGate->outputValue(0) == Status::Active; },
                           "NOT changed before its propagation delay elapsed")) {
        return;
    }

    QVERIFY(sim->currentTime() >= t0 + kDelay);
    QCOMPARE(notGate->outputValue(0), Status::Inactive); // NOT(1) = 0, once the delay has elapsed
}

void TestUnifiedTimed::testChainedDelay_data()
{
    addDelayAndWindowRows();
}

void TestUnifiedTimed::testChainedDelay()
{
    QFETCH(quint64, nsPerTick);
    QFETCH(quint64, delayScale);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *not1 = new Not();
    auto *not2 = new Not();
    builder.add(sw, not1, not2);
    builder.connect(sw, 0, not1, 0);
    builder.connect(not1, 0, not2, 0);
    Simulation *sim = builder.initSimulation();

    // Baseline: sw=0 -> not1=1 -> not2=0.
    sw->setOn(false);
    sim->update();
    QCOMPARE(not1->outputValue(0), Status::Active);
    QCOMPARE(not2->outputValue(0), Status::Inactive);

    const SimTime kDelay = 3 * static_cast<SimTime>(delayScale);
    sim->setElementDelay(not1, kDelay);
    sim->setElementDelay(not2, kDelay);
    sim->setTimePerTick(static_cast<SimTime>(nsPerTick));

    const SimTime t0 = sim->currentTime();
    sw->setOn(true); // not1 flips at t0 + 3, not2 at t0 + 6 (delays accumulate down the chain)

    if (nsPerTick == 0) {
        sim->update();
        QCOMPARE(not1->outputValue(0), Status::Inactive);
        QCOMPARE(not2->outputValue(0), Status::Active);
        QCOMPARE(sim->currentTime(), t0);
        return;
    }

    if (!advanceToDeadline(sim, t0 + kDelay,
                           [not1] { return not1->outputValue(0) == Status::Active; },
                           "the first NOT changed before its own delay elapsed")) {
        return;
    }
    QCOMPARE(not1->outputValue(0), Status::Inactive);

    if (!advanceToDeadline(sim, t0 + 2 * kDelay,
                           [not2] { return not2->outputValue(0) == Status::Inactive; },
                           "the second NOT changed before the accumulated chain delay elapsed")) {
        return;
    }
    QCOMPARE(not2->outputValue(0), Status::Active);
}

void TestUnifiedTimed::testGlitchAbsorbed_data()
{
    // The pulse width is expressed in TICKS, not nanoseconds, so it is exact at every window:
    // driving the input low again after N ticks produces a pulse of exactly N * nsPerTick,
    // whereas asking for "2 ns" at a 3 ns window silently yields 3. Every row keeps that width
    // strictly below the delay, which is the condition for inertial absorption -- the boundary
    // case (width == delay, which PROPAGATES) is pinned separately by
    // testPropertyPulseExactlyEqualToDelayPropagates.
    //
    // Functional mode has no row: with a zero window sim time never advances, so "a pulse two
    // units wide" is not expressible -- the toggle happens inside one instant.
    QTest::addColumn<quint64>("nsPerTick");
    QTest::addColumn<quint64>("delay");
    QTest::addColumn<int>("pulseTicks");

    QTest::newRow("1 ns/tick, 2 of 5")        << quint64{1}    << quint64{5}    << 2;
    QTest::newRow("1 ns/tick, 4 of 5 (just under)") << quint64{1} << quint64{5} << 4;
    QTest::newRow("3 ns/tick, 9 of 10")       << quint64{3}    << quint64{10}   << 3;
    QTest::newRow("7 ns/tick, 98 of 100")     << quint64{7}    << quint64{100}  << 14;
    QTest::newRow("20 ns/tick, 80 of 100")    << quint64{20}   << quint64{100}  << 4;
    QTest::newRow("1000 ns/tick, 4000 of 5000") << quint64{1000} << quint64{5000} << 4;
}

void TestUnifiedTimed::testGlitchAbsorbed()
{
    QFETCH(quint64, nsPerTick);
    QFETCH(quint64, delay);
    QFETCH(int, pulseTicks);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *notGate = new Not();
    builder.add(sw, notGate);
    builder.connect(sw, 0, notGate, 0);
    Simulation *sim = builder.initSimulation();

    sw->setOn(false);
    sim->update();
    QCOMPARE(notGate->outputValue(0), Status::Active);

    sim->setElementDelay(notGate, static_cast<SimTime>(delay));
    sim->setTimePerTick(static_cast<SimTime>(nsPerTick));

    const SimTime pulseWidth = static_cast<SimTime>(nsPerTick) * static_cast<SimTime>(pulseTicks);
    QVERIFY2(pulseWidth < delay, "precondition: the row must describe a pulse narrower than the delay");

    // Inertial delay by supersession: when the scheduled publication fires, the input has
    // already returned low, so the NOT re-stages the value it already published and the pulse
    // never appears on the output. Asserted after EVERY tick rather than at two sampled
    // instants -- a glitch one tick wide between the samples would otherwise go unseen.
    sw->setOn(true);
    for (int i = 0; i < pulseTicks; ++i) {
        sim->update();
        QVERIFY2(notGate->outputValue(0) == Status::Active,
                 qPrintable(QString("the output fell during the pulse, at t=%1").arg(sim->currentTime())));
    }
    sw->setOn(false);

    // Run out well past both the pulse and the delay: the superseded publication would land in
    // here if it were not absorbed.
    const SimTime deadline = sim->currentTime() + 2 * static_cast<SimTime>(delay);
    while (sim->currentTime() < deadline) {
        sim->update();
        QVERIFY2(notGate->outputValue(0) == Status::Active,
                 qPrintable(QString("an absorbed pulse reached the output at t=%1").arg(sim->currentTime())));
    }
}

void TestUnifiedTimed::testRippleFlipFlopReevaluation_data()
{
    addDelayAndWindowRows();
}

void TestUnifiedTimed::testRippleFlipFlopReevaluation()
{
    QFETCH(quint64, nsPerTick);
    QFETCH(quint64, delayScale);

    // FF1's Q feeds FF2's Clock directly (no gate in between) — a ripple/derived-clock
    // topology. Regression test for a bug where a flip-flop's committed change never
    // propagated to a directly-fed downstream flip-flop: successor scheduling inside
    // processEvents() is gated on outputChanged(), which stays false for a deferred
    // (staged) Memory-group write, and the post-edge resettle pass explicitly skips
    // Memory-group elements so it never re-clocks them either. Without the fix, FF2 never
    // reacts to FF1's Q, in either mode, on any edge.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *swD1 = new InputSwitch();
    auto *swClk1 = new InputSwitch();
    auto *swD2 = new InputSwitch();
    auto *ff1 = new DFlipFlop();
    auto *ff2 = new DFlipFlop();
    builder.add(swD1, swClk1, swD2, ff1, ff2);
    builder.connect(swD1, 0, ff1, 0);   // D
    builder.connect(swClk1, 0, ff1, 1); // Clock
    builder.connect(ff1, 0, ff2, 1);    // FF1.Q -> FF2.Clock (ripple, no gate)
    builder.connect(swD2, 0, ff2, 0);   // D
    Simulation *sim = builder.initSimulation();

    swD1->setOn(true);
    swD2->setOn(true);
    swClk1->setOn(false);
    sim->update(); // baseline settle: no edge yet, both flip-flops stay at power-on default
    QCOMPARE(ff1->outputValue(0), Status::Inactive);
    QCOMPARE(ff2->outputValue(0), Status::Inactive);

    // --- Functional mode: FF2 must react to FF1's edge within the SAME update() call. ---
    swClk1->setOn(true);
    sim->update();
    QCOMPARE(ff1->outputValue(0), Status::Active);
    QCOMPARE(ff2->outputValue(0), Status::Active); // FF2 must capture D2 on FF1's rising edge

    // Drive FF1.Q back to Inactive (a FALLING transition on FF2's clock — FF2, edge-triggered
    // on rising only, must NOT recapture here) before producing a second, genuine rising edge.
    swD1->setOn(false);
    swClk1->setOn(false);
    sim->update();
    swClk1->setOn(true);
    sim->update();
    QCOMPARE(ff1->outputValue(0), Status::Inactive);
    QCOMPARE(ff2->outputValue(0), Status::Active); // unchanged: FF1's Q fell, not rose

    // A second genuine RISING edge on FF1.Q — FF2 must react to THIS one too, not just the
    // very first (proves this isn't a one-shot fluke of the initial full network seed).
    swD1->setOn(true);
    swD2->setOn(false);
    swClk1->setOn(false);
    sim->update();
    swClk1->setOn(true);
    sim->update();
    QCOMPARE(ff1->outputValue(0), Status::Active);
    QCOMPARE(ff2->outputValue(0), Status::Inactive); // FF2 must react to this edge too

    // --- Temporal mode: the same chain, with real, elapsing per-element delays. ---
    // Drive FF1.Q back to Inactive first (still functional mode) so the upcoming temporal
    // edge is a genuine rising transition, not a same-value re-assertion.
    swD1->setOn(false);
    swClk1->setOn(false);
    sim->update();
    swClk1->setOn(true);
    sim->update();
    QCOMPARE(ff1->outputValue(0), Status::Inactive);

    swClk1->setOn(false);
    swD1->setOn(true);
    swD2->setOn(true);
    sim->update();

    // The ripple must cross both stages at every tick window, not just at the 1 ns this once
    // pinned: a ripple chain advancing by element delays WITHIN a tick is precisely what E15
    // showed the Arduino export getting wrong, and what a window wider than the delay exercises
    // differently from a window narrower than it.
    const SimTime delay = 5 * static_cast<SimTime>(delayScale);
    sim->setElementDelay(ff1, delay);
    sim->setElementDelay(ff2, delay);
    sim->setTimePerTick(static_cast<SimTime>(nsPerTick));

    swClk1->setOn(true); // rising edge for FF1

    if (nsPerTick == 0) {
        // Functional mode: the whole chain crosses within one update().
        sim->update();
    } else {
        // Three delays is one more than the chain needs, so the deadline is reached at every
        // window without the test having to know how many ticks that takes.
        const SimTime deadline = sim->currentTime() + 3 * delay;
        int guard = 0;
        while (sim->currentTime() < deadline) {
            sim->update();
            QVERIFY(++guard < 10000);
        }
    }

    QCOMPARE(ff1->outputValue(0), Status::Active);
    QCOMPARE(ff2->outputValue(0), Status::Active); // FF2 must react once FF1 actually commits
}

void TestUnifiedTimed::testStructuralEditDropsPendingEvents()
{
    // initialize() must drop pending events. They are stale twice over: SimEvent::target is a
    // raw pointer into the old netlist (structural commands genuinely FREE elements — delete,
    // morph, split-undo), and the baked-in priority comes from the pre-edit topological sort.
    // In temporal mode a pending cross-tick event is easy to produce: give a gate a delay far
    // beyond the tick window. Pre-fix, draining past that timestamp after the edit dereferences
    // the freed element (deterministic use-after-free under ASan).
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *doomed = new Not();
    auto *keep = new Not(); // survives the edit; proves the reseeded circuit still works
    builder.add(sw, doomed, keep);
    builder.connect(sw, 0, doomed, 0);
    builder.connect(sw, 0, keep, 0);
    Simulation *sim = builder.initSimulation();

    sw->setOn(false);
    sim->update(); // functional baseline settle
    QCOMPARE(doomed->outputValue(0), Status::Active);
    QCOMPARE(keep->outputValue(0), Status::Active);

    sim->setElementDelay(doomed, 100); // far beyond the 1-unit tick window below
    sim->setTimePerTick(1);
    sw->setOn(true);
    sim->update(); // doomed's re-evaluation is now pending ~100 units in the future

    // The real structural-edit path: DeleteItemsCommand::redo() severs the wire, FREES the
    // element, and re-initializes via setCircuitUpdateRequired().
    workspace.scene()->receiveCommand(
        new DeleteItemsCommand(QList<QGraphicsItem *>{doomed}, workspace.scene()));

    // Drain far past the stale event's timestamp: nothing may touch the freed element, and
    // the surviving gate settles from the post-edit reseed.
    for (int i = 0; i < 300; ++i) {
        sim->update();
    }
    QCOMPARE(keep->outputValue(0), Status::Inactive); // NOT(1), reseeded and settled
}

void TestUnifiedTimed::testTimedRunBracketDelaysInsideAndRestoresAfter()
{
    // The BeWavedDolphin sweep's contract: open a bracket, drive update() once per column, and
    // hand the live session back exactly the tick window it had. Inside the bracket a 5-unit
    // NOT must lag its input by 5 columns; outside it, the live window is whatever it was.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *notGate = new Not();
    builder.add(sw, notGate);
    builder.connect(sw, 0, notGate, 0);
    Simulation *sim = builder.initSimulation();

    sw->setOn(false);
    sim->update(); // live (functional) baseline: NOT(0) = 1

    sim->setElementDelay(notGate, 5);
    sim->beginTimedRun(1); // 1 unit of sim-time per "column"
    QCOMPARE(sim->currentTime(), SimTime{0});

    sim->update(); // column 0: whole-network re-seed; the tick closes at t = 1
    QCOMPARE(notGate->outputValue(0), Status::Active);
    QCOMPARE(sim->currentTime(), SimTime{1});

    // The input rises before the next column, so the engine observes it at t = 1 and schedules
    // the NOT for t = 1 + 5. Four more columns must not be enough.
    sw->setOn(true);
    for (int column = 1; column <= 4; ++column) {
        sim->update();
        QVERIFY2(notGate->outputValue(0) == Status::Active,
                 qPrintable(QString("NOT fell early, at column %1 (t=%2)")
                                .arg(column).arg(sim->currentTime())));
    }

    sim->update(); // this column reaches t = 6, the scheduled instant
    QCOMPARE(sim->currentTime(), SimTime{6});
    QCOMPARE(notGate->outputValue(0), Status::Inactive);

    // Closing the bracket restores the live window: the next update() settles in-tick again.
    sim->endTimedRun(0);
    QCOMPARE(sim->timePerTick(), SimTime{0});
    sw->setOn(false);
    sim->update();
    QCOMPARE(notGate->outputValue(0), Status::Active); // zero-delay again, no lag
}

void TestUnifiedTimed::testTimedRunZeroWindowIgnoresLiveTemporalMode()
{
    // A functional sweep must run at 0 ns/tick rather than inherit whatever window the live
    // session left behind — otherwise a chain would ripple one column per stage in a sweep the
    // user explicitly asked to be zero-delay.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *not1 = new Not();
    auto *not2 = new Not();
    builder.add(sw, not1, not2);
    builder.connect(sw, 0, not1, 0);
    builder.connect(not1, 0, not2, 0);
    Simulation *sim = builder.initSimulation();

    sim->setElementDelay(not1, 5);
    sim->setElementDelay(not2, 5);
    sim->setTimePerTick(1); // the live session is temporal
    sw->setOn(false);
    sim->update();

    const SimTime liveWindow = sim->timePerTick();
    sim->beginTimedRun(0); // the sweep asks for functional
    sim->update();         // re-seed: the whole chain settles at this instant
    sw->setOn(true);
    sim->update();         // one tick is enough — both gates are zero-delay here
    QCOMPARE(not1->outputValue(0), Status::Inactive);
    QCOMPARE(not2->outputValue(0), Status::Active);
    QCOMPARE(sim->currentTime(), SimTime{0}); // a 0-window bracket never advances sim time

    sim->endTimedRun(liveWindow);
    QCOMPARE(sim->timePerTick(), liveWindow);
}

// ===========================================================================
// Shipped temporal example circuits
// ===========================================================================

namespace {

/// Loads Examples/<name>.panda into \a workspace. Returns false (with a message) on failure.
bool loadExample(WorkSpace &workspace, const QString &name, QString &error)
{
    const QFileInfo fileInfo(QDir(TestUtils::examplesDir()).filePath(name));
    if (!fileInfo.exists()) {
        error = QString("Example %1 not found at %2").arg(name, fileInfo.absoluteFilePath());
        return false;
    }

    QFile pandaFile(fileInfo.absoluteFilePath());
    if (!pandaFile.open(QIODevice::ReadOnly)) {
        error = QString("Could not open %1").arg(fileInfo.absoluteFilePath());
        return false;
    }

    QDataStream stream(&pandaFile);
    const QVersionNumber version = Serialization::readPandaHeader(stream);
    workspace.load(stream, version, fileInfo.absolutePath());
    return true;
}

/// Returns the first element in \a scene whose label is \a label, or nullptr.
GraphicElement *elementByLabel(Scene *scene, const QString &label)
{
    const auto elements = scene->elements();
    const auto it = std::find_if(elements.cbegin(), elements.cend(),
                                 [&label](GraphicElement *elm) { return elm && elm->label() == label; });
    return it == elements.cend() ? nullptr : *it;
}

/// Drives \a ticks updates and records every value \a element's output port 0 takes, with the
/// sim time it took it.
QVector<QPair<SimTime, Status>> sampleOutput(Simulation *simulation, GraphicElement *element, int ticks)
{
    QVector<QPair<SimTime, Status>> samples;
    samples.append({simulation->currentTime(), element->outputValue(0)});
    for (int tick = 0; tick < ticks; ++tick) {
        simulation->update();
        const Status current = element->outputValue(0);
        if (current != samples.last().second) {
            samples.append({simulation->currentTime(), current});
        }
    }
    return samples;
}

} // namespace

void TestUnifiedTimed::testExampleRingOscillator()
{
    WorkSpace workspace;
    QString error;
    QVERIFY2(loadExample(workspace, "temporal_ring_oscillator.panda", error), qPrintable(error));

    auto *n1 = elementByLabel(workspace.scene(), "n1");
    QVERIFY2(n1 && n1->elementType() == ElementType::Not, "ring inverter n1 missing from the example");

    auto *simulation = workspace.scene()->simulation();
    QVERIFY(simulation->initialize());

    // Functional mode: a 3-inverter ring has no stable value, so the engine canonicalizes the
    // feedback region to Unknown rather than freezing an arbitrary phase. That is the example's
    // teaching point — it looks "dead" until swept with delays.
    simulation->update();
    QCOMPARE(n1->outputValue(0), Status::Unknown);

    // Temporal mode: the loop resolves over time instead, toggling every 3 x 5 ns. Reset every
    // element to power-on defaults first, exactly as WaveformSimulator::sweep() does before a
    // BeWavedDolphin run — without it the ring stays stuck on the Unknown the functional pass
    // just canonicalized it to, since NOT(Unknown) = Unknown is itself a fixed point.
    for (auto *element : workspace.scene()->elements()) {
        if (element && element->type() == GraphicElement::Type) {
            element->resetSimState();
        }
    }
    simulation->beginTimedRun(1);
    const auto samples = sampleOutput(simulation, n1, 120);
    simulation->endTimedRun(0);

    const bool sawUnknown = std::any_of(samples.cbegin(), samples.cend(),
                                        [](const auto &s) { return s.second == Status::Unknown; });
    QVERIFY2(!sawUnknown, "the temporal ring went Unknown — it hit the oscillation cap instead of oscillating");
    QVERIFY2(samples.size() >= 4,
             qPrintable(QString("Expected the ring to toggle repeatedly, saw %1 distinct values")
                            .arg(samples.size())));
}

void TestUnifiedTimed::testExampleStaticHazard()
{
    WorkSpace workspace;
    QString error;
    QVERIFY2(loadExample(workspace, "temporal_static_hazard.panda", error), qPrintable(error));

    auto *switchA = elementByLabel(workspace.scene(), "A");
    auto *notGate = elementByLabel(workspace.scene(), "NOT_A");
    auto *orGate = elementByLabel(workspace.scene(), "F");
    QVERIFY2(switchA && notGate && orGate, "static-hazard elements missing from the example");

    // The tuned overrides must survive the .panda round-trip, or the hazard never appears.
    QCOMPARE(notGate->propagationDelay(), SimTime{10});
    QCOMPARE(orGate->propagationDelay(), SimTime{3});

    auto *input = dynamic_cast<GraphicElementInput *>(switchA);
    QVERIFY(input);

    auto *simulation = workspace.scene()->simulation();
    QVERIFY(simulation->initialize());

    input->setOn(true);
    simulation->update();
    QCOMPARE(orGate->outputValue(0), Status::Active); // F = A OR NOT A is 1 in steady state

    // Drop A: the direct branch falls at once while the inverted branch is still 10 ns behind,
    // so F dips before recovering — invisible in functional mode, where F is a tautology.
    simulation->beginTimedRun(1);
    simulation->update();
    input->setOn(false);
    const auto samples = sampleOutput(simulation, orGate, 60);
    simulation->endTimedRun(0);

    const bool sawLow = std::any_of(samples.cbegin(), samples.cend(),
                                    [](const auto &s) { return s.second == Status::Inactive; });
    QVERIFY2(sawLow, "the static-1 hazard glitch never appeared on F");
    QCOMPARE(orGate->outputValue(0), Status::Active); // and it settles back to 1
}

void TestUnifiedTimed::testExampleGateDelayChain()
{
    WorkSpace workspace;
    QString error;
    QVERIFY2(loadExample(workspace, "temporal_gate_delay_chain.panda", error), qPrintable(error));

    auto *switchA = elementByLabel(workspace.scene(), "A");
    auto *n1 = elementByLabel(workspace.scene(), "n1");
    auto *n4 = elementByLabel(workspace.scene(), "n4");
    QVERIFY2(switchA && n1 && n4, "gate-delay-chain elements missing from the example");

    auto *input = dynamic_cast<GraphicElementInput *>(switchA);
    QVERIFY(input);

    auto *simulation = workspace.scene()->simulation();
    QVERIFY(simulation->initialize());

    input->setOn(false);
    simulation->update();

    simulation->beginTimedRun(1);
    simulation->update();
    input->setOn(true);

    // Walk the timeline once, noting when the first and last taps flip.
    SimTime firstTapChangedAt = SIM_TIME_UNSET;
    SimTime lastTapChangedAt = SIM_TIME_UNSET;
    const Status n1Initial = n1->outputValue(0);
    const Status n4Initial = n4->outputValue(0);
    for (int tick = 0; tick < 80; ++tick) {
        simulation->update();
        if (firstTapChangedAt == SIM_TIME_UNSET && n1->outputValue(0) != n1Initial) {
            firstTapChangedAt = simulation->currentTime();
        }
        if (lastTapChangedAt == SIM_TIME_UNSET && n4->outputValue(0) != n4Initial) {
            lastTapChangedAt = simulation->currentTime();
        }
    }
    simulation->endTimedRun(0);

    QVERIFY2(firstTapChangedAt != SIM_TIME_UNSET && lastTapChangedAt != SIM_TIME_UNSET,
             "the chain's taps never changed");
    QVERIFY2(lastTapChangedAt > firstTapChangedAt,
             qPrintable(QString("stage 4 must lag stage 1 (stage 1 at %1, stage 4 at %2)")
                            .arg(firstTapChangedAt).arg(lastTapChangedAt)));
}

// ===========================================================================
// Metamorphic properties — acceptance criteria for publish-side delay
// ===========================================================================

namespace {

/// Advances \a sim one tick at a time up to \a ticks, invoking \a atTick(t) before each update
/// (t is the tick index, 1-based), and returns the sim times at which \a watched's output 0
/// changed value. Sampling per tick is what lets a property assert on *relationships between*
/// times rather than on one golden instant.
QVector<SimTime> runAndRecord(Simulation *sim, GraphicElement *watched, int ticks,
                              const std::function<void(int)> &atTick)
{
    QVector<SimTime> transitions;
    Status last = watched->outputValue(0);
    for (int t = 1; t <= ticks; ++t) {
        atTick(t);
        sim->update();
        const Status now = watched->outputValue(0);
        if (now != last) {
            transitions.append(sim->currentTime());
            last = now;
        }
    }
    return transitions;
}

/// Rows for the two pulse-train properties. The half period is in TICKS, so its width in sim
/// time is halfPeriodTicks * nsPerTick and each test derives its own delay from that -- five
/// times the width for the "narrower than the delay" property, exactly the width for the
/// boundary one. Pinning nsPerTick to 1, as both did, tested one point of that relation.
void addPulseTrainRows()
{
    QTest::addColumn<quint64>("nsPerTick");
    QTest::addColumn<int>("halfPeriodTicks");

    QTest::newRow("1 ns/tick, 4-tick half period")   << quint64{1}    << 4;
    QTest::newRow("1 ns/tick, 1-tick half period")   << quint64{1}    << 1;
    QTest::newRow("3 ns/tick, 4-tick half period")   << quint64{3}    << 4;
    QTest::newRow("7 ns/tick, 3-tick half period")   << quint64{7}    << 3;
    QTest::newRow("20 ns/tick, 2-tick half period")  << quint64{20}   << 2;
    QTest::newRow("1000 ns/tick, 1-tick half period") << quint64{1000} << 1;
}

} // namespace

void TestUnifiedTimed::testPropertyNoOutputRunShorterThanDelay_data()
{
    addPulseTrainRows();
}

void TestUnifiedTimed::testPropertyNoOutputRunShorterThanDelay()
{
    // A gate fed a square wave five times faster than its own delay must stay FLAT: every pulse
    // is far narrower than the delay, so an inertial model absorbs all of them. Transport delay
    // passes them through.
    QFETCH(quint64, nsPerTick);
    QFETCH(int, halfPeriodTicks);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *notGate = new Not();
    builder.add(sw, notGate);
    builder.connect(sw, 0, notGate, 0);
    Simulation *sim = builder.initSimulation();

    bool level = false;
    sw->setOn(level);
    sim->update();

    const SimTime pulseWidth = static_cast<SimTime>(nsPerTick) * static_cast<SimTime>(halfPeriodTicks);
    const SimTime delay = 5 * pulseWidth;
    sim->setElementDelay(notGate, delay);
    sim->setTimePerTick(static_cast<SimTime>(nsPerTick));

    const auto transitions = runAndRecord(sim, notGate, 200, [&](int t) {
        if (t % halfPeriodTicks == 0) {
            level = !level;
            sw->setOn(level);
        }
    });

    QVERIFY2(transitions.isEmpty(),
             qPrintable(QString("a %1 ns pulse train passed a %2 ns gate: %3 output transitions "
                                "(inertial delay must absorb every pulse narrower than the delay)")
                            .arg(pulseWidth).arg(delay).arg(transitions.size())));
}

void TestUnifiedTimed::testPropertyPulseExactlyEqualToDelayPropagates_data()
{
    addPulseTrainRows();
}

void TestUnifiedTimed::testPropertyPulseExactlyEqualToDelayPropagates()
{
    // testPropertyNoOutputRunShorterThanDelay covers w < D (a 4 ns pulse into a 20 ns gate).
    // w == D is the boundary, and it PROPAGATES -- which is both the conventional inertial-delay
    // rule (absorb strictly narrower than the delay) and what this engine does. Measured, after
    // an argument from the event comparator predicted the opposite: reasoning about which of
    // Evaluate/Publish wins at a shared instant is not a substitute for running it, because the
    // wake is seeded at the tick's start time rather than at the toggle instant.
    QFETCH(quint64, nsPerTick);
    QFETCH(int, halfPeriodTicks);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *notGate = new Not();
    builder.add(sw, notGate);
    builder.connect(sw, 0, notGate, 0);
    Simulation *sim = builder.initSimulation();

    bool level = false;
    sw->setOn(level);
    sim->update();

    // A square wave whose half period is exactly the gate delay, at every window.
    const SimTime kDelay = static_cast<SimTime>(nsPerTick) * static_cast<SimTime>(halfPeriodTicks);
    sim->setElementDelay(notGate, kDelay);
    sim->setTimePerTick(static_cast<SimTime>(nsPerTick));

    const auto transitions = runAndRecord(sim, notGate, 200, [&](int t) {
        if (t % halfPeriodTicks == 0) {
            level = !level;
            sw->setOn(level);
        }
    });

    QVERIFY2(!transitions.isEmpty(),
             "a pulse exactly as wide as the gate delay must propagate: inertial delay absorbs "
             "pulses strictly NARROWER than the delay, not equal to it");
}

void TestUnifiedTimed::testSeedSettlesWholeNetworkAtPowerOnEvenInTemporalMode()
{
    // Two 20 ns inverters in series, driven high. If power-on settling were spread across
    // propagation delays -- the other defensible choice -- then one 1 ns tick after power-on
    // would have moved nothing past the first stage. The seed instead evaluates every element
    // once, in topological order, at the current instant, so the whole chain is already settled.
    //
    // The expected value must DIFFER from the power-on default, or the test cannot tell "settled"
    // from "has not moved yet". NOT(NOT(1)) = 1, against a default of Inactive. THREE inverters
    // would settle to Inactive -- exactly the default -- and would pass even with the seed
    // replaced by event-driven power-on settling.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *n1 = new Not();
    auto *n2 = new Not();
    builder.add(sw, n1, n2);
    builder.connect(sw, 0, n1, 0);
    builder.connect(n1, 0, n2, 0);
    Simulation *sim = builder.initSimulation();

    sw->setOn(true);
    sim->update();
    const QVector<GraphicElement *> chain{n1, n2};
    for (auto *elm : chain) {
        sim->setElementDelay(elm, 20);
    }

    // Power-on: clear every element back to its defaults and force a full re-seed, then advance
    // a single 1 ns tick -- far less than one gate delay.
    for (auto *element : workspace.scene()->elements()) {
        if (element && element->type() == GraphicElement::Type) {
            element->resetSimState();
        }
    }
    sim->resetEventTracking();
    sim->beginTimedRun(1);
    sim->update();

    QVERIFY2(n2->outputValue(0) == Status::Active,
             qPrintable(QString("after one 1 ns tick the far end of a 2 x 20 ns chain must already "
                                "be settled to Active (power-on settles at the instant, not over "
                                "max-path-delay); got %1")
                            .arg(static_cast<int>(n2->outputValue(0)))));
    sim->endTimedRun(0);
}

void TestUnifiedTimed::testPowerOnCaptureIsModeIndependent()
{
    // A flip-flop whose clock is already high at power-on sees its first evaluation as a rising
    // edge (m_simLastClk starts Inactive) and captures D. Because the seed settles at the instant
    // in both modes, WHAT it captures must not depend on the mode -- only when it becomes
    // visible. This is the interaction the seed semantics left unverified.
    const auto powerOnCapture = [](const SimTime window) {
        WorkSpace workspace;
        CircuitBuilder builder(workspace.scene());
        auto *din = new InputSwitch();
        auto *clk = new InputSwitch();
        auto *ff = new DFlipFlop();
        builder.add(din, clk, ff);
        builder.connect(din, 0, ff, 0);
        builder.connect(clk, 0, ff, 1);
        Simulation *sim = builder.initSimulation();

        // Both high BEFORE the network is ever settled: this is the power-on condition.
        din->setOn(true);
        clk->setOn(true);
        for (auto *element : workspace.scene()->elements()) {
            if (element && element->type() == GraphicElement::Type) {
                element->resetSimState();
            }
        }
        sim->resetEventTracking();

        if (window > 0) {
            sim->setElementDelay(ff, 20);
            sim->beginTimedRun(window);
        }
        // Enough ticks for a temporal run to publish its capture; a functional run settles on
        // the first.
        for (int t = 0; t < 40; ++t) {
            sim->update();
        }
        const Status q = ff->outputValue(0);
        if (window > 0) {
            sim->endTimedRun(0);
        }
        return q;
    };

    const Status functional = powerOnCapture(0);
    const Status temporal = powerOnCapture(5);

    QVERIFY2(functional == Status::Active,
             qPrintable(QString("precondition: a clock already high at power-on is a rising edge, "
                                "so D=1 must be captured; got %1")
                            .arg(static_cast<int>(functional))));
    QVERIFY2(temporal == functional,
             qPrintable(QString("power-on capture must not depend on the simulation mode: "
                                "functional=%1 temporal=%2")
                            .arg(static_cast<int>(functional))
                            .arg(static_cast<int>(temporal))));
}

void TestUnifiedTimed::testPropertyCaptureInvariantUnderDataArrival()
{
    // D is stable BEFORE the edge in every case, so the capture must not depend on how long
    // before. Q must rise exactly one propagation delay after the edge, for every gap.
    constexpr SimTime kDelay = 20;
    for (const SimTime gap : {SimTime{40}, SimTime{20}, SimTime{19}, SimTime{5}}) {
        WorkSpace workspace;
        CircuitBuilder builder(workspace.scene());
        auto *din = new InputSwitch();
        auto *clk = new InputSwitch();
        auto *ff = new DFlipFlop();
        builder.add(din, clk, ff);
        builder.connect(din, 0, ff, 0);
        builder.connect(clk, 0, ff, 1);
        Simulation *sim = builder.initSimulation();

        din->setOn(false);
        clk->setOn(false);
        sim->update();

        sim->setElementDelay(ff, kDelay);
        sim->setTimePerTick(1);

        const SimTime start = sim->currentTime();
        const SimTime edge = start + 100;
        const auto transitions = runAndRecord(sim, ff, 300, [&](int) {
            const SimTime next = sim->currentTime() + 1; // the instant this update will reach
            if (next == edge - gap) din->setOn(true);
            if (next == edge)       clk->setOn(true);
        });

        QVERIFY2(!transitions.isEmpty(),
                 qPrintable(QString("gap %1 ns: the clock edge was swallowed — Q never changed")
                                .arg(gap)));
        // One tick of slack: inputs are applied before the update that reaches the target time,
        // so the engine observes them one tick early. The property is that the capture lands a
        // propagation delay after the edge for EVERY gap — not the exact instant.
        const SimTime lag = transitions.first() - edge;
        QVERIFY2(lag >= kDelay - 1 && lag <= kDelay + 1,
                 qPrintable(QString("gap %1 ns: Q rose %2 ns after the edge, expected ~%3")
                                .arg(gap).arg(lag).arg(kDelay)));
    }
}

void TestUnifiedTimed::testPropertyBystanderLagUnaffectedByCommit()
{
    // One scene, two runs differing only in whether the flip-flop ever commits. The NOT gate
    // shares nothing with it, so its lag must be its own delay in both runs.
    constexpr SimTime kGateDelay = 90;
    QVector<SimTime> lag;
    for (const bool commit : {false, true}) {
        WorkSpace workspace;
        CircuitBuilder builder(workspace.scene());
        auto *swa = new InputSwitch();
        auto *notGate = new Not();
        auto *din = new InputSwitch();
        auto *clk = new InputSwitch();
        auto *ff = new DFlipFlop();
        builder.add(swa, notGate, din, clk, ff);
        builder.connect(swa, 0, notGate, 0);
        builder.connect(din, 0, ff, 0);
        builder.connect(clk, 0, ff, 1);
        Simulation *sim = builder.initSimulation();

        swa->setOn(false);
        din->setOn(commit); // the only difference between the two runs
        clk->setOn(false);
        sim->update();

        sim->setElementDelay(notGate, kGateDelay);
        sim->setElementDelay(ff, 20);
        sim->setTimePerTick(1);

        const SimTime start = sim->currentTime();
        const SimTime swaRise = start + 20;
        const SimTime clkRise = start + 60; // the commit lands mid-flight for the NOT
        const auto transitions = runAndRecord(sim, notGate, 300, [&](int) {
            const SimTime next = sim->currentTime() + 1;
            if (next == swaRise) swa->setOn(true);
            if (next == clkRise) clk->setOn(true);
        });

        QVERIFY2(!transitions.isEmpty(), "the bystander gate never changed");
        lag.append(transitions.first() - swaRise);
    }

    // The metamorphic core is that the two runs agree; the absolute value is asserted with one
    // tick of slack because inputs are applied before the update that reaches the target time.
    QVERIFY2(lag.at(0) == lag.at(1),
             qPrintable(QString("bystander lag was %1 ns without a commit and %2 ns with one; "
                                "a commit elsewhere must not move an unrelated gate")
                            .arg(lag.at(0)).arg(lag.at(1))));
    QVERIFY2(lag.at(0) >= kGateDelay - 1 && lag.at(0) <= kGateDelay + 1,
             qPrintable(QString("bystander lag was %1 ns; the gate's own delay is %2 ns")
                            .arg(lag.at(0)).arg(kGateDelay)));
}

void TestUnifiedTimed::testPropertyRippleLagInvariantUnderTickWindow()
{
    // A tick-sampled hop cannot express window-invariance: at a coarse window both correct and
    // deferred behaviour quantise to the same boundaries, and at a fine window the one-tick
    // deferral is invisible. The measurable signature of the deferral is the one confirmed on
    // Examples/counter.panda — at a window far wider than every delay, a ripple chain must
    // settle completely inside each tick, so temporal mode must reproduce functional mode
    // exactly. Deferring each hop by a whole tick is precisely what breaks that.
    constexpr SimTime kDelay = 20;
    constexpr SimTime kCoarseWindow = 1000; // 50x every delay in the circuit
    constexpr int kTicks = 40;

    const auto runChain = [](SimTime window) {
        WorkSpace workspace;
        CircuitBuilder builder(workspace.scene());
        auto *din = new InputSwitch();
        auto *clk = new InputSwitch();
        auto *ff1 = new DFlipFlop();
        auto *ff2 = new DFlipFlop();
        auto *d2 = new InputSwitch();
        builder.add(din, clk, ff1, ff2, d2);
        builder.connect(din, 0, ff1, 0);
        builder.connect(clk, 0, ff1, 1);
        builder.connect(d2, 0, ff2, 0);
        builder.connect(ff1, 0, ff2, 1); // FF1.Q clocks FF2
        Simulation *sim = builder.initSimulation();

        din->setOn(true);
        d2->setOn(true);
        clk->setOn(false);
        sim->update();

        sim->setElementDelay(ff1, kDelay);
        sim->setElementDelay(ff2, kDelay);
        sim->setTimePerTick(window);

        QVector<QPair<Status, Status>> perTick;
        bool level = false;
        for (int t = 1; t <= kTicks; ++t) {
            if (t % 4 == 0) {
                level = !level;
                clk->setOn(level);
            }
            sim->update();
            perTick.append({ff1->outputValue(0), ff2->outputValue(0)});
        }
        return perTick;
    };

    const auto functional = runChain(0);
    const auto temporal = runChain(kCoarseWindow);

    QCOMPARE(functional.size(), temporal.size());
    for (int t = 0; t < functional.size(); ++t) {
        QVERIFY2(functional.at(t) == temporal.at(t),
                 qPrintable(QString("tick %1: functional gave (%2,%3) but temporal at %4 ns/tick "
                                    "gave (%5,%6) — a window 50x wider than every delay must "
                                    "settle the whole chain inside each tick")
                                .arg(t)
                                .arg(static_cast<int>(functional.at(t).first))
                                .arg(static_cast<int>(functional.at(t).second))
                                .arg(kCoarseWindow)
                                .arg(static_cast<int>(temporal.at(t).first))
                                .arg(static_cast<int>(temporal.at(t).second))));
    }
}

void TestUnifiedTimed::testPropertyCaptureSurvivesPostEdgeDataChange()
{
    constexpr SimTime kDelay = 20;
    // Gaps AFTER the edge, all inside the flip-flop's own delay, plus one just outside it.
    for (const SimTime gap : {SimTime{1}, SimTime{8}, SimTime{19}, SimTime{21}}) {
        WorkSpace workspace;
        CircuitBuilder builder(workspace.scene());
        auto *din = new InputSwitch();
        auto *clk = new InputSwitch();
        auto *ff = new DFlipFlop();
        builder.add(din, clk, ff);
        builder.connect(din, 0, ff, 0);
        builder.connect(clk, 0, ff, 1);
        Simulation *sim = builder.initSimulation();

        din->setOn(true);      // D HIGH before the edge, so the capture must be 1
        clk->setOn(false);
        sim->update();
        QCOMPARE(ff->outputValue(0), Status::Inactive);

        sim->setElementDelay(ff, kDelay);
        sim->setTimePerTick(1);

        const SimTime start = sim->currentTime();
        const SimTime edge = start + 100;
        const auto transitions = runAndRecord(sim, ff, 300, [&](int) {
            const SimTime next = sim->currentTime() + 1;
            if (next == edge)       { clk->setOn(true); }
            if (next == edge + gap) { din->setOn(false); }  // AFTER the edge
        });

        QVERIFY2(!transitions.isEmpty(),
                 qPrintable(QString("gap +%1 ns: the capture was lost — Q never changed").arg(gap)));
        const SimTime lag = transitions.first() - edge;
        QVERIFY2(lag >= kDelay - 1 && lag <= kDelay + 1,
                 qPrintable(QString("gap +%1 ns: Q rose %2 ns after the edge, expected ~%3 — a data "
                                    "change after the edge must not push the transition out")
                                .arg(gap).arg(lag).arg(kDelay)));
    }
}

void TestUnifiedTimed::testShiftRegisterShiftsOneStagePerEdgeFromPowerOn()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *din = new InputSwitch();
    auto *clk = new Clock();
    auto *ff1 = new DFlipFlop();
    auto *ff2 = new DFlipFlop();
    builder.add(din, clk, ff1, ff2);
    builder.connect(din, 0, ff1, 0);   // D1
    builder.connect(clk, 0, ff1, 1);   // CLK1
    builder.connect(ff1, 0, ff2, 0);   // Q1 -> D2
    builder.connect(clk, 0, ff2, 1);   // CLK2

    din->setOn(true);

    Simulation *sim = builder.initSimulation();
    sim->update();   // first tick after initialize(): the clock was reset HIGH, so this is an edge

    QCOMPARE(ff1->outputValue(0), Status::Active);
    QVERIFY2(ff2->outputValue(0) == Status::Inactive,
             "stage 2 must sample stage 1's PRE-edge output: the seed pass has to hold Memory "
             "elements back and commit them together, exactly as the drain does");
}

void TestUnifiedTimed::testTemporalTickIsBoundedRegardlessOfWindow()
{
    WorkSpace workspace;
    QString error;
    QVERIFY2(loadExample(workspace, "temporal_ring_oscillator.panda", error), qPrintable(error));

    auto *simulation = workspace.scene()->simulation();

    // Re-initialize AFTER choosing the window. A functional pass canonicalises this ring to
    // Unknown, and NOT(Unknown) is itself a fixed point, so measuring a tick after one would
    // time a circuit that is no longer oscillating -- and pass in tens of milliseconds against
    // a bound meant for the oscillating case.
    simulation->setTimePerTick(1'000'000);   // the default speed setting
    QVERIFY(simulation->initialize());
    const SimTime before = simulation->currentTime();

    QElapsedTimer timer;
    timer.start();
    simulation->update();
    const qint64 ms = timer.elapsed();

    QVERIFY2(ms < 150,
             qPrintable(QString("one update() at 1,000,000 ns/tick took %1 ms; a tick's work must "
                                "be bounded rather than scaling with the window").arg(ms)));

    // Sim-time advances by AT MOST the window: a budgeted tick stops where the drain actually
    // got to, so the clock stays honest about how far the circuit has been simulated.
    QVERIFY2(simulation->currentTime() <= before + 1'000'000,
             "a budgeted tick must not claim more sim-time than it drained");

    // And a small window is unaffected -- the budget must not throttle ordinary stepping.
    simulation->setTimePerTick(1);
    for (int i = 0; i < 50; ++i) {
        simulation->update();
    }
}

void TestUnifiedTimed::testPropertyLagScalesWithTheDelay_data()
{
    // The multiplier is the parameter: whatever the lag is at delay D, it must be exactly k
    // times that at delay k*D. Stated as a ratio rather than as absolute instants, so the
    // property does not encode any particular delay as correct.
    QTest::addColumn<int>("multiplier");
    QTest::newRow("x2")  << 2;
    QTest::newRow("x3")  << 3;
    QTest::newRow("x10") << 10;
}

void TestUnifiedTimed::testPropertyLagScalesWithTheDelay()
{
    QFETCH(int, multiplier);
    constexpr SimTime kBaseDelay = 6;

    // Returns the sim time elapsed between the input rising and the output falling.
    const auto lagWithDelay = [](const SimTime delay) -> SimTime {
        WorkSpace workspace;
        CircuitBuilder builder(workspace.scene());
        auto *sw = new InputSwitch();
        auto *notGate = new Not();
        builder.add(sw, notGate);
        builder.connect(sw, 0, notGate, 0);
        Simulation *sim = builder.initSimulation();

        sw->setOn(false);
        sim->update();

        sim->setElementDelay(notGate, delay);
        sim->setTimePerTick(1);

        const SimTime t0 = sim->currentTime();
        sw->setOn(true);
        int guard = 0;
        while (notGate->outputValue(0) != Status::Inactive) {
            sim->update();
            if (++guard > 100000) {
                return 0; // never transitioned; the caller's comparison will fail loudly
            }
        }
        return sim->currentTime() - t0;
    };

    const SimTime base = lagWithDelay(kBaseDelay);
    QVERIFY2(base > 0, "precondition: the output must actually transition at the base delay");
    QCOMPARE(base, kBaseDelay);

    const SimTime scaled = lagWithDelay(kBaseDelay * static_cast<SimTime>(multiplier));
    QCOMPARE(scaled, base * static_cast<SimTime>(multiplier));
}

void TestUnifiedTimed::testPropertyBothModesReachTheSameSteadyState_data()
{
    // Temporal windows only. Functional is the reference the rows are compared against, so it
    // is not itself a row.
    QTest::addColumn<quint64>("nsPerTick");
    QTest::newRow("1 ns/tick")    << quint64{1};
    QTest::newRow("7 ns/tick")    << quint64{7};
    QTest::newRow("100 ns/tick")  << quint64{100};
    QTest::newRow("1000 ns/tick") << quint64{1000};
}

void TestUnifiedTimed::testPropertyBothModesReachTheSameSteadyState()
{
    // MODE AGREEMENT: delays change WHEN a value appears, never WHAT it is. Once every delay
    // has elapsed, a temporal run must hold exactly what the same stimulus produces in
    // functional mode -- across a sequential element, so the agreement covers a captured state
    // and not just combinational output. This is the property the whole "one engine, two modes"
    // design rests on, and only individual instances of it were pinned.
    QFETCH(quint64, nsPerTick);

    // Drives a D flip-flop through three clock edges with changing data, returning what the
    // flip-flop and a gate reading it hold at the end.
    const auto run = [](const SimTime window) -> QVector<Status> {
        WorkSpace workspace;
        CircuitBuilder builder(workspace.scene());
        auto *swD = new InputSwitch();
        auto *swClk = new InputSwitch();
        auto *ff = new DFlipFlop();
        auto *reader = new Not();
        builder.add(swD, swClk, ff, reader);
        builder.connect(swD, 0, ff, 0);
        builder.connect(swClk, 0, ff, 1);
        builder.connect(ff, 0, reader, 0);
        Simulation *sim = builder.initSimulation();

        swD->setOn(false);
        swClk->setOn(false);
        sim->update();

        sim->setTimePerTick(window);

        // Settles the circuit: in functional mode one tick suffices; in temporal mode run out
        // far past the longest per-element delay so every pending publication has landed.
        const auto settle = [&] {
            if (window == 0) {
                sim->update();
                return;
            }
            const SimTime deadline = sim->currentTime() + 500;
            int guard = 0;
            while (sim->currentTime() < deadline && ++guard < 100000) {
                sim->update();
            }
        };

        for (const bool data : {true, false, true}) {
            swD->setOn(data);
            swClk->setOn(false);
            settle();
            swClk->setOn(true);
            settle();
        }
        return {ff->outputValue(0), reader->outputValue(0)};
    };

    const QVector<Status> functional = run(0);
    QVERIFY2(functional.at(0) != Status::Unknown,
             "precondition: the functional reference must be a definite captured value");

    QCOMPARE(run(static_cast<SimTime>(nsPerTick)), functional);
}
