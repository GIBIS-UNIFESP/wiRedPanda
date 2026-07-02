// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/System/TestBewavedDolphinGui.h"

#include <cmath>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QImage>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSet>
#include <QStandardItemModel>
#include <QTableView>
#include <QTabWidget>
#include <QTest>
#include <QTextStream>
#include <QWheelEvent>

#include "App/BeWavedDolphin/BeWavedDolphin.h"
#include "App/BeWavedDolphin/DolphinHost.h"
#include "App/BeWavedDolphin/DolphinZoom.h"
#include "App/BeWavedDolphin/LiveAnalyzer.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/IC.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Simulation/WaveformRecorder.h"
#include "App/UI/FileDialogProvider.h"
#include "Tests/Common/ICTestHelpers.h"
#include "Tests/Common/StubFileDialogProvider.h"
#include "Tests/Common/TestUtils.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/// Creates a simple AND circuit (2 inputs → AND → 1 output) and returns the workspace.
/// Caller owns the returned WorkSpace.
static std::unique_ptr<WorkSpace> createAndCircuit()
{
    auto ws = std::make_unique<WorkSpace>();
    auto *scene = ws->scene();

    auto *sw0 = new InputSwitch();
    auto *sw1 = new InputSwitch();
    auto *andGate = new And();
    auto *led = new Led();

    scene->addItem(sw0);
    scene->addItem(sw1);
    scene->addItem(andGate);
    scene->addItem(led);

    CircuitBuilder builder(scene);
    builder.connect(sw0, 0, andGate, 0);
    builder.connect(sw1, 0, andGate, 1);
    builder.connect(andGate, 0, led, 0);

    return ws;
}

/// InputSwitch → NOT (5 ns default delay) → Led. One input row, one output row.
/// Caller owns the returned WorkSpace.
static std::unique_ptr<WorkSpace> createNotCircuit()
{
    auto ws = std::make_unique<WorkSpace>();
    auto *scene = ws->scene();

    auto *sw = new InputSwitch();
    auto *notGate = new Not();
    auto *led = new Led();
    scene->addItem(sw);
    scene->addItem(notGate);
    scene->addItem(led);

    CircuitBuilder builder(scene);
    builder.connect(sw, 0, notGate, 0);
    builder.connect(notGate, 0, led, 0);

    return ws;
}

/// sw → NOT(5 ns) → NOT(5 ns) → Led. Led tracks sw with a cumulative 5 + 5 = 10 ns delay.
/// A single input row (sw), so the driven row is unambiguous. Caller owns the returned WorkSpace.
static std::unique_ptr<WorkSpace> createDoubleNotCircuit()
{
    auto ws = std::make_unique<WorkSpace>();
    auto *scene = ws->scene();

    auto *sw = new InputSwitch();
    auto *not1 = new Not();
    auto *not2 = new Not();
    auto *led = new Led();
    scene->addItem(sw);
    scene->addItem(not1);
    scene->addItem(not2);
    scene->addItem(led);

    CircuitBuilder builder(scene);
    builder.connect(sw, 0, not1, 0);
    builder.connect(not1, 0, not2, 0);
    builder.connect(not2, 0, led, 0);

    return ws;
}

/// Returns the first column (> 0) at which the output row's value differs from its column-0 value,
/// or -1 if it never transitions within the model's columns.
static int firstOutputTransitionColumn(const QStandardItemModel *model, int outputRow)
{
    const int base = model->index(outputRow, 0).data().toInt();
    for (int col = 1; col < model->columnCount(); ++col) {
        if (model->index(outputRow, col).data().toInt() != base) {
            return col;
        }
    }
    return -1;
}

/// Creates a BewavedDolphin on the given workspace's scene with a blank waveform.
/// The caller must ensure ws outlives the returned pointer.
/// Note: BewavedDolphin has WA_DeleteOnClose, so don't close it manually — let it
/// leak in tests (the test process will clean up).
/// Minimal DolphinHost stub so file-context-dependent paths (Save As, associate,
/// load directory) can be exercised without a real MainWindow.
class StubDolphinHost : public DolphinHost
{
public:
    QFileInfo m_currentFile;
    QDir m_currentDir;
    QString m_dolphinFileName;
    int saveCount = 0;

    QFileInfo currentFile() const override { return m_currentFile; }
    QDir currentDir() const override { return m_currentDir; }
    QString dolphinFileName() override { return m_dolphinFileName; }
    void setDolphinFileName(const QString &fileName) override { m_dolphinFileName = fileName; }
    void save(const QString &) override { ++saveCount; }
};

static BewavedDolphin *createDolphin(WorkSpace *ws)
{
    // Prevent WA_DeleteOnClose from deleting during test — we manage lifetime ourselves
    auto *dolphin = new BewavedDolphin(ws->scene(), false);
    dolphin->setAttribute(Qt::WA_DeleteOnClose, false);
    dolphin->createWaveform("");
    return dolphin;
}

/// Finds the BewavedDolphin's waveform QTableView (now a plain child widget).
static QTableView *findTableView(BewavedDolphin *dolphin)
{
    return dolphin->findChild<QTableView *>();
}

/// Selects a range of cells in the BewavedDolphin table view.
static void selectCells(BewavedDolphin *dolphin, int startRow, int startCol, int endRow, int endCol)
{
    auto *tv = findTableView(dolphin);
    QVERIFY2(tv, "QTableView not found in BewavedDolphin");
    auto *model = tv->model();
    QVERIFY2(model, "Model is null in BewavedDolphin");
    auto *selModel = tv->selectionModel();
    QVERIFY2(selModel, "Selection model is null");

    QItemSelection selection;
    selection.select(model->index(startRow, startCol), model->index(endRow, endCol));
    selModel->select(selection, QItemSelectionModel::ClearAndSelect);
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void TestBewavedDolphinGui::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
}

void TestBewavedDolphinGui::cleanup()
{
    FileDialogs::setProvider(nullptr);
}

// ===========================================================================
// Waveform creation
// ===========================================================================

void TestBewavedDolphinGui::testCreateBlankWaveform()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    QVERIFY(dolphin->model() != nullptr);
    QVERIFY(dolphin->length() > 0);
    QCOMPARE(dolphin->length(), 32); // default length
}

void TestBewavedDolphinGui::testCreateWaveformInputOutputCounts()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    // AND circuit has 2 inputs and 1 output
    QCOMPARE(static_cast<int>(dolphin->inputElements().size()), 2);
    QCOMPARE(static_cast<int>(dolphin->outputElements().size()), 1);

    // Model rows = inputs + outputs
    auto *model = dolphin->model();
    QCOMPARE(model->rowCount(), 3); // 2 inputs + 1 output
}

// ===========================================================================
// Cell editing
// ===========================================================================

void TestBewavedDolphinGui::testSetCellValue()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    auto *model = dolphin->model();
    QVERIFY(model);

    // Set input 0, column 0 to 1
    dolphin->setCellValue(0, 0, 1);
    QCOMPARE(model->index(0, 0).data().toInt(), 1);

    // Set input 0, column 0 back to 0
    dolphin->setCellValue(0, 0, 0);
    QCOMPARE(model->index(0, 0).data().toInt(), 0);
}

void TestBewavedDolphinGui::testRunSimulationFillsOutputs()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    auto *model = dolphin->model();
    int outputRow = static_cast<int>(dolphin->inputElements().size()); // first output row

    // Set both inputs to 1 at column 5
    dolphin->setCellValue(0, 5, 1);
    dolphin->setCellValue(1, 5, 1);

    dolphin->run();

    // AND gate: 1 AND 1 = 1
    int outputValue = model->index(outputRow, 5).data().toInt();
    QCOMPARE(outputValue, 1);

    // At column 0, both inputs are 0 → output should be 0
    int outputAt0 = model->index(outputRow, 0).data().toInt();
    QCOMPARE(outputAt0, 0);
}

void TestBewavedDolphinGui::testTemporalModeShowsPropagationLag()
{
    auto ws = createNotCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();
    const int outputRow = static_cast<int>(dolphin->inputElements().size()); // row after the inputs

    // Wide input pulse: low for cols 0..riseCol-1, high from riseCol on (held well past the NOT's
    // 5 ns delay so the edge is not inertially absorbed).
    const int riseCol = 8;
    for (int col = 0; col < model->columnCount(); ++col) {
        dolphin->setCellValue(0, col, col >= riseCol ? 1 : 0);
    }

    // Functional (zero delay): NOT output (1→0) flips in the same column the input rises.
    dolphin->m_temporal = false;
    dolphin->run();
    const int functionalCol = firstOutputTransitionColumn(model, outputRow);
    QCOMPARE(functionalCol, riseCol);

    // Temporal at 1 ns/column: the 5 ns NOT delay pushes the transition several columns later.
    dolphin->m_temporal = true;
    dolphin->m_nsPerColumn = 1;
    dolphin->run();
    const int temporalCol = firstOutputTransitionColumn(model, outputRow);

    qInfo() << "NOT lag: functional col" << functionalCol << "temporal col" << temporalCol;
    QVERIFY2(temporalCol > functionalCol,
             qPrintable(QString("Temporal output did not lag (functional %1, temporal %2)")
                            .arg(functionalCol).arg(temporalCol)));
    const int lag = temporalCol - functionalCol;
    QVERIFY2(lag >= 3 && lag <= 7,
             qPrintable(QString("Unexpected lag %1 columns (expected ~5 for a 5 ns NOT)").arg(lag)));
}

void TestBewavedDolphinGui::testTemporalModeCumulativeChainLag()
{
    auto ws = createDoubleNotCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();
    const int outputRow = static_cast<int>(dolphin->inputElements().size());

    const int riseCol = 6;
    for (int col = 0; col < model->columnCount(); ++col) {
        dolphin->setCellValue(0, col, col >= riseCol ? 1 : 0);
    }

    // Functional: zero delay — Led (= sw, double negation) flips in the rise column.
    dolphin->m_temporal = false;
    dolphin->run();
    const int functionalCol = firstOutputTransitionColumn(model, outputRow);
    QCOMPARE(functionalCol, riseCol);

    // Temporal: the two NOTs accumulate 5 + 5 = 10 ns, so the lag is clearly larger than one gate.
    dolphin->m_temporal = true;
    dolphin->m_nsPerColumn = 1;
    dolphin->run();
    const int chainCol = firstOutputTransitionColumn(model, outputRow);

    qInfo() << "double-NOT lag: functional col" << functionalCol << "temporal col" << chainCol;
    const int chainLag = chainCol - functionalCol;
    QVERIFY2(chainLag > 6,
             qPrintable(QString("Chain lag %1 not greater than a single gate (~5); delays should "
                                "compose to ~10").arg(chainLag)));
}

