// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestSimulationTraceSnapshot.h"

#include <algorithm>
#include <chrono>
#include <memory>
#include <optional>
#include <thread>
#include <utility>

#include <QBitArray>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QTest>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInput.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationThrottleDisabler.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

namespace SimulationTraceSnapshot {

/// Steps of the seeded walk an element or a circuit goes through.
constexpr int kWalkSteps = 160;
constexpr int kCircuitWalkSteps = 120;

/// The most inputs driven through every combination: a Mux's eleven, 2048 steps. Only the 14- and 16-segment displays
/// have more, and they show what they receive.
constexpr int kMaxExhaustiveInputs = 11;

/// Steps per line of an element's trace.
constexpr qsizetype kStepsPerLine = 32;

/// The widest step a circuit's trace writes out; a wider one is written as a hash of itself.
constexpr qsizetype kMaxStepWidth = 120;

/// A seeded 31-bit linear congruential generator: the same sequence on every platform and compiler.
class Lcg
{
public:
    explicit Lcg(const quint32 seed)
        : m_state(seed & 0x7fffffffU)
    {
    }

    quint32 next()
    {
        m_state = (m_state * 1103515245U + 12345U) & 0x7fffffffU;
        return m_state;
    }

    bool nextBit() { return ((next() >> 16) & 1U) != 0U; }

    /// A value from 0 to \a bound - 1.
    int below(const int bound) { return static_cast<int>((next() >> 8) % static_cast<quint32>(bound)); }

private:
    quint32 m_state;
};

/// FNV-1a over \a text's UTF-8 bytes, so each run's walk is seeded by its name; qHash() is seeded per process.
quint32 seedOf(const QString &text)
{
    quint32 hash = 2166136261U;
    const QByteArray bytes = text.toUtf8();
    for (const char byte : bytes) {
        hash ^= static_cast<quint32>(static_cast<quint8>(byte));
        hash *= 16777619U;
    }
    return hash;
}

QString statuses(const QVector<Status> &values)
{
    QString text;
    for (const Status status : values) {
        switch (status) {
        case Status::Inactive: text += QLatin1Char('0'); break;
        case Status::Active:   text += QLatin1Char('1'); break;
        case Status::Error:    text += QLatin1Char('E'); break;
        case Status::Unknown:  text += QLatin1Char('U'); break;
        }
    }
    return text;
}

/**
 * \brief What \a element drives, read from its output ports, or, when it drives nothing, what its input ports carry,
 * which is what a display or an Led paints from: the 7-, 14- and 16-segment displays have no logic of their own, so
 * nothing refreshes their simulation inputs.
 */
QVector<Status> observed(const Element *element)
{
    QVector<Status> values;
    if (element->outputSize() > 0) {
        for (int index = 0; index < element->outputSize(); ++index) {
            values << element->outputPort(index)->status();
        }
        return values;
    }
    for (int index = 0; index < element->inputSize(); ++index) {
        values << element->inputPort(index)->status();
    }
    return values;
}

/// Steps \a scene's simulation by hand: one update() per step, every phase complete whatever the interactive mode
/// says, its timer never started, so a clock keeps the level the first update gives it.
class Stepper
{
public:
    explicit Stepper(Scene *scene)
        : m_simulation(scene->simulation())
        , m_noThrottle(m_simulation)
    {
    }

    /// Whether the simulation's timer runs, which would let clocks follow the wall clock.
    bool timerRuns() { return m_simulation->isRunning(); }

