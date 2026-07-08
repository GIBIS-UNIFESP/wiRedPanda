// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/BeWavedDolphin.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QHeaderView>
#include <QMessageBox>
#include <QTableView>
#include <QTextStream>
#include <QWheelEvent>

#include "App/BeWavedDolphin/DolphinClipboard.h"
#include "App/BeWavedDolphin/DolphinEdits.h"
#include "App/BeWavedDolphin/DolphinExporter.h"
#include "App/BeWavedDolphin/DolphinFile.h"
#include "App/BeWavedDolphin/DolphinHost.h"
#include "App/BeWavedDolphin/DolphinModelBuilder.h"
#include "App/BeWavedDolphin/DolphinZoom.h"
#include "App/BeWavedDolphin/Serializer.h"
#include "App/BeWavedDolphin/WaveformSimulator.h"
#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/SentryHelpers.h"
#include "App/Core/Settings.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Exercise/ExerciseOverlay.h"
#include "App/Scene/GraphicsView.h"
#include "App/UI/ClockDialog.h"
#include "App/UI/FileDialogProvider.h"
#include "App/UI/LengthDialog.h"

BewavedDolphin::BewavedDolphin(Scene *scene, const bool askConnection, DolphinHost *host, QWidget *parent)
    : QMainWindow(parent)
    , m_ui(std::make_unique<BewavedDolphinUi>())
    , m_host(host)
    , m_externalScene(scene)
    // askConnection controls whether closing triggers a save-changes prompt and
    // whether saving offers to link the .dolphin file to the parent .panda file
    , m_askConnection(askConnection)
{
    m_ui->setupUi(this);
    m_ui->retranslateUi(this);

    // WA_DeleteOnClose ensures the window is freed when closed without the caller
    // needing to track its lifetime
    setAttribute(Qt::WA_DeleteOnClose);
    // Modal so the user cannot interact with the main circuit while the waveform is open
    setWindowModality(Qt::WindowModal);
    setWindowTitle(tr("beWavedDolphin Simulator"));

    resize(800, 500);

    restoreGeometry(Settings::dolphinGeometry());

    // The delegate paints the waveform cells; the table is the central widget directly
    m_delegate = new SignalDelegate(this);
    m_signalTableView->setItemDelegate(m_delegate);

    // Zoom state + view metrics live in DolphinZoom; the controller keeps the UI glue
    // (breadcrumbs, action enable-state via zoomChanged(), and the wheel event filter).
    m_zoom = std::make_unique<DolphinZoom>(m_signalTableView);

    // Native scrollbars let long waveforms scroll; zoom changes row/column metrics instead
    m_signalTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_signalTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_ui->verticalLayout->addWidget(m_signalTableView);

    // The mouse wheel over the table zooms the columns (see eventFilter)
    m_signalTableView->viewport()->installEventFilter(this);

    m_ui->mainToolBar->setToolButtonStyle(Settings::labelsUnderIcons() ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);

    connect(m_ui->actionAbout,         &QAction::triggered,            this, &BewavedDolphin::on_actionAbout_triggered);
    connect(m_ui->actionAboutQt,       &QAction::triggered,            this, &BewavedDolphin::on_actionAboutQt_triggered);
    connect(m_ui->actionClear,         &QAction::triggered,            this, &BewavedDolphin::on_actionClear_triggered);
    connect(m_ui->actionCombinational, &QAction::triggered,            this, &BewavedDolphin::on_actionCombinational_triggered);
    connect(m_ui->actionCopy,          &QAction::triggered,            this, &BewavedDolphin::on_actionCopy_triggered);
    connect(m_ui->actionCut,           &QAction::triggered,            this, &BewavedDolphin::on_actionCut_triggered);
    connect(m_ui->actionExit,          &QAction::triggered,            this, &BewavedDolphin::on_actionExit_triggered);
    connect(m_ui->actionExportToPdf,   &QAction::triggered,            this, &BewavedDolphin::on_actionExportToPdf_triggered);
    connect(m_ui->actionExportToPng,   &QAction::triggered,            this, &BewavedDolphin::on_actionExportToPng_triggered);
    connect(m_ui->actionFitScreen,     &QAction::triggered,            this, &BewavedDolphin::on_actionFitScreen_triggered);
    connect(m_ui->actionInvert,        &QAction::triggered,            this, &BewavedDolphin::on_actionInvert_triggered);
    connect(m_ui->actionLoad,          &QAction::triggered,            this, &BewavedDolphin::on_actionLoad_triggered);
    connect(m_ui->actionPaste,         &QAction::triggered,            this, &BewavedDolphin::on_actionPaste_triggered);
    connect(m_ui->actionResetZoom,     &QAction::triggered,            this, &BewavedDolphin::on_actionResetZoom_triggered);
    connect(m_ui->actionSave,          &QAction::triggered,            this, &BewavedDolphin::on_actionSave_triggered);
    connect(m_ui->actionSaveAs,        &QAction::triggered,            this, &BewavedDolphin::on_actionSaveAs_triggered);
    connect(m_ui->actionSetClockWave,  &QAction::triggered,            this, &BewavedDolphin::on_actionSetClockWave_triggered);
    connect(m_ui->actionSetLength,     &QAction::triggered,            this, &BewavedDolphin::on_actionSetLength_triggered);
    connect(m_ui->actionSetTo0,        &QAction::triggered,            this, &BewavedDolphin::on_actionSetTo0_triggered);
    connect(m_ui->actionSetTo1,        &QAction::triggered,            this, &BewavedDolphin::on_actionSetTo1_triggered);
    connect(m_ui->actionShowNumbers,   &QAction::triggered,            this, &BewavedDolphin::on_actionShowNumbers_triggered);
    connect(m_ui->actionShowWaveforms, &QAction::triggered,            this, &BewavedDolphin::on_actionShowWaveforms_triggered);
    connect(m_ui->actionZoomIn,        &QAction::triggered,            this, &BewavedDolphin::on_actionZoomIn_triggered);
    connect(m_ui->actionZoomOut,       &QAction::triggered,            this, &BewavedDolphin::on_actionZoomOut_triggered);
    connect(m_ui->actionAutoCrop,      &QAction::triggered,            this, &BewavedDolphin::on_actionAutoCrop_triggered);
}