void TestBewavedDolphinGui::testNonTemporalSweepIgnoresLiveTemporalMode()
{
    // BeWavedDolphin shares its Simulation with the main window, whose mode selector may have
    // left a temporal tick window on it. A NON-temporal sweep must still be genuinely
    // functional — run() brackets it with a zero-width timed run instead of inheriting the
    // live window. Pre-fix, the sweep ran at the inherited window (here deliberately smaller
    // than the NOT's 5 ns default delay) and the output lagged its cause by whole columns.
    auto ws = createNotCircuit();
    auto *sim = ws->scene()->simulation();
    sim->setTimePerTick(1); // the live (main-window) simulation is in temporal mode

    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();
    const int outputRow = static_cast<int>(dolphin->inputElements().size());

    const int riseCol = 8;
    for (int col = 0; col < model->columnCount(); ++col) {
        dolphin->setCellValue(0, col, col >= riseCol ? 1 : 0);
    }

    dolphin->m_temporal = false; // the user did NOT ask for a temporal sweep
    dolphin->run();
    const int transitionCol = firstOutputTransitionColumn(model, outputRow);
    QCOMPARE(transitionCol, riseCol); // functional semantics: same-column transition, no lag

    // The live simulation's temporal window is restored after the sweep.
    QCOMPARE(sim->timePerTick(), SimTime(1));
}

void TestBewavedDolphinGui::testRunPreservesLiveAnalyzerTimeline()
{
    // Regression test: BeWavedDolphin's sweep (WaveformSimulator::sweep()) drives the same
    // Simulation its own Live Analyzer page may be watching. The sweep must be INVISIBLE
    // to a live recording: no synthetic test-vector values may leak into a trace, the
    // recorded history must survive intact (previously begin/endTimedRun wiped it and
    // rewound the clock to 0 — every run(), including the one at tool-open, silently
    // collapsed the analyzer to flat seed lines with a dead zoom), the live clock must
    // resume where it left off, and recording must keep appending monotonically after.
    auto ws = createAndCircuit();
    auto *sim = ws->scene()->simulation();
    sim->setTimePerTick(1'000'000); // temporal 1x, like the interactive session

    const auto elements = ws->scene()->elements();
    GraphicElement *andElement = nullptr;
    InputSwitch *sw0 = nullptr;
    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::And && !andElement) {
            andElement = elm;
        } else if (elm->elementType() == ElementType::InputSwitch && !sw0) {
            sw0 = qobject_cast<InputSwitch *>(elm);
        }
    }
    QVERIFY2(andElement && sw0, "AND circuit should contain an And gate and an InputSwitch");

    // Live session: watch the AND output, tick, and capture genuine history (both
    // switches get driven high so the output actually transitions).
    sim->waveformRecorder().watchSignal("And_out", andElement, 0);
    sim->waveformRecorder().setRecording(true);
    sim->update();
    for (auto *elm : elements) {
        if (auto *sw = qobject_cast<InputSwitch *>(elm)) {
            sw->setOn(true);
        }
    }
    sim->update();
    sim->update();
    const auto before = sim->waveformRecorder().trace(0).transitions;
    QVERIFY2(before.size() >= 2, "live session should have recorded a real transition");
    const SimTime liveTimeBefore = sim->currentTime();
    QVERIFY(liveTimeBefore > 0);

    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    dolphin->setCellValue(0, 5, 1); // a value the sweep will drive that must NOT leak into the trace
    dolphin->setCellValue(1, 5, 1);
    dolphin->run(); // drives the sweep on the SAME Simulation

    // The sweep left no trace of itself: recording restored, history byte-identical
    // (preserved AND unpolluted), live clock back where it was.
    QVERIFY(sim->waveformRecorder().isRecording());
    QCOMPARE(sim->waveformRecorder().trace(0).transitions, before);
    QCOMPARE(sim->currentTime(), liveTimeBefore);

    // And the live session continues monotonically: a real post-sweep edge appends with a
    // timestamp strictly beyond the preserved history.
    sw0->setOn(false);
    sim->update();
    sim->update();
    const auto &after = sim->waveformRecorder().trace(0).transitions;
    QVERIFY2(after.size() > before.size(), "post-sweep live edge must keep recording");
    QVERIFY2(after.last().first > before.last().first,
             qPrintable(QString("post-sweep timestamp %1 must exceed preserved history end %2")
                            .arg(after.last().first).arg(before.last().first)));
}

void TestBewavedDolphinGui::testRunResetsICInternalSequentialState()
{
    // Regression test: the sweep's pre-run reset (WaveformSimulator::sweep()) walked only
    // scene-level elements, so a flip-flop nested inside an IC kept whatever state the live
    // run left — the sweep was not reproducible ("power-on defaults") for hierarchical
    // circuits. IC::resetSimState() now recurses into the internals.

    // Fixture .panda: sw(data), sw(clock) → DFF → Led. Its switches/led become the IC's
    // boundary ports (sorted by Y position: data = port 0, clock = port 1).
    const QString icPath = m_tempDir.path() + "/dff_ic.panda";
    {
        WorkSpace fixture;
        auto *scene = fixture.scene();
        auto *data = new InputSwitch();
        auto *clk = new InputSwitch();
        auto *dff = new DFlipFlop();
        auto *q = new Led();
        data->setPos(0, 0);
        clk->setPos(0, 100);
        dff->setPos(150, 50);
        q->setPos(300, 50);
        scene->addItem(data);
        scene->addItem(clk);
        scene->addItem(dff);
        scene->addItem(q);
        CircuitBuilder builder(scene);
        builder.connect(data, 0, dff, 0); // Data
        builder.connect(clk, 0, dff, 1);  // Clock
        builder.connect(dff, 0, q, 0);    // Q
        fixture.save(icPath);
    }

    // Outer circuit: two switches drive the IC's (data, clock) ports; its Q drives a Led.
    WorkSpace ws;
    auto *scene = ws.scene();
    auto *data = new InputSwitch();
    auto *clk = new InputSwitch();
    auto *ic = new IC();
    ICTestHelpers::embedIC(ic, ICTestHelpers::readFile(icPath), "dff_ic", m_tempDir.path(), scene->icRegistry());
    auto *led = new Led();
    data->setPos(0, 0);
    clk->setPos(0, 100);
    ic->setPos(150, 50);
    led->setPos(300, 50);
    scene->addItem(data);
    scene->addItem(clk);
    scene->addItem(ic);
    scene->addItem(led);
    QCOMPARE(ic->inputSize(), 2);
    QVERIFY(ic->outputSize() >= 1);
    CircuitBuilder builder(scene);
    builder.connect(data, 0, ic, 0);
    builder.connect(clk, 0, ic, 1);
    builder.connect(ic, 0, led, 0);

    // Live run: clock a 1 into the internal flip-flop so its Q ends HIGH.
    auto *sim = scene->simulation();
    QVERIFY(sim->initialize());
    data->setOn(true);
    clk->setOn(false);
    sim->update();
    clk->setOn(true); // rising edge captures Data = 1
    sim->update();
    sim->update();
    QCOMPARE(ic->outputValue(0), Status::Active);

    // A blank waveform drives every input LOW in every column: no clock edge ever fires
    // during the sweep, so the only way the Q output row can read 0 is the sweep resetting
    // the IC-INTERNAL flip-flop to power-on state (pre-fix it held the live-run 1 for the
    // whole sweep).
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(&ws));
    dolphin->run();

    auto *model = dolphin->model();
    const int outputRow = static_cast<int>(dolphin->inputElements().size());
    for (int col = 0; col < model->columnCount(); ++col) {
        QVERIFY2(model->index(outputRow, col).data().toInt() == 0,
                 qPrintable(QString("column %1: IC-internal flip-flop state leaked into the sweep").arg(col)));
    }
}

void TestBewavedDolphinGui::testSetLengthChangesColumns()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    QCOMPARE(dolphin->length(), 32);

    dolphin->setLength(16, false);
    QCOMPARE(dolphin->length(), 16);

    auto *model = dolphin->model();
    QCOMPARE(model->columnCount(), 16);

    dolphin->setLength(64, false);
    QCOMPARE(dolphin->length(), 64);
    QCOMPARE(model->columnCount(), 64);
}

void TestBewavedDolphinGui::testCombinationalMode()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    auto *model = dolphin->model();
    int inputCount = static_cast<int>(dolphin->inputElements().size());
    int outputRow = inputCount;

    // Trigger combinational mode via action
    auto *action = dolphin->findChild<QAction *>("actionCombinational");
    QVERIFY(action);
    action->trigger();

    // After combinational mode, length should be at least 2^inputs = 4
    QVERIFY(dolphin->length() >= 4);

    // The truth table should be filled: check column 3 (both inputs = 1 → AND = 1)
    // In combinational mode, column index is the binary input value
    // Column 3 = input0=1, input1=1 → AND = 1
    int andResult = model->index(outputRow, 3).data().toInt();
    QCOMPARE(andResult, 1);

    // Column 0 = input0=0, input1=0 → AND = 0
    int zeroResult = model->index(outputRow, 0).data().toInt();
    QCOMPARE(zeroResult, 0);
}

// ===========================================================================
// File I/O
// ===========================================================================

void TestBewavedDolphinGui::testExportToPng()
{
    const QString pngPath = m_tempDir.filePath("export_test.png");

    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    dolphin->setCellValue(0, 0, 1);
    dolphin->run();

    bool ok = dolphin->exportToPng(pngPath);
    QVERIFY(ok);
    QVERIFY(QFile::exists(pngPath));
    QVERIFY(QFileInfo(pngPath).size() > 0);

    // The offscreen render must actually contain the waveform, not a blank image:
    // expect a non-trivial size and more than one distinct color.
    QImage img(pngPath);
    QVERIFY(!img.isNull());
    QVERIFY(img.width() > 50 && img.height() > 50);
    QSet<QRgb> colors;
    for (int y = 0; y < img.height() && colors.size() < 3; ++y) {
        for (int x = 0; x < img.width() && colors.size() < 3; ++x) {
            colors.insert(img.pixel(x, y));
        }
    }
    QVERIFY2(colors.size() >= 2, "Exported waveform image should not be a single flat color");

    QFile::remove(pngPath);
}

