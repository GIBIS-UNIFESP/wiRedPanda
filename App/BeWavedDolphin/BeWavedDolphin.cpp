// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/BeWavedDolphin.h"

#include <cmath>
#include <iostream>

#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QMimeData>
#include <QPrinter>
#include <QSaveFile>
#include <QTextStream>

#include "App/BeWavedDolphin/BeWavedDolphinUI.h"
#include "App/Core/Common.h"
#include "App/Core/Settings.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/GlobalProperties.h"
#include "App/IO/Serialization.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/UI/ClockDialog.h"
#include "App/UI/LengthDialog.h"
#include "App/UI/MainWindow.h"

SignalModel::SignalModel(const int inputs, const int rows, const int columns, QObject *parent)
    : QStandardItemModel(rows, columns, parent)
    // m_inputCount separates input rows (editable by the user) from output rows
    // (computed by the simulation); it is the boundary index used by BewavedDolphin::paste()
    , m_inputCount(inputs)
{
}

Qt::ItemFlags SignalModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    // Cells are read-only in the model; editing is done programmatically via createElement()
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

SignalDelegate::SignalDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

void SignalDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant value = index.data(Qt::DecorationRole);

    // In waveform (Line) mode, cells store a QPixmap representing the signal segment
    // (low, high, rising, falling). In Number mode there is no pixmap, so we fall through.
    if (value.canConvert<QPixmap>()) {
        QPixmap pixmap = qvariant_cast<QPixmap>(value);

        QRect cellRect = option.rect;
        QSize targetSize = cellRect.size();
        // Stretch the pre-rendered SVG pixmap to fill the cell so waveforms tile seamlessly
        QPixmap scaledPixmap = pixmap.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        // Draw the selection highlight behind the waveform image rather than over it
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        }

        int x = option.rect.x() + (option.rect.width() - scaledPixmap.width()) / 2;
        int y = option.rect.y() + (option.rect.height() - scaledPixmap.height()) / 2;

        painter->drawPixmap(x, y, scaledPixmap);

        return;
    }

    // Number mode: delegate to the standard item delegate to render the "0"/"1" text
    QItemDelegate::paint(painter, option, index);
}

DolphinGraphicsView::DolphinGraphicsView(QWidget *parent)
    : GraphicsView(parent) {}

bool DolphinGraphicsView::canZoomOut() const
{
    // Level 0 is the baseline (reset) zoom; don't allow zooming below it
    return m_zoomLevel > 0;
}

bool DolphinGraphicsView::canZoomIn() const
{
    // Cap at 6 discrete steps to prevent the waveform from becoming too large to render
    return m_zoomLevel < 6;
}

void DolphinGraphicsView::zoomIn()
{
    ++m_zoomLevel;
    emit zoomChanged();
}

void DolphinGraphicsView::zoomOut()
{
    --m_zoomLevel;
    emit zoomChanged();
}

void DolphinGraphicsView::resetZoom()
{
    m_zoomLevel = 0;
    emit zoomChanged();
}

void DolphinGraphicsView::wheelEvent(QWheelEvent *event)
{
    const int zoomDirection = event->angleDelta().y();

    // When redirect is enabled, the actual scaling is handled by BewavedDolphin
    // (which also resizes the table columns). Without redirect, the view scales itself.
    if (zoomDirection > 0 && canZoomIn()) {
        if (m_redirectZoom) {
            emit scaleIn();
        } else {
            zoomIn();
        }
    } else if (zoomDirection < 0 && canZoomOut()) {
        if (m_redirectZoom) {
            emit scaleOut();
        } else {
            zoomOut();
        }
    }

    // Keep the waveform centered under the cursor rather than jumping to the scene origin
    centerOn(QCursor::pos());

    event->accept();
}

BewavedDolphin::BewavedDolphin(Scene *scene, const bool askConnection, MainWindow *parent)
    : QMainWindow(parent)
    , m_ui(std::make_unique<BewavedDolphinUi>())
    , m_mainWindow(parent)
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

    restoreGeometry(Settings::value("beWavedDolphin/geometry").toByteArray());

    // Custom delegate renders SVG waveform segments instead of plain text in Line mode
    m_signalTableView->setItemDelegate(new SignalDelegate(this));

    // Embed the QTableView inside a QGraphicsScene so the whole waveform can be
    // uniformly scaled via the scene's transform without resizing individual rows/cols
    m_scene->addWidget(m_signalTableView);

    m_view.setScene(m_scene);
    // Scrollbars are suppressed because the scene is always resized to exactly fill the view
    m_view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // Redirect zoom events to BewavedDolphin so column widths are updated alongside the transform
    m_view.setRedirectZoom(true);
    m_ui->verticalLayout->addWidget(&m_view);

    m_ui->mainToolBar->setToolButtonStyle(Settings::value("labelsUnderIcons").toBool() ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);

    loadPixmaps();

    connect(&m_view,                   &DolphinGraphicsView::scaleIn,  this, &BewavedDolphin::on_actionZoomIn_triggered);
    connect(&m_view,                   &DolphinGraphicsView::scaleOut, this, &BewavedDolphin::on_actionZoomOut_triggered);
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
    Settings::setValue("beWavedDolphin/geometry", saveGeometry());
}