BewavedDolphin::~BewavedDolphin()
{
    Settings::setDolphinGeometry(saveGeometry());
}

void BewavedDolphin::createWaveform(const QString &fileName)
{
    prepare(fileName);

    if (fileName.isEmpty()) {
        // No saved waveform — start with all-zero inputs and run once to populate outputs
        setWindowTitle(tr("beWavedDolphin Simulator"));
        run();
    } else {
        // Try the stored path as-is first (handles an absolute path); if that doesn't
        // resolve, fall back to just the filename relative to the main window's working
        // directory so that relative paths stored inside .panda files still resolve.
        QFileInfo fileInfo(fileName);
        if (fileInfo.isRelative() || !fileInfo.exists()) {
            fileInfo.setFile(m_host->currentDir(), QFileInfo(fileName).fileName());
        }

        if (!fileInfo.exists()) {
            m_ui->statusbar->showMessage(tr("File \"%1\" does not exist!").arg(fileName), 4000);
            return;
        }

        load(fileInfo.absoluteFilePath());
    }

    qCDebug(zero) << "Resuming digital circuit main window after waveform simulation is finished.";
    // Reset edit flag — loading a file or a fresh run does not constitute a user edit
    m_edited = false;
}

void BewavedDolphin::createWaveform()
{
    prepare();
    loadFromTerminal();
}

void BewavedDolphin::loadFromTerminal()
{
    // DolphinFile owns the stdin protocol parsing; applyWaveformData() applies the parsed
    // rows the same way a file load does (setLength + fill inputs + run).
    QTextStream cin(stdin);
    applyWaveformData(DolphinFile::parseTerminal(cin, m_inputPorts));
}

void BewavedDolphin::prepare(const QString &fileName)
{
    qCDebug(zero) << "Updating window name with current: " << fileName;
    m_simulation = m_externalScene->simulation();
    // Construct the sweep driver before loadNewTable(), whose initial clear triggers run().
    m_simDriver = std::make_unique<WaveformSimulator>(m_externalScene, m_simulation);

    qCDebug(zero) << "Collecting and ordering the scene's input/output elements.";
    const auto tableSignals = DolphinModelBuilder::collect(m_externalScene);
    m_inputs     = tableSignals.inputs;
    m_outputs    = tableSignals.outputs;
    m_inputPorts = tableSignals.inputPorts;

    qCDebug(zero) << "Loading initial data into the table.";
    loadNewTable(tableSignals.inputLabels, tableSignals.outputLabels);
}

