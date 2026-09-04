// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickDolphinController.h"

#include <cmath>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QItemSelectionModel>
#include <QRegularExpression>
#include <QSet>
#include <QTemporaryDir>
#include <QTextStream>

#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string

#include "App/BeWavedDolphin/DolphinFile.h"
#include "App/BeWavedDolphin/SignalModel.h"
#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Dolphin/QuickDolphinController.h"
#include "App/QuickShell/Dolphin/QuickDolphinExporter.h"
#include "App/Wiring/Connection.h"

namespace {

/// Builds a real InputSwitch/InputSwitch -> And -> Led circuit on \a canvas. Mirrors
/// TestBewavedDolphinGui.cpp's identical createAndCircuit() helper.
void buildAndCircuit(CanvasItem &canvas)
{
    auto *sw0 = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *sw1 = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *gate = ElementFactory::buildElement(ElementType::And);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    sw0->setPos(0, 0);
    sw1->setPos(0, 80);
    gate->setPos(160, 40);
    led->setPos(320, 40);

    auto *c1 = new Connection();
    c1->setStartPort(sw0->outputPort(0));
    c1->setEndPort(gate->inputPort(0));
    auto *c2 = new Connection();
    c2->setStartPort(sw1->outputPort(0));
    c2->setEndPort(gate->inputPort(1));
    auto *c3 = new Connection();
    c3->setStartPort(gate->outputPort(0));
    c3->setEndPort(led->inputPort(0));

    canvas.receiveCommand(new CanvasAddItemsCommand({sw0, sw1, gate, led}, &canvas));
}

/// Selects [startRow,startCol]..[endRow,endCol] on the controller's real selection model.
/// Mirrors TestBewavedDolphinGui.cpp's identical selectCells() helper against QTableView.
void selectCells(QuickDolphinController &controller, int startRow, int startCol, int endRow, int endCol)
{
    auto *model = controller.model();
    auto *selModel = controller.selectionModel();
    QItemSelection selection;
    selection.select(model->index(startRow, startCol), model->index(endRow, endCol));
    selModel->select(selection, QItemSelectionModel::ClearAndSelect);
}

} // namespace

// ===========================================================================
// Waveform creation
// ===========================================================================

void TestQuickDolphinController::testCreateBlankWaveform()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    QVERIFY(controller.model() != nullptr);
    QCOMPARE(controller.length(), 32); // default length
}

void TestQuickDolphinController::testCreateWaveformInputOutputCounts()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    QCOMPARE(controller.inputRows(), 2);
    QCOMPARE(controller.model()->rowCount(), 3); // 2 inputs + 1 output
}

// ===========================================================================
// Cell editing
// ===========================================================================

void TestQuickDolphinController::testSetCellValue()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.setCellValue(0, 0, 1);
    QCOMPARE(controller.cellValue(0, 0), 1);

    controller.setCellValue(0, 0, 0);
    QCOMPARE(controller.cellValue(0, 0), 0);
}

void TestQuickDolphinController::testRunSimulationFillsOutputs()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    const int outputRow = controller.inputRows();

    controller.setCellValue(0, 5, 1);
    controller.setCellValue(1, 5, 1);
    controller.run();

    QCOMPARE(controller.cellValue(outputRow, 5), 1); // AND(1,1) = 1
    QCOMPARE(controller.cellValue(outputRow, 0), 0); // AND(0,0) = 0
}

void TestQuickDolphinController::testSetLengthChangesColumns()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    QCOMPARE(controller.length(), 32);

    controller.setLength(16);
    QCOMPARE(controller.length(), 16);
    QCOMPARE(controller.model()->columnCount(), 16);

    controller.setLength(64);
    QCOMPARE(controller.length(), 64);
    QCOMPARE(controller.model()->columnCount(), 64);
}

void TestQuickDolphinController::testCombinationalMode()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    const int outputRow = controller.inputRows();

    controller.combinational();

    QVERIFY(controller.length() >= 4);
    QCOMPARE(controller.cellValue(outputRow, 3), 1); // input0=1,input1=1 -> AND = 1
    QCOMPARE(controller.cellValue(outputRow, 0), 0); // input0=0,input1=0 -> AND = 0
}