    void step() { m_simulation->update(); }

private:
    Simulation *m_simulation;
    SimulationThrottleDisabler m_noThrottle;
};

/// Drives \a input one step of \a walk: a rotary selects one of its positions, any other input takes a level.
void drive(ElementInput *input, Lcg &walk)
{
    // Drawn one after the other, not as two arguments of one call, whose order the compiler picks.
    const bool level = walk.nextBit();
    const int position = walk.below(input->outputSize());
    input->setOn(level, position);
}

/// Writes \a steps kStepsPerLine to a line, each line led by its first step's index.
QStringList traceLines(const QStringList &steps)
{
    QStringList lines;
    for (qsizetype first = 0; first < steps.size(); first += kStepsPerLine) {
        lines << QString("  %1: %2").arg(QString::number(first), steps.mid(first, kStepsPerLine).join(' '));
    }
    return lines;
}

/// Types left out of the element traces: the IC is traced through the circuits; a clock follows the wall clock,
/// which stepping cannot replay; the buzzer and the audio box start playback and only receive, as an Led does; text
/// and lines have no simulation behaviour.
bool isTraced(const ElementType type)
{
    switch (type) {
    case ElementType::IC:
    case ElementType::Clock:
    case ElementType::AudioBox:
    case ElementType::Buzzer:
    case ElementType::Text:
    case ElementType::Line:
        return false;
    default:
        return true;
    }
}

/**
 * \brief One run of \a type with \a inputs and \a outputs ports. An element with inputs gets a switch on each, all
 * wired unless \a lastOpen leaves the last one unwired, and goes through every combination of the wired ones and then
 * a seeded walk; an input element is driven itself by a seeded walk.
 * \return The run's lines, or an empty list after failing the running test.
 */
QStringList elementRun(const ElementType type, const int inputs, const int outputs, const bool lastOpen)
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *element = ElementFactory::buildElement(type);
    TestUtils::applyPortCounts(element, inputs, outputs);
    const QString mode = (element->inputSize() == 0) ? QString("direct") : QString(lastOpen ? "last-open" : "connected");
    const QString name = QString("%1 in=%2 out=%3 %4")
                             .arg(ElementFactory::typeToText(type), QString::number(element->inputSize()),
                                  QString::number(element->outputSize()), mode);

    if (auto *table = qobject_cast<TruthTable *>(element)) {
        Lcg bits(seedOf(name + " key"));
        QBitArray key(table->key().size());
        for (qsizetype bit = 0; bit < key.size(); ++bit) {
            key.setBit(bit, bits.nextBit());
        }
        table->setkey(key);
    }
    element->setPos(400, 0);
    scene->addItem(element);

    const int wired = std::max(element->inputSize() - (lastOpen ? 1 : 0), 0);
    QList<InputSwitch *> switches;
    for (int index = 0; index < wired; ++index) {
        auto *source = new InputSwitch();
        source->setPos(0, index * 80);
        scene->addItem(source);
        auto *wire = new Connection();
        wire->setStartPort(source->outputPort());
        wire->setEndPort(element->inputPort(index));
        scene->addItem(wire);
        switches << source;
    }

    Stepper stepper(scene);
    if (stepper.timerRuns()) {
        QTest::qFail("the simulation's timer runs", __FILE__, __LINE__);
        return {};
    }
    QStringList steps;
    const auto step = [&] {
        stepper.step();
        steps << statuses(observed(element));
    };

    step();
    if ((wired > 0) && (wired <= kMaxExhaustiveInputs)) {
        for (int pattern = 0; pattern < (1 << wired); ++pattern) {
            for (int index = 0; index < wired; ++index) {
                switches.at(index)->setOn(((pattern >> index) & 1) != 0);
            }
            step();
        }
    }

    Lcg walk(seedOf(name));
    auto *input = (element->inputSize() == 0) ? dynamic_cast<ElementInput *>(element) : nullptr;
    if ((wired > 0) || input) {
        for (int index = 0; index < kWalkSteps; ++index) {
            if (input) {
                drive(input, walk);
            }
            for (auto *source : std::as_const(switches)) {
                source->setOn(walk.nextBit());
            }
            step();
        }
    }

    return QStringList{"T " + name} + traceLines(steps);
}