void BewavedDolphin::loadNewTable(const QStringList &inputLabels, const QStringList &outputLabels)
{
    qCDebug(zero) << "Snapshotting current input values into oldvalues, to restore after simulation.";
    // Snapshot the live input port states before the simulation sweep overwrites them, so
    // they can be restored after the sweep completes.
    m_oldInputValues = WaveformSimulator::captureInputs(m_inputs, m_inputPorts);

    qCDebug(zero) << "Num iter = " << m_length;

    // Rows = total signals (inputs + outputs); columns = simulation length in time steps.
    // loadNewTable() runs exactly once per window (each is WA_DeleteOnClose), so this is a
    // single allocation; parenting to `this` ties its lifetime to the window — not re-created.
    m_model = new SignalModel(static_cast<int>(inputLabels.size() + outputLabels.size()), m_length, this);
    m_signalTableView->setModel(m_model);

    // Input rows come first, then output rows — the split point is inputLabels.size()
    m_model->setVerticalHeaderLabels(inputLabels + outputLabels);
    m_model->setInputRows(static_cast<int>(inputLabels.size()));

    // The delegate derives each cell's waveform from the model; it owns the current mode.

    // A cell's rising/falling edge depends on its left neighbour, so any value change must
    // repaint the whole visible grid (update() coalesces, so a full sweep is one repaint).
    connect(m_model, &QAbstractItemModel::dataChanged, this, [this] {
        m_signalTableView->viewport()->update();
    });

    m_signalTableView->setAlternatingRowColors(true);
    m_signalTableView->setShowGrid(false);

    // Fixed section sizes keep waveforms aligned; applyZoom() drives the actual
    // row/column sizes from the current zoom factor.
    m_signalTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    m_signalTableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);

    qCDebug(zero) << "Inputs: " << inputLabels.size() << ", outputs: " << outputLabels.size();

    // Initialise all input cells to 0 and compute the first output sweep
    on_actionClear_triggered();

    connect(m_signalTableView,                   &QAbstractItemView::doubleClicked,      this, &BewavedDolphin::on_tableView_cellDoubleClicked);
    connect(m_signalTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &BewavedDolphin::on_tableView_selectionChanged);

    // Size rows/columns/font for the current zoom so the table is correct even
    // before the window is shown (e.g. in headless tests and offscreen export).
    applyZoom();
}

void BewavedDolphin::on_tableView_cellDoubleClicked()
{
    // Toggle each selected cell between 0 and 1, then re-simulate
    DolphinEdits::applyToCells(*m_model, m_signalTableView->selectionModel()->selectedIndexes(),
                               [](int v) { return (v + 1) % 2; });
    run();
}

void BewavedDolphin::on_tableView_selectionChanged()
{
    m_externalScene->clearSelection();

    const auto indexes = m_signalTableView->selectionModel()->selectedIndexes();

    m_ui->actionSetClockWave->setEnabled(!indexes.isEmpty());

    // Highlight the corresponding input element in the circuit editor when the user
    // selects a waveform row, giving visual feedback about which signal they are editing.
    // Output rows (index >= m_inputs.size()) have no element to highlight.
    for (auto &index : indexes) {
        if (index.row() < m_inputs.size()) {
            m_inputs.at(index.row())->setSelected(true);
        }
    }

    m_externalScene->view()->update();
}

bool BewavedDolphin::elementsStillLive() const
{
    const auto sceneElements = m_externalScene->elements();
    for (auto *elm : std::as_const(m_inputs)) {
        if (!sceneElements.contains(elm)) {
            return false;
        }
    }
    for (auto *elm : std::as_const(m_outputs)) {
        if (!sceneElements.contains(elm)) {
            return false;
        }
    }
    return true;
}