void TestQuickDolphinController::testOutputRowsIgnoreSetValue()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    const int outputRow = controller.inputRows();

    controller.run();
    const int outputVal = controller.cellValue(outputRow, 0);

    // Attempting to set an output cell directly has no lasting effect -- the next run()
    // recomputes it from logic, not from the manual override.
    controller.setCellValue(outputRow, 0, outputVal == 0 ? 1 : 0);
    controller.run();

    QCOMPARE(controller.cellValue(outputRow, 0), 0); // AND(0,0) = 0
}

// ===========================================================================
// Export
// ===========================================================================

void TestQuickDolphinController::testExportToPng()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString pngPath = tmpDir.filePath("export_test.png");

    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);
    controller.setCellValue(0, 0, 1);
    controller.run();

    QVERIFY(QuickDolphinExporter::exportToPng(controller.model(), PlotType::Line, pngPath));
    QVERIFY(QFile::exists(pngPath));
    QVERIFY(QFileInfo(pngPath).size() > 0);

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
}

void TestQuickDolphinController::testExportToPdf()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString pdfPath = tmpDir.filePath("export_test.pdf");

    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);
    controller.setCellValue(0, 0, 1);
    controller.run();

    QuickDolphinExporter::exportToPdf(controller.model(), PlotType::Line, pdfPath);
    QVERIFY(QFile::exists(pdfPath));
    QVERIFY(QFileInfo(pdfPath).size() > 0);
}

// ===========================================================================
// Editing actions
// ===========================================================================

void TestQuickDolphinController::testClearAction()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.setCellValue(0, 0, 1);
    controller.setCellValue(0, 1, 1);
    controller.setCellValue(1, 0, 1);
    QCOMPARE(controller.cellValue(0, 0), 1);

    controller.clearInputs();

    for (int col = 0; col < controller.model()->columnCount(); ++col) {
        QCOMPARE(controller.cellValue(0, col), 0);
        QCOMPARE(controller.cellValue(1, col), 0);
    }
}

void TestQuickDolphinController::testAutoCropAction()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.setLength(32);
    QCOMPARE(controller.length(), 32);
    controller.setCellValue(0, 3, 1);

    controller.autoCrop();

    QCOMPARE(controller.length(), 4); // trims to col 3 + 1
}

// ===========================================================================
// Selection-based editing
// ===========================================================================

void TestQuickDolphinController::testSetTo0WithSelection()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.setCellValue(0, 0, 1);
    controller.setCellValue(0, 1, 1);

    selectCells(controller, 0, 0, 0, 1);
    controller.setSelectedTo0();

    QCOMPARE(controller.cellValue(0, 0), 0);
    QCOMPARE(controller.cellValue(0, 1), 0);
}

void TestQuickDolphinController::testSetTo1WithSelection()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    QCOMPARE(controller.cellValue(0, 0), 0);

    selectCells(controller, 0, 0, 0, 3);
    controller.setSelectedTo1();

    for (int col = 0; col <= 3; ++col) {
        QCOMPARE(controller.cellValue(0, col), 1);
    }
}

void TestQuickDolphinController::testInvertWithSelection()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.setCellValue(0, 0, 0);
    controller.setCellValue(0, 1, 1);
    controller.setCellValue(0, 2, 0);
    controller.setCellValue(0, 3, 1);

    selectCells(controller, 0, 0, 0, 3);
    controller.invertSelected();

    QCOMPARE(controller.cellValue(0, 0), 1);
    QCOMPARE(controller.cellValue(0, 1), 0);
    QCOMPARE(controller.cellValue(0, 2), 1);
    QCOMPARE(controller.cellValue(0, 3), 0);
}

void TestQuickDolphinController::testCopyPasteWithSelection()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.setCellValue(0, 0, 1);
    controller.setCellValue(0, 1, 0);
    controller.setCellValue(0, 2, 1);
    controller.setCellValue(0, 3, 0);

    selectCells(controller, 0, 0, 0, 3);
    controller.copy();

    selectCells(controller, 0, 4, 0, 7);
    controller.paste();

    QCOMPARE(controller.cellValue(0, 4), 1);
    QCOMPARE(controller.cellValue(0, 5), 0);
    QCOMPARE(controller.cellValue(0, 6), 1);
    QCOMPARE(controller.cellValue(0, 7), 0);
}