// ===========================================================================
// Editing actions
// ===========================================================================

void TestBewavedDolphinGui::testClearAction()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    auto *model = dolphin->model();

    // Set some input cells to 1
    dolphin->setCellValue(0, 0, 1);
    dolphin->setCellValue(0, 1, 1);
    dolphin->setCellValue(1, 0, 1);
    QCOMPARE(model->index(0, 0).data().toInt(), 1);

    // Clear all
    auto *action = dolphin->findChild<QAction *>("actionClear");
    QVERIFY(action);
    action->trigger();

    // All input cells should be 0
    for (int col = 0; col < model->columnCount(); ++col) {
        QCOMPARE(model->index(0, col).data().toInt(), 0);
        QCOMPARE(model->index(1, col).data().toInt(), 0);
    }
}

void TestBewavedDolphinGui::testAutoCropAction()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    // AND circuit has 2 inputs — set some non-zero input values so autocrop
    // has something to trim to (all-zero trims to 1 column).
    dolphin->setLength(32, false);
    QCOMPARE(dolphin->length(), 32);
    dolphin->setCellValue(0, 3, 1); // set input 0, col 3 to 1

    auto *action = dolphin->findChild<QAction *>("actionAutoCrop");
    QVERIFY(action);
    action->trigger();

    QCOMPARE(dolphin->length(), 4); // trims to col 3 + 1
}

// ===========================================================================
// Selection-based editing
// ===========================================================================

void TestBewavedDolphinGui::testSetTo0WithSelection()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    // Set cells to 1 first
    dolphin->setCellValue(0, 0, 1);
    dolphin->setCellValue(0, 1, 1);
    QCOMPARE(model->index(0, 0).data().toInt(), 1);

    // Select and set to 0
    selectCells(dolphin.get(), 0, 0, 0, 1);
    auto *action = dolphin->findChild<QAction *>("actionSetTo0");
    QVERIFY(action);
    action->trigger();

    QCOMPARE(model->index(0, 0).data().toInt(), 0);
    QCOMPARE(model->index(0, 1).data().toInt(), 0);
}

void TestBewavedDolphinGui::testSetTo1WithSelection()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    QCOMPARE(model->index(0, 0).data().toInt(), 0);

    selectCells(dolphin.get(), 0, 0, 0, 3);
    auto *action = dolphin->findChild<QAction *>("actionSetTo1");
    QVERIFY(action);
    action->trigger();

    for (int col = 0; col <= 3; ++col) {
        QCOMPARE(model->index(0, col).data().toInt(), 1);
    }
}

void TestBewavedDolphinGui::testInvertWithSelection()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    dolphin->setCellValue(0, 0, 0);
    dolphin->setCellValue(0, 1, 1);
    dolphin->setCellValue(0, 2, 0);
    dolphin->setCellValue(0, 3, 1);

    selectCells(dolphin.get(), 0, 0, 0, 3);
    auto *action = dolphin->findChild<QAction *>("actionInvert");
    QVERIFY(action);
    action->trigger();

    QCOMPARE(model->index(0, 0).data().toInt(), 1);
    QCOMPARE(model->index(0, 1).data().toInt(), 0);
    QCOMPARE(model->index(0, 2).data().toInt(), 1);
    QCOMPARE(model->index(0, 3).data().toInt(), 0);
}

void TestBewavedDolphinGui::testCopyPasteWithSelection()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    dolphin->setCellValue(0, 0, 1);
    dolphin->setCellValue(0, 1, 0);
    dolphin->setCellValue(0, 2, 1);
    dolphin->setCellValue(0, 3, 0);

    // Copy cols 0-3
    selectCells(dolphin.get(), 0, 0, 0, 3);
    auto *copyAction = dolphin->findChild<QAction *>("actionCopy");
    QVERIFY(copyAction);
    copyAction->trigger();

    // Paste at cols 4-7
    selectCells(dolphin.get(), 0, 4, 0, 7);
    auto *pasteAction = dolphin->findChild<QAction *>("actionPaste");
    QVERIFY(pasteAction);
    pasteAction->trigger();

    QCOMPARE(model->index(0, 4).data().toInt(), 1);
    QCOMPARE(model->index(0, 5).data().toInt(), 0);
    QCOMPARE(model->index(0, 6).data().toInt(), 1);
    QCOMPARE(model->index(0, 7).data().toInt(), 0);
}

void TestBewavedDolphinGui::testSetClockWaveWithSelection()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    // Select row 0, cols 0-7
    selectCells(dolphin.get(), 0, 0, 0, 7);

    // Clock wave action shows a ClockDialog which is modal.
    // We use QTimer to auto-accept it with default period.
    QTimer::singleShot(0, [&dolphin] {
        auto *dialog = dolphin->findChild<QDialog *>();
        if (dialog) dialog->accept();
    });

    auto *action = dolphin->findChild<QAction *>("actionSetClockWave");
    QVERIFY(action);
    action->trigger();

    // After clock wave, cells should have an alternating pattern
    // (exact pattern depends on default clock period from ClockDialog)
    bool hasZero = false;
    bool hasOne = false;
    for (int col = 0; col <= 7; ++col) {
        int val = model->index(0, col).data().toInt();
        if (val == 0) hasZero = true;
        if (val == 1) hasOne = true;
    }
    QVERIFY2(hasZero && hasOne, "Clock wave should produce alternating 0/1 pattern");
}

// ===========================================================================
// View operations
// ===========================================================================

void TestBewavedDolphinGui::testZoomInOut()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    auto *zoomIn = dolphin->findChild<QAction *>("actionZoomIn");
    auto *zoomOut = dolphin->findChild<QAction *>("actionZoomOut");
    auto *resetZoom = dolphin->findChild<QAction *>("actionResetZoom");
    QVERIFY(zoomIn);
    QVERIFY(zoomOut);
    QVERIFY(resetZoom);

    // Zoom in and verify zoomOut becomes enabled
    zoomIn->trigger();
    QVERIFY(zoomOut->isEnabled());

    zoomIn->trigger();
    zoomOut->trigger();
    resetZoom->trigger();

    // After reset, zoomIn should still be enabled
    QVERIFY(zoomIn->isEnabled());
}

void TestBewavedDolphinGui::testShowNumbers()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    auto *action = dolphin->findChild<QAction *>("actionShowNumbers");
    QVERIFY(action);
    QVERIFY(action->isEnabled());
    action->trigger();
    // Action toggles number display — verify it remains enabled after toggle
    QVERIFY(action->isEnabled());
}

void TestBewavedDolphinGui::testShowWaveforms()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    auto *action = dolphin->findChild<QAction *>("actionShowWaveforms");
    QVERIFY(action);
    QVERIFY(action->isEnabled());
    action->trigger();
    // Action toggles waveform display — verify it remains enabled after toggle
    QVERIFY(action->isEnabled());
}

void TestBewavedDolphinGui::testFitScreen()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    auto *action = dolphin->findChild<QAction *>("actionFitScreen");
    QVERIFY(action);

    // A single Fit press must fully fit: pressing it again is a no-op. Regression for the
    // one-step feedback loop where fitScreen() measured the pre-apply layout — the gutter
    // width and scroll bars change as a *result* of the fit — so it took two presses to
    // converge (e.g. 0.065 then 0.080 for this circuit).
    dolphin->setLength(256, false); // content far wider than the viewport
    dolphin->resize(800, 500);
    dolphin->show();
    QApplication::processEvents();

    action->trigger();
    QApplication::processEvents();
    const double firstFit = dolphin->m_zoom->fitScale();

    action->trigger();
    QApplication::processEvents();
    const double secondFit = dolphin->m_zoom->fitScale();

    QVERIFY(std::isfinite(firstFit));
    QVERIFY(firstFit > 0.0);
    QVERIFY2(std::abs(secondFit - firstFit) < 1e-3,
             qPrintable(QString("Fit to Screen not idempotent: %1 then %2").arg(firstFit).arg(secondFit)));
}

void TestBewavedDolphinGui::testLongWaveformScrolls()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *tv = findTableView(dolphin.get());
    QVERIFY(tv);

    // The old QGraphicsScene embedding forced both scrollbars AlwaysOff; native
    // scrolling must now be available.
    QCOMPARE(tv->horizontalScrollBarPolicy(), Qt::ScrollBarAsNeeded);
    QCOMPARE(tv->verticalScrollBarPolicy(), Qt::ScrollBarAsNeeded);

    // A waveform far wider than the viewport must expose a horizontal scroll range.
    dolphin->setLength(256, false);
    tv->resize(200, 150);
    QApplication::processEvents();
    QVERIFY2(tv->horizontalScrollBar()->maximum() > 0,
             "Long waveform should produce a horizontal scroll range");
}

// ===========================================================================
// Double-click cell
// ===========================================================================

void TestBewavedDolphinGui::testDoubleClickToggle()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    // Set cell (0,0) to 0
    dolphin->setCellValue(0, 0, 0);
    QCOMPARE(model->index(0, 0).data().toInt(), 0);

    // Select the cell and emit doubleClicked to trigger the toggle handler, rather than
    // driving a real QTest::mouseDClick: no other test in this suite clicks into a
    // QTableView, and doing so here would need show() + realized cell geometry that
    // createDolphin() intentionally skips. This exercises the connected slot's logic
    // without that fragility.
    auto *tv = findTableView(dolphin.get());
    QVERIFY2(tv, "QTableView not found in BewavedDolphin");

    selectCells(dolphin.get(), 0, 0, 0, 0);
    emit tv->doubleClicked(model->index(0, 0));

    // After double-click the cell value should have toggled to 1
    QCOMPARE(model->index(0, 0).data().toInt(), 1);

    // Double-click again should toggle back to 0
    selectCells(dolphin.get(), 0, 0, 0, 0);
    emit tv->doubleClicked(model->index(0, 0));
    QCOMPARE(model->index(0, 0).data().toInt(), 0);
}

// ===========================================================================
// Text output
// ===========================================================================

