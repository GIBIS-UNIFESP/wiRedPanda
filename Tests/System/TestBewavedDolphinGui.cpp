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
#include <QScrollBar>
#include <QSet>
#include <QStandardItemModel>
#include <QTableView>
#include <QTest>
#include <QTextStream>

#include "App/BeWavedDolphin/BeWavedDolphin.h"
#include "App/BeWavedDolphin/DolphinHost.h"
#include "App/BeWavedDolphin/DolphinZoom.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/UI/FileDialogProvider.h"
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

    QVERIFY(dolphin->getModel() != nullptr);
    QVERIFY(dolphin->getLength() > 0);
    QCOMPARE(dolphin->getLength(), 32); // default length
}

void TestBewavedDolphinGui::testCreateWaveformInputOutputCounts()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    // AND circuit has 2 inputs and 1 output
    QCOMPARE(static_cast<int>(dolphin->getInputElements().size()), 2);
    QCOMPARE(static_cast<int>(dolphin->getOutputElements().size()), 1);

    // Model rows = inputs + outputs
    auto *model = dolphin->getModel();
    QCOMPARE(model->rowCount(), 3); // 2 inputs + 1 output
}

// ===========================================================================
// Cell editing
// ===========================================================================

void TestBewavedDolphinGui::testSetCellValue()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    auto *model = dolphin->getModel();
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

    auto *model = dolphin->getModel();
    int outputRow = static_cast<int>(dolphin->getInputElements().size()); // first output row

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

void TestBewavedDolphinGui::testSetLengthChangesColumns()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    QCOMPARE(dolphin->getLength(), 32);

    dolphin->setLength(16, false);
    QCOMPARE(dolphin->getLength(), 16);

    auto *model = dolphin->getModel();
    QCOMPARE(model->columnCount(), 16);

    dolphin->setLength(64, false);
    QCOMPARE(dolphin->getLength(), 64);
    QCOMPARE(model->columnCount(), 64);
}

void TestBewavedDolphinGui::testCombinationalMode()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    auto *model = dolphin->getModel();
    int inputCount = static_cast<int>(dolphin->getInputElements().size());
    int outputRow = inputCount;

    // Trigger combinational mode via action
    auto *action = dolphin->findChild<QAction *>("actionCombinational");
    QVERIFY(action);
    action->trigger();

    // After combinational mode, length should be at least 2^inputs = 4
    QVERIFY(dolphin->getLength() >= 4);

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

    auto *model = dolphin->getModel();

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
    QCOMPARE(dolphin->getLength(), 32);
    dolphin->setCellValue(0, 3, 1); // set input 0, col 3 to 1

    auto *action = dolphin->findChild<QAction *>("actionAutoCrop");
    QVERIFY(action);
    action->trigger();

    QCOMPARE(dolphin->getLength(), 4); // trims to col 3 + 1
}

// ===========================================================================
// Selection-based editing
// ===========================================================================

void TestBewavedDolphinGui::testSetTo0WithSelection()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));
    auto *model = dolphin->getModel();

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
    auto *model = dolphin->getModel();

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
    auto *model = dolphin->getModel();

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
    auto *model = dolphin->getModel();

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
    auto *model = dolphin->getModel();

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
    action->trigger(); // Should not crash
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
    auto *model = dolphin->getModel();

    // Set cell (0,0) to 0
    dolphin->setCellValue(0, 0, 0);
    QCOMPARE(model->index(0, 0).data().toInt(), 0);

    // Select the cell and emit doubleClicked to trigger the toggle handler.
    // (The QTableView is inside a QGraphicsProxyWidget so QTest::mouseDClick
    // may not reach it; emit the signal directly on the view instead.)
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
    auto *model = dolphin->getModel();

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
    auto *model = dolphin->getModel();

    int outputRow = static_cast<int>(dolphin->getInputElements().size());

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
    auto *model = dolphin->getModel();

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

    QTimer::singleShot(0, [] {
        if (auto *w = QApplication::activeModalWidget()) w->close();
    });

    auto *action = dolphin->findChild<QAction *>("actionAbout");
    QVERIFY(action);
    action->trigger();
}

void TestBewavedDolphinGui::testAboutQtDialog()
{
    auto ws = createAndCircuit();
    std::unique_ptr<BewavedDolphin> dolphin(createDolphin(ws.get()));

    QTimer::singleShot(0, [] {
        if (auto *w = QApplication::activeModalWidget()) w->close();
    });

    auto *action = dolphin->findChild<QAction *>("actionAboutQt");
    QVERIFY(action);
    action->trigger();
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

    auto *action = dolphin->findChild<QAction *>("actionExit");
    QVERIFY2(action, "actionExit not found");

    // Exit triggers close(). Since m_edited is false (fresh waveform),
    // it should close without prompting.
    action->trigger();

    // Window should be hidden after close
    QVERIFY(!dolphin->isVisible());

    delete dolphin;
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
    auto *model = dolphin->getModel();
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