void TestQuickDolphinController::testCutWithSelection()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.setCellValue(0, 0, 1);
    controller.setCellValue(0, 1, 1);
    controller.setCellValue(0, 2, 0);
    controller.setCellValue(0, 3, 0);

    selectCells(controller, 0, 0, 0, 1);
    controller.cut();

    QCOMPARE(controller.cellValue(0, 0), 0);
    QCOMPARE(controller.cellValue(0, 1), 0);

    selectCells(controller, 0, 4, 0, 5);
    controller.paste();

    QCOMPARE(controller.cellValue(0, 4), 1);
    QCOMPARE(controller.cellValue(0, 5), 1);
}

void TestQuickDolphinController::testPasteAtBoundary()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.setLength(8);
    controller.setCellValue(0, 0, 1);
    controller.setCellValue(0, 1, 0);
    controller.setCellValue(0, 2, 1);
    controller.setCellValue(0, 3, 0);

    selectCells(controller, 0, 0, 0, 3);
    controller.copy();

    // Only 2 columns available before the model's own boundary -- must not crash and must
    // fill whatever space is available.
    selectCells(controller, 0, 6, 0, 7);
    controller.paste();

    QCOMPARE(controller.cellValue(0, 6), 1);
    QCOMPARE(controller.cellValue(0, 7), 0);
}

void TestQuickDolphinController::testSetClockWaveWithSelection()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    selectCells(controller, 0, 0, 0, 7);
    controller.applyClockWave(4); // period=4, no modal dialog needed on the Quick side

    bool hasZero = false;
    bool hasOne = false;
    for (int col = 0; col <= 7; ++col) {
        const int val = controller.cellValue(0, col);
        if (val == 0) hasZero = true;
        if (val == 1) hasOne = true;
    }
    QVERIFY2(hasZero && hasOne, "Clock wave should produce an alternating 0/1 pattern");
}

void TestQuickDolphinController::testHasSelectionTracksSelectionState()
{
    // Quick's applyClockWave() itself has no empty-selection guard (DolphinWindow.qml's own
    // button is gated on hasSelection instead, mirroring the original's actionSetClockWave
    // enabled state) -- so the real thing worth locking in is that hasSelection tracks the
    // selection model.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    QVERIFY(!controller.hasSelection());

    selectCells(controller, 0, 0, 0, 3);
    QVERIFY(controller.hasSelection());

    controller.selectionModel()->clearSelection();
    QVERIFY(!controller.hasSelection());
}

void TestQuickDolphinController::testSelectCellPlainClickReplacesSelection()
{
    // SignalTable.qml's per-cell MouseArea has no built-in TableView selection to fall back on
    // (unlike QTableView) -- selectCell() is what a plain click wires up. Mirrors
    // QAbstractItemView::ExtendedSelection's own plain-click behavior: replaces whatever was
    // selected before with just this one cell.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    selectCells(controller, 0, 0, 0, 3);
    QCOMPARE(controller.selectionModel()->selectedIndexes().size(), 4);

    controller.selectCell(0, 1, Qt::NoModifier);

    const auto selected = controller.selectionModel()->selectedIndexes();
    QCOMPARE(selected.size(), 1);
    QCOMPARE(selected.constFirst().row(), 0);
    QCOMPARE(selected.constFirst().column(), 1);
}

void TestQuickDolphinController::testSelectCellCtrlClickTogglesSelection()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.selectCell(0, 0, Qt::NoModifier);
    controller.selectCell(0, 1, Qt::ControlModifier);
    QCOMPARE(controller.selectionModel()->selectedIndexes().size(), 2);

    // Ctrl-clicking an already-selected cell removes just that cell, keeping the rest.
    controller.selectCell(0, 0, Qt::ControlModifier);
    const auto selected = controller.selectionModel()->selectedIndexes();
    QCOMPARE(selected.size(), 1);
    QCOMPARE(selected.constFirst().column(), 1);
}

