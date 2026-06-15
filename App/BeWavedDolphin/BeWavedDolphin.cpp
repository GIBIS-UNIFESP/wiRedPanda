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
#include <QMimeData>
#include <QPainter>
#include <QPrinter>
#include <QTableView>
#include <QTextStream>
#include <QWheelEvent>

#include "App/BeWavedDolphin/DolphinClipboard.h"
#include "App/BeWavedDolphin/DolphinFile.h"
#include "App/BeWavedDolphin/DolphinHost.h"
#include "App/BeWavedDolphin/Serializer.h"
#include "App/BeWavedDolphin/WaveformSimulator.h"
#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/SentryHelpers.h"
#include "App/Core/Settings.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/IO/Serialization.h"
#include "App/Scene/GraphicsView.h"
#include "App/UI/ClockDialog.h"
#include "App/UI/FileDialogProvider.h"
#include "App/UI/LengthDialog.h"

static constexpr int    kDefaultColumnWidth = 38;    ///<  Per-column pixel width at zoom 1.0 (matches the pre-refactor on-screen size).
static constexpr int    kDefaultRowHeight   = 30;    ///<  Per-row pixel height at zoom 1.0 (matches the pre-refactor on-screen size).
static constexpr int    kMaxSimLength       = 2048;  ///<  Maximum allowed simulation length.
static constexpr double kZoomStep           = 1.25;  ///<  Multiplicative factor per column-zoom step.
static constexpr int    kMaxZoomLevel       = 6;     ///<  Maximum discrete column-zoom step (baseline = 0).
static constexpr double kMinFitScale        = 0.05;  ///<  Smallest allowed Fit Screen scale.
static constexpr double kMaxFitScale        = 20.0;  ///<  Largest allowed Fit Screen scale.
static constexpr int    kExportCellWidth    = 50;    ///<  Per-column pixel width for PNG/PDF export.
static constexpr int    kExportCellHeight   = 40;    ///<  Per-row pixel height for PNG/PDF export.

BewavedDolphin::BewavedDolphin(Scene *scene, const bool askConnection, DolphinHost *host)
    : QMainWindow(nullptr)
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
        // Resolve the file relative to the main window's working directory so that
        // relative paths stored inside .panda files resolve correctly
        QFileInfo fileInfo(m_host->currentDir(), QFileInfo(fileName).fileName());

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
    // Protocol: first line is "rows,cols"; subsequent lines contain comma-separated 0/1 values per row.
    // This allows driving the simulator from scripts without a GUI file dialog.
    QTextStream cin(stdin);
    QString str = cin.readLine();
    const auto wordList(str.split(','));

    if (wordList.size() < 2) {
        throw PANDACEPTION("Invalid header: expected 'rows,cols' on the first line.");
    }

    int rows = wordList.at(0).toInt();
    const int cols = wordList.at(1).toInt();

    // Clamp rows to the number of actual input ports to avoid out-of-bounds writes
    if (rows > m_inputPorts) {
        rows = m_inputPorts;
    }

    if ((cols < 1) || (cols > kMaxSimLength)) {
        throw PANDACEPTION("Invalid column count %1: must be between 1 and %2.", QString::number(cols), QString::number(kMaxSimLength));
    }

    setLength(cols, false);

    for (int row = 0; row < rows; ++row) {
        str = cin.readLine();
        const auto wordList2(str.split(','));

        if (wordList2.size() < cols) {
            throw PANDACEPTION("Row %1 has %2 value(s) but %3 are required.", QString::number(row), QString::number(wordList2.size()), QString::number(cols));
        }

        for (int col = 0; col < cols; ++col) {
            const int value = wordList2.at(col).toInt();
            m_model->setValue(row, col, value);
        }
    }

    run();
}