void BewavedDolphin::loadPixmaps()
{
    // Pre-render waveform segment SVGs to pixmaps once at startup.
    // Green = output rows, Blue = input rows. 100x38 is the baseline cell size
    // (matching the default column width of 49px × row height ≈ 38px) so the SVGs
    // are rasterised at a resolution that avoids blurring at the default zoom level.
    // The delegate stretches these pixmaps to fit the actual cell at render time.
    m_lowGreen = QPixmap(":/Interface/Dolphin/low_green.svg").scaled(100, 38);
    m_highGreen = QPixmap(":/Interface/Dolphin/high_green.svg").scaled(100, 38);
    m_fallingGreen = QPixmap(":/Interface/Dolphin/falling_green.svg").scaled(100, 38);
    m_risingGreen = QPixmap(":/Interface/Dolphin/rising_green.svg").scaled(100, 38);

    m_lowBlue = QPixmap(":/Interface/Dolphin/low_blue.svg").scaled(100, 38);
    m_highBlue = QPixmap(":/Interface/Dolphin/high_blue.svg").scaled(100, 38);
    m_fallingBlue = QPixmap(":/Interface/Dolphin/falling_blue.svg").scaled(100, 38);
    m_risingBlue = QPixmap(":/Interface/Dolphin/rising_blue.svg").scaled(100, 38);
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
        QFileInfo fileInfo(m_mainWindow->currentDir(), QFileInfo(fileName).fileName());

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
        throw PANDACEPTION("");
    }

    int rows = wordList.at(0).toInt();
    const int cols = wordList.at(1).toInt();

    // Clamp rows to the number of actual input ports to avoid out-of-bounds writes
    if (rows > m_inputPorts) {
        rows = m_inputPorts;
    }

    // 2 is the minimum meaningful simulation (need at least two time steps);
    // 2048 is the maximum simulation length enforced throughout the app
    if ((cols < 2) || (cols > 2048)) {
        throw PANDACEPTION("");
    }

    setLength(cols, false);

    for (int row = 0; row < rows; ++row) {
        str = cin.readLine();
        const auto wordList2(str.split(','));

        if (wordList2.size() < cols) {
            throw PANDACEPTION("");
        }

        for (int col = 0; col < cols; ++col) {
            const int value = wordList2.at(col).toInt();
            createElement(row, col, value, true);
        }
    }

    run();
}

void BewavedDolphin::prepare(const QString &fileName)
{
    qCDebug(zero) << "Updating window name with current: " << fileName;
    m_simulation = m_externalScene->simulation();

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

    // Topological sort ensures inputs are visited before the logic elements that depend on them,
    // giving a stable traversal order that matches the simulation update sequence
    const auto elements = Common::sortGraphicElements(m_externalScene->elements());

    if (elements.isEmpty()) {
        throw PANDACEPTION("Could not load enough elements for the simulation.");
    }

    for (auto *elm : elements) {
        // GraphicElement::Type is the QGraphicsItem type tag; non-graphic items (ports,
        // connections) share the scene and must be skipped here
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

    if (m_inputs.isEmpty() || m_outputs.isEmpty()) {
        throw PANDACEPTION("Could not load enough elements for the simulation.");
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

    // Rows = total signals (inputs + outputs); columns = simulation length in time steps.
    // inputLabels.size() is passed as the input count so SignalModel can distinguish
    // editable input rows from read-only output rows.
    m_model = new SignalModel(static_cast<int>(inputLabels.size()), static_cast<int>(inputLabels.size() + outputLabels.size()), m_length, this);
    m_signalTableView->setModel(m_model);

    // Input rows come first, then output rows — the split point is inputLabels.size()
    m_model->setVerticalHeaderLabels(inputLabels + outputLabels);

    m_signalTableView->setAlternatingRowColors(true);
    m_signalTableView->setShowGrid(false);

    // Fixed section sizes prevent the user from resizing columns, keeping waveforms aligned
    m_signalTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    m_signalTableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);

    // Start at minimum width of 1px so the table does not try to pre-allocate a wide
    // layout before zoom is applied; on_actionResetZoom_triggered() immediately sets
    // each column to 49px, which is the baseline width matching the 100×38 pixmaps
    m_signalTableView->horizontalHeader()->setDefaultSectionSize(1);

    qCDebug(zero) << "Inputs: " << inputLabels.size() << ", outputs: " << outputLabels.size();

    // Initialise all input cells to 0 and compute the first output sweep
    on_actionClear_triggered();

    connect(m_signalTableView,                   &QAbstractItemView::doubleClicked,      this, &BewavedDolphin::on_tableView_cellDoubleClicked);
    connect(m_signalTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &BewavedDolphin::on_tableView_selectionChanged);
}

void BewavedDolphin::on_tableView_cellDoubleClicked()
{
    const auto indexes = m_signalTableView->selectionModel()->selectedIndexes();

    // Toggle each selected cell between 0 and 1, then re-simulate
    for (auto &index : indexes) {
        int value = m_model->index(index.row(), index.column(), QModelIndex()).data().toInt();
        value = (value + 1) % 2;
        createElement(index.row(), index.column(), value);
    }

    run();
}

void BewavedDolphin::on_tableView_selectionChanged()
{
    m_externalScene->clearSelection();

    const auto indexes = m_signalTableView->selectionModel()->selectedIndexes();

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
    QVector<Status> oldValues(m_inputPorts);
    int oldIndex = 0;

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

            // Snapshot the live port state before the simulation sweep overwrites it
            oldValues[oldIndex] = input->outputPort(port)->status();
            ++oldIndex;
        }
    }

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

    m_oldInputValues = oldValues;
}