void TestQuickDolphinController::testSelectCellShiftClickSelectsRange()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.selectCell(0, 1, Qt::NoModifier); // anchor
    controller.selectCell(0, 4, Qt::ShiftModifier);

    const auto selected = controller.selectionModel()->selectedIndexes();
    QCOMPARE(selected.size(), 4); // columns 1..4 inclusive

    // A double-click's underlying invertSelected() then acts on the whole shift-selected range,
    // not just the last-clicked cell -- mirrors BewavedDolphin::on_tableView_cellDoubleClicked().
    controller.setCellValue(0, 1, 0);
    controller.setCellValue(0, 2, 0);
    controller.setCellValue(0, 3, 0);
    controller.setCellValue(0, 4, 0);
    controller.invertSelected();
    for (int col = 1; col <= 4; ++col) {
        QCOMPARE(controller.cellValue(0, col), 1);
    }
}

void TestQuickDolphinController::testSelectCellShiftClickCanShrinkAfterGrowing()
{
    // A second consecutive Shift-click must be able to shrink the range back down from the
    // first Shift-click's wider extent, not just grow further -- real
    // QAbstractItemViewPrivate::extendedSelectionCommand() returns SelectCurrent (Select |
    // Current) for Shift, and the Current flag is what keeps each Shift-click's range
    // provisional/replaceable rather than permanently baked into the selection model on every
    // call.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.selectCell(0, 1, Qt::NoModifier); // anchor
    controller.selectCell(0, 4, Qt::ShiftModifier); // grow to columns 1..4
    controller.selectCell(0, 2, Qt::ShiftModifier); // shrink back to columns 1..2

    QSet<int> selectedColumns;
    for (const auto &index : controller.selectionModel()->selectedIndexes()) {
        selectedColumns.insert(index.column());
    }
    QCOMPARE(selectedColumns, QSet<int>({1, 2}));
}

void TestQuickDolphinController::testSelectCellShiftClickUnionsWithPriorCtrlSelection()
{
    // Real QAbstractItemView::ExtendedSelection's extendedSelectionCommand() returns
    // SelectCurrent (a union, not a clear) for a Shift-click, never ClearAndSelect -- a
    // Ctrl-built non-contiguous selection must survive a later Shift-click range extension.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.selectCell(0, 0, Qt::NoModifier);
    controller.selectCell(0, 2, Qt::ControlModifier); // Ctrl-click builds {col 0, col 2}
    controller.selectCell(0, 4, Qt::ShiftModifier); // anchor is col 2 (the last plain/Ctrl click)

    const auto selected = controller.selectionModel()->selectedIndexes();
    QSet<int> selectedColumns;
    for (const auto &index : selected) {
        selectedColumns.insert(index.column());
    }
    QVERIFY2(selectedColumns.contains(0), "Ctrl-selected column 0 must survive the later Shift-click");
    for (int col = 2; col <= 4; ++col) {
        QVERIFY2(selectedColumns.contains(col), "the Shift-click range (columns 2..4) must be selected");
    }
}

void TestQuickDolphinController::testSelectionChangedHighlightsCorrespondingInputElement()
{
    // Mirrors BewavedDolphin::on_tableView_selectionChanged(): selecting a waveform row clears
    // the main canvas selection and highlights the input element that row corresponds to.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    const auto elements = canvas.elements();
    auto *sw0 = elements.at(0);
    auto *sw1 = elements.at(1);
    auto *led = elements.at(3);
    sw0->setSelected(false);
    sw1->setSelected(false);
    led->setSelected(true); // some unrelated pre-existing canvas selection

    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    // DolphinModelBuilder::collect() sorts input rows by label, not canvas insertion order --
    // look up sw0's real row instead of assuming row 0.
    const int sw0Row = controller.inputRow(sw0->label());
    QVERIFY(sw0Row >= 0);

    controller.selectCell(sw0Row, 0, Qt::NoModifier);

    QVERIFY(sw0->isSelected());
    QVERIFY(!sw1->isSelected());
    QVERIFY(!led->isSelected()); // the prior unrelated selection was cleared
}

// ===========================================================================
// View operations
// ===========================================================================