void BewavedDolphin::prepare(const QString &fileName)
{
    qCDebug(zero) << "Updating window name with current: " << fileName;
    m_simulation = m_externalScene->simulation();
    // Construct the sweep driver before loadNewTable(), whose initial clear triggers run().
    m_simDriver = std::make_unique<WaveformSimulator>(m_externalScene, m_simulation);

    qCDebug(zero) << "Loading elements. All elements initially in elements vector. Then, inputs and outputs are extracted from it.";
    loadElements();

    qCDebug(zero) << "Loading initial data into the table.";
    loadNewTable();
}

void BewavedDolphin::loadElements()
{
    m_inputs.clear();
    m_outputs.clear();
    m_inputPorts = 0;

    const auto elements = m_externalScene->elements();

    if (elements.isEmpty()) {
        throw PANDACEPTION("The circuit is empty. Add input and output elements to generate a waveform.");
    }

    for (auto *elm : elements) {
        if (!elm || (elm->type() != GraphicElement::Type)) {
            continue;
        }

        if (elm->elementGroup() == ElementGroup::Input) {
            m_inputs.append(qobject_cast<GraphicElementInput *>(elm));
            // Multi-bit inputs (e.g. rotary encoder) contribute multiple port rows
            m_inputPorts += elm->outputSize();
        }

        if (elm->elementGroup() == ElementGroup::Output) {
            m_outputs.append(elm);
        }
    }

    // Stable sort by label so the waveform table ordering is deterministic and
    // matches what the user expects from the label names they assigned
    std::stable_sort(m_inputs.begin(), m_inputs.end(), [](const auto &elm1, const auto &elm2) {
        return QString::compare(elm1->label(), elm2->label(), Qt::CaseInsensitive) < 0;
    });

    std::stable_sort(m_outputs.begin(), m_outputs.end(), [](const auto &elm1, const auto &elm2) {
        return QString::compare(elm1->label(), elm2->label(), Qt::CaseInsensitive) < 0;
    });

    if (m_inputs.isEmpty() && m_outputs.isEmpty()) {
        throw PANDACEPTION("The circuit has no input or output elements. Add at least one input (e.g. Switch) and one output (e.g. LED) to generate a waveform.");
    }

    if (m_inputs.isEmpty()) {
        throw PANDACEPTION("The circuit has no input elements. Add at least one input (e.g. Switch, Button, or Clock) to generate a waveform.");
    }

    if (m_outputs.isEmpty()) {
        throw PANDACEPTION("The circuit has no output elements. Add at least one output (e.g. LED or Display) to generate a waveform.");
    }
}

void BewavedDolphin::loadNewTable()
{
    qCDebug(zero) << "Getting initial value from inputs and writing them to oldvalues. Used to save current state of inputs and restore it after simulation. Not saving memory states though...";
    qCDebug(zero) << "Also getting the name of the inputs. If no label is given, the element type is used as a name.";
    QStringList inputLabels;
    QStringList outputLabels;
    // loadSignals also snapshots current input port values into m_oldInputValues so
    // they can be restored after the simulation sweep completes
    loadSignals(inputLabels, outputLabels);

    qCDebug(zero) << "Num iter = " << m_length;

    // Rows = total signals (inputs + outputs); columns = simulation length in time steps
    m_model = new SignalModel(static_cast<int>(inputLabels.size() + outputLabels.size()), m_length, this);
    m_signalTableView->setModel(m_model);

    // Input rows come first, then output rows — the split point is inputLabels.size()
    m_model->setVerticalHeaderLabels(inputLabels + outputLabels);
    m_model->setInputRows(static_cast<int>(inputLabels.size()));

    // The delegate derives each cell's waveform from the model; tell it the current mode.
    m_delegate->setWaveformMode(m_type == PlotType::Line);

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
    const auto indexes = m_signalTableView->selectionModel()->selectedIndexes();

    // Toggle each selected cell between 0 and 1, then re-simulate
    for (auto &index : indexes) {
        int value = m_model->value(index.row(), index.column());
        value = (value + 1) % 2;
        m_model->setValue(index.row(), index.column(), value);
    }

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

void BewavedDolphin::loadSignals(QStringList &inputLabels, QStringList &outputLabels)
{
    for (auto *input : std::as_const(m_inputs)) {
        QString label = input->label();

        // Fall back to the element type name when the user hasn't given it a label
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(input->elementType());
        }

        for (int port = 0; port < input->outputSize(); ++port) {
            // Multi-port inputs (e.g. bus inputs) get indexed labels like "A[0]", "A[1]"
            if (input->outputSize() > 1) {
                inputLabels.append(label + "[" + QString::number(port) + "]");
            } else {
                inputLabels.append(label);
            }
        }
    }

    // Snapshot the live input port states before the simulation sweep overwrites them
    m_oldInputValues = WaveformSimulator::captureInputs(m_inputs, m_inputPorts);

    qCDebug(zero) << "Getting the name of the outputs. If no label is given, element type is used as a name.";

    for (auto *output : std::as_const(m_outputs)) {
        QString label = output->label();

        if (label.isEmpty()) {
            label = ElementFactory::translatedName(output->elementType());
        }

        // Outputs with multiple input ports (e.g. multi-bit displays) get indexed labels
        for (int port = 0; port < output->inputSize(); ++port) {
            if (output->inputSize() > 1) {
                outputLabels.append(label + "[" + QString::number(port) + "]");
            } else {
                outputLabels.append(label);
            }
        }
    }
}