void BewavedDolphin::run()
{
    // Block the live simulation timer while we drive the circuit manually column by column
    qCDebug(zero) << "Creating class to pause main window simulator while creating waveform.";
    SimulationBlocker simulationBlocker(m_simulation);

    // --- Step through each time column and compute circuit outputs ---
    for (int column = 0; column < m_model->columnCount(); ++column) {
        qCDebug(four) << "Itr: " << column << ", inputs: " << m_inputs.size();
        int row = 0;

        for (auto *input : std::as_const(m_inputs)) {
            const bool isRotary = qobject_cast<InputRotary *>(input);
            for (int port = 0; port < input->outputSize(); ++port) {
                const bool value = m_model->index(row++, column).data().toBool();

                // Rotary encoders only accept "setOn" for the high state; low is implicit
                if (isRotary && value) {
                    input->setOn(1, port);
                } else if (!isRotary) {
                    input->setOn(value, port);
                }
            }
        }

        qCDebug(four) << "Updating the values of the circuit logic based on current input values.";
        m_simulation->update();

        // Write computed output port states back into the model's output rows
        qCDebug(four) << "Setting the computed output values to the waveform results.";
        row = m_inputPorts;

        for (auto *output : std::as_const(m_outputs)) {
            for (int port = 0; port < output->inputSize(); ++port) {
                const int value = static_cast<int>(output->inputPort(port)->status());
                // isInput=false → green output pixmaps; changeNext=false → caller will refresh
                createElement(row, column, value, false);
                ++row;
            }
        }
    }

    // Restore the circuit to the state it was in before the sweep so the live simulation
    // resumes correctly when the SimulationBlocker is destroyed
    qCDebug(three) << "Setting inputs back to old values.";
    restoreInputs();
}

void BewavedDolphin::restoreInputs()
{
    qCDebug(zero) << "Restoring old values to inputs, prior to simulation.";

    // Uses m_oldInputValues captured at loadSignals() time. Note that index maps to
    // the element level, not the port level — multi-port inputs share the same saved value.
    // This means a rotary encoder with N output ports restores all ports to the single
    // boolean snapshot taken before the sweep; fine-grained per-port restore is not needed
    // because the live simulation re-computes port values on the next tick anyway.
    for (int index = 0; index < m_inputs.size(); ++index) {
        auto *input = m_inputs.at(index);
        if (!input) {
            continue;
        }
        for (int port = 0; port < input->outputSize(); ++port) {
            const bool oldValue = static_cast<bool>(m_oldInputValues.at(index));

            if (input->outputSize() > 1) {
                input->setOn(oldValue, port);
            } else {
                input->setOn(oldValue);
            }
        }
    }
}

void BewavedDolphin::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    resizeScene();
}

void BewavedDolphin::resizeScene()
{
    const int newWidth = m_ui->centralwidget->width();
    // Subtract 2px to avoid a persistent vertical scrollbar appearing at the boundary
    const int newHeight = m_ui->centralwidget->height() - 2;

    // 4000px is a practical upper bound; beyond this Qt rendering becomes unreliable
    // and memory usage spikes. Reset zoom to recover a safe state.
    if (newWidth > 4000 or newHeight > 4000) {
        on_actionResetZoom_triggered();
        throw PANDACEPTION("Waveform would be too big! Resetting zoom.");
    }

    // The QTableView lives inside a QGraphicsProxyWidget which is then scaled by
    // m_scale via the QGraphicsView transform.  To make the scaled proxy fill the
    // viewport, the proxy's logical size must be (viewport_size / m_scale).
    // The additional 0.8 factor leaves ~20% breathing room for the vertical header
    // and avoids a last-pixel scrollbar from appearing at the boundary.
    m_signalTableView->resize(static_cast<int>(newWidth / (m_scale * 0.8)),
                              static_cast<int>(newHeight / (m_scale * 0.8)));
    m_scene->setSceneRect(m_scene->itemsBoundingRect());
}

void BewavedDolphin::on_actionExit_triggered()
{
    close();
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

void BewavedDolphin::createElement(const int row, const int col, const int value, const bool isInput, const bool changeNext)
{
    (value == 0) ? createZeroElement(row, col, isInput, changeNext)
                 : createOneElement(row, col, isInput, changeNext);
}

void BewavedDolphin::createZeroElement(const int row, const int col, const bool isInput, const bool changeNext)
{
    const auto index = m_model->index(row, col);

    // Capture the old value before overwriting so we know whether the next cell's
    // transition type (rising/falling) needs to be updated
    qCDebug(three) << "Getting current value to check if need to refresh next cell";
    const int currentValue = index.data().toInt();

    qCDebug(three) << "Changing current item.";
    m_model->setData(index, 0, Qt::DisplayRole);

    if (m_type == PlotType::Number) {
        m_model->setData(index, static_cast<uint>(Qt::AlignCenter), Qt::TextAlignmentRole);
    }

    if (m_type == PlotType::Line) {
        m_model->setData(index, static_cast<uint>(Qt::AlignLeft), Qt::TextAlignmentRole);

        const auto previousIndex = index.siblingAtColumn(col - 1);
        const bool hasPreviousItem = previousIndex.isValid();
        const bool isPreviousHigh = hasPreviousItem ? previousIndex.data().toInt() == 1 : false;

        // A 0 cell preceded by a 1 needs the falling-edge segment; otherwise flat-low
        if (isInput) {
            m_model->setData(index,
                             hasPreviousItem && isPreviousHigh ? m_fallingBlue : m_lowBlue,
                             Qt::DecorationRole);
        } else {
            m_model->setData(index,
                             hasPreviousItem && isPreviousHigh ? m_fallingGreen : m_lowGreen,
                             Qt::DecorationRole);
        }

        if (!changeNext) {
            return;
        }

        // If this cell changed from 1 to 0, the next cell's leading-edge type may have
        // changed (e.g. from flat-high to rising), so refresh it recursively
        const auto nextIndex = m_model->index(row, col + 1);

        if (nextIndex.isValid() && (currentValue == 1)) {
            qCDebug(three) << "Changing next item.";
            createElement(row, col + 1, nextIndex.data().toInt(), isInput, false);
        }
    }
}

void BewavedDolphin::createOneElement(const int row, const int col, const bool isInput, const bool changeNext)
{
    const auto index = m_model->index(row, col);

    // Capture the old value before overwriting so we know whether the next cell's
    // transition type needs to be updated
    qCDebug(three) << "Getting current value to check if need to refresh next cell";
    const int currentValue = index.data().toInt();

    qCDebug(three) << "Changing current item.";
    m_model->setData(index, 1, Qt::DisplayRole);

    if (m_type == PlotType::Number) {
        m_model->setData(index, static_cast<uint>(Qt::AlignCenter), Qt::TextAlignmentRole);
    }

    if (m_type == PlotType::Line) {
        m_model->setData(index, static_cast<uint>(Qt::AlignLeft), Qt::TextAlignmentRole);

        const auto previousIndex = index.siblingAtColumn(col - 1);
        const bool hasPreviousItem = previousIndex.isValid();
        const bool isPreviousLow = hasPreviousItem ? previousIndex.data().toInt() == 0 : false;

        // A 1 cell preceded by a 0 needs the rising-edge segment; otherwise flat-high
        if (isInput) {
            m_model->setData(index,
                             hasPreviousItem && isPreviousLow ? m_risingBlue : m_highBlue,
                             Qt::DecorationRole);
        } else {
            m_model->setData(index,
                             hasPreviousItem && isPreviousLow ? m_risingGreen : m_highGreen,
                             Qt::DecorationRole);
        }

        if (!changeNext) {
            return;
        }

        // If this cell changed from 0 to 1, the next cell may have become a falling edge
        const auto nextIndex = m_model->index(row, col + 1);

        if (nextIndex.isValid() && (currentValue == 0)) {
            qCDebug(three) << "Changing next item.";
            createElement(row, col + 1, nextIndex.data().toInt(), isInput, false);
        }
    }
}

void BewavedDolphin::show()
{
    QMainWindow::show();
    qCDebug(zero) << "Getting table dimensions.";
    resizeScene();
}

void BewavedDolphin::print()
{
    // Outputs in the same CSV format used by loadFromTerminal() / save(QSaveFile &),
    // allowing round-trip scripted use without a GUI
    std::cout << m_model->rowCount() << ",";
    std::cout << m_model->columnCount() << ",\n";

    for (int row = 0; row < m_model->rowCount(); ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            std::cout << m_model->item(row, col)->text().toStdString() << ",";
        }

        std::cout << "\n";
    }
}