void TestQuickDolphinController::testZoomInOut()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.zoomIn();
    QVERIFY(controller.canZoomOut());

    controller.zoomIn();
    controller.zoomOut();
    controller.resetZoom();

    QVERIFY(controller.canZoomIn());
    QVERIFY(!controller.canZoomOut());
}

void TestQuickDolphinController::testShowNumbers()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    QVERIFY(!controller.numberMode());
    controller.showNumbers();
    QVERIFY(controller.numberMode());
}

void TestQuickDolphinController::testShowWaveforms()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.showNumbers();
    QVERIFY(controller.numberMode());
    controller.showWaveforms();
    QVERIFY(!controller.numberMode());
}

void TestQuickDolphinController::testFitScreenIsIdempotent()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);
    controller.setLength(256); // content far wider than the viewport

    controller.fitScreen(800.0, 500.0, 60.0, 24.0);
    const double firstFit = controller.fontScale();

    controller.fitScreen(800.0, 500.0, 60.0, 24.0);
    const double secondFit = controller.fontScale();

    QVERIFY(std::isfinite(firstFit));
    QVERIFY(firstFit > 0.0);
    QCOMPARE(secondFit, firstFit);
}

void TestQuickDolphinController::testZoomScaleTrackingA26()
{
    // Zoom In/Out is a horizontal "time stretch": a discrete level (0..6) scales the column
    // width only -- row height stays fixed -- and it is floored at the baseline.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    static constexpr double kZoomStep = 1.25;
    static constexpr qreal kDefaultColumnWidth = 38;
    static constexpr qreal kDefaultRowHeight = 30;

    QVERIFY(!controller.canZoomOut());
    QVERIFY(controller.canZoomIn());
    QCOMPARE(controller.columnWidth(), kDefaultColumnWidth);
    QCOMPARE(controller.rowHeight(), kDefaultRowHeight);

    controller.zoomIn();
    QCOMPARE(controller.columnWidth(), kDefaultColumnWidth * kZoomStep);
    QCOMPARE(controller.rowHeight(), kDefaultRowHeight); // untouched by column zoom
    QVERIFY(controller.canZoomOut());

    controller.zoomOut();
    QCOMPARE(controller.columnWidth(), kDefaultColumnWidth);
    QVERIFY(!controller.canZoomOut());

    // Floored at the baseline -- repeated triggers never go below it.
    controller.zoomOut();
    controller.zoomOut();
    QCOMPARE(controller.columnWidth(), kDefaultColumnWidth);

    // Capped at the max level -- repeated triggers never exceed it.
    for (int i = 0; i < 10; ++i) {
        controller.zoomIn();
    }
    QVERIFY(!controller.canZoomIn());
    const qreal maxWidth = controller.columnWidth();
    QVERIFY(maxWidth > kDefaultColumnWidth);

    controller.resetZoom();
    QCOMPARE(controller.columnWidth(), kDefaultColumnWidth);
    QVERIFY(!controller.canZoomOut());
    QVERIFY(controller.canZoomIn());
}

void TestQuickDolphinController::testFitScreenClampsAndGuardsA26()
{
    // FitScreen computes a uniform scale clamped to [kMinFitScale, kMaxFitScale] and must
    // never blow up to +inf from a tiny divisor.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    static constexpr double kMinFitScale = 0.05;
    static constexpr double kMaxFitScale = 20.0;

    // Huge viewport relative to minimal content -- would blow up without the max clamp.
    controller.fitScreen(1'000'000.0, 1'000'000.0, 0.0, 0.0);
    QVERIFY(std::isfinite(controller.fontScale()));
    QCOMPARE(controller.fontScale(), kMaxFitScale);

    // Tiny viewport relative to a lot of content -- would collapse toward zero without the
    // min clamp.
    controller.setLength(SignalModel::kMaxColumns);
    controller.fitScreen(1.0, 1.0, 0.0, 0.0);
    QVERIFY(std::isfinite(controller.fontScale()));
    QCOMPARE(controller.fontScale(), kMinFitScale);

    // Degenerate viewport (the header alone exceeds it) is a documented no-op, not a crash --
    // the scale from the previous call must survive unchanged, still finite.
    const double beforeDegenerate = controller.fontScale();
    controller.fitScreen(5.0, 5.0, 10.0, 10.0);
    QVERIFY(std::isfinite(controller.fontScale()));
    QCOMPARE(controller.fontScale(), beforeDegenerate);
}