void BewavedDolphin::run()
{
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
    // Two independent zoom axes: the discrete column-zoom (m_zoomLevel) widens columns
    // only — a horizontal "time stretch" — while Fit Screen (m_fitScale) scales columns,
    // rows, and font together. Sections use Fixed resize mode, so setting the header
    // default section size resizes every row/column uniformly without a per-cell loop.
    const double colScale = m_fitScale * std::pow(kZoomStep, m_zoomLevel);
    m_signalTableView->horizontalHeader()->setDefaultSectionSize(
        static_cast<int>(std::lround(kDefaultColumnWidth * colScale)));
    m_signalTableView->verticalHeader()->setDefaultSectionSize(
        static_cast<int>(std::lround(kDefaultRowHeight * m_fitScale)));

    // Scale the cell/header font with the Fit Screen factor only (column zoom leaves text
    // untouched), from the application's base point size.
    QFont font = QApplication::font();
    const double basePointSize = font.pointSizeF() > 0 ? font.pointSizeF() : 10.0;
    font.setPointSizeF(basePointSize * m_fitScale);
    m_signalTableView->setFont(font);
    m_signalTableView->horizontalHeader()->setFont(font);
    m_signalTableView->verticalHeader()->setFont(font);

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

void BewavedDolphin::createElement(const int row, const int col, const int value)
{
    // The model stores only the logic value; the delegate derives the waveform segment
    // (from this cell and its left neighbour) and the input/output colour at paint time.
    m_model->setValue(row, col, value);
}

void BewavedDolphin::show()
{
    QMainWindow::show();
    applyZoom();
}

void BewavedDolphin::print()
{
    // Outputs in the same CSV format used by loadFromTerminal() / the CSV save path,
    // allowing round-trip scripted use without a GUI. Read through SignalModel::value()
    // so never-set cells read as 0 instead of dereferencing a null item().
    std::cout << m_model->rowCount() << ",";
    std::cout << m_model->columnCount() << ",\n";

    for (int row = 0; row < m_model->rowCount(); ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            std::cout << m_model->value(row, col) << ",";
        }

        std::cout << "\n";
    }
}