void BewavedDolphin::saveToTxt(QTextStream &stream)
{
    // Force combinational mode so the truth table covers all 2^n input combinations
    on_actionCombinational_triggered();

    // Expand the length to exactly the full truth table; capped at 2048 by setLength()
    const int truthTableSize = static_cast<int>(std::pow(2, m_inputPorts));
    setLength(truthTableSize, false);

    // Write input rows first, then output rows, each followed by its signal label
    for (int row = 0; row < m_inputs.size(); ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            stream << m_model->item(row, col)->text();
        }

        QString label = m_model->verticalHeaderItem(row)->text();
        stream << " : \"" << label << "\"\n";
    }

    stream << "\n";

    for (int row = static_cast<int>(m_inputs.size()); row < m_model->rowCount(); ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            stream << m_model->item(row, col)->text();
        }

        QString label = m_model->verticalHeaderItem(row)->text();
        stream << " : \"" << label << "\"\n";
    }
}

void BewavedDolphin::on_actionSetTo0_triggered()
{
    qCDebug(zero) << "Pressed 0.";
    const auto itemList = m_signalTableView->selectionModel()->selectedIndexes();

    for (const auto &item : itemList) {
        const int row = item.row();
        const int col = item.column();
        qCDebug(zero) << "Editing value.";
        createZeroElement(row, col);
    }

    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionSetTo1_triggered()
{
    qCDebug(zero) << "Pressed 0.";
    const auto itemList = m_signalTableView->selectionModel()->selectedIndexes();

    for (const auto &item : itemList) {
        const int row = item.row();
        const int col = item.column();
        qCDebug(zero) << "Editing value.";
        createOneElement(row, col);
    }

    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionInvert_triggered()
{
    qCDebug(zero) << "Pressed Not.";
    const auto itemList = m_signalTableView->selectionModel()->selectedIndexes();

    for (const auto &item : itemList) {
        const int row = item.row();
        const int col = item.column();
        int value = m_model->index(row, col, QModelIndex()).data().toInt();
        value = (value + 1) % 2;
        qCDebug(zero) << "Editing value.";
        createElement(row, col, value);
    }

    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

int BewavedDolphin::sectionFirstColumn(const QItemSelection &ranges)
{
    int firstCol = m_model->columnCount() - 1;

    for (const auto &range : ranges) {
        if (range.left() < firstCol) {
            firstCol = range.left();
        }
    }

    return firstCol;
}

int BewavedDolphin::sectionFirstRow(const QItemSelection &ranges)
{
    int firstRow = m_model->rowCount() - 1;

    for (const auto &range : ranges) {
        if (range.top() < firstRow) {
            firstRow = range.top();
        }
    }

    return firstRow;
}

void BewavedDolphin::on_actionSetClockWave_triggered()
{
    qCDebug(zero) << "Getting first column.";
    const auto ranges = m_signalTableView->selectionModel()->selection();

    if (ranges.isEmpty()) {
        throw PANDACEPTION("No cells selected.");
    }

    // Anchor the clock phase to the leftmost selected column so the generated
    // waveform always starts with a LOW edge at the beginning of the selection,
    // regardless of where in the overall timeline the selection starts
    const int firstCol = sectionFirstColumn(ranges);

    qCDebug(zero) << "Setting the signal according to its column and clock period.";
    ClockDialog dialog(m_clockPeriod, this);
    const int clockPeriod = dialog.frequency();

    if (clockPeriod < 0) {
        return;
    }

    m_clockPeriod = clockPeriod;

    // First half of each period is LOW, second half is HIGH (fixed 50% duty cycle).
    // Integer division is intentional: odd periods round down, so the LOW half is
    // one step shorter than the HIGH half — acceptable for waveform visualisation.
    const int halfClockPeriod = clockPeriod / 2;
    const auto itemList = m_signalTableView->selectionModel()->selectedIndexes();

    for (const auto &item : itemList) {
        const int row = item.row();
        const int col = item.column();
        const int value = ((col - firstCol) % clockPeriod < halfClockPeriod ? 0 : 1);
        qCDebug(zero) << "Editing value.";
        createElement(row, col, value);
    }

    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionCombinational_triggered()
{
    // Ensure the table is at least as long as the full truth table (2^n columns)
    const int truthTableSize = static_cast<int>(std::min(2048., std::pow(2, m_inputPorts)));

    if (m_length < truthTableSize) {
        setLength(truthTableSize, false);
    }

    // Generate a standard binary counting pattern: the least-significant input (row 0)
    // toggles every column (period 2), row 1 toggles every 2 columns (period 4), etc.
    // This enumerates all 2^n input combinations in ascending binary order, which is
    // equivalent to a binary truth table with LSB on the first row.
    qCDebug(zero) << "Setting the signal according to its columns and clock period.";
    int halfClockPeriod = 1;
    int clockPeriod = 2;

    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            const int value = (col % clockPeriod < halfClockPeriod ? 0 : 1);
            createElement(row, col, value);
        }

        // Double the period for each successive input bit so each row toggles half as
        // often as the previous one.  524288 / 1048576 are 2^19 / 2^20 — chosen so that
        // 2*clockPeriod can never overflow a 32-bit int even after many doublings.
        halfClockPeriod = std::min(clockPeriod, 524288);
        clockPeriod = std::min(2 * clockPeriod, 1048576);
    }

    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionSetLength_triggered()
{
    qCDebug(zero) << "Setting the simulation length.";
    const int currentLength = m_length > 0 ? m_length : m_model->columnCount();
    LengthDialog dialog(currentLength, this);
    const int simLength = dialog.length();

    if (simLength < 0) {
        return;
    }

    setLength(simLength, true);
}

void BewavedDolphin::setLength(const int simLength, const bool runSimulation)
{
    if (m_length == simLength) {
        return;
    }

    m_length = simLength;

    if (simLength <= m_model->columnCount()) {
        // Shrinking: Qt's setColumnCount removes trailing columns automatically
        qCDebug(zero) << "Reducing or keeping the simulation length.";
        m_model->setColumnCount(simLength);
        resizeScene();
        m_edited = true;
        return;
    }

    // Growing: new input columns must be explicitly filled with zeros because
    // QStandardItemModel::setColumnCount leaves new cells as null QStandardItems;
    // output columns are populated by run() and don't need pre-filling
    qCDebug(zero) << "Increasing the simulation length.";
    const int oldLength = m_model->columnCount();
    m_model->setColumnCount(simLength);

    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = oldLength; col < simLength; ++col) {
            // changeNext=false: avoid cascading into further new (still unset) cells
            // which would attempt to read a null QStandardItem and crash
            createZeroElement(row, col, true, false);
        }
    }

    resizeScene();
    m_edited = true;
    qCDebug(zero) << "Running simulation.";

    if (runSimulation) {
        run();
    }
}

void BewavedDolphin::on_actionZoomOut_triggered()
{
    m_view.zoomOut();

    // Shrink column widths to match the reduced scene scale so pixmaps still tile correctly
    for (int col = 0; col < m_signalTableView->model()->columnCount(); ++col) {
        m_signalTableView->setColumnWidth(col, static_cast<int>(m_signalTableView->columnWidth(col) / m_scale));
    }

    resizeScene();
    zoomChanged();
}

void BewavedDolphin::on_actionZoomIn_triggered()
{
    m_view.zoomIn();

    // Grow column widths proportionally so waveform segments remain at their natural aspect ratio
    for (int col = 0; col < m_signalTableView->model()->columnCount(); ++col) {
        m_signalTableView->setColumnWidth(col, static_cast<int>(m_signalTableView->columnWidth(col) * m_scale));
    }

    resizeScene();
    zoomChanged();
}

void BewavedDolphin::on_actionResetZoom_triggered()
{
    m_view.resetZoom();
    // 1.25 is the base zoom scale factor — each zoom step multiplies/divides by this value.
    // Resetting m_scale here keeps it consistent with m_view's internal zoom level (0).
    // 49px is the column width that produces natural proportions for the 100×38 pixmaps
    // when rendered through a 1.25× transform.
    m_scale = 1.25;

    for (int col = 0; col < m_signalTableView->model()->columnCount(); ++col) {
        m_signalTableView->setColumnWidth(col, 49);
    }

    resizeScene();
    zoomChanged();
}

void BewavedDolphin::zoomChanged()
{
    m_ui->actionZoomIn->setEnabled(m_view.canZoomIn());
    m_ui->actionZoomOut->setEnabled(m_view.canZoomOut());
}

void BewavedDolphin::on_actionFitScreen_triggered()
{
    // First undo the current scale transform so the measurements below are in logical pixels
    m_view.scale(1.0 / m_scale, 1.0 / m_scale);
    // Compute the scale needed to fit all columns and all rows within the current view.
    // horizontalHeader()->length() gives the total pixel width of all column sections.
    // We add one extra columnWidth(0) / rowHeight(0) to account for the header section
    // itself (vertical header width / horizontal header height) which is not included in
    // the body length() value.  The +10 on the height is a small guard against the
    // horizontal scrollbar appearing when both axes are exactly at the boundary.
    const double wScale = static_cast<double>(m_view.width()) / (m_signalTableView->horizontalHeader()->length() + m_signalTableView->columnWidth(0));
    const double hScale = static_cast<double>(m_view.height()) / (m_signalTableView->verticalHeader()->length() + m_signalTableView->rowHeight(0) + 10);
    // Use the smaller scale so neither axis overflows the view
    m_scale = std::min(wScale, hScale);
    m_view.scale(1.0 * m_scale, 1.0 * m_scale);
    resizeScene();
}

void BewavedDolphin::on_actionClear_triggered()
{
    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            createZeroElement(row, col);
        }
    }

    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionAutoCrop_triggered()
{
    // Crop (or extend) the simulation to exactly the full truth table size for the
    // current number of input elements, then re-run to refresh output rows
    setLength(static_cast<int>(std::pow(2, m_inputs.length())), true);
}