void BewavedDolphin::run()
{
    // Guard against a caller invoking run() before prepare()/loadNewTable() has built
    // the sweep driver and table model (every current call site prepares first, but
    // this is a public entry point with no compile-time guarantee of that ordering).
    if (!m_simDriver || !m_model) {
        return;
    }

    // An input/output element this waveform was built from may have been deleted from the
    // live scene since prepare() ran — via the main canvas, or an MCP client, neither of
    // which this window's modality blocks (MCP mutates the scene directly, bypassing Qt
    // input routing entirely). Skip the sweep rather than dereferencing freed elements.
    if (!elementsStillLive()) {
        return;
    }

    // Drive the circuit across every time column. Inputs are read from the model and the
    // computed outputs (isInput=false → green; changeNext=false → caller refreshes) are
    // written back, then the original input states are restored so the live simulation
    // resumes correctly.
    m_simDriver->sweep(
        m_inputs, m_outputs, m_inputPorts, m_model->columnCount(),
        [this](int row, int col) { return m_model->value(row, col) != 0; },
        [this](int row, int col, int value) { m_model->setValue(row, col, value); });

    qCDebug(three) << "Setting inputs back to old values.";
    WaveformSimulator::restoreInputs(m_inputs, m_oldInputValues);
}

bool BewavedDolphin::eventFilter(QObject *watched, QEvent *event)
{
    // The mouse wheel zooms the columns (the original beWavedDolphin behavior): any
    // wheel over the table viewport widens/narrows the columns and is consumed, so the
    // table never scrolls on wheel. Use the scrollbar to pan a long waveform.
    if (watched == m_signalTableView->viewport() && event->type() == QEvent::Wheel) {
        auto *wheel = static_cast<QWheelEvent *>(event);
        if (wheel->angleDelta().y() > 0) {
            on_actionZoomIn_triggered();
        } else if (wheel->angleDelta().y() < 0) {
            on_actionZoomOut_triggered();
        }
        return true;
    }

    return QMainWindow::eventFilter(watched, event);
}

void BewavedDolphin::applyZoom()
{
    // The zoom math lives in DolphinZoom; the controller drives the action enable-state.
    m_zoom->apply();
    zoomChanged();
}

void BewavedDolphin::on_actionExit_triggered()
{
    Application::guardedSlot(this, [this] {
        close();
    });
}

void BewavedDolphin::closeEvent(QCloseEvent *event)
{
    (m_askConnection && checkSave()) ? event->accept() : event->ignore();
}

void BewavedDolphin::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (m_exerciseOverlay && m_exerciseOverlay->isVisible()) {
        m_exerciseOverlay->repositionToParent();
    }
}

void BewavedDolphin::setExerciseOverlay(ExerciseOverlay *overlay)
{
    m_exerciseOverlay = overlay;
}

QToolBar *BewavedDolphin::mainToolBar() const
{
    return m_ui->mainToolBar;
}

QAction *BewavedDolphin::actionCombinational() const
{
    return m_ui->actionCombinational;
}

void BewavedDolphin::triggerCombinational()
{
    on_actionCombinational_triggered();
}