void BewavedDolphin::saveToTxt(QTextStream &stream)
{
    // Dump the full combinational truth table. Build it in a throwaway model (like
    // renderWaveform() uses a throwaway view) so the live document is never mutated —
    // exporting must not clobber the user's waveform (e.g. an MCP persistent session).
    const int columns = static_cast<int>(std::pow(2, m_inputPorts));
    SignalModel truthTable(m_model->rowCount(), columns);

    fillCombinationalInputs(&truthTable, columns);

    // Compute the outputs for every input combination, then restore the live inputs the
    // sweep perturbed (the same capture/restore contract run() relies on).
    m_simDriver->sweep(
        m_inputs, m_outputs, m_inputPorts, columns,
        [&truthTable](int row, int col) { return truthTable.value(row, col) != 0; },
        [&truthTable](int row, int col, int value) { truthTable.setValue(row, col, value); });
    WaveformSimulator::restoreInputs(m_inputs, m_oldInputValues);

    // Write input rows first, then output rows, each followed by its signal label. Labels
    // come from the live model's headers, which the throwaway computation leaves untouched.
    for (int row = 0; row < m_inputs.size(); ++row) {
        for (int col = 0; col < columns; ++col) {
            stream << truthTable.value(row, col);
        }

        stream << " : \"" << m_model->verticalHeaderItem(row)->text() << "\"\n";
    }

    stream << "\n";

    for (int row = static_cast<int>(m_inputs.size()); row < m_model->rowCount(); ++row) {
        for (int col = 0; col < columns; ++col) {
            stream << truthTable.value(row, col);
        }

        stream << " : \"" << m_model->verticalHeaderItem(row)->text() << "\"\n";
    }
}

bool BewavedDolphin::exportToPng(const QString &filename)
{
    try {
        return renderWaveform(kExportCellWidth, kExportCellHeight).save(filename);
    } catch (...) {
        return false;
    }
}

void BewavedDolphin::applyToSelectedCells(const std::function<int(int)> &valueFn)
{
    const auto itemList = m_signalTableView->selectionModel()->selectedIndexes();
    for (const auto &item : itemList) {
        m_model->setValue(item.row(), item.column(), valueFn(item.data().toInt()));
    }
    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::fillCombinationalInputs(SignalModel *model, const int columns) const
{
    // Gray-code-like input patterns: row 0 toggles every 1 column (period=2), row 1 every
    // 2 columns (period=4), etc. Together they enumerate all input combinations.
    int halfClockPeriod = 1;
    int clockPeriod     = 2;

    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = 0; col < columns; ++col) {
            model->setValue(row, col, (col % clockPeriod < halfClockPeriod ? 0 : 1));
        }

        // Double the period for each successive input bit; cap at max int-safe values
        halfClockPeriod = (std::min)(clockPeriod, 524288);
        clockPeriod     = (std::min)(2 * clockPeriod, 1048576);
    }
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
        const int clockPeriod = dialog.frequency();

        if (clockPeriod < 0) {
            return;
        }

        m_clockPeriod = clockPeriod;

        // First half of each period is LOW, second half is HIGH (50% duty cycle)
        const int halfClockPeriod = clockPeriod / 2;
        const auto itemList = m_signalTableView->selectionModel()->selectedIndexes();

        for (const auto &item : itemList) {
            const int value = ((item.column() - firstCol) % clockPeriod < halfClockPeriod ? 0 : 1);
            m_model->setValue(item.row(), item.column(), value);
        }

        m_edited = true;
        qCDebug(zero) << "Running simulation.";
        run();
    });
}

void BewavedDolphin::on_actionCombinational_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Combinational mode"));
        const int truthTableSize = static_cast<int>((std::min)(static_cast<double>(kMaxSimLength), std::pow(2, m_inputPorts)));
        setLength(truthTableSize, false);

        qCDebug(zero) << "Setting the signal according to its columns and clock period.";
        fillCombinationalInputs(m_model, m_model->columnCount());

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
        const int simLength = dialog.length();

        if (simLength < 0) {
            return;
        }

        setLength(simLength, true);
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

    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = oldLength; col < simLength; ++col) {
            // changeNext=false: avoid cascading into further new (still unset) cells
            m_model->setValue(row, col, 0);
        }
    }

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
        // Column-width only; floored at the baseline (no shrink below the default width).
        if (m_zoomLevel > 0) {
            --m_zoomLevel;
        }
        applyZoom();
    });
}

void BewavedDolphin::on_actionZoomIn_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Zoom in"));
        // Column-width only; capped at kMaxZoomLevel discrete steps.
        if (m_zoomLevel < kMaxZoomLevel) {
            ++m_zoomLevel;
        }
        applyZoom();
    });
}