/// Every traced type at the fewest and at the most ports it takes, connected and with its last input open.
QString elementTraces()
{
    QStringList lines;
    for (const ElementType type : TestUtils::dumpedTypes()) {
        if (!ElementFactory::hasCreator(type)) {
            lines << QString("%1 %2").arg(QLatin1String(TestUtils::kNoCreator), TestUtils::snapshotType(type));
            continue;
        }
        if (!isTraced(type)) {
            continue;
        }

        const std::unique_ptr<Element> probe(ElementFactory::buildElement(type));
        QList<std::pair<int, int>> counts{{probe->minInputSize(), probe->minOutputSize()}};
        if ((probe->maxInputSize() != probe->minInputSize()) || (probe->maxOutputSize() != probe->minOutputSize())) {
            counts.append({probe->maxInputSize(), probe->maxOutputSize()});
        }

        for (const auto &[inputs, outputs] : std::as_const(counts)) {
            lines << elementRun(type, inputs, outputs, false);
            if (probe->maxInputSize() > 0) {
                lines << elementRun(type, inputs, outputs, true);
            }
            if (QTest::currentTestFailed()) {
                return {};
            }
        }
    }
    return lines.join('\n') + '\n';
}

/// One step of a circuit: what each observed element produced, or a hash of that when it is too wide to read.
QString circuitStep(const QList<const Element *> &elements)
{
    QStringList parts;
    for (const auto *element : elements) {
        parts << statuses(observed(element));
    }
    const QString text = parts.join(' ');
    if (text.size() <= kMaxStepWidth) {
        return text;
    }
    return '#' + QString::fromLatin1(QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Md5).toHex().left(16));
}

/// The circuit in \a filePath through \a steps steps of its seeded walk, its clocks never toggling: every input
/// element other than a clock is driven, in id order, and every output element and IC observed.
QString circuitTrace(const QString &filePath, const int steps)
{
    WorkSpace workspace;
    TestUtils::loadCircuit(workspace, filePath);
    if (QTest::currentTestFailed()) {
        return {};
    }
    auto *scene = workspace.scene();

    QList<ElementInput *> inputs;
    QList<const Element *> traced;
    for (auto *element : TestUtils::elementsById(scene)) {
        auto *input = dynamic_cast<ElementInput *>(element);
        if (input && (element->elementType() != ElementType::Clock)) {
            inputs << input;
        }
        if ((element->elementGroup() == ElementGroup::Output) || (element->elementType() == ElementType::IC)) {
            traced << element;
        }
    }

    Stepper stepper(scene);
    if (stepper.timerRuns()) {
        QTest::qFail("the simulation's timer runs", __FILE__, __LINE__);
        return {};
    }
    QStringList lines{QString("inputs=%1 observed=%2").arg(QString::number(inputs.size()), QString::number(traced.size()))};
    stepper.step();
    lines << QString("  0: %1").arg(circuitStep(traced));

    Lcg walk(seedOf(QFileInfo(filePath).completeBaseName()));
    for (int step = 1; step <= steps; ++step) {
        for (auto *input : std::as_const(inputs)) {
            drive(input, walk);
        }
        stepper.step();
        lines << QString("  %1: %2").arg(QString::number(step), circuitStep(traced));
    }
    return lines.join('\n') + '\n';
}

/// A directory of traced circuits and the subdirectory of the goldens that trace them.
struct CircuitSet {
    QString goldens;
    QString sources;
};

QList<CircuitSet> circuitSets()
{
    const QString testsDir(QUOTE(CURRENTDIR));
    return {
        {"Fixtures", testsDir + "/Fixtures/"},
        {"Components", TestUtils::cpuComponentsDir()},
        {"Perf", testsDir + "/Fixtures/Perf/"},
    };
}

/// The traced circuits of \a set by name, without extension, and the steps each runs: every one in the directory, or
/// for the perf set, the named ones.
QList<std::pair<QString, int>> tracedCircuits(const CircuitSet &set)
{
    if (set.goldens == "Perf") {
        // Named, not listed: the other perf fixtures are not traced, and these are generated, so a missing one must
        // fail rather than drop out. ic_heavy has one switch to walk.
        return {{"perf_ic_4gate", kCircuitWalkSteps}, {"ic_heavy", 24}};
    }
    QList<std::pair<QString, int>> circuits;
    const auto files = QDir(set.sources).entryInfoList({"*.panda"}, QDir::Files, QDir::Name);
    for (const auto &file : files) {
        circuits.append({file.completeBaseName(), kCircuitWalkSteps});
    }
    return circuits;
}