bool BewavedDolphin::checkSave()
{
    if (!m_edited) {
        return true;
    }

    auto reply =
            QMessageBox::question(
                this,
                tr("wiRedPanda - beWavedDolphin"),
                tr("Save simulation before closing?"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (reply) {
    // After saving, check m_edited again: if the save itself failed (e.g. user dismissed
    // the Save As dialog), m_edited stays true and we should not allow the close
    case QMessageBox::Save:    on_actionSave_triggered(); return (!m_edited);
    case QMessageBox::Discard: return true;
    case QMessageBox::Cancel:  return false;
    default:                   return true;
    }
}

void BewavedDolphin::setCellValue(const int row, const int col, const int value)
{
    // The model stores only the logic value; the delegate derives the waveform segment
    // (from this cell and its left neighbour) and the input/output colour at paint time.
    m_model->setValue(row, col, value);
}

int BewavedDolphin::inputRow(const QString &label) const
{
    for (int row = 0; row < m_inputs.size(); ++row) {
        if (m_inputs.at(row)->label() == label) {
            return row;
        }
    }

    return -1;
}

BewavedDolphin::WaveformSnapshot BewavedDolphin::snapshot(const int duration) const
{
    // Read the computed waveform into a plain DTO so automation consumers (the MCP server)
    // need neither the live model nor the element vectors. Input rows come first; output
    // rows follow at the m_inputs.size() offset (mirroring the table's row layout).
    WaveformSnapshot result;

    for (int row = 0; row < m_inputs.size(); ++row) {
        Signal signal;
        signal.label = m_inputs.at(row)->label();
        for (int col = 0; col < duration; ++col) {
            signal.values.append(m_model->value(row, col));
        }
        result.inputs.append(signal);
    }

    for (int row = 0; row < m_outputs.size(); ++row) {
        Signal signal;
        signal.label = m_outputs.at(row)->label();
        const int outputRowIndex = static_cast<int>(m_inputs.size()) + row;
        for (int col = 0; col < duration; ++col) {
            signal.values.append(m_model->value(outputRowIndex, col));
        }
        result.outputs.append(signal);
    }

    return result;
}

void BewavedDolphin::show()
{
    QMainWindow::show();
    applyZoom();
}

void BewavedDolphin::print()
{
    if (!m_model) {
        return;
    }
    // Outputs in the same CSV format used by loadFromTerminal() / the CSV save path,
    // allowing round-trip scripted use without a GUI.
    std::cout << DolphinExporter::csvText(m_model).toStdString();
}

void BewavedDolphin::saveToTxt(QTextStream &stream)
{
    if (!m_model) {
        return;
    }

    // Same staleness hazard as run() (see elementsStillLive()); saveToTxt() dereferences
    // m_inputs/m_outputs via sweep() below too. Throw instead of silently no-op'ing so an
    // MCP client (the only realistic caller once run() already guards interactive use) gets
    // a clean error instead of a "successful" empty/truncated export.
    if (!elementsStillLive()) {
        throw PANDACEPTION("Cannot export: the circuit this waveform was built from has changed.");
    }

    // Dump the full combinational truth table. Build it in a throwaway model (like
    // renderWaveform() uses a throwaway view) so the live document is never mutated —
    // exporting must not clobber the user's waveform (e.g. an MCP persistent session).
    // Clamp to SignalModel::kMaxColumns, exactly like on_actionCombinational_triggered's
    // identical cap below — otherwise a circuit with ~25+ effective input ports allocates
    // tens of millions of cells, and m_inputPorts >= 31 overflows the int cast (UB).
    const int columns = static_cast<int>((std::min)(static_cast<double>(SignalModel::kMaxColumns), std::pow(2, m_inputPorts)));
    SignalModel truthTable(m_model->rowCount(), columns);

    // Carry the live model's row labels onto the throwaway model so the exporter, which
    // reads labels and values from a single model, formats the same headers as before.
    QStringList labels;
    labels.reserve(m_model->rowCount());
    for (int row = 0; row < m_model->rowCount(); ++row) {
        labels.append(m_model->verticalHeaderItem(row)->text());
    }
    truthTable.setVerticalHeaderLabels(labels);

    DolphinEdits::combinational(truthTable, m_inputPorts, columns);

    // Compute the outputs for every input combination, then restore the live inputs the
    // sweep perturbed (the same capture/restore contract run() relies on).
    m_simDriver->sweep(
        m_inputs, m_outputs, m_inputPorts, columns,
        [&truthTable](int row, int col) { return truthTable.value(row, col) != 0; },
        [&truthTable](int row, int col, int value) { truthTable.setValue(row, col, value); });
    WaveformSimulator::restoreInputs(m_inputs, m_oldInputValues);

    DolphinExporter::writeTruthTableText(stream, &truthTable, static_cast<int>(m_inputs.size()));
}

bool BewavedDolphin::exportToPng(const QString &filename)
{
    // Public façade for the MCP server: swallow exceptions and report success as a bool.
    try {
        return DolphinExporter::exportToPng(m_model, m_delegate->plotType(), filename);
    } catch (...) {
        return false;
    }
}

void BewavedDolphin::applyToSelectedCells(const std::function<int(int)> &valueFn)
{
    DolphinEdits::applyToCells(*m_model, m_signalTableView->selectionModel()->selectedIndexes(), valueFn);
    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionSetTo0_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Set cells to 0"));
        qCDebug(zero) << "Pressed 0.";
        applyToSelectedCells([](int) { return 0; });
    });
}

void BewavedDolphin::on_actionSetTo1_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Set cells to 1"));
        qCDebug(zero) << "Pressed 1.";
        applyToSelectedCells([](int) { return 1; });
    });
}

void BewavedDolphin::on_actionInvert_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Invert cells"));
        qCDebug(zero) << "Pressed Not.";
        applyToSelectedCells([](int v) { return (v + 1) % 2; });
    });
}