void BewavedDolphin::on_actionCopy_triggered()
{
    const auto ranges = m_signalTableView->selectionModel()->selection();

    if (ranges.isEmpty()) {
        QApplication::clipboard()->clear();
        return;
    }

    // Serialise using a versioned header so paste can verify format compatibility
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writeDolphinHeader(stream);
    copy(ranges, stream);

    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-bewaveddolphin-waveform", itemData);

    QApplication::clipboard()->setMimeData(mimeData);
}

void BewavedDolphin::copy(const QItemSelection &ranges, QDataStream &stream)
{
    qCDebug(zero) << "Serializing data into data stream.";
    const int firstRow = sectionFirstRow(ranges);
    const int firstCol = sectionFirstColumn(ranges);
    const auto itemList = m_signalTableView->selectionModel()->selectedIndexes();
    stream << itemList.size();

    for (const auto &item : itemList) {
        const int row = item.row();
        const int col = item.column();
        const int data_ = m_model->index(row, col).data().toInt();
        // Store offsets relative to the selection origin so paste can re-anchor
        // the data at any target cell regardless of absolute position
        stream << static_cast<qint64>(row - firstRow);
        stream << static_cast<qint64>(col - firstCol);
        stream << static_cast<qint64>(data_);
    }
}

void BewavedDolphin::on_actionCut_triggered()
{
    const auto ranges = m_signalTableView->selectionModel()->selection();

    if (ranges.isEmpty()) {
        QApplication::clipboard()->clear();
        return;
    }

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writeDolphinHeader(stream);
    cut(ranges, stream);

    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-bewaveddolphin-waveform", itemData);

    QApplication::clipboard()->setMimeData(mimeData);

    m_edited = true;
}