QString goldensDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/SimulationTraceSnapshot/";
}

} // namespace SimulationTraceSnapshot

void TestSimulationTraceSnapshot::testElementTracesMatchGolden()
{
    const QString traces = SimulationTraceSnapshot::elementTraces();
    if (QTest::currentTestFailed()) {
        return;
    }
    const QString mismatch = TestUtils::compareWithGolden(SimulationTraceSnapshot::goldensDir() + "elements.txt", traces);
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}

void TestSimulationTraceSnapshot::testCircuitTracesMatchGolden_data()
{
    QTest::addColumn<QString>("filePath");
    QTest::addColumn<QString>("goldenPath");
    QTest::addColumn<int>("steps");

    for (const auto &set : SimulationTraceSnapshot::circuitSets()) {
        for (const auto &[name, steps] : SimulationTraceSnapshot::tracedCircuits(set)) {
            QTest::newRow(qPrintable(set.goldens + '/' + name))
                << set.sources + name + ".panda" << SimulationTraceSnapshot::goldensDir() + set.goldens + '/' + name + ".txt" << steps;
        }
    }
}

void TestSimulationTraceSnapshot::testCircuitTracesMatchGolden()
{
    QFETCH(QString, filePath);
    QFETCH(QString, goldenPath);
    QFETCH(int, steps);

    const QString missing = TestUtils::requireInputFile(filePath);
    QVERIFY2(missing.isEmpty(), qPrintable(missing));
    const QString trace = SimulationTraceSnapshot::circuitTrace(filePath, steps);
    if (QTest::currentTestFailed()) {
        return;
    }
    const QString mismatch = TestUtils::compareWithGolden(goldenPath, trace);
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}

void TestSimulationTraceSnapshot::testClocksHoldWhileStepped()
{
    // The traced circuits' clocks run at 1 Hz and their runs end inside one half-period, so their goldens would not
    // show a clock that follows the wall clock: this one would toggle on every step, and already within the first, whose
    // rebuild sets it HIGH and takes longer than its half-period.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *clock = new Clock();
    clock->setFrequency(1000.0);
    scene->addItem(clock);
    auto *led = new Led();
    led->setPos(200, 0);
    scene->addItem(led);
    auto *wire = new Connection();
    wire->setStartPort(clock->outputPort());
    wire->setEndPort(led->inputPort());
    scene->addItem(wire);

    SimulationTraceSnapshot::Stepper stepper(scene);
    QVERIFY(!stepper.timerRuns());
    stepper.step();
    QCOMPARE(led->inputPort()->status(), Status::Active);
    for (int step = 1; step <= 8; ++step) {
        // Two half-periods, so an advancing clock is due to toggle whenever this step's update runs.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        stepper.step();
        QCOMPARE(led->inputPort()->status(), Status::Active);
    }
}

void TestSimulationTraceSnapshot::testEveryGoldenHasACircuit()
{
    // A golden whose circuit was removed or renamed would otherwise stay in the tree, compared with nothing.
    for (const auto &set : SimulationTraceSnapshot::circuitSets()) {
        QStringList traced;
        for (const auto &circuit : SimulationTraceSnapshot::tracedCircuits(set)) {
            traced << circuit.first + ".txt";
        }
        const auto goldens = QDir(SimulationTraceSnapshot::goldensDir() + set.goldens).entryList({"*.txt"}, QDir::Files, QDir::Name);
        for (const QString &golden : goldens) {
            QVERIFY2(traced.contains(golden), qPrintable(QString("%1/%2 traces no circuit").arg(set.goldens, golden)));
        }
    }
}