void BewavedDolphin::on_actionResetZoom_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Zoom reset"));
        m_zoomLevel = 0;
        m_fitScale  = 1.0;
        applyZoom();
    });
}

void BewavedDolphin::zoomChanged()
{
    m_ui->actionZoomIn->setEnabled(m_zoomLevel < kMaxZoomLevel);
    m_ui->actionZoomOut->setEnabled(m_zoomLevel > 0);
}

void BewavedDolphin::on_actionFitScreen_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Fit screen"));
        // Fit Screen scales everything (columns, rows, font) uniformly to fit, and resets
        // the discrete column zoom. The factor is computed analytically from the default
        // cell metrics so it is independent of the current zoom state.
        const int cols = m_model->columnCount();
        const int rows = m_model->rowCount();
        // Degenerate geometry (empty/hidden table): nothing to fit, leave the zoom untouched.
        if (cols <= 0 || rows <= 0) {
            return;
        }
        const QSize viewport = m_signalTableView->viewport()->size();
        // Subtract the fixed chrome (row-label gutter, column-header strip) from the
        // available area; the remainder must hold the unscaled cell grid.
        const double availW = viewport.width()  - m_signalTableView->verticalHeader()->width();
        const double availH = viewport.height() - m_signalTableView->horizontalHeader()->height();
        const double sW = availW / (kDefaultColumnWidth * cols);
        const double sH = availH / (kDefaultRowHeight   * rows);
        // A hidden or too-small viewport yields a non-positive scale; leave the zoom untouched.
        if (sW <= 0 || sH <= 0) {
            return;
        }
        m_fitScale  = std::clamp((std::min)(sW, sH), kMinFitScale, kMaxFitScale);
        m_zoomLevel = 0;
        applyZoom();
    });
}

void BewavedDolphin::on_actionClear_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Clear input"));
        for (int row = 0; row < m_inputPorts; ++row) {
            for (int col = 0; col < m_model->columnCount(); ++col) {
                m_model->setValue(row, col, 0);
            }
        }

        m_edited = true;
        qCDebug(zero) << "Running simulation.";
        run();
    });
}

void BewavedDolphin::on_actionAutoCrop_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Auto crop"));
        int lastNonZero = 0;
        for (int col = m_model->columnCount() - 1; col >= 0; --col) {
            bool allZero = true;
            for (int row = 0; row < m_inputPorts; ++row) {
                if (m_model->value(row, col) != 0) {
                    allZero = false;
                    break;
                }
            }
            if (!allZero) {
                lastNonZero = col;
                break;
            }
        }
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

        // Serialise using a versioned header so paste can verify format compatibility
        QByteArray itemData;
        QDataStream stream(&itemData, QIODevice::WriteOnly);
        Serialization::writeDolphinHeader(stream);
        DolphinClipboard::copy(*m_model, ranges, stream);

        auto *mimeData = new QMimeData();
        mimeData->setData("application/x-bewaveddolphin-waveform", itemData);

        QApplication::clipboard()->setMimeData(mimeData);
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

        QByteArray itemData;
        QDataStream stream(&itemData, QIODevice::WriteOnly);
        Serialization::writeDolphinHeader(stream);
        // Cut = copy the selection, then clear it (which re-runs the simulation).
        DolphinClipboard::copy(*m_model, ranges, stream);
        on_actionSetTo0_triggered();

        auto *mimeData = new QMimeData();
        mimeData->setData("application/x-bewaveddolphin-waveform", itemData);

        QApplication::clipboard()->setMimeData(mimeData);

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

        const auto *mimeData = QApplication::clipboard()->mimeData();
        QByteArray itemData;

        // Support both the legacy "bdolphin/copydata" MIME type and the current one,
        // so files or clipboard data from older versions of the app can still be pasted
        if (mimeData->hasFormat("bdolphin/copydata")) {
            itemData = mimeData->data("bdolphin/copydata");
        }

        if (mimeData->hasFormat("application/x-bewaveddolphin-waveform")) {
            itemData = mimeData->data("application/x-bewaveddolphin-waveform");
        }

        if (!itemData.isEmpty()) {
            QDataStream stream(&itemData, QIODevice::ReadOnly);
            Serialization::readDolphinHeader(stream);
            DolphinClipboard::paste(*m_model, ranges, stream);
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
                    tr("CSV files (*.csv);;Dolphin files (*.dolphin);;All supported files (*.dolphin *.csv)")
                  : tr("Dolphin files (*.dolphin);;CSV files (*.csv);;All supported files (*.dolphin *.csv)");

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
            tr("All supported files (*.dolphin *.csv);;Dolphin files (*.dolphin);;CSV files (*.csv)"));

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
        m_type = PlotType::Number;
        m_delegate->setWaveformMode(false);
        m_signalTableView->viewport()->update();
    });
}