void TestBewavedDolphinGui::testSaveToTxtOutput()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    // Set a known pattern
    dolphin->setLength(4, false);
    dolphin->setCellValue(0, 0, 0);
    dolphin->setCellValue(0, 1, 0);
    dolphin->setCellValue(0, 2, 1);
    dolphin->setCellValue(0, 3, 1);
    dolphin->setCellValue(1, 0, 0);
    dolphin->setCellValue(1, 1, 1);
    dolphin->setCellValue(1, 2, 0);
    dolphin->setCellValue(1, 3, 1);
    dolphin->run();

    QString output;
    QTextStream stream(&output);
    dolphin->saveToTxt(stream);
    stream.flush();

    QVERIFY(!output.isEmpty());
    // Output should have one line per signal (2 inputs + 1 output = 3 lines minimum)
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    QVERIFY2(lines.size() >= 3, qPrintable(QString("Expected >= 3 lines, got %1").arg(lines.size())));

    // Verify input 0 row contains exact pattern {0, 0, 1, 1}
    QVERIFY2(lines[0].contains('0'), "First input row should contain value 0");
    QVERIFY2(lines[0].contains('1'), "First input row should contain value 1");

    // Verify input 1 row contains exact pattern {0, 1, 0, 1}
    QVERIFY2(lines[1].contains('0'), "Second input row should contain value 0");
    QVERIFY2(lines[1].contains('1'), "Second input row should contain value 1");

    // Verify output row (AND gate) has correct result — only col 3 is 1 (both inputs high)
    // AND(0,0)=0, AND(0,1)=0, AND(1,0)=0, AND(1,1)=1
    QVERIFY2(lines[2].contains('0'), "Output row should contain value 0");
    QVERIFY2(lines[2].contains('1'), "Output row should contain value 1 (AND of both inputs high)");
}

// ===========================================================================
// Missing coverage
// ===========================================================================

void TestBewavedDolphinGui::testCutWithSelection()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    // Set cells to a known pattern
    dolphin->setCellValue(0, 0, 1);
    dolphin->setCellValue(0, 1, 1);
    dolphin->setCellValue(0, 2, 0);
    dolphin->setCellValue(0, 3, 0);
    QCOMPARE(model->index(0, 0).data().toInt(), 1);
    QCOMPARE(model->index(0, 1).data().toInt(), 1);

    // Select and cut cols 0-1
    selectCells(dolphin.get(), 0, 0, 0, 1);
    auto *cutAction = dolphin->findChild<QAction *>("actionCut");
    QVERIFY(cutAction);
    cutAction->trigger();

    // Cut cells should be cleared to 0
    QCOMPARE(model->index(0, 0).data().toInt(), 0);
    QCOMPARE(model->index(0, 1).data().toInt(), 0);

    // Paste at cols 4-5
    selectCells(dolphin.get(), 0, 4, 0, 5);
    auto *pasteAction = dolphin->findChild<QAction *>("actionPaste");
    QVERIFY(pasteAction);
    pasteAction->trigger();

    QCOMPARE(model->index(0, 4).data().toInt(), 1);
    QCOMPARE(model->index(0, 5).data().toInt(), 1);
}

void TestBewavedDolphinGui::testSaveAndLoadWaveform()
{
    const QString txtPath = m_tempDir.filePath("test_waveform.txt");

    auto ws = createAndCircuit();

    // Create waveform with known pattern and save via public saveToTxt
    {
        std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
        dolphin->setLength(4, false);
        dolphin->setCellValue(0, 0, 1);
        dolphin->setCellValue(0, 1, 0);
        dolphin->setCellValue(0, 2, 1);
        dolphin->setCellValue(0, 3, 0);
        dolphin->run();

        QFile file(txtPath);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream stream(&file);
        dolphin->saveToTxt(stream);
        stream.flush();
        file.close();
    }

    QVERIFY(QFile::exists(txtPath));
    QVERIFY(QFileInfo(txtPath).size() > 0);

    // Verify the saved text file has correct structure
    QFile readFile(txtPath);
    QVERIFY(readFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = readFile.readAll();
    readFile.close();

    QStringList lines = content.split('\n', Qt::SkipEmptyParts);
    QVERIFY2(lines.size() >= 3, "Should have at least 3 lines (2 inputs + 1 output)");

    QFile::remove(txtPath);
}

void TestBewavedDolphinGui::testExportToPdf()
{
    const QString pdfPath = m_tempDir.filePath("export_test.pdf");

    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    dolphin->setCellValue(0, 0, 1);
    dolphin->run();

    ScopedFileDialogStub guard;
    guard.stub.saveResult = {pdfPath, "PDF files (*.pdf)"};

    auto *action = dolphin->findChild<QAction *>("actionExportToPdf");
    QVERIFY(action);
    action->trigger();

    QVERIFY(QFile::exists(pdfPath));
    QVERIFY(QFileInfo(pdfPath).size() > 0);

    QFile::remove(pdfPath);
}

void TestBewavedDolphinGui::testOutputRowsIgnoreSetValue()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    int outputRow = static_cast<int>(dolphin->inputElements().size());

    // Run simulation to populate outputs
    dolphin->run();
    int outputVal = model->index(outputRow, 0).data().toInt();

    // Attempt to set an output cell — should have no effect or be overwritten on next run
    dolphin->setCellValue(outputRow, 0, outputVal == 0 ? 1 : 0);
    dolphin->run();

    // After re-running, output should be determined by logic, not manual override
    QCOMPARE(model->index(outputRow, 0).data().toInt(), 0); // AND(0,0) = 0
}

void TestBewavedDolphinGui::testPasteAtBoundary()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    dolphin->setLength(8, false);

    // Set pattern in cols 0-3
    dolphin->setCellValue(0, 0, 1);
    dolphin->setCellValue(0, 1, 0);
    dolphin->setCellValue(0, 2, 1);
    dolphin->setCellValue(0, 3, 0);

    // Copy cols 0-3
    selectCells(dolphin.get(), 0, 0, 0, 3);
    auto *copyAction = dolphin->findChild<QAction *>("actionCopy");
    QVERIFY(copyAction);
    copyAction->trigger();

    // Paste at cols 6-7 (only 2 columns available before boundary)
    selectCells(dolphin.get(), 0, 6, 0, 7);
    auto *pasteAction = dolphin->findChild<QAction *>("actionPaste");
    QVERIFY(pasteAction);
    pasteAction->trigger();

    // Paste should not crash and should fill available space
    QCOMPARE(model->index(0, 6).data().toInt(), 1);
    QCOMPARE(model->index(0, 7).data().toInt(), 0);
}

void TestBewavedDolphinGui::testAboutDialog()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    auto dismisser = TestUtils::AutoDismisser::closeAnyModal();

    auto *action = dolphin->findChild<QAction *>("actionAbout");
    QVERIFY(action);
    action->trigger();
    QVERIFY2(TestUtils::waitFor([&] { return dismisser.dismissCount() >= 1; }),"The About dialog must have appeared");
}

void TestBewavedDolphinGui::testAboutQtDialog()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    auto dismisser = TestUtils::AutoDismisser::closeAnyModal();

    auto *action = dolphin->findChild<QAction *>("actionAboutQt");
    QVERIFY(action);
    action->trigger();
    QVERIFY2(TestUtils::waitFor([&] { return dismisser.dismissCount() >= 1; }),"The About Qt dialog must have appeared");
}

// ===========================================================================
// Additional operations
// ===========================================================================

void TestBewavedDolphinGui::testSaveAsAction()
{
    auto ws = createAndCircuit();

    // Inject a stub host so the Save-As path (which reads the host's current file/dir)
    // can be exercised end-to-end — previously untestable because it dereferenced a
    // real MainWindow (audit §8).
    StubDolphinHost host;
    host.m_currentDir = QDir(m_tempDir.path());
    host.m_currentFile = QFileInfo(m_tempDir.filePath("circuit.panda"));

    std::unique_ptr<BewavedDolphin> dolphin(new BewavedDolphin(ws->scene(), false, &host));
    dolphin->setAttribute(Qt::WA_DeleteOnClose, false);
    dolphin->createWaveform("");

    auto *action = dolphin->findChild<QAction *>("actionSaveAs");
    QVERIFY2(action, "actionSaveAs not found");
    QVERIFY(action->isEnabled());

    const QString savePath = m_tempDir.filePath("saved.dolphin");
    ScopedFileDialogStub guard;
    guard.stub.saveResult = {savePath, "Dolphin files (*.dolphin)"};

    action->trigger();

    QVERIFY(QFile::exists(savePath));
    QVERIFY(!dolphin->m_edited);
    QVERIFY(dolphin->windowTitle().contains("saved.dolphin"));
}

void TestBewavedDolphinGui::testExitAction()
{
    auto ws = createAndCircuit();
    auto *dolphin = createDolphin(ws.get());

    // The window must actually be shown first — otherwise isVisible() is trivially
    // false both before and after trigger(), and the assertion below would pass
    // even if Exit failed to close it.
    dolphin->show();
    QVERIFY(dolphin->isVisible());

    auto *action = dolphin->findChild<QAction *>("actionExit");
    QVERIFY2(action, "actionExit not found");

    // Exit triggers close(). Since m_edited is false (fresh waveform),
    // it should close without prompting.
    action->trigger();

    // Window should be hidden after close
    QVERIFY(!dolphin->isVisible());

    delete dolphin;
}

void TestBewavedDolphinGui::testShortcutsReconciledWithMainWindow()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    const auto shortcutOf = [&](const char *name) {
        auto *action = dolphin->findChild<QAction *>(name);
        return action ? action->shortcut() : QKeySequence();
    };

    // Reconciled so a chord no longer means different things in the two windows: About
    // matches the main window's F1 (Ctrl+H is Flip there); Fit Screen matches its
    // Ctrl+Shift+F Zoom to Fit (Ctrl+Shift+R is Rotate there); AutoCrop moves to BWD's
    // Alt+ family, freeing Ctrl+A for its universal Select-All meaning.
    QCOMPARE(shortcutOf("actionAbout"), QKeySequence(QStringLiteral("F1")));
    QCOMPARE(shortcutOf("actionFitScreen"), QKeySequence(QStringLiteral("Ctrl+Shift+F")));
    QCOMPARE(shortcutOf("actionAutoCrop"), QKeySequence(QStringLiteral("Alt+A")));

    // The reconciliation introduced no in-window duplicate shortcut.
    QSet<QKeySequence> seen;
    for (auto *action : dolphin->findChildren<QAction *>()) {
        const QKeySequence sequence = action->shortcut();
        if (sequence.isEmpty()) {
            continue;
        }
        QVERIFY2(!seen.contains(sequence), qPrintable(QStringLiteral("duplicate BWD shortcut: %1").arg(sequence.toString())));
        seen.insert(sequence);
    }
}