void BewavedDolphin::on_actionSetClockWave_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Set clock wave"));
        qCDebug(zero) << "Getting first column.";
        const auto ranges = m_signalTableView->selectionModel()->selection();

        if (ranges.isEmpty()) {
            throw PANDACEPTION("No cells selected.");
        }

        // Anchor the clock phase to the leftmost selected column so the waveform
        // starts at 0 regardless of where in the timeline the selection begins
        const int firstCol = DolphinClipboard::firstColumn(*m_model, ranges);

        qCDebug(zero) << "Setting the signal according to its column and clock period.";
        ClockDialog dialog(m_clockPeriod, this);

        if (dialog.exec() != QDialog::Accepted) {
            return;
        }

        const int clockPeriod = dialog.period();
        m_clockPeriod = clockPeriod;

        DolphinEdits::clockWave(*m_model, m_signalTableView->selectionModel()->selectedIndexes(), firstCol, clockPeriod);

        m_edited = true;
        qCDebug(zero) << "Running simulation.";
        run();
    });
}

void BewavedDolphin::on_actionCombinational_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Combinational mode"));
        const int truthTableSize = static_cast<int>((std::min)(static_cast<double>(SignalModel::kMaxColumns), std::pow(2, m_inputPorts)));
        setLength(truthTableSize, false);

        qCDebug(zero) << "Setting the signal according to its columns and clock period.";
        DolphinEdits::combinational(*m_model, m_inputPorts, m_model->columnCount());

        m_edited = true;
        qCDebug(zero) << "Running simulation.";
        run();
    });
}

void BewavedDolphin::on_actionSetLength_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Set length dialog"));
        qCDebug(zero) << "Setting the simulation length.";
        const int currentLength = m_length > 0 ? m_length : m_model->columnCount();
        LengthDialog dialog(currentLength, this);

        if (dialog.exec() != QDialog::Accepted) {
            return;
        }

        setLength(dialog.length(), true);
    });
}

void BewavedDolphin::setLength(const int simLength, const bool runSimulation)
{
    if (m_length == simLength) {
        return;
    }

    m_length = simLength;

    if (simLength <= m_model->columnCount()) {
        // Shrinking: Qt's setColumnCount removes trailing columns automatically.
        // New/removed columns inherit the Fixed default section size, so no resize needed.
        qCDebug(zero) << "Reducing or keeping the simulation length.";
        m_model->setColumnCount(simLength);
        m_edited = true;
        return;
    }

    // Growing: new input columns must be explicitly filled with zeros; output columns
    // are populated by run() and don't need pre-filling
    qCDebug(zero) << "Increasing the simulation length.";
    const int oldLength = m_model->columnCount();
    m_model->setColumnCount(simLength);
    DolphinEdits::growInputColumns(*m_model, m_inputPorts, oldLength, simLength);

    m_edited = true;
    qCDebug(zero) << "Running simulation.";

    if (runSimulation) {
        run();
    }
}

void BewavedDolphin::on_actionZoomOut_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Zoom out"));
        m_zoom->zoomOut();
        zoomChanged();
    });
}

void BewavedDolphin::on_actionZoomIn_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Zoom in"));
        m_zoom->zoomIn();
        zoomChanged();
    });
}

void BewavedDolphin::on_actionResetZoom_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Zoom reset"));
        m_zoom->reset();
        zoomChanged();
    });
}

void BewavedDolphin::zoomChanged()
{
    m_ui->actionZoomIn->setEnabled(m_zoom->canZoomIn());
    m_ui->actionZoomOut->setEnabled(m_zoom->canZoomOut());
}

void BewavedDolphin::on_actionFitScreen_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Fit screen"));
        m_zoom->fitScreen();
        zoomChanged();
    });
}

void BewavedDolphin::on_actionClear_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Clear input"));
        DolphinEdits::clearInputs(*m_model, m_inputPorts);

        m_edited = true;
        qCDebug(zero) << "Running simulation.";
        run();
    });
}

void BewavedDolphin::on_actionAutoCrop_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Auto crop"));
        const int lastNonZero = DolphinEdits::lastNonZeroColumn(*m_model, m_inputPorts);
        setLength(lastNonZero + 1, true);
    });
}