void BewavedDolphin::cut(const QItemSelection &ranges, QDataStream &stream)
{
    copy(ranges, stream);
    on_actionSetTo0_triggered();
}

void BewavedDolphin::on_actionPaste_triggered()
{
    const auto ranges = m_signalTableView->selectionModel()->selection();

    if (ranges.isEmpty()) {
        return;
    }

    const auto *mimeData = QApplication::clipboard()->mimeData();
    QByteArray itemData;

    // Support both the legacy "bdolphin/copydata" MIME type (used before the format
    // was renamed) and the current one, so clipboard data from older app versions
    // can still be pasted.  The current format takes precedence if both are present.
    if (mimeData->hasFormat("bdolphin/copydata")) {
        itemData = mimeData->data("bdolphin/copydata");
    }

    if (mimeData->hasFormat("application/x-bewaveddolphin-waveform")) {
        itemData = mimeData->data("application/x-bewaveddolphin-waveform");
    }

    if (!itemData.isEmpty()) {
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        Serialization::readDolphinHeader(stream);
        paste(ranges, stream);
        m_edited = true;
    }
}

void BewavedDolphin::paste(const QItemSelection &ranges, QDataStream &stream)
{
    const int firstCol = sectionFirstColumn(ranges);
    const int firstRow = sectionFirstRow(ranges);
    quint64 itemListSize; stream >> itemListSize;

    for (int i = 0; i < static_cast<int>(itemListSize); ++i) {
        quint64 row;  stream >> row;
        quint64 col;  stream >> col;
        quint64 data_; stream >> data_;
        // Re-anchor the stored relative offsets to the paste-target cell
        const int newRow = static_cast<int>(static_cast<quint64>(firstRow) + row);
        const int newCol = static_cast<int>(static_cast<quint64>(firstCol) + col);

        // Silently drop cells that land outside the input rows or past the simulation length;
        // output rows are never editable
        if ((newRow < m_inputPorts) && (newCol < m_model->columnCount())) {
            createElement(newRow, newCol, static_cast<int>(data_));
        }
    }

    run();
}

void BewavedDolphin::on_actionSave_triggered()
{
    if (m_currentFile.fileName().isEmpty()) {
        on_actionSaveAs_triggered();
        return;
    }

    save(m_currentFile.absoluteFilePath());
    m_ui->statusbar->showMessage(tr("Saved file successfully."), 4000);
    m_edited = false;
}