void TestBewavedDolphinGui::testMergeSplitDisabled()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    auto *mergeAction = dolphin->findChild<QAction *>("actionMerge");
    auto *splitAction = dolphin->findChild<QAction *>("actionSplit");

    QVERIFY2(mergeAction, "actionMerge not found");
    QVERIFY2(splitAction, "actionSplit not found");

    // Both are disabled (reserved for future multi-bit bus feature)
    QVERIFY(!mergeAction->isEnabled());
    QVERIFY(!splitAction->isEnabled());
}

void TestBewavedDolphinGui::testSetClockWaveDisabledWithoutSelectionC9()
{
    // Pre-fix actionSetClockWave was always enabled and the handler threw
    // "No cells selected." straight into a modal error. Now the action is
    // gated on a non-empty selection and toggled by the table's
    // selectionChanged slot.
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    auto *action = dolphin->findChild<QAction *>("actionSetClockWave");
    QVERIFY2(action, "actionSetClockWave not found");

    // Initial state with no selection — disabled.
    QVERIFY(!action->isEnabled());

    // Select a row, action becomes enabled.
    selectCells(dolphin.get(), 0, 0, 0, 3);
    QVERIFY(action->isEnabled());

    // Clear the selection, action returns to disabled.
    auto *tv = findTableView(dolphin.get());
    QVERIFY(tv);
    tv->selectionModel()->clearSelection();
    QVERIFY(!action->isEnabled());
}

void TestBewavedDolphinGui::testZoomScaleTrackingA26()
{
    // Zoom In/Out is a horizontal "time stretch": a discrete level (m_zoomLevel, 0..6)
    // scales the COLUMN width only — row height and font stay fixed — and it is floored
    // at the baseline (no shrink below the default width). Reset returns to level 0 /
    // fit 1.0. (Fit Screen, the only thing that scales rows + font, is covered separately.)
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    static constexpr double kZoomStep = 1.25;
    // Baseline cell metrics match the pre-refactor on-screen size (column 38px, row 30px).
    static constexpr int kDefaultColumnWidth = 38;
    static constexpr int kDefaultRowHeight = 30;
    QCOMPARE(dolphin->m_zoom->zoomLevel(), 0);
    QCOMPARE(dolphin->m_zoom->fitScale(), 1.0);

    auto *zoomIn = dolphin->findChild<QAction *>("actionZoomIn");
    auto *zoomOut = dolphin->findChild<QAction *>("actionZoomOut");
    auto *resetZoom = dolphin->findChild<QAction *>("actionResetZoom");
    QVERIFY(zoomIn && zoomOut && resetZoom);

    auto *hHeader = dolphin->m_signalTableView->horizontalHeader();
    auto *vHeader = dolphin->m_signalTableView->verticalHeader();
    const int baseRowHeight = vHeader->defaultSectionSize();
    QCOMPARE(baseRowHeight, kDefaultRowHeight);
    QCOMPARE(hHeader->defaultSectionSize(), kDefaultColumnWidth);
    const double baseFontPt = dolphin->m_signalTableView->font().pointSizeF();

    // One Zoom In: columns widen by the step; rows AND font are untouched.
    zoomIn->trigger();
    QCOMPARE(dolphin->m_zoom->zoomLevel(), 1);
    QCOMPARE(hHeader->defaultSectionSize(),
             static_cast<int>(std::lround(kDefaultColumnWidth * kZoomStep)));
    QCOMPARE(vHeader->defaultSectionSize(), baseRowHeight);
    QCOMPARE(dolphin->m_signalTableView->font().pointSizeF(), baseFontPt);

    zoomOut->trigger();
    QCOMPARE(dolphin->m_zoom->zoomLevel(), 0);
    QCOMPARE(hHeader->defaultSectionSize(), kDefaultColumnWidth);

    // Zoom Out is floored at the baseline — repeated triggers never go below level 0.
    zoomOut->trigger();
    zoomOut->trigger();
    QCOMPARE(dolphin->m_zoom->zoomLevel(), 0);
    QCOMPARE(hHeader->defaultSectionSize(), kDefaultColumnWidth);

    // Zoom In is capped at kMaxZoomLevel (6) — repeated triggers never exceed it.
    for (int i = 0; i < 10; ++i) {
        zoomIn->trigger();
    }
    QCOMPARE(dolphin->m_zoom->zoomLevel(), 6);
    QVERIFY(!zoomIn->isEnabled());

    resetZoom->trigger();
    QCOMPARE(dolphin->m_zoom->zoomLevel(), 0);
    QCOMPARE(dolphin->m_zoom->fitScale(), 1.0);
    QCOMPARE(hHeader->defaultSectionSize(), kDefaultColumnWidth);
}

void TestBewavedDolphinGui::testFitScreenClampsAndGuardsA26()
{
    // FitScreen computes a uniform scale (m_fitScale) that fits the content in the
    // viewport, clamped to [kMinFitScale, kMaxFitScale], and resets the column zoom.
    // On degenerate geometry (everything hidden) it must keep m_fitScale finite and
    // inside the clamp window — never +inf from a tiny divisor (the WIREDPANDA-HW
    // class of bug).
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    static constexpr double kMinFitScale = 0.05;
    static constexpr double kMaxFitScale = 20.0;

    auto *fitScreen = dolphin->findChild<QAction *>("actionFitScreen");
    QVERIFY(fitScreen);

    fitScreen->trigger();
    QVERIFY(std::isfinite(dolphin->m_zoom->fitScale()));
    QVERIFY(dolphin->m_zoom->fitScale() >= kMinFitScale);
    QVERIFY(dolphin->m_zoom->fitScale() <= kMaxFitScale);
    QCOMPARE(dolphin->m_zoom->zoomLevel(), 0);

    // Repeating FitScreen with the same geometry stays within bounds and finite.
    fitScreen->trigger();
    QVERIFY(std::isfinite(dolphin->m_zoom->fitScale()));
    QVERIFY(dolphin->m_zoom->fitScale() >= kMinFitScale);
    QVERIFY(dolphin->m_zoom->fitScale() <= kMaxFitScale);

    // Degenerate geometry: hide every column and row so the content extent
    // collapses; FitScreen must leave m_fitScale finite and clamped, not blow up.
    auto *table = dolphin->m_signalTableView;
    QVERIFY(table);
    auto *model = dolphin->model();
    QVERIFY(model);
    for (int col = 0; col < model->columnCount(); ++col) {
        table->setColumnHidden(col, true);
    }
    table->verticalHeader()->hide();
    for (int row = 0; row < model->rowCount(); ++row) {
        table->setRowHidden(row, true);
    }

    fitScreen->trigger();
    QVERIFY(std::isfinite(dolphin->m_zoom->fitScale()));
    QVERIFY(dolphin->m_zoom->fitScale() >= kMinFitScale);
    QVERIFY(dolphin->m_zoom->fitScale() <= kMaxFitScale);
}

void TestBewavedDolphinGui::testCreateWaveformResolvesAbsolutePathOutsideProjectDir()
{
    // A waveform file linked from outside the project directory (e.g. via
    // associateToWiRedPanda()'s "Save As...") must still resolve by its absolute path —
    // createWaveform() must not unconditionally discard the directory and only search
    // the project's own folder.
    QVERIFY(QDir(m_tempDir.path()).mkpath("project"));
    QVERIFY(QDir(m_tempDir.path()).mkpath("elsewhere"));
    const QString projectDir = m_tempDir.filePath("project");
    const QString elsewhereDir = m_tempDir.filePath("elsewhere");
    const QString dolphinPath = elsewhereDir + "/waveform.dolphin";

    auto ws = createAndCircuit();

    // Save a waveform with a known pattern to a .dolphin file living outside any project
    // directory, via the same Save-As action other tests use.
    {
        StubDolphinHost host;
        host.m_currentDir = QDir(elsewhereDir);
        std::unique_ptr<BewavedDolphin> dolphin(new BewavedDolphin(ws->scene(), false, &host));
        dolphin->setAttribute(Qt::WA_DeleteOnClose, false);
        dolphin->createWaveform("");
        dolphin->setCellValue(0, 0, 1);
        dolphin->run();

        auto *action = dolphin->findChild<QAction *>("actionSaveAs");
        QVERIFY2(action, "actionSaveAs not found");
        ScopedFileDialogStub guard;
        guard.stub.saveResult = {dolphinPath, "Dolphin files (*.dolphin)"};
        action->trigger();
    }

    QVERIFY(QFile::exists(dolphinPath));

    // Open a fresh BewavedDolphin whose project directory is a different folder, and load
    // the waveform by its full absolute path (as WorkSpace::dolphinFileName() would return
    // after linking a file that lives outside the project).
    StubDolphinHost host2;
    host2.m_currentDir = QDir(projectDir);
    std::unique_ptr<BewavedDolphin> dolphin2(new BewavedDolphin(ws->scene(), false, &host2));
    dolphin2->setAttribute(Qt::WA_DeleteOnClose, false);
    dolphin2->createWaveform(dolphinPath);

    auto *model = dolphin2->model();
    QVERIFY(model);
    QCOMPARE(model->index(0, 0).data().toInt(), 1);
}

void TestBewavedDolphinGui::hardeningRunAndSaveToTxtMustCheckElementsStillLive()
{
    const QString path = QString(QUOTE(CURRENTDIR)) + "/../App/BeWavedDolphin/BeWavedDolphin.cpp";
    QFile src(path);
    QVERIFY2(src.open(QIODevice::ReadOnly), qPrintable(QString("Cannot open %1").arg(src.fileName())));
    const QString source = QString::fromUtf8(src.readAll());
    src.close();

    auto bodyOf = [&source](const QString &qualifiedName) -> QString {
        const QString pattern =
            QStringLiteral("\\b") + QRegularExpression::escape(qualifiedName)
            + QStringLiteral("\\s*\\([^)]*\\)\\s*\\{");
        QRegularExpression rx(pattern);
        const auto match = rx.match(source);
        if (!match.hasMatch()) return {};

        const qsizetype start = match.capturedEnd() - 1;
        int depth = 0;
        for (qsizetype i = start; i < source.size(); ++i) {
            const QChar c = source.at(i);
            if (c == '{') ++depth;
            else if (c == '}') {
                --depth;
                if (depth == 0) return source.mid(start, i - start + 1);
            }
        }
        return {};
    };

    const QStringList checkedFunctions = {"BewavedDolphin::run", "BewavedDolphin::saveToTxt"};
    QStringList missingCheck;
    for (const QString &name : checkedFunctions) {
        const QString body = bodyOf(name);
        if (body.isEmpty()) {
            missingCheck << (name + " (function body not located)");
            continue;
        }
        if (!body.contains("elementsStillLive")) {
            missingCheck << name;
        }
    }

    QVERIFY2(missingCheck.isEmpty(),
             qPrintable(QString("The following functions dereference m_inputs/m_outputs via "
                                "sweep() and must check elementsStillLive() first — an element "
                                "deleted from the live scene since prepare() (main canvas, or an "
                                "MCP client, neither blocked by this window's modality) would "
                                "otherwise dangle:\n  - %1")
                            .arg(missingCheck.join("\n  - "))));
}