void BewavedDolphin::on_actionCopy_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("clipboard", QStringLiteral("Waveform copy"));
        const auto ranges = m_signalTableView->selectionModel()->selection();

        if (ranges.isEmpty()) {
            QApplication::clipboard()->clear();
            return;
        }

        DolphinClipboard::copyToClipboard(*m_model, ranges);
    });
}

void BewavedDolphin::on_actionCut_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("clipboard", QStringLiteral("Waveform cut"));
        const auto ranges = m_signalTableView->selectionModel()->selection();

        if (ranges.isEmpty()) {
            QApplication::clipboard()->clear();
            return;
        }

        // Cut = copy the selection, then clear it (which re-runs the simulation).
        DolphinClipboard::copyToClipboard(*m_model, ranges);
        on_actionSetTo0_triggered();
        m_edited = true;
    });
}

void BewavedDolphin::on_actionPaste_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("clipboard", QStringLiteral("Waveform paste"));
        const auto ranges = m_signalTableView->selectionModel()->selection();

        if (ranges.isEmpty()) {
            return;
        }

        if (DolphinClipboard::pasteFromClipboard(*m_model, ranges)) {
            m_edited = true;
            run();
        }
    });
}

void BewavedDolphin::on_actionSave_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("file", QStringLiteral("Waveform save"));
        if (m_currentFile.fileName().isEmpty()) {
            on_actionSaveAs_triggered();
            return;
        }

        DolphinFile::save(*m_model, m_currentFile.absoluteFilePath(), m_inputPorts);
        m_ui->statusbar->showMessage(tr("Saved file successfully."), 4000);
        m_edited = false;
    });
}

void BewavedDolphin::on_actionSaveAs_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("file", QStringLiteral("Waveform save as"));

        // List the format that matches the current file first so it is the default selection
        const QString fileFilter = m_currentFile.fileName().endsWith(".csv") ?
                    tr("CSV files") + " (*.csv);;" + tr("Dolphin files") + " (*.dolphin);;" + tr("All supported files") + " (*.dolphin *.csv)"
                  : tr("Dolphin files") + " (*.dolphin);;" + tr("CSV files") + " (*.csv);;" + tr("All supported files") + " (*.dolphin *.csv)";

        const QString initialPath = m_currentFile.fileName().isEmpty()
                                        ? m_host->currentFile().absolutePath()
                                        : m_currentFile.absoluteFilePath();

        const auto result = FileDialogs::provider()->getSaveFileName(this, tr("Save File as..."), initialPath, fileFilter);
        QString fileName = result.fileName;

        if (fileName.isEmpty()) {
            return;
        }

        // Append the correct extension when the user types a bare name without one,
        // inferring the format from whichever filter was active in the dialog
        if (!fileName.endsWith(".dolphin") && !fileName.endsWith(".csv")) {
            if (result.selectedFilter.contains("dolphin")) {
                fileName.append(".dolphin");
            } else {
                fileName.append(".csv");
            }
        }

        DolphinFile::save(*m_model, fileName, m_inputPorts);
        m_currentFile = QFileInfo(fileName);
        associateToWiRedPanda(fileName);
        setWindowTitle(tr("beWavedDolphin Simulator") + " [" + m_currentFile.fileName() + "]");
        m_ui->statusbar->showMessage(tr("Saved file successfully."), 4000);
        m_edited = false;
    });
}

void BewavedDolphin::associateToWiRedPanda(const QString &fileName)
{
    // Only prompt when the file is new (not already linked) and we are in interactive mode;
    // non-interactive (command-line / test) sessions skip the dialog entirely
    if ((m_host->dolphinFileName() != fileName) && Application::interactiveMode) {
        const auto reply =
            QMessageBox::question(
                this,
                tr("wiRedPanda - beWavedDolphin"),
                tr("Do you want to link this beWavedDolphin file to your current wiRedPanda file and save it?"),
                QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            m_host->setDolphinFileName(fileName);
            m_host->save({});
        }
    }
}