void BewavedDolphin::on_actionSaveAs_triggered()
{
    const QString path = m_mainWindow->currentFile().absolutePath();

    QFileDialog fileDialog;
    fileDialog.setObjectName(tr("Save File as..."));

    // List the format that matches the current file first so it is the default selection
    const QString fileFilter = m_currentFile.fileName().endsWith(".csv") ?
                tr("CSV files (*.csv);;Dolphin files (*.dolphin);;All supported files (*.dolphin *.csv)")
              : tr("Dolphin files (*.dolphin);;CSV files (*.csv);;All supported files (*.dolphin *.csv)");

    fileDialog.setNameFilter(fileFilter);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setDirectory(path);
    fileDialog.setFileMode(QFileDialog::AnyFile);

    if (fileDialog.exec() == QDialog::Rejected) {
        return;
    }

    const auto files = fileDialog.selectedFiles();
    QString fileName = files.constFirst();

    if (fileName.isEmpty()) {
        return;
    }

    // Append the correct extension when the user types a bare name without one,
    // inferring the format from whichever filter was active in the dialog
    if (!fileName.endsWith(".dolphin") && !fileName.endsWith(".csv")) {
        if (fileDialog.selectedNameFilter().contains("dolphin")) {
            fileName.append(".dolphin");
        } else {
            fileName.append(".csv");
        }
    }

    save(fileName);
    m_currentFile = QFileInfo(fileName);
    associateToWiRedPanda(fileName);
    setWindowTitle(tr("beWavedDolphin Simulator") + " [" + m_currentFile.fileName() + "]");
    m_ui->statusbar->showMessage(tr("Saved file successfully."), 4000);
    m_edited = false;
}

void BewavedDolphin::save(const QString &fileName)
{
    // QSaveFile writes to a temp file and atomically renames on commit,
    // preventing data loss if the process is interrupted during a write
    QSaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        throw PANDACEPTION("Error opening file: %1", file.errorString());
    }

    if (fileName.endsWith(".dolphin")) {
        qCDebug(zero) << "Saving dolphin file.";
        QDataStream stream(&file);
        Serialization::writeDolphinHeader(stream);
        save(stream);
    } else {
        qCDebug(zero) << "Saving CSV file.";
        save(file);
    }

    if (!file.commit()) {
        throw PANDACEPTION("Error saving file: %1", file.errorString());
    }
}

void BewavedDolphin::save(QDataStream &stream)
{
    qCDebug(zero) << "Serializing data into data stream.";
    // .dolphin binary layout: header | inputPortCount | columnCount | values (col-major, inputs only).
    // Only input rows are stored because output rows are always fully determined by run();
    // this also keeps file size small when the circuit has many outputs.
    // qint64 is used for all numeric fields to ensure the format is stable across
    // 32-bit and 64-bit builds regardless of sizeof(int).
    stream << static_cast<qint64>(m_inputPorts);
    stream << static_cast<qint64>(m_model->columnCount());

    for (int col = 0; col < m_model->columnCount(); ++col) {
        for (int row = 0; row < m_inputPorts; ++row) {
            const int val = m_model->index(row, col).data().toInt();
            stream << static_cast<qint64>(val);
        }
    }
}

void BewavedDolphin::save(QSaveFile &file)
{
    // CSV format: first line is "rows,cols,"; subsequent lines are comma-separated row values
    // Both input and output rows are written so the CSV is human-readable without re-running
    file.write(QString::number(m_model->rowCount()).toUtf8());
    file.write(",");
    file.write(QString::number(m_model->columnCount()).toUtf8());
    file.write(",\n");

    for (int row = 0; row < m_model->rowCount(); ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            const QString val = m_model->index(row, col).data().toString();
            file.write(val.toUtf8());
            file.write(",");
        }

        file.write("\n");
    }
}

void BewavedDolphin::associateToWiRedPanda(const QString &fileName)
{
    if ((m_mainWindow->dolphinFileName() != fileName) && GlobalProperties::verbose) {
        const auto reply =
            QMessageBox::question(
                this,
                tr("wiRedPanda - beWavedDolphin"),
                tr("Do you want to link this beWavedDolphin file to your current wiRedPanda file and save it?"),
                QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            m_mainWindow->setDolphinFileName(fileName);
            m_mainWindow->save();
        }
    }
}

void BewavedDolphin::on_actionLoad_triggered()
{
    QDir defaultDirectory;

    // Prefer the last-used dolphin file's directory; fall back to the main window's
    // working directory, and finally to the home directory
    if (m_currentFile.exists()) {
        defaultDirectory.setPath(m_currentFile.absolutePath());
    } else {
        if (m_mainWindow->currentFile().exists()) {
            m_mainWindow->currentFile().dir();
        } else {
            defaultDirectory.setPath(QDir::homePath());
        }
    }

    const QString homeDir(m_mainWindow->currentDir().absolutePath());

    QFileDialog fileDialog;
    fileDialog.setObjectName(tr("Open File"));
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilter(tr("All supported files (*.dolphin *.csv);;Dolphin files (*.dolphin);;CSV files (*.csv)"));
    fileDialog.setDirectory(homeDir);

    if (fileDialog.exec() == QDialog::Rejected) {
        return;
    }

    const auto files = fileDialog.selectedFiles();
    const QString fileName = files.constFirst();

    if (fileName.isEmpty()) {
        return;
    }

    load(fileName);
    m_edited = false;
    m_ui->statusbar->showMessage(tr("File loaded successfully."), 4000);
}

void BewavedDolphin::load(const QString &fileName)
{
    QFile file(fileName);

    if (!file.exists()) {
        throw PANDACEPTION("File \"%1\" does not exist!", fileName);
    }

    qCDebug(zero) << "File exists.";

    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(zero) << "Could not open file in ReadOnly mode: " << file.errorString();
        throw PANDACEPTION("Could not open file in ReadOnly mode: %1", file.errorString());
    }

    if (fileName.endsWith(".dolphin")) {
        qCDebug(zero) << "Dolphin file opened.";
        QDataStream stream(&file);
        Serialization::readDolphinHeader(stream);
        qCDebug(zero) << "Loading in editor.";
        load(stream);
        qCDebug(zero) << "Current file set.";
        m_currentFile = QFileInfo(fileName);
    } else if (fileName.endsWith(".csv")) {
        qCDebug(zero) << "CSV file opened.";
        qCDebug(zero) << "Loading in editor.";
        load(file);
        qCDebug(zero) << "Current file set.";
        m_currentFile = QFileInfo(fileName);
    } else {
        qCDebug(zero) << "Format not supported. Could not open file: " << fileName;
        throw PANDACEPTION("Format not supported. Could not open file: %1", fileName);
    }

    qCDebug(zero) << "Closing file.";
    file.close();
    associateToWiRedPanda(fileName);
    setWindowTitle(tr("beWavedDolphin Simulator") + " [" + m_currentFile.fileName() + "]");
}