// ===========================================================================
// Text output
// ===========================================================================

void TestQuickDolphinController::testSaveToTxtOutput()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.setLength(4);
    controller.setCellValue(0, 0, 0);
    controller.setCellValue(0, 1, 0);
    controller.setCellValue(0, 2, 1);
    controller.setCellValue(0, 3, 1);
    controller.setCellValue(1, 0, 0);
    controller.setCellValue(1, 1, 1);
    controller.setCellValue(1, 2, 0);
    controller.setCellValue(1, 3, 1);
    controller.run();

    QString output;
    QTextStream stream(&output);
    controller.saveToTxt(stream);
    stream.flush();

    QVERIFY(!output.isEmpty());
    const QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    QVERIFY2(lines.size() >= 3, qPrintable(QString("Expected >= 3 lines, got %1").arg(lines.size())));

    QVERIFY2(lines[0].contains('0'), "First input row should contain value 0");
    QVERIFY2(lines[0].contains('1'), "First input row should contain value 1");
    QVERIFY2(lines[1].contains('0'), "Second input row should contain value 0");
    QVERIFY2(lines[1].contains('1'), "Second input row should contain value 1");
    QVERIFY2(lines[2].contains('0'), "Output row should contain value 0");
    QVERIFY2(lines[2].contains('1'), "Output row should contain value 1 (AND of both inputs high)");
}

void TestQuickDolphinController::testSaveToTxtClampsColumnCountForManyInputPorts()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    QList<ItemWithId *> items;
    for (int i = 0; i < 12; ++i) {
        items.append(ElementFactory::buildElement(ElementType::InputSwitch));
    }
    items.append(ElementFactory::buildElement(ElementType::Led));
    canvas.receiveCommand(new CanvasAddItemsCommand(items, &canvas));

    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    QString text;
    QTextStream stream(&text);
    controller.saveToTxt(stream);

    const QString firstLine = text.section('\n', 0, 0);
    const qsizetype columnCount = firstLine.section(" : ", 0, 0).length();
    QCOMPARE(columnCount, static_cast<qsizetype>(SignalModel::kMaxColumns));
}

// ===========================================================================
// File round trip
// ===========================================================================

void TestQuickDolphinController::testSaveAndLoadWaveform()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString txtPath = tmpDir.filePath("test_waveform.txt");

    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);
    controller.setLength(4);
    controller.setCellValue(0, 0, 1);
    controller.setCellValue(0, 1, 0);
    controller.setCellValue(0, 2, 1);
    controller.setCellValue(0, 3, 0);
    controller.run();

    {
        QFile file(txtPath);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream stream(&file);
        controller.saveToTxt(stream);
        stream.flush();
    }

    QVERIFY(QFile::exists(txtPath));
    QVERIFY(QFileInfo(txtPath).size() > 0);

    QFile readFile(txtPath);
    QVERIFY(readFile.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = readFile.readAll();
    readFile.close();

    const QStringList lines = content.split('\n', Qt::SkipEmptyParts);
    QVERIFY2(lines.size() >= 3, "Should have at least 3 lines (2 inputs + 1 output)");
}

void TestQuickDolphinController::testCreateWaveformResolvesAbsolutePathOutsideProjectDir()
{
    // A waveform file living outside any project directory must still resolve by its
    // absolute path -- createWaveform() tries the stored path as-is first, only falling back
    // to the host's directory for a relative/missing path (and this controller has no
    // DolphinHost set at all here, so that fallback isn't even available).
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    QVERIFY(QDir(tmpDir.path()).mkpath("elsewhere"));
    const QString path = tmpDir.filePath("elsewhere/waveform.dolphin");

    CanvasItem canvas1(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas1);
    QuickDolphinController controller1;
    controller1.createWaveform(&canvas1);
    controller1.setCellValue(0, 0, 1);
    controller1.run();
    DolphinFile::save(*controller1.model(), path, controller1.inputRows());
    QVERIFY(QFile::exists(path));

    CanvasItem canvas2(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas2);
    QuickDolphinController controller2;
    controller2.createWaveform(&canvas2, path);

    QCOMPARE(controller2.cellValue(0, 0), 1);
}