void BewavedDolphin::on_actionLoad_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("file", QStringLiteral("Waveform load dialog"));
        QDir defaultDirectory;

        // Prefer the last-used dolphin file's directory; fall back to the main window's
        // working directory, and finally to the home directory
        if (m_currentFile.exists()) {
            defaultDirectory.setPath(m_currentFile.absolutePath());
        } else {
            if (m_host->currentFile().exists()) {
                m_host->currentFile().dir();
            } else {
                defaultDirectory.setPath(QDir::homePath());
            }
        }

        const QString homeDir(m_host->currentDir().absolutePath());

        const QString fileName = FileDialogs::provider()->getOpenFileName(
            this, tr("Open File"), homeDir,
            tr("All supported files") + " (*.dolphin *.csv);;" + tr("Dolphin files") + " (*.dolphin);;" + tr("CSV files") + " (*.csv)");

        if (fileName.isEmpty()) {
            return;
        }

        load(fileName);
        m_edited = false;
        m_ui->statusbar->showMessage(tr("File loaded successfully."), 4000);
    });
}

void BewavedDolphin::load(const QString &fileName)
{
    // DolphinFile handles the on-disk format; we apply the parsed input rows and
    // record the association with the circuit file.
    applyWaveformData(DolphinFile::load(fileName, m_inputPorts));
    m_currentFile = QFileInfo(fileName);
    associateToWiRedPanda(fileName);
    setWindowTitle(tr("beWavedDolphin Simulator") + " [" + m_currentFile.fileName() + "]");
}

void BewavedDolphin::applyWaveformData(const DolphinSerializer::WaveformData &fileData)
{
    setLength(fileData.columns, false);
    qCDebug(zero) << "Update table.";

    for (int row = 0; row < fileData.inputPorts; ++row) {
        for (int col = 0; col < fileData.columns; ++col) {
            m_model->setValue(row, col, fileData.values[row * fileData.columns + col]);
        }
    }

    run();
}

void BewavedDolphin::on_actionShowNumbers_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Show numbers"));
        // Display mode is a pure view concern now: the model keeps the same values and the
        // delegate switches between numeric text and waveform rendering.
        m_delegate->setPlotType(PlotType::Number);
        m_signalTableView->viewport()->update();
    });
}

void BewavedDolphin::on_actionShowWaveforms_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Show waveforms"));
        m_delegate->setPlotType(PlotType::Line);
        m_signalTableView->viewport()->update();
    });
}

void BewavedDolphin::on_actionExportToPng_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("export", QStringLiteral("Waveform export PNG"));
        QString pngFile = FileDialogs::provider()->getSaveFileName(this, tr("Export to Image"), m_currentFile.absolutePath(), tr("PNG files") + " (*.png)").fileName;

        if (pngFile.isEmpty()) {
            return;
        }

        if (!pngFile.endsWith(".png", Qt::CaseInsensitive)) {
            pngFile.append(".png");
        }

        DolphinExporter::exportToPng(m_model, m_delegate->plotType(), pngFile);
    });
}

void BewavedDolphin::on_actionExportToPdf_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("export", QStringLiteral("Waveform export PDF"));
        QString pdfFile = FileDialogs::provider()->getSaveFileName(this, tr("Export to PDF"), m_currentFile.absolutePath(), tr("PDF files") + " (*.pdf)").fileName;

        if (pdfFile.isEmpty()) {
            return;
        }

        if (!pdfFile.endsWith(".pdf", Qt::CaseInsensitive)) {
            pdfFile.append(".pdf");
        }

        DolphinExporter::exportToPdf(m_model, m_delegate->plotType(), pdfFile);
    });
}

void BewavedDolphin::on_actionAbout_triggered()
{
    Application::guardedSlot(this, [this] {
        QMessageBox::about(this,
            "beWavedDolphin",
            tr("<p>beWavedDolphin is a waveform simulator for wiRedPanda, developed by the Federal University of São Paulo"
               " to help students learn about logic circuits.</p>"
               "<p>Software version: %1</p>"
               "<p><strong>Creators:</strong></p>"
               "<ul>"
               "<li> Prof. Fábio Cappabianco, Ph.D. </li>"
               "</ul>"
               "<p> beWavedDolphin is currently maintained by Prof. Fábio Cappabianco, Ph.D. and his students.</p>"
               "<p> Please file a report at our GitHub page if you find a bug or want to request a new feature.</p>"
               "<p><a href=\"http://gibis-unifesp.github.io/wiRedPanda/\">Visit our website!</a></p>")
                .arg(QApplication::applicationVersion()));
    });
}

void BewavedDolphin::on_actionAboutQt_triggered()
{
    Application::guardedSlot(this, [this] {
        QMessageBox::aboutQt(this);
    });
}