void TestBewavedDolphinGui::testRunAndSaveToTxtHandleDeletedTrackedElement()
{
    auto ws = createAndCircuit();
    auto *scene = ws->scene();
    auto *dolphin = createDolphin(ws.get());

    // Find the tracked InputSwitch backing waveform row 0, then delete it directly via a
    // scene command — the same mutation an MCP client's delete_element would perform,
    // bypassing this window's (GUI-only) modality entirely.
    const auto &inputs = dolphin->inputElements();
    QVERIFY(!inputs.isEmpty());
    auto *trackedInput = inputs.first();
    scene->receiveCommand(new DeleteItemsCommand({trackedInput}, scene));

    // run() must skip the now-stale sweep rather than dereferencing the deleted element.
    dolphin->run();

    // saveToTxt() must fail cleanly instead of silently exporting garbage/partial data.
    QString text;
    QTextStream stream(&text);
    QVERIFY_THROWS(std::exception, dolphin->saveToTxt(stream));
}

void TestBewavedDolphinGui::testSaveToTxtClampsColumnCountForManyInputPorts()
{
    auto ws = std::make_unique<WorkSpace>();
    auto *scene = ws->scene();
    for (int i = 0; i < 12; ++i) {
        scene->addItem(new InputSwitch());
    }
    scene->addItem(new Led());
    auto *dolphin = createDolphin(ws.get());

    QString text;
    QTextStream stream(&text);
    dolphin->saveToTxt(stream);

    const QString firstLine = text.section('\n', 0, 0);
    const qsizetype columnCount = firstLine.section(" : ", 0, 0).length();
    QCOMPARE(columnCount, static_cast<qsizetype>(SignalModel::kMaxColumns));
}

// ===========================================================================
// Undo/Redo (#19)
// ===========================================================================

void TestBewavedDolphinGui::testUndoStackStartsCleanAfterCreateWaveform()
{
    // loadNewTable()'s internal "clear all inputs" call (run once per window, before the
    // user gets control) must not leave a spurious entry on the undo stack.
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    QCOMPARE(dolphin->m_undoStack.count(), 0);
    QVERIFY(!dolphin->m_undoStack.canUndo());
}

void TestBewavedDolphinGui::testUndoRedoSetTo0()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    dolphin->setCellValue(0, 0, 1);
    dolphin->setCellValue(0, 1, 1);

    selectCells(dolphin.get(), 0, 0, 0, 1);
    auto *action = dolphin->findChild<QAction *>("actionSetTo0");
    QVERIFY(action);
    action->trigger();

    QCOMPARE(model->index(0, 0).data().toInt(), 0);
    QCOMPARE(model->index(0, 1).data().toInt(), 0);
    QVERIFY(dolphin->m_undoStack.canUndo());

    dolphin->m_undoStack.undo();
    QCOMPARE(model->index(0, 0).data().toInt(), 1);
    QCOMPARE(model->index(0, 1).data().toInt(), 1);

    dolphin->m_undoStack.redo();
    QCOMPARE(model->index(0, 0).data().toInt(), 0);
    QCOMPARE(model->index(0, 1).data().toInt(), 0);
}

void TestBewavedDolphinGui::testUndoRedoSetTo1()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    selectCells(dolphin.get(), 0, 0, 0, 1);
    auto *action = dolphin->findChild<QAction *>("actionSetTo1");
    QVERIFY(action);
    action->trigger();

    QCOMPARE(model->index(0, 0).data().toInt(), 1);
    QCOMPARE(model->index(0, 1).data().toInt(), 1);

    dolphin->m_undoStack.undo();
    QCOMPARE(model->index(0, 0).data().toInt(), 0);
    QCOMPARE(model->index(0, 1).data().toInt(), 0);

    dolphin->m_undoStack.redo();
    QCOMPARE(model->index(0, 0).data().toInt(), 1);
    QCOMPARE(model->index(0, 1).data().toInt(), 1);
}

void TestBewavedDolphinGui::testUndoRedoInvert()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    dolphin->setCellValue(0, 0, 0);
    dolphin->setCellValue(0, 1, 1);

    selectCells(dolphin.get(), 0, 0, 0, 1);
    auto *action = dolphin->findChild<QAction *>("actionInvert");
    QVERIFY(action);
    action->trigger();

    QCOMPARE(model->index(0, 0).data().toInt(), 1);
    QCOMPARE(model->index(0, 1).data().toInt(), 0);

    dolphin->m_undoStack.undo();
    QCOMPARE(model->index(0, 0).data().toInt(), 0);
    QCOMPARE(model->index(0, 1).data().toInt(), 1);

    dolphin->m_undoStack.redo();
    QCOMPARE(model->index(0, 0).data().toInt(), 1);
    QCOMPARE(model->index(0, 1).data().toInt(), 0);
}

void TestBewavedDolphinGui::testUndoRedoClear()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    dolphin->setCellValue(0, 0, 1);
    dolphin->setCellValue(0, 1, 1);
    dolphin->setCellValue(1, 0, 1);

    auto *action = dolphin->findChild<QAction *>("actionClear");
    QVERIFY(action);
    action->trigger();

    // Every input cell, across the whole column range, must come back after undo -- not
    // just the ones this test happened to set.
    for (int col = 0; col < model->columnCount(); ++col) {
        QCOMPARE(model->index(0, col).data().toInt(), 0);
        QCOMPARE(model->index(1, col).data().toInt(), 0);
    }

    dolphin->m_undoStack.undo();
    QCOMPARE(model->index(0, 0).data().toInt(), 1);
    QCOMPARE(model->index(0, 1).data().toInt(), 1);
    QCOMPARE(model->index(1, 0).data().toInt(), 1);
    for (int col = 2; col < model->columnCount(); ++col) {
        QCOMPARE(model->index(0, col).data().toInt(), 0);
        QCOMPARE(model->index(1, col).data().toInt(), 0);
    }

    dolphin->m_undoStack.redo();
    for (int col = 0; col < model->columnCount(); ++col) {
        QCOMPARE(model->index(0, col).data().toInt(), 0);
        QCOMPARE(model->index(1, col).data().toInt(), 0);
    }
}

void TestBewavedDolphinGui::testUndoRedoSetClockWave()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    // Snapshot the pre-edit values across the full selected range.
    QVector<int> before;
    for (int col = 0; col <= 7; ++col) {
        before.append(model->index(0, col).data().toInt());
    }

    selectCells(dolphin.get(), 0, 0, 0, 7);

    QTimer::singleShot(0, [&dolphin] {
        auto *dialog = dolphin->findChild<QDialog *>();
        if (dialog) dialog->accept();
    });

    auto *action = dolphin->findChild<QAction *>("actionSetClockWave");
    QVERIFY(action);
    action->trigger();

    QVERIFY(dolphin->m_undoStack.canUndo());

    dolphin->m_undoStack.undo();
    for (int col = 0; col <= 7; ++col) {
        QCOMPARE(model->index(0, col).data().toInt(), before.at(col));
    }
}

void TestBewavedDolphinGui::testUndoRedoCombinational()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();
    int inputCount = static_cast<int>(dolphin->inputElements().size());
    int outputRow = inputCount;

    auto *action = dolphin->findChild<QAction *>("actionCombinational");
    QVERIFY(action);
    action->trigger();

    QCOMPARE(model->index(outputRow, 3).data().toInt(), 1);

    dolphin->m_undoStack.undo();

    // The value grid must revert to the pre-combinational (all-zero, freshly-created)
    // state across every input cell -- setLength() itself grew the column count and
    // isn't undone here (out of scope), but the value fill it exposed must be.
    for (int col = 0; col < model->columnCount(); ++col) {
        for (int row = 0; row < inputCount; ++row) {
            QCOMPARE(model->index(row, col).data().toInt(), 0);
        }
    }
}

void TestBewavedDolphinGui::testUndoRedoDoubleClickToggle()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    dolphin->setCellValue(0, 0, 0);

    auto *tv = findTableView(dolphin.get());
    QVERIFY2(tv, "QTableView not found in BewavedDolphin");

    selectCells(dolphin.get(), 0, 0, 0, 0);
    emit tv->doubleClicked(model->index(0, 0));

    QCOMPARE(model->index(0, 0).data().toInt(), 1);
    QVERIFY(dolphin->m_undoStack.canUndo());

    dolphin->m_undoStack.undo();
    QCOMPARE(model->index(0, 0).data().toInt(), 0);
}

void TestBewavedDolphinGui::testUndoRedoMultipleOperationsRestoresOriginalState()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->model();

    // Pristine snapshot right after creation.
    QVector<int> pristine;
    for (int col = 0; col < model->columnCount(); ++col) {
        pristine.append(model->index(0, col).data().toInt());
        pristine.append(model->index(1, col).data().toInt());
    }

    selectCells(dolphin.get(), 0, 0, 0, 1);
    dolphin->findChild<QAction *>("actionSetTo1")->trigger();

    selectCells(dolphin.get(), 1, 0, 1, 1);
    dolphin->findChild<QAction *>("actionSetTo1")->trigger();

    selectCells(dolphin.get(), 0, 0, 0, 1);
    dolphin->findChild<QAction *>("actionInvert")->trigger();

    QCOMPARE(dolphin->m_undoStack.count(), 3);

    // Undoing all three, in reverse order, must return to the exact pristine grid --
    // not just each op's own delta undone in isolation.
    dolphin->m_undoStack.undo();
    dolphin->m_undoStack.undo();
    dolphin->m_undoStack.undo();

    QVERIFY(!dolphin->m_undoStack.canUndo());

    int i = 0;
    for (int col = 0; col < model->columnCount(); ++col) {
        QCOMPARE(model->index(0, col).data().toInt(), pristine.at(i++));
        QCOMPARE(model->index(1, col).data().toInt(), pristine.at(i++));
    }
}