// ===========================================================================
// Hardening
// ===========================================================================

void TestQuickDolphinController::hardeningRunAndSaveToTxtMustCheckElementsStillLive()
{
    const QString path = QString(QUOTE(CURRENTDIR)) + "/../App/QuickShell/Dolphin/QuickDolphinController.cpp";
    QFile src(path);
    QVERIFY2(src.open(QIODevice::ReadOnly), qPrintable(QString("Cannot open %1").arg(src.fileName())));
    const QString source = QString::fromUtf8(src.readAll());
    src.close();

    auto bodyOf = [&source](const QString &qualifiedName) -> QString {
        const QString pattern =
            QStringLiteral("\\b") + QRegularExpression::escape(qualifiedName)
            + QStringLiteral("\\s*\\([^)]*\\)\\s*(const)?\\s*\\{");
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

    const QStringList checkedFunctions = {"QuickDolphinController::run", "QuickDolphinController::saveToTxt"};
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
             qPrintable(QString("The following functions dereference m_inputs/m_outputs and must "
                                "check elementsStillLive() first -- an element deleted from the "
                                "live canvas since createWaveform() ran would otherwise dangle:\n  - %1")
                            .arg(missingCheck.join("\n  - "))));
}

void TestQuickDolphinController::testRunAndSaveToTxtHandleDeletedTrackedElement()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    // Delete the tracked InputSwitch directly via a real canvas command -- the same kind of
    // mutation an MCP client's delete_element would perform, entirely independent of this
    // controller's own edit paths.
    GraphicElement *trackedInput = nullptr;
    for (auto *elm : canvas.elements()) {
        if (elm->elementType() == ElementType::InputSwitch) {
            trackedInput = elm;
            break;
        }
    }
    QVERIFY(trackedInput);
    canvas.receiveCommand(new CanvasDeleteItemsCommand({trackedInput}, &canvas));

    controller.run(); // must skip the now-stale sweep, not dereference the deleted element

    QString text;
    QTextStream stream(&text);
    bool threw = false;
    try {
        controller.saveToTxt(stream);
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(threw, "saveToTxt() must fail cleanly, not silently export garbage/partial data");
}

// ===========================================================================
// Undo/redo
// ===========================================================================

void TestQuickDolphinController::testUndoStackStartsCleanAfterCreateWaveform()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    QVERIFY(!controller.canUndo());
}

void TestQuickDolphinController::testUndoRedoSetTo0()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.setCellValue(0, 0, 1);
    controller.setCellValue(0, 1, 1);

    selectCells(controller, 0, 0, 0, 1);
    controller.setSelectedTo0();

    QCOMPARE(controller.cellValue(0, 0), 0);
    QVERIFY(controller.canUndo());

    controller.undo();
    QCOMPARE(controller.cellValue(0, 0), 1);
    QCOMPARE(controller.cellValue(0, 1), 1);

    controller.redo();
    QCOMPARE(controller.cellValue(0, 0), 0);
    QCOMPARE(controller.cellValue(0, 1), 0);
}

void TestQuickDolphinController::testUndoRedoSetTo1()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    selectCells(controller, 0, 0, 0, 1);
    controller.setSelectedTo1();

    QCOMPARE(controller.cellValue(0, 0), 1);
    QCOMPARE(controller.cellValue(0, 1), 1);

    controller.undo();
    QCOMPARE(controller.cellValue(0, 0), 0);
    QCOMPARE(controller.cellValue(0, 1), 0);

    controller.redo();
    QCOMPARE(controller.cellValue(0, 0), 1);
    QCOMPARE(controller.cellValue(0, 1), 1);
}