void BewavedDolphin::load(QDataStream &stream)
{
    qint64 rows; stream >> rows;
    qint64 cols; stream >> cols;

    // If the saved file has more input rows than the current circuit, ignore the excess
    if (rows > m_inputPorts) {
        rows = m_inputPorts;
    }

    if ((cols < 2) || (cols > 2048)) {
        throw PANDACEPTION("Invalid number of columns.");
    }

    // Resize the table to match the file (runSimulation=false: run() is called below)
    setLength(static_cast<int>(cols), false);
    qCDebug(zero) << "Update table.";

    // Data is stored col-major in the .dolphin binary format
    for (int col = 0; col < cols; ++col) {
        for (int row = 0; row < rows; ++row) {
            qint64 value; stream >> value;
            createElement(row, col, static_cast<int>(value), true);
        }
    }

    run();
}

void BewavedDolphin::load(QFile &file)
{
    // CSV is a single flat comma-separated string: "rows,cols,v00,v01,...,v(rows-1)(cols-1),"
    // (trailing comma on each row is tolerated; split(',') produces a trailing empty element)
    const QByteArray content = file.readAll();
    const auto wordList(content.split(','));
    if (wordList.size() < 2) {
        throw PANDACEPTION("Invalid CSV format: insufficient data.");
    }
    int rows = wordList.at(0).toInt();
    const int cols = wordList.at(1).toInt();

    if (rows > m_inputPorts) {
        rows = m_inputPorts;
    }

    if ((cols < 2) || (cols > 2048)) {
        throw PANDACEPTION("Invalid number of columns.");
    }

    setLength(cols, false);

    qCDebug(zero) << "Update table.";

    // Validate before indexing to avoid out-of-bounds access on corrupt files
    const int expectedSize = 2 + rows * cols;
    if (wordList.size() < expectedSize) {
        throw PANDACEPTION("Invalid CSV format: expected %1 elements, got %2.", expectedSize, static_cast<int>(wordList.size()));
    }

    // Values are stored row-major in the CSV: index = 2 + row*cols + col
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int value = wordList.at(2 + col + row * cols).toInt();
            createElement(row, col, value, true);
        }
    }

    run();
}

void BewavedDolphin::on_actionShowNumbers_triggered()
{
    m_type = PlotType::Number;

    // Clear the DecorationRole pixmaps that Line mode stored in each cell; if not
    // cleared, SignalDelegate::paint() would still render the old waveform image
    // behind (or on top of) the numeric "0"/"1" text produced by the base delegate
    for (int row = 0; row < m_model->rowCount(); ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            m_model->setData(m_model->index(row, col), QVariant(), Qt::DecorationRole);
        }
    }

    // Re-apply input cells through createElement so alignment roles are set for Number mode
    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            createElement(row, col, m_model->index(row, col).data().toInt());
        }
    }

    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionShowWaveforms_triggered()
{
    m_type = PlotType::Line;

    // Re-create all input cells so they receive the correct rising/falling pixmaps
    // for the current value sequence; output cells are refreshed by run() below
    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            createElement(row, col, m_model->index(row, col).data().toInt());
        }
    }

    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionExportToPng_triggered()
{
    QString pngFile = QFileDialog::getSaveFileName(this, tr("Export to Image"), m_currentFile.absolutePath(), tr("PNG files (*.png)"));

    if (pngFile.isEmpty()) {
        return;
    }

    if (!pngFile.endsWith(".png", Qt::CaseInsensitive)) {
        pngFile.append(".png");
    }

    QRectF sceneRect = m_scene->sceneRect();
    // Create a pixmap exactly the size of the scene so there is no blank padding
    QPixmap pixmap(sceneRect.size().toSize());

    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    // Render the full scene into the pixmap without any additional clipping
    m_scene->render(&painter, QRectF(), sceneRect);
    painter.end();

    pixmap.toImage().save(pngFile);
}

void BewavedDolphin::on_actionExportToPdf_triggered()
{
    QString pdfFile = QFileDialog::getSaveFileName(this, tr("Export to PDF"), m_currentFile.absolutePath(), tr("PDF files (*.pdf)"));

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

    // Expand the source rect by 64px on each side to add a visible margin around the
    // waveform in the PDF; without this the header labels would be clipped at the edge
    m_scene->render(&painter, QRectF(), m_scene->sceneRect().adjusted(-64, -64, 64, 64));
    painter.end();
}

void BewavedDolphin::on_actionAbout_triggered()
{
    QMessageBox::about(this,
        "beWavedDolphin",
        tr("<p>beWavedDolphin is a waveform simulator for the wiRedPanda software developed by the Federal University of São Paulo."
           " This project was created in order to help students learn about logic circuits.</p>"
           "<p>Software version: %1</p>"
           "<p><strong>Creators:</strong></p>"
           "<ul>"
           "<li> Prof. Fábio Cappabianco, Ph.D. </li>"
           "</ul>"
           "<p> beWavedDolphin is currently maintained by Prof. Fábio Cappabianco, Ph.D. and his students</p>"
           "<p> Please file a report at our GitHub page if bugs are found or if you wish for a new functionality to be implemented.</p>"
           "<p><a href=\"http://gibis-unifesp.github.io/wiRedPanda/\">Visit our website!</a></p>")
            .arg(QApplication::applicationVersion()));
}

void BewavedDolphin::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this);
}