void BewavedDolphin::on_actionShowWaveforms_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("waveform", QStringLiteral("Show waveforms"));
        m_type = PlotType::Line;
        m_delegate->setWaveformMode(true);
        m_signalTableView->viewport()->update();
    });
}

void BewavedDolphin::on_actionExportToPng_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("export", QStringLiteral("Waveform export PNG"));
        QString pngFile = FileDialogs::provider()->getSaveFileName(this, tr("Export to Image"), m_currentFile.absolutePath(), tr("PNG files (*.png)")).fileName;

        if (pngFile.isEmpty()) {
            return;
        }

        if (!pngFile.endsWith(".png", Qt::CaseInsensitive)) {
            pngFile.append(".png");
        }

        exportWaveformToPng(pngFile);
    });
}

bool BewavedDolphin::exportWaveformToPng(const QString &filename)
{
    return renderWaveform(kExportCellWidth, kExportCellHeight).save(filename);
}

QPixmap BewavedDolphin::renderWaveform(const int cellW, const int cellH) const
{
    // Render through a throwaway table bound to the same model, so the live view
    // (its zoom, selection, scroll position) is never disturbed by an export.
    QTableView view;
    view.setModel(m_model);
    auto *delegate = new SignalDelegate(&view);
    delegate->setWaveformMode(m_type == PlotType::Line);
    view.setItemDelegate(delegate);
    view.setShowGrid(false);
    view.setAlternatingRowColors(true);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    view.verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    view.horizontalHeader()->setDefaultSectionSize(cellW);
    view.verticalHeader()->setDefaultSectionSize(cellH);

    // Size the view to its full content so grab() captures every row/column with
    // no scrollbars and no blank padding.
    const int contentW = view.horizontalHeader()->length() + view.verticalHeader()->width();
    const int contentH = view.verticalHeader()->length() + view.horizontalHeader()->height();
    view.resize(contentW, contentH);

    return view.grab();
}

void BewavedDolphin::on_actionExportToPdf_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("export", QStringLiteral("Waveform export PDF"));
        QString pdfFile = FileDialogs::provider()->getSaveFileName(this, tr("Export to PDF"), m_currentFile.absolutePath(), tr("PDF files (*.pdf)")).fileName;

        if (pdfFile.isEmpty()) {
            return;
        }

        if (!pdfFile.endsWith(".pdf", Qt::CaseInsensitive)) {
            pdfFile.append(".pdf");
        }

        // Landscape A4 fits a reasonably long waveform without excessive scaling
        QPrinter printer(QPrinter::HighResolution);
        printer.setPageSize(QPageSize(QPageSize::A4));
        printer.setPageOrientation(QPageLayout::Orientation::Landscape);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(pdfFile);

        QPainter painter;

        if (!painter.begin(&printer)) {
            throw PANDACEPTION("Could not print this circuit to PDF.");
        }

        // Render the waveform offscreen, then scale it to fit the page preserving aspect
        const QPixmap pixmap = renderWaveform(kExportCellWidth, kExportCellHeight);
        const QSize target = pixmap.size().scaled(painter.viewport().size(), Qt::KeepAspectRatio);
        painter.drawPixmap(QRect(QPoint(0, 0), target), pixmap);
        painter.end();
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