void TestQuickDolphinController::testUndoRedoInvert()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.setCellValue(0, 0, 0);
    controller.setCellValue(0, 1, 1);

    selectCells(controller, 0, 0, 0, 1);
    controller.invertSelected();

    QCOMPARE(controller.cellValue(0, 0), 1);
    QCOMPARE(controller.cellValue(0, 1), 0);

    controller.undo();
    QCOMPARE(controller.cellValue(0, 0), 0);
    QCOMPARE(controller.cellValue(0, 1), 1);

    controller.redo();
    QCOMPARE(controller.cellValue(0, 0), 1);
    QCOMPARE(controller.cellValue(0, 1), 0);
}

void TestQuickDolphinController::testUndoRedoClear()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    controller.setCellValue(0, 0, 1);
    controller.setCellValue(0, 1, 1);
    controller.setCellValue(1, 0, 1);

    controller.clearInputs();

    for (int col = 0; col < controller.model()->columnCount(); ++col) {
        QCOMPARE(controller.cellValue(0, col), 0);
        QCOMPARE(controller.cellValue(1, col), 0);
    }

    controller.undo();
    QCOMPARE(controller.cellValue(0, 0), 1);
    QCOMPARE(controller.cellValue(0, 1), 1);
    QCOMPARE(controller.cellValue(1, 0), 1);
    for (int col = 2; col < controller.model()->columnCount(); ++col) {
        QCOMPARE(controller.cellValue(0, col), 0);
        QCOMPARE(controller.cellValue(1, col), 0);
    }

    controller.redo();
    for (int col = 0; col < controller.model()->columnCount(); ++col) {
        QCOMPARE(controller.cellValue(0, col), 0);
        QCOMPARE(controller.cellValue(1, col), 0);
    }
}

void TestQuickDolphinController::testUndoRedoSetClockWave()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    QVector<int> before;
    for (int col = 0; col <= 7; ++col) {
        before.append(controller.cellValue(0, col));
    }

    selectCells(controller, 0, 0, 0, 7);
    controller.applyClockWave(4);

    QVERIFY(controller.canUndo());

    controller.undo();
    for (int col = 0; col <= 7; ++col) {
        QCOMPARE(controller.cellValue(0, col), before.at(col));
    }
}

void TestQuickDolphinController::testUndoRedoCombinational()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);
    const int inputCount = controller.inputRows();
    const int outputRow = inputCount;

    controller.combinational();
    QCOMPARE(controller.cellValue(outputRow, 3), 1);

    controller.undo();

    // The value grid must revert to the pre-combinational (all-zero, freshly-created) state
    // across every input cell -- setLength() itself grew the column count and isn't undone
    // here (out of scope), but the value fill it exposed must be.
    for (int col = 0; col < controller.model()->columnCount(); ++col) {
        for (int row = 0; row < inputCount; ++row) {
            QCOMPARE(controller.cellValue(row, col), 0);
        }
    }
}

void TestQuickDolphinController::testUndoRedoMultipleOperationsRestoresOriginalState()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    QVector<int> pristine;
    for (int col = 0; col < controller.model()->columnCount(); ++col) {
        pristine.append(controller.cellValue(0, col));
        pristine.append(controller.cellValue(1, col));
    }

    selectCells(controller, 0, 0, 0, 1);
    controller.setSelectedTo1();

    selectCells(controller, 1, 0, 1, 1);
    controller.setSelectedTo1();

    selectCells(controller, 0, 0, 0, 1);
    controller.invertSelected();

    controller.undo();
    controller.undo();
    controller.undo();

    QVERIFY(!controller.canUndo());

    int i = 0;
    for (int col = 0; col < controller.model()->columnCount(); ++col) {
        QCOMPARE(controller.cellValue(0, col), pristine.at(i++));
        QCOMPARE(controller.cellValue(1, col), pristine.at(i++));
    }
}

void TestQuickDolphinController::testSetCellValueIsNotUndoable()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    buildAndCircuit(canvas);
    QuickDolphinController controller;
    controller.createWaveform(&canvas);

    QVERIFY(!controller.canUndo());
    controller.setCellValue(0, 0, 1);
    QCOMPARE(controller.cellValue(0, 0), 1);

    // Behavioral difference from BewavedDolphin::on_tableView_cellDoubleClicked() (which pushed
    // a SetCellsCommand): setCellValue() -- the Quick double-click entry point -- mutates
    // directly and never touches the undo stack.
    QVERIFY(!controller.canUndo());
}