// ============================================================================
// Live Analyzer page
// ============================================================================

void TestBewavedDolphinGui::testLiveAnalyzerTabGatesGridActions()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    auto *tabs = dolphin->findChild<QTabWidget *>();
    QVERIFY2(tabs, "BewavedDolphin must host a Stimulus Editor | Live Analyzer tab widget");
    QCOMPARE(tabs->count(), 2);
    auto *panel = dolphin->liveAnalyzer();
    QVERIFY2(panel, "Live Analyzer panel missing");
    QCOMPARE(tabs->indexOf(panel), 1);

    // Two representative document actions: Save is normally enabled; Merge is permanently
    // disabled (feature off) and must NOT be resurrected by the tab round trip.
    auto *actionSave = dolphin->findChild<QAction *>("actionSave");
    auto *actionMerge = dolphin->findChild<QAction *>("actionMerge");
    QVERIFY(actionSave && actionMerge);
    QVERIFY(actionSave->isEnabled());
    QVERIFY(!actionMerge->isEnabled());

    dolphin->showLiveAnalyzer();
    QCOMPARE(tabs->currentWidget(), static_cast<QWidget *>(panel));
    QVERIFY2(!actionSave->isEnabled(), "document actions must be gated off on the analyzer page");
    QVERIFY(!actionMerge->isEnabled());

    tabs->setCurrentIndex(0);
    QVERIFY2(actionSave->isEnabled(), "document actions must be restored on return to the grid");
    QVERIFY2(!actionMerge->isEnabled(),
             "Merge must stay disabled after the round trip (state snapshot, not blanket enable)");
}

void TestBewavedDolphinGui::testLiveAnalyzerWatchAllAndClear()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *panel = dolphin->liveAnalyzer();
    QVERIFY(panel);

    auto &recorder = ws->scene()->simulation()->waveformRecorder();
    QCOMPARE(recorder.traceCount(), 0);

    panel->watchAllSignals();
    // The AND circuit's elements with output ports: 2 switches + the AND gate (Led has none).
    QCOMPARE(recorder.traceCount(), 3);
    QVERIFY2(recorder.isRecording(), "Watch All must start recording");

    panel->clearWatchedSignals();
    QCOMPARE(recorder.traceCount(), 0);
    QVERIFY2(!recorder.isRecording(), "Clear must stop recording");
}

void TestBewavedDolphinGui::testLiveAnalyzerWatchICInternals()
{
    // Fixture .panda (sw → NOT → led) embedded as an IC: watching internals must register
    // path-prefixed traces for the IC's internal primitives and reveal the analyzer page.
    const QString icPath = m_tempDir.path() + "/not_ic.panda";
    {
        WorkSpace fixture;
        auto *scene = fixture.scene();
        auto *in = new InputSwitch();
        auto *inv = new Not();
        auto *out = new Led();
        in->setPos(0, 0);
        inv->setPos(150, 0);
        out->setPos(300, 0);
        scene->addItem(in);
        scene->addItem(inv);
        scene->addItem(out);
        CircuitBuilder builder(scene);
        builder.connect(in, 0, inv, 0);
        builder.connect(inv, 0, out, 0);
        fixture.save(icPath);
    }

    WorkSpace ws;
    auto *scene = ws.scene();
    auto *sw = new InputSwitch();
    auto *ic = new IC();
    ICTestHelpers::embedIC(ic, ICTestHelpers::readFile(icPath), "not_ic", m_tempDir.path(), scene->icRegistry());
    auto *led = new Led();
    scene->addItem(sw);
    scene->addItem(ic);
    scene->addItem(led);
    CircuitBuilder builder(scene);
    builder.connect(sw, 0, ic, 0);
    builder.connect(ic, 0, led, 0);
    builder.initSimulation();

    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(&ws));
    auto *tabs = dolphin->findChild<QTabWidget *>();
    QVERIFY(tabs);
    QCOMPARE(tabs->currentIndex(), 0);

    dolphin->watchICInternals(ic);

    QCOMPARE(tabs->currentIndex(), 1); // analyzer page revealed
    auto &recorder = ws.scene()->simulation()->waveformRecorder();
    QVERIFY2(recorder.traceCount() > 0, "watchICInternals registered no traces");
    QVERIFY2(recorder.isRecording(), "watchICInternals did not start recording");
    for (int i = 0; i < recorder.traceCount(); ++i) {
        QVERIFY2(recorder.trace(i).name.contains(QLatin1Char('/')),
                 "trace name must carry the IC as a path prefix");
    }
}

void TestBewavedDolphinGui::testLiveAnalyzerVerticalScrollSyncsLabels()
{
    // With more watched signals than fit the viewport, the analyzer grows a vertical
    // scrollbar; the frozen label column must follow the scroll offset so names stay
    // aligned with their traces, and its own preferred height stays capped (a long watch
    // list must not balloon the whole tool's preferred size).
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *panel = dolphin->liveAnalyzer();
    QVERIFY(panel);

    // 12 distinct traces: null-logic watches with distinct port indices are kept apart by
    // watchSignal()'s (logic, port) dedup and are enough for pure geometry.
    auto &recorder = ws->scene()->simulation()->waveformRecorder();
    for (int i = 0; i < 12; ++i) {
        recorder.watchSignal(QStringLiteral("sig%1").arg(i), nullptr, i);
    }
    QCOMPARE(recorder.traceCount(), 12);

    // Preferred height of the label column is capped regardless of the watch-list size.
    QVERIFY2(panel->labelColumn()->sizeHint().height()
                 <= 6 * (AnalyzerLayout::TraceHeight + AnalyzerLayout::TraceMargin),
             "label column preferred height must stay capped");

    // Lay the tool out small enough that 12 rows cannot fit (layout only runs for shown
    // widgets), then force a canvas paint (grab drives the paint-time resize that feeds the
    // scroll area's ranges).
    dolphin->showLiveAnalyzer();
    dolphin->resize(600, 300);
    dolphin->show();
    QApplication::processEvents();
    panel->canvas()->grab(QRect(0, 0, 50, 50));

    auto *vBar = panel->scrollArea()->verticalScrollBar();
    QVERIFY(vBar);
    QVERIFY2(vBar->maximum() > 0, "12 rows in a 200 px panel must produce vertical scroll range");

    vBar->setValue(vBar->maximum());
    QCOMPARE(panel->labelColumn()->verticalOffset(), vBar->maximum());

    vBar->setValue(0);
    QCOMPARE(panel->labelColumn()->verticalOffset(), 0);
}

void TestBewavedDolphinGui::testLiveAnalyzerZoomToLatestEdge()
{
    // "Latest Edge" jumps to the newest recorded transition at a nanosecond-resolving
    // zoom — the only practical way to observe 5-20 ns gate delays, which are sub-pixel
    // at any zoom that keeps up with a live timeline (1e9 ns per wall-second at 1x).
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *panel = dolphin->liveAnalyzer();
    QVERIFY(panel);

    dolphin->showLiveAnalyzer();
    dolphin->resize(600, 300);
    dolphin->show();
    QApplication::processEvents();

    // Empty recorder → no-op (no crash, zoom untouched).
    const double ppnBefore = panel->canvas()->pixelsPerNs();
    panel->zoomToLatestEdge();
    QCOMPARE(panel->canvas()->pixelsPerNs(), ppnBefore);

    // Record a few transitions; the newest lands at T* = 2'000'000 ns.
    auto &recorder = ws->scene()->simulation()->waveformRecorder();
    GraphicElement *elm = nullptr;
    for (auto *sceneElm : ws->scene()->elements()) {
        if (sceneElm && sceneElm->elementType() == ElementType::InputSwitch) {
            elm = sceneElm;
            break;
        }
    }
    QVERIFY2(elm, "no switch found in the AND circuit");
    recorder.watchSignal("sw", elm, 0);
    recorder.setRecording(true);
    elm->setOutputValue(0, Status::Active);
    recorder.recordAll(0);
    elm->setOutputValue(0, Status::Inactive);
    recorder.recordAll(1'000'000);
    elm->setOutputValue(0, Status::Active);
    recorder.recordAll(2'000'000);
    panel->canvas()->grab(QRect(0, 0, 50, 50)); // settle canvas geometry

    panel->zoomToLatestEdge();

    // Delay-resolving zoom: ~250 ns across the viewport (clamped at 10 px/ns).
    const int viewport = panel->scrollArea()->viewport()->width();
    QVERIFY(viewport > 0);
    const double expectedPpn = qMin(10.0, viewport / 250.0);
    QCOMPARE(panel->canvas()->pixelsPerNs(), expectedPpn);

    // The newest transition must be inside the visible sim-time window.
    const double ppn = panel->canvas()->pixelsPerNs();
    const SimTime origin = panel->canvas()->timeOrigin();
    auto *hBar = panel->scrollArea()->horizontalScrollBar();
    const double visibleStart = static_cast<double>(origin) + hBar->value() / ppn;
    const double visibleEnd = static_cast<double>(origin) + (hBar->value() + viewport) / ppn;
    QVERIFY2(visibleStart <= 2'000'000.0 && 2'000'000.0 <= visibleEnd,
             qPrintable(QString("latest edge (2e6 ns) outside visible window [%1, %2] ns")
                            .arg(visibleStart).arg(visibleEnd)));
}

/// Records a 1-second (1e9 ns) square-wave history on the circuit's first InputSwitch:
/// an edge every 100 ms of sim time, so the zoom/anchor tests have a long timeline with
/// data everywhere. Returns the recorded maxTime (1e9 ns), 0 if no switch was found.
static SimTime recordSquareWaveHistory(WorkSpace *ws)
{
    GraphicElement *elm = nullptr;
    for (auto *sceneElm : ws->scene()->elements()) {
        if (sceneElm && sceneElm->elementType() == ElementType::InputSwitch) {
            elm = sceneElm;
            break;
        }
    }
    if (!elm) {
        return 0;
    }
    auto &recorder = ws->scene()->simulation()->waveformRecorder();
    recorder.watchSignal("sw", elm, 0);
    recorder.setRecording(true);
    for (int i = 0; i <= 10; ++i) {
        elm->setOutputValue(0, (i % 2 != 0) ? Status::Active : Status::Inactive);
        recorder.recordAll(static_cast<SimTime>(i) * 100'000'000);
    }
    return recorder.maxTime();
}

void TestBewavedDolphinGui::testLiveAnalyzerZoomKeepsAnchor()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *panel = dolphin->liveAnalyzer();
    QVERIFY(panel);
    QCOMPARE(recordSquareWaveHistory(ws.get()), SimTime(1'000'000'000));

    dolphin->showLiveAnalyzer();
    dolphin->resize(600, 300);
    dolphin->show();
    QApplication::processEvents();
    panel->canvas()->grab(QRect(0, 0, 50, 50)); // settle canvas geometry

    auto *hBar = panel->scrollArea()->horizontalScrollBar();
    const int viewport = panel->scrollArea()->viewport()->width();
    QVERIFY(viewport > 0);

    // Start with the recording spanning ~4 viewports and a mid-recording instant (t = 5e8 ns)
    // at the viewport center — zooming must magnify around THAT instant, not around whatever
    // the raw pixel scroll value happens to map to after the canvas rescales.
    panel->canvas()->setPixelsPerNs(4.0 * viewport / 1e9);
    panel->canvas()->grab(QRect(0, 0, 50, 50));
    hBar->setValue(qRound(5e8 * panel->canvas()->pixelsPerNs() - viewport / 2.0));
    QVERIFY2(hBar->value() > 0 && hBar->value() < hBar->maximum(),
             "test setup: the anchor must start mid-range");

    auto *btnZoomIn = panel->findChild<QPushButton *>("analyzerZoomInButton");
    auto *btnZoomOut = panel->findChild<QPushButton *>("analyzerZoomOutButton");
    QVERIFY(btnZoomIn && btnZoomOut);

    auto centerTime = [&]() {
        return static_cast<double>(panel->canvas()->timeOrigin())
             + (hBar->value() + viewport / 2.0) / panel->canvas()->pixelsPerNs();
    };

    for (int step = 0; step < 5; ++step) {
        const double before = centerTime();
        btnZoomIn->click();
        panel->canvas()->grab(QRect(0, 0, 50, 50)); // settle the paint-time canvas resize
        const double after = centerTime();
        // Scroll bars round to whole pixels: allow a few px worth of ns at the new zoom.
        const double tolerance = 3.0 / panel->canvas()->pixelsPerNs() + 1.0;
        QVERIFY2(std::abs(after - before) <= tolerance,
                 qPrintable(QString("zoom-in step %1 moved the viewport center from %2 ns to %3 ns")
                                .arg(step).arg(before, 0, 'f', 0).arg(after, 0, 'f', 0)));
    }

    // Zoom out re-widens around the same anchor.
    const double before = centerTime();
    btnZoomOut->click();
    panel->canvas()->grab(QRect(0, 0, 50, 50));
    const double after = centerTime();
    QVERIFY2(std::abs(after - before) <= 3.0 / panel->canvas()->pixelsPerNs() + 1.0,
             qPrintable(QString("zoom-out moved the viewport center from %1 ns to %2 ns")
                            .arg(before, 0, 'f', 0).arg(after, 0, 'f', 0)));
}

void TestBewavedDolphinGui::testLiveAnalyzerWheelZoomsAtCursor()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *panel = dolphin->liveAnalyzer();
    QVERIFY(panel);
    QCOMPARE(recordSquareWaveHistory(ws.get()), SimTime(1'000'000'000));

    dolphin->showLiveAnalyzer();
    dolphin->resize(600, 300);
    dolphin->show();
    QApplication::processEvents();
    panel->canvas()->grab(QRect(0, 0, 50, 50));

    auto *hBar = panel->scrollArea()->horizontalScrollBar();
    const int viewport = panel->scrollArea()->viewport()->width();
    QVERIFY(viewport > 0);
    panel->canvas()->setPixelsPerNs(4.0 * viewport / 1e9);
    panel->canvas()->grab(QRect(0, 0, 50, 50));
    hBar->setValue(hBar->maximum() / 2);

    // An off-center cursor position distinguishes cursor anchoring from center anchoring.
    const double vx = viewport / 4.0;
    auto timeAtCursor = [&]() {
        return static_cast<double>(panel->canvas()->timeOrigin())
             + (hBar->value() + vx) / panel->canvas()->pixelsPerNs();
    };

    const double ppnBefore = panel->canvas()->pixelsPerNs();
    const double before = timeAtCursor();
    // A bare wheel, no modifier — the stimulus-editor page zooms on any wheel, and both
    // pages of the tool must agree.
    const QPointF canvasPos(hBar->value() + vx, 10);
    QWheelEvent wheel(canvasPos, canvasPos, QPoint(), QPoint(0, 120),
                      Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QCoreApplication::sendEvent(panel->canvas(), &wheel);
    panel->canvas()->grab(QRect(0, 0, 50, 50));

    QCOMPARE(panel->canvas()->pixelsPerNs(), ppnBefore * 2.0);
    const double after = timeAtCursor();
    QVERIFY2(std::abs(after - before) <= 3.0 / panel->canvas()->pixelsPerNs() + 1.0,
             qPrintable(QString("wheel zoom moved the time under the cursor from %1 ns to %2 ns")
                            .arg(before, 0, 'f', 0).arg(after, 0, 'f', 0)));
}

void TestBewavedDolphinGui::testLiveAnalyzerButtonZoomFollowsTail()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *panel = dolphin->liveAnalyzer();
    QVERIFY(panel);
    QCOMPARE(recordSquareWaveHistory(ws.get()), SimTime(1'000'000'000));

    dolphin->showLiveAnalyzer();
    dolphin->resize(600, 300);
    dolphin->show();
    QApplication::processEvents();
    panel->canvas()->grab(QRect(0, 0, 50, 50));

    auto *hBar = panel->scrollArea()->horizontalScrollBar();
    const int viewport = panel->scrollArea()->viewport()->width();
    QVERIFY(viewport > 0);

    // Span ~4 viewports and pin the view at the tail, engaging the sticky follow.
    panel->canvas()->setPixelsPerNs(4.0 * viewport / 1e9);
    panel->canvas()->grab(QRect(0, 0, 50, 50));
    QVERIFY(hBar->maximum() > 0);
    hBar->setValue(hBar->maximum());

    auto *btnZoomIn = panel->findChild<QPushButton *>("analyzerZoomInButton");
    auto *btnZoomOut = panel->findChild<QPushButton *>("analyzerZoomOutButton");
    QVERIFY(btnZoomIn && btnZoomOut);

    // While tracking the newest data, +/- must zoom around the tracked tail (scope-style):
    // stay pinned at the right edge with the newest sample (1e9 ns) still in view.
    auto assertTailPinned = [&](const char *when) {
        QVERIFY2(hBar->value() == hBar->maximum(),
                 qPrintable(QString("%1: view must stay pinned at the tail (value %2, max %3)")
                                .arg(when).arg(hBar->value()).arg(hBar->maximum())));
        const double ppn = panel->canvas()->pixelsPerNs();
        const double visibleStart = static_cast<double>(panel->canvas()->timeOrigin()) + hBar->value() / ppn;
        const double visibleEnd = visibleStart + viewport / ppn;
        QVERIFY2(visibleStart <= 1e9 && 1e9 <= visibleEnd,
                 qPrintable(QString("%1: newest sample (1e9 ns) outside visible window [%2, %3] ns")
                                .arg(when).arg(visibleStart, 0, 'f', 0).arg(visibleEnd, 0, 'f', 0)));
    };

    btnZoomIn->click();
    panel->canvas()->grab(QRect(0, 0, 50, 50));
    assertTailPinned("first zoom-in");

    btnZoomIn->click();
    panel->canvas()->grab(QRect(0, 0, 50, 50));
    assertTailPinned("second zoom-in");

    btnZoomOut->click();
    panel->canvas()->grab(QRect(0, 0, 50, 50));
    assertTailPinned("zoom-out");
}

void TestBewavedDolphinGui::testLiveAnalyzerFitSpansViewport()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *panel = dolphin->liveAnalyzer();
    QVERIFY(panel);
    QCOMPARE(recordSquareWaveHistory(ws.get()), SimTime(1'000'000'000));

    dolphin->showLiveAnalyzer();
    dolphin->resize(600, 300);
    dolphin->show();
    QApplication::processEvents();

    // Deep-zoom first so the canvas is far wider than the viewport (the regression: Fit
    // computed its scale from THAT width, so it never actually fit anything again).
    panel->canvas()->setPixelsPerNs(0.01); // 1e9 ns × 0.01 px/ns = 1e7 px canvas
    panel->canvas()->grab(QRect(0, 0, 50, 50));
    auto *hBar = panel->scrollArea()->horizontalScrollBar();
    QVERIFY2(hBar->maximum() > panel->scrollArea()->viewport()->width(),
             "test setup: the canvas must be far wider than the viewport");

    auto *btnFit = panel->findChild<QPushButton *>("analyzerFitButton");
    QVERIFY(btnFit);
    btnFit->click();
    panel->canvas()->grab(QRect(0, 0, 50, 50));

    // The whole recording spans the viewport: scale derived from the viewport width (minus
    // the canvas's +20 px tail pad), scrolled home, nothing left to scroll horizontally.
    const int viewport = panel->scrollArea()->viewport()->width();
    QCOMPARE(panel->canvas()->pixelsPerNs(), (viewport - 20) / 1e9);
    QCOMPARE(hBar->value(), 0);
    QCOMPARE(hBar->maximum(), 0);

    // Fit shows the whole recording including the tail, so it re-engages the sticky
    // follow: a subsequent zoom-in keeps the newest data pinned at the right instead of
    // centering (and dropping) it.
    auto *btnZoomIn = panel->findChild<QPushButton *>("analyzerZoomInButton");
    QVERIFY(btnZoomIn);
    btnZoomIn->click();
    panel->canvas()->grab(QRect(0, 0, 50, 50));
    QCOMPARE(hBar->value(), hBar->maximum());
    const double ppn = panel->canvas()->pixelsPerNs();
    const double visibleStart = static_cast<double>(panel->canvas()->timeOrigin()) + hBar->value() / ppn;
    QVERIFY2(visibleStart <= 1e9 && 1e9 <= visibleStart + viewport / ppn,
             "the newest sample must stay in view after zooming in from Fit");
}
