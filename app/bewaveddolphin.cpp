// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bewaveddolphin.h"
#include "ui_bewaveddolphin.h"

#include "clockdialog.h"
#include "common.h"
#include "editor.h"
#include "elementfactory.h"
#include "elementmapping.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "graphicsview.h"
#include "graphicsviewzoom.h"
#include "input.h"
#include "lengthdialog.h"
#include "mainwindow.h"
#include "qneport.h"
#include "scstop.h"
#include "settings.h"
#include "simulationcontroller.h"

#include <QClipboard>
#include <QCloseEvent>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPrinter>
#include <QSaveFile>
#include <QTableView>
#include <cstring>

SignalModel::SignalModel(int rows, int inputs, int columns, QObject *parent)
    : QStandardItemModel(rows, columns, parent)
    , m_inputs(inputs)
{
}

Qt::ItemFlags SignalModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags;
    if (index.row() >= m_inputs) {
        flags = ~(Qt::ItemIsEditable | Qt::ItemIsSelectable);
    } else {
        flags = ~(Qt::ItemIsEditable | Qt::ItemIsSelectable) | Qt::ItemIsSelectable;
    }
    return flags;
}

SignalDelegate::SignalDelegate(int margin, QObject *parent)
    : QItemDelegate(parent)
    , m_margin(margin)
{
}

void SignalDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem itemOption(option);
    itemOption.rect.adjust(-m_margin, 0, 0, 0);
    QItemDelegate::paint(painter, itemOption, index);
}

BewavedDolphin::BewavedDolphin(Editor *editor, QWidget *parent, bool ask_connection)
    : QMainWindow(parent)
    , m_ask_connection(ask_connection)
    , m_ui(new Ui::BewavedDolphin)
    , m_editor(editor)
    , m_mainWindow(dynamic_cast<MainWindow *>(parent))
    , m_type(PlotType::Line)
{
    m_ui->setupUi(this);
    m_scale = 1.0;
    resize(800, 500);
    setWindowTitle(tr("beWavedDolphin Simulator"));
    setWindowFlags(Qt::Window);
    Settings::beginGroup("beWavedDolphin");
    restoreGeometry(Settings::value("geometry").toByteArray());
    Settings::endGroup();
    m_gv = new GraphicsView(this);
    m_ui->verticalLayout->addWidget(m_gv);
    m_scene = new QGraphicsScene(this);
    m_gv->setScene(m_scene);
    m_signalTableView = new QTableView;
    m_scene->installEventFilter(m_signalTableView);
    auto *delegate = new SignalDelegate(4, this);
    m_signalTableView->setItemDelegate(delegate);
    m_scene->addWidget(m_signalTableView);
    QList<QKeySequence> zoom_in_shortcuts;
    zoom_in_shortcuts << QKeySequence("Ctrl++") << QKeySequence("Ctrl+=");
    m_ui->actionZoom_In->setShortcuts(zoom_in_shortcuts);
    // connect(gv->gvzoom(), &GraphicsViewZoom::zoomed, this, &BewavedDolphin::zoomChanged);
    m_gv->gvzoom()->setZoomFactorBase(m_SCALE_FACTOR);
    drawPixMaps();
    m_edited = false;
    m_ui->retranslateUi(this);

    connect(m_ui->actionAbout, &QAction::triggered, this, &BewavedDolphin::on_actionAbout_triggered);
    connect(m_ui->actionAbout_Qt, &QAction::triggered, this, &BewavedDolphin::on_actionAbout_Qt_triggered);
    connect(m_ui->actionClear, &QAction::triggered, this, &BewavedDolphin::on_actionClear_triggered);
    connect(m_ui->actionCombinational, &QAction::triggered, this, &BewavedDolphin::on_actionCombinational_triggered);
    connect(m_ui->actionCopy, &QAction::triggered, this, &BewavedDolphin::on_actionCopy_triggered);
    connect(m_ui->actionCut, &QAction::triggered, this, &BewavedDolphin::on_actionCut_triggered);
    connect(m_ui->actionExit, &QAction::triggered, this, &BewavedDolphin::on_actionExit_triggered);
    connect(m_ui->actionExport_to_PDF, &QAction::triggered, this, &BewavedDolphin::on_actionExport_to_PDF_triggered);
    connect(m_ui->actionExport_to_PNG, &QAction::triggered, this, &BewavedDolphin::on_actionExport_to_PNG_triggered);
    connect(m_ui->actionInvert, &QAction::triggered, this, &BewavedDolphin::on_actionInvert_triggered);
    connect(m_ui->actionLoad, &QAction::triggered, this, &BewavedDolphin::on_actionLoad_triggered);
    connect(m_ui->actionPaste, &QAction::triggered, this, &BewavedDolphin::on_actionPaste_triggered);
    connect(m_ui->actionReset_Zoom, &QAction::triggered, this, &BewavedDolphin::on_actionReset_Zoom_triggered);
    connect(m_ui->actionSave, &QAction::triggered, this, &BewavedDolphin::on_actionSave_triggered);
    connect(m_ui->actionSave_as, &QAction::triggered, this, &BewavedDolphin::on_actionSave_as_triggered);
    connect(m_ui->actionSet_Length, &QAction::triggered, this, &BewavedDolphin::on_actionSet_Length_triggered);
    connect(m_ui->actionSet_clock_wave, &QAction::triggered, this, &BewavedDolphin::on_actionSet_clock_wave_triggered);
    connect(m_ui->actionSet_to_0, &QAction::triggered, this, &BewavedDolphin::on_actionSet_to_0_triggered);
    connect(m_ui->actionSet_to_1, &QAction::triggered, this, &BewavedDolphin::on_actionSet_to_1_triggered);
    connect(m_ui->actionShowCurve, &QAction::triggered, this, &BewavedDolphin::on_actionShowCurve_triggered);
    connect(m_ui->actionShowValues, &QAction::triggered, this, &BewavedDolphin::on_actionShowValues_triggered);
    connect(m_ui->actionZoom_In, &QAction::triggered, this, &BewavedDolphin::on_actionZoom_In_triggered);
    connect(m_ui->actionZoom_Range, &QAction::triggered, this, &BewavedDolphin::on_actionZoom_Range_triggered);
    connect(m_ui->actionZoom_out, &QAction::triggered, this, &BewavedDolphin::on_actionZoom_out_triggered);
}

BewavedDolphin::~BewavedDolphin()
{
    Settings::beginGroup("beWavedDolphin");
    Settings::setValue("geometry", saveGeometry());
    Settings::endGroup();
    delete m_ui;
}

void BewavedDolphin::zoomChanged()
{
    m_ui->actionZoom_In->setEnabled(m_gv->gvzoom()->canZoomIn());
    m_ui->actionZoom_out->setEnabled(m_gv->gvzoom()->canZoomOut());
}

void BewavedDolphin::drawPixMaps()
{
    m_lowGreen = QPixmap(":/dolphin/low_green.png").scaled(100, 38);
    m_lowBlue = QPixmap(":/dolphin/low_blue.png").scaled(100, 38);
    m_highGreen = QPixmap(":/dolphin/high_green.png").scaled(100, 38);
    m_highBlue = QPixmap(":/dolphin/high_blue.png").scaled(100, 38);
    m_fallingGreen = QPixmap(":/dolphin/falling_green.png").scaled(100, 38);
    m_fallingBlue = QPixmap(":/dolphin/falling_blue.png").scaled(100, 38);
    m_risingGreen = QPixmap(":/dolphin/rising_green.png").scaled(100, 38);
    m_risingBlue = QPixmap(":/dolphin/rising_blue.png").scaled(100, 38);
}

void BewavedDolphin::closeEvent(QCloseEvent *e)
{
    if (m_ask_connection && checkSave()) {
        m_mainWindow->setEnabled(true);
        e->accept();
    } else {
        e->ignore();
    }
}

void BewavedDolphin::on_actionExit_triggered()
{
    close();
}

bool BewavedDolphin::checkSave()
{
    if (!m_edited) {
        return true;
    }

    auto reply =
            QMessageBox::question(
                this,
                tr("WiRedPanda - beWavedDolphin"),
                tr("Save simulation before closing?"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch(reply){
    case QMessageBox::Save: on_actionSave_triggered(); return (!m_edited);
    case QMessageBox::Discard: return true;
    case QMessageBox::Cancel: return false;
    default: return true;
    }
}

bool BewavedDolphin::loadElements()
{
    QVector<GraphicElement *> elements = m_editor->getScene()->getElements();
    m_inputs.clear();
    m_outputs.clear();
    m_input_ports = 0;
    if (elements.isEmpty()) {
        return false;
    }
    elements = ElementMapping::sortGraphicElements(elements);
    for (auto *elm : qAsConst(elements)) {
        if (elm && (elm->type() == GraphicElement::Type)) {
            if (elm->elementGroup() == ElementGroup::Input) {
                m_inputs.append(elm);
                m_input_ports += elm->outputSize();
            } else if (elm->elementGroup() == ElementGroup::Output) {
                m_outputs.append(elm);
            }
        }
    }
    std::stable_sort(m_inputs.begin(), m_inputs.end(), [](const auto &elm1, const auto &elm2) {
        return QString::compare(elm1->getLabel(), elm2->getLabel(), Qt::CaseInsensitive) < 0;
    });
    std::stable_sort(m_outputs.begin(), m_outputs.end(), [](const auto &elm1, const auto &elm2) {
        return QString::compare(elm1->getLabel(), elm2->getLabel(), Qt::CaseInsensitive) < 0;
    });

    return (!m_inputs.isEmpty() && !m_outputs.isEmpty());
}

void BewavedDolphin::CreateElement(int row, int col, int value, bool isInput, bool changePrevious)
{
    if (value == 0) {
        return CreateZeroElement(row, col, isInput, changePrevious);
    }
    return CreateOneElement(row, col, isInput, changePrevious);
}

void BewavedDolphin::CreateZeroElement(int row, int col, bool isInput, bool changePrevious)
{
    qCDebug(three) << "Getting previous value to check previous cell refresh requirement.";
    // QMessageBox::warning(this, "Zero: row, col", (row + 48) + QString(" ") + (col + 48));
    int previous_value;
    if (m_model->item(row, col) == nullptr) {
        previous_value = -1;
    } else if (m_model->item(row, col)->text().toInt() == 0) {
        previous_value = 0;
    } else {
        previous_value = 1;
    }
    qCDebug(three) << "Changing current item.";
    auto index = m_model->index(row, col);
    m_model->setData(index, "0", Qt::DisplayRole);
    m_model->setData(index, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (m_type == PlotType::Line) {
        m_model->setData(index, Qt::AlignLeft, Qt::TextAlignmentRole);
        if ((col != m_model->columnCount() - 1) && (m_model->item(row, col + 1) != nullptr) && (m_model->item(row, col + 1)->text().toInt() == 1)) {
            m_model->setData(index, m_risingGreen, Qt::DecorationRole);
        } else {
            m_model->setData(index, m_lowGreen, Qt::DecorationRole);
        }
    } else {
        m_model->setData(index, Qt::AlignCenter, Qt::TextAlignmentRole);
    }
    qCDebug(three) << "Changing previous item, if needed.";
    if ((changePrevious) && (col != 0) && (previous_value == 1)) {
        CreateElement(row, col - 1, m_model->item(row, col - 1)->text().toInt(), isInput, false);
    }
}

void BewavedDolphin::CreateOneElement(int row, int col, bool isInput, bool changePrevious)
{
    qCDebug(three) << "Getting previous value to check previous cell refresh requirement.";
    // QMessageBox::warning(this, "One: row, col", (row + 48) + QString(" ") + (col + 48));
    int previous_value;
    if (m_model->item(row, col) == nullptr) {
        previous_value = -1;
    } else if (m_model->item(row, col)->text().toInt() == 0) {
        previous_value = 0;
    } else {
        previous_value = 1;
    }
    qCDebug(three) << "Changing current item.";
    auto index = m_model->index(row, col);
    m_model->setData(index, "1", Qt::DisplayRole);
    if (m_type == PlotType::Line) {
        m_model->setData(index, Qt::AlignLeft, Qt::TextAlignmentRole);
        if ((col != m_model->columnCount() - 1) && (m_model->item(row, col + 1) != nullptr) && (m_model->item(row, col + 1)->text().toInt() == 0)) {
            m_model->setData(index, m_fallingGreen, Qt::DecorationRole);
        } else {
            m_model->setData(index, m_highGreen, Qt::DecorationRole);
        }
    } else {
        m_model->setData(index, Qt::AlignCenter, Qt::TextAlignmentRole);
    }
    qCDebug(three) << "Changing previous item, if needed.";
    if ((changePrevious) && (col != 0) && (previous_value == 0)) {
        CreateElement(row, col - 1, m_model->item(row, col - 1)->text().toInt(), isInput, false);
    }
}

void BewavedDolphin::restoreInputs()
{
    qCDebug(zero) << "Restoring old values to inputs, prior to simulation.";
    size_t old_idx = 0;
    for (int in = 0; in < m_inputs.size(); ++in) {
        for (int port = 0; port < m_inputs[in]->outputSize(); ++port) {
            if (m_inputs[in]->outputSize() > 1) {
                dynamic_cast<Input *>(m_inputs[in])->setOn(m_oldInputValues[in], port);
            } else {
                dynamic_cast<Input *>(m_inputs[in])->setOn(m_oldInputValues[in]);
            }
            ++old_idx;
        }
    }
}

void BewavedDolphin::run()
{
    SCStop scst(m_sc);
    for (int itr = 0; itr < m_model->columnCount(); ++itr) {
        qCDebug(four) << "itr:" << itr << ", inputs:" << m_inputs.size();
        int table_line = 0;
        for (auto *input : qAsConst(m_inputs)) {
            int outSz = input->outputSize();
            for (int port = 0; port < outSz; ++port) {
                int val = m_model->item(table_line, itr)->text().toInt();
                dynamic_cast<Input *>(input)->setOn(val, port);
                ++table_line;
            }
        }
        qCDebug(four) << "Updating the values of the circuit logic based on current input values.";
        m_sc->update();
        m_sc->updateAll();
        qCDebug(four) << "Setting the computed output values to the waveform results.";
        int counter = m_input_ports;
        for (auto *output : qAsConst(m_outputs)) {
            int inSz = output->inputSize();
            for (int port = 0; port < inSz; ++port) {
                int value = static_cast<int>(output->input(port)->value());
                CreateElement(counter, itr, value, false);
                counter++;
            }
        }
    }
    qCDebug(three) << "Setting inputs back to old values.";
    restoreInputs();
    m_signalTableView->viewport()->update();
    scst.release();
}

void BewavedDolphin::loadNewTable(const QStringList &input_labels, const QStringList &output_labels)
{
    int iterations = 32;
    qCDebug(zero) << "Num iter =" << iterations;
    m_model = new SignalModel(input_labels.size() + output_labels.size(), input_labels.size(), iterations, this);
    m_signalTableView->setModel(m_model);
    QStringList horizontalHeaderLabels;
    for( int idx = 0; idx < iterations; ++idx ) {
      horizontalHeaderLabels.append( QString::fromStdString(std::to_string( idx ) ) );
    }
    m_model->setVerticalHeaderLabels(input_labels + output_labels);
    m_model->setHorizontalHeaderLabels(horizontalHeaderLabels);
    m_signalTableView->setAlternatingRowColors(true);
    m_signalTableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    m_signalTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    qCDebug(zero) << "Inputs:" << input_labels.size() << ", outputs:" << output_labels.size();
    m_edited = false;
    on_actionClear_triggered();
}

QVector<char> BewavedDolphin::loadSignals(QStringList &input_labels, QStringList &output_labels)
{
    QVector<char> oldValues(m_input_ports);
    int old_idx = 0;
    for (auto *input : qAsConst(m_inputs)) {
        QString label = input->getLabel();
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(input->elementType());
        }
        for (int port = 0; port < input->outputSize(); ++port) {
            if (input->outputSize() > 1) {
//                QMessageBox::warning(this, "Append", "Appending label: " + label + ", port: " + QString::number(port));
                input_labels.append(label + "_" + QString::number(port));
                oldValues[old_idx] = input->output(port)->value();
            } else {
//                 QMessageBox::warning(this, "Append", "Appending label " + label);
                input_labels.append(label);
                oldValues[old_idx] = input->output()->value();
            }
            ++old_idx;
        }
    }
    qCDebug(zero) << "Getting the name of the outputs. If no label is given, element type is used as a name. What if there are 2 outputs without name or 2 identical labels?";
    for (auto *output : qAsConst(m_outputs)) {
        QString label = output->getLabel();
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(output->elementType());
        }
        for (int port = 0; port < output->inputSize(); ++port) {
            if (output->inputSize() > 1) {
                output_labels.append(label + "_" + QString::number(port));
            } else {
                output_labels.append(label);
            }
        }
    }
    // QMessageBox::warning(this, "loading...", "inputs: " + QString::number(input_labels.size()) + ", outputs: " + QString::number(output_labels.size()));
    return oldValues;
}

bool BewavedDolphin::createWaveform(const QString &filename)
{
    qCDebug(zero) << "Updating window name with current filename.";
    qCDebug(zero) << "Creating class to pause main window simulator while creating waveform.";
    m_sc = m_editor->getSimulationController();
    SCStop scst(m_sc);
    m_mainWindow->setDisabled(true);
    this->setEnabled(true);
    qCDebug(zero) << "Loading elements. All elements initially in elements vector. Then, inputs and outputs are extracted from it.";
    if (!loadElements() && GlobalProperties::verbose) {
        QMessageBox::critical(parentWidget(), tr("Error"), tr("Could not load enough elements for the simulation."));
        return false;
    }
    QStringList input_labels;
    QStringList output_labels;
    qCDebug(zero) << "Getting initial value from inputs and writing them to oldvalues. Used to save current state of inputs and restore it after simulation. Not saving memory states though...";
    qCDebug(zero) << "Also getting the name of the inputs. If no label is given, the element type is used as a name. Bug here? What if there are 2 inputs without name or two identical labels?";
    m_oldInputValues = loadSignals(input_labels, output_labels);
    qCDebug(zero) << "Loading initial data into the table.";
    loadNewTable(input_labels, output_labels);
    if (filename.isEmpty()) {
        setWindowTitle(tr("beWavedDolphin Simulator"));
        m_currentFile = QFileInfo(filename);
    } else {
        QFileInfo finfo(m_mainWindow->getCurrentDir(), QFileInfo(filename).fileName());
        if (!load(finfo.absoluteFilePath())) {
            setWindowTitle(tr("beWavedDolphin Simulator"));
            m_currentFile = QFileInfo();
        }
    }
    qCDebug(zero) << "Resuming digital circuit main window after waveform simulation is finished.";
    scst.release();
    m_edited = false;
    return true;
}

void BewavedDolphin::show()
{
    QMainWindow::show();
    qCDebug(zero) << "Getting table dimensions.";
    resizeScene();
}

void BewavedDolphin::print()
{
    std::cout << std::to_string(m_model->rowCount()).c_str() << ",";
    std::cout << std::to_string(m_model->columnCount()).c_str() << ",\n";
    for (int row = 0; row < m_model->rowCount(); ++row) {
        //std::cout << m_model->verticalHeaderItem(row)->text().toStdString() << ": ";
        for (int col = 0; col < m_model->columnCount(); ++col) {
            QString val = m_model->item(row, col)->text();
            std::cout << val.toStdString().c_str() << ",";
        }
        std::cout << "\n";
    }
}

void BewavedDolphin::on_actionSet_to_0_triggered()
{
    qCDebug(zero) << "Pressed 0.";
    // auto itemList = signalTableView->selectedItems();
    auto itemList = m_signalTableView->selectionModel()->selectedIndexes();
    for (auto item : qAsConst(itemList)) {
        int row = item.row();
        int col = item.column();
        qCDebug(zero) << "Editing value.";
        CreateZeroElement(row, col);
    }
    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionSet_to_1_triggered()
{
    qCDebug(zero) << "Pressed 0.";
    auto itemList = m_signalTableView->selectionModel()->selectedIndexes();
    for (auto item : qAsConst(itemList)) {
        int row = item.row();
        int col = item.column();
        qCDebug(zero) << "Editing value.";
        CreateOneElement(row, col);
    }
    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionInvert_triggered()
{
    qCDebug(zero) << "Pressed Not.";
    auto itemList = m_signalTableView->selectionModel()->selectedIndexes();
    for (auto item : qAsConst(itemList)) {
        int row = item.row();
        int col = item.column();
        int value = m_model->index(row, col, QModelIndex()).data().toInt();
        value = (value + 1) % 2;
        qCDebug(zero) << "Editing value.";
        CreateElement(row, col, value);
    }
    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

int BewavedDolphin::sectionFirstColumn(const QItemSelection &ranges)
{
    int first_col = m_model->columnCount() - 1;
    for (const auto &range : ranges) {
        if (range.left() < first_col) {
            first_col = range.left();
        }
    }
    return first_col;
}

int BewavedDolphin::sectionFirstRow(const QItemSelection &ranges)
{
    int first_row = m_model->rowCount() - 1;
    for (const auto &range : ranges) {
        if (range.top() < first_row) {
            first_row = range.top();
        }
    }
    return first_row;
}

void BewavedDolphin::on_actionSet_clock_wave_triggered()
{
    qCDebug(zero) << "Getting first column.";
    QItemSelection ranges = m_signalTableView->selectionModel()->selection();
    int first_col = sectionFirstColumn(ranges);
    qCDebug(zero) << "Setting the signal according to its column and clock period.";
    ClockDialog dialog(this);
    int clock_period = dialog.getFrequency();
    if (clock_period < 0) {
        return;
    }
    int half_clock_period = clock_period / 2;
    auto itemList = m_signalTableView->selectionModel()->selectedIndexes();
    for (auto item : qAsConst(itemList)) {
        int row = item.row();
        int col = item.column();
        int value = ((col - first_col) % clock_period < half_clock_period ? 0 : 1);
        qCDebug(zero) << "Editing value.";
        CreateElement(row, col, value);
    }
    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionCombinational_triggered()
{
    qCDebug(zero) << "Setting the signal according to its columns and clock period.";
    int clock_period = 2;
    int half_clock_period = 1;
    for (int row = 0; row < m_input_ports; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            int value = (col % clock_period < half_clock_period ? 0 : 1);
            CreateElement(row, col, value);
        }
        half_clock_period = std::min(clock_period, 524288);
        clock_period = std::min(2 * clock_period, 1048576);
    }
    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionSet_Length_triggered()
{
    qCDebug(zero) << "Setting the simulation length.";
    LengthDialog dialog(this);
    int sim_length = dialog.getFrequency();
    if (sim_length < 0) {
        return;
    }
    setLength(sim_length);
}

void BewavedDolphin::setLength(int sim_length, bool run_simulation)
{
    if (sim_length <= m_model->columnCount()) {
        qCDebug(zero) << "Reducing or keeping the simulation length.";
        m_model->setColumnCount(sim_length);
        QStringList horizontalHeaderLabels;
        for( int idx = 0; idx < sim_length; ++idx ) {
          horizontalHeaderLabels.append( QString::fromStdString(std::to_string( idx ) ) );
        }
        m_model->setHorizontalHeaderLabels(horizontalHeaderLabels);
        resizeScene();
        m_edited = true;
        return;
    }
    qCDebug(zero) << "Increasing the simulation length.";
    int old_length = m_model->columnCount();
    m_model->setColumnCount(sim_length);
    QStringList horizontalHeaderLabels;
    for( int idx = 0; idx < sim_length; ++idx ) {
      horizontalHeaderLabels.append( QString::fromStdString(std::to_string( idx ) ) );
    }
    m_model->setHorizontalHeaderLabels(horizontalHeaderLabels);
    for (int row = 0; row < m_input_ports; ++row) {
        for (int col = old_length; col < m_model->columnCount(); ++col) {
            CreateZeroElement(row, col, true, false);
        }
    }
    resizeScene();
    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    if (run_simulation) {
        run();
    }
}

void BewavedDolphin::on_actionZoom_out_triggered()
{
    // gv->gvzoom()->zoomOut();
    m_scale *= m_SCALE_FACTOR;
    m_gv->scale(m_SCALE_FACTOR, m_SCALE_FACTOR);
    resizeScene();
}

void BewavedDolphin::on_actionZoom_In_triggered()
{
    // gv->gvzoom()->zoomIn();
    m_scale /= m_SCALE_FACTOR;
    m_gv->scale(1.0 / m_SCALE_FACTOR, 1.0 / m_SCALE_FACTOR);
    resizeScene();
}

void BewavedDolphin::on_actionReset_Zoom_triggered()
{
    // gv->gvzoom()->resetZoom();
    m_gv->scale(1.0 / m_scale, 1.0 / m_scale);
    m_scale = 1.0;
    resizeScene();
}

void BewavedDolphin::on_actionZoom_Range_triggered()
{
    m_gv->scale(1.0 / m_scale, 1.0 / m_scale);
    double w_scale = static_cast<double>(m_gv->width()) / (m_signalTableView->horizontalHeader()->length() + m_signalTableView->columnWidth(0));
    double h_scale = static_cast<double>(m_gv->height()) / (m_signalTableView->verticalHeader()->length() + m_signalTableView->rowHeight(0) + 10);
    m_scale = std::min(w_scale, h_scale);
    m_gv->scale(1.0 * m_scale, 1.0 * m_scale);
    resizeScene();
}

void BewavedDolphin::on_actionClear_triggered()
{
    for (int row = 0; row < m_input_ports; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            CreateZeroElement(row, col);
        }
    }
    qCDebug(zero) << "Running simulation.";
    m_edited = true;
    run();
}

void BewavedDolphin::on_actionCopy_triggered()
{
    QItemSelection ranges = m_signalTableView->selectionModel()->selection();
    if (ranges.isEmpty()) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->clear();
        return;
    }
    QClipboard *clipboard = QApplication::clipboard();
    auto *mimeData = new QMimeData;
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    copy(ranges, dataStream);
    mimeData->setData("bdolphin/copydata", itemData);
    clipboard->setMimeData(mimeData);
}

void BewavedDolphin::on_actionCut_triggered()
{
    QItemSelection ranges = m_signalTableView->selectionModel()->selection();
    if (ranges.isEmpty()) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->clear();
        return;
    }
    QClipboard *clipboard = QApplication::clipboard();
    auto *mimeData = new QMimeData();
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    cut(ranges, dataStream);
    mimeData->setData("bdolphin/copydata", itemData);
    clipboard->setMimeData(mimeData);
    m_edited = true;
}

void BewavedDolphin::on_actionPaste_triggered()
{
    QItemSelection ranges = m_signalTableView->selectionModel()->selection();
    if (ranges.isEmpty()) {
        return;
    }
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if (mimeData->hasFormat("bdolphin/copydata")) {
        QByteArray itemData = mimeData->data("bdolphin/copydata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        paste(ranges, dataStream);
    }
    m_edited = true;
}

void BewavedDolphin::cut(const QItemSelection &ranges, QDataStream &ds)
{
    copy(ranges, ds);
    on_actionSet_to_0_triggered();
}

void BewavedDolphin::copy(const QItemSelection &ranges, QDataStream &ds)
{
    qCDebug(zero) << "Serializing data into data stream.";
    int first_row = sectionFirstRow(ranges);
    int first_col = sectionFirstColumn(ranges);
    auto itemList = m_signalTableView->selectionModel()->selectedIndexes();
    ds << static_cast<qint64>(itemList.size());
    for (auto item : qAsConst(itemList)) {
        int row = item.row();
        int col = item.column();
        int data_ = m_model->item(row, col)->text().toInt();
        ds << static_cast<qint64>(row - first_row);
        ds << static_cast<qint64>(col - first_col);
        ds << static_cast<qint64>(data_);
    }
}

void BewavedDolphin::paste(QItemSelection &ranges, QDataStream &ds)
{
    int first_col = sectionFirstColumn(ranges);
    int first_row = sectionFirstRow(ranges);
    quint64 itemListSize;
    ds >> itemListSize;
    for (int elm = 0; elm < static_cast<int>(itemListSize); ++elm) {
        quint64 row;
        quint64 col;
        quint64 data_;
        ds >> row;
        ds >> col;
        ds >> data_;
        int new_row = static_cast<int>(first_row + row);
        int new_col = static_cast<int>(first_col + col);
        if ((new_row < m_input_ports) && (new_col < m_model->columnCount())) {
            CreateElement(new_row, new_col, data_);
        }
    }
    run();
}

void BewavedDolphin::associateToWiredPanda(const QString &fname)
{
    if (m_mainWindow->getDolphinFilename() != fname && GlobalProperties::verbose) {
        auto reply =
            QMessageBox::question(
                this,
                tr("WiRedPanda - beWavedDolphin"),
                tr("Do you want to link this beWavedDolphin file to your current WiRedPanda file and save it?"),
                QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            m_mainWindow->setDolphinFilename(fname);
            m_mainWindow->save();
        }
    }
}

void BewavedDolphin::on_actionSave_as_triggered()
{
    QString fname = m_currentFile.absoluteFilePath();
    QString path = m_mainWindow->getCurrentFile().absolutePath();
    QFileDialog fileDialog;
    fileDialog.setObjectName(tr("Save File as..."));
    fileDialog.setNameFilter(tr("Dolphin files (*.dolphin);;CSV files (*.csv);;All supported files (*.dolphin *.csv)"));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setDirectory(path);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    connect(&fileDialog, &QFileDialog::directoryEntered, this, [&fileDialog, path](const QString &new_dir) {
        qCDebug(zero) << "Changing dir to" << new_dir << ", home:" << path;
        if (new_dir != path) {
            fileDialog.setDirectory(path);
        }
    });
    if (!fileDialog.exec()) {
        return;
    }
    auto files = fileDialog.selectedFiles();
    fname = files.first();
    if (fname.isEmpty()) {
        return;
    }
    if ((!fname.endsWith(".dolphin")) && (!fname.endsWith(".csv"))) {
        if (fileDialog.selectedNameFilter().contains("dolphin")) {
            // if (selected_filter->contains("dolphin")) {
            fname.append(".dolphin");
        } else {
            fname.append(".csv");
        }
    }
    if (save(fname)) {
        m_currentFile = QFileInfo(fname);
        associateToWiredPanda(fname);
        setWindowTitle(tr("beWavedDolphin Simulator") + " [" + m_currentFile.fileName() + "]");
        m_ui->statusbar->showMessage(tr("Saved file successfully."), 2000);
        m_edited = false;
    } else {
        m_ui->statusbar->showMessage(tr("Could not save file: ") + fname + ".", 2000);
    }
}

void BewavedDolphin::on_actionSave_triggered()
{
    if (m_currentFile.fileName().isEmpty()) {
        on_actionSave_as_triggered();
        return;
    }
    QString fname = m_currentFile.absoluteFilePath();
    if (save(fname)) {
        m_ui->statusbar->showMessage(tr("Saved file successfully."), 2000);
        m_edited = false;
    } else {
        m_ui->statusbar->showMessage(tr("Could not save file: ") + fname + ".", 2000);
    }
}

void BewavedDolphin::on_actionLoad_triggered()
{
    QDir defaultDirectory;
    if (m_currentFile.exists()) {
        defaultDirectory.setPath(m_currentFile.absolutePath());
    } else {
        if (m_mainWindow->getCurrentFile().exists()) {
            m_mainWindow->getCurrentFile().dir();
        } else {
            defaultDirectory.setPath(QDir::homePath());
        }
    }
    const QString homeDir(m_mainWindow->getCurrentDir().absolutePath());
    QFileDialog fileDialog;
    fileDialog.setObjectName(tr("Open File"));
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilter(tr("All supported files (*.dolphin *.csv);;Dolphin files (*.dolphin);;CSV files (*.csv)"));
    fileDialog.setDirectory(homeDir);
    connect(&fileDialog, &QFileDialog::directoryEntered, this, [&fileDialog, homeDir](const QString &new_dir) {
        qCDebug(zero) << "Changing dir to" << new_dir << ", home:" << homeDir;
        if (new_dir != homeDir) {
            fileDialog.setDirectory(homeDir);
        }
    });
    if (!fileDialog.exec()) {
        return;
    }
    auto files = fileDialog.selectedFiles();
    QString fname = files.first();
    if (fname.isEmpty()) {
        return;
    }
    load(fname);
    m_edited = false;
    m_ui->statusbar->showMessage(tr("File loaded successfully."), 2000);
}

bool BewavedDolphin::save(const QString &fname)
{
    QSaveFile fl(fname);
    if (fl.open(QFile::WriteOnly)) {
        if (fname.endsWith(".dolphin")) {
            qCDebug(zero) << "Saving dolphin file.";
            QDataStream ds(&fl);
            try {
                save(ds);
            } catch (std::runtime_error &e) {
                m_ui->statusbar->showMessage(tr("Could not save file: ") + e.what(), 2000);
                return false;
            }
        } else {
            qCDebug(zero) << "Saving CSV file.";
            try {
                save(fl);
            } catch (std::runtime_error &e) {
                m_ui->statusbar->showMessage(tr("Could not save file: ") + e.what(), 2000);
                return false;
            }
        }
    }
    return fl.commit();
}

void BewavedDolphin::save(QDataStream &ds)
{
    ds << QString("beWavedDolphin 1.0");
    qCDebug(zero) << "Serializing data into data stream.";
    ds << static_cast<qint64>(m_input_ports);
    ds << static_cast<qint64>(m_model->columnCount());
    for (int itr = 0; itr < m_model->columnCount(); ++itr) {
        for (int in = 0; in < m_input_ports; ++in) {
            int val = m_model->item(in, itr)->text().toInt();
            ds << static_cast<qint64>(val);
        }
    }
}

void BewavedDolphin::save(QSaveFile &fl)
{
    fl.write(std::to_string(m_model->rowCount()).c_str());
    fl.write(",");
    fl.write(std::to_string(m_model->columnCount()).c_str());
    fl.write(",\n");
    for (int row = 0; row < m_model->rowCount(); ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            QString val = m_model->item(row, col)->text();
            fl.write(val.toStdString().c_str());
            fl.write(",");
        }
        fl.write("\n");
    }
}

bool BewavedDolphin::load(const QString &fname)
{
    QFile fl(fname);
    if (!fl.exists()) {
        QMessageBox::critical(this, tr("Error!"), tr("File \"%1\" does not exist!").arg(fname), QMessageBox::Ok, QMessageBox::NoButton);
        return false;
    }
    qCDebug(zero) << "File exists.";
    if (fl.open(QFile::ReadOnly)) {
        if (fname.endsWith(".dolphin")) {
            qCDebug(zero) << "Dolphin file opened.";
            QDataStream ds(&fl);
            qCDebug(zero) << "Current file set.";
            try {
                qCDebug(zero) << "Loading in editor.";
                load(ds);
                qCDebug(zero) << "Finished updating changed by signal.";
                m_currentFile = QFileInfo(fname);
            } catch (std::runtime_error &e) {
                qCDebug(zero) << tr("Error loading project:") << e.what();
                QMessageBox::critical(this, tr("Error!"), tr("Could not open file.\nError: %1").arg(e.what()), QMessageBox::Ok, QMessageBox::NoButton);
                return false;
            }
        } else if (fname.endsWith(".csv")) {
            qCDebug(zero) << "CSV file opened.";
            try {
                qCDebug(zero) << "Loading in editor.";
                load(fl);
                qCDebug(zero) << "Finished updating changed by signal.";
                m_currentFile = QFileInfo(fname);
            } catch (std::runtime_error &e) {
                qCDebug(zero) << tr("Error loading project:") << e.what();
                QMessageBox::critical(this, tr("Error!"), tr("Could not open file.\nError: %1").arg(e.what()), QMessageBox::Ok, QMessageBox::NoButton);
                return false;
            }
        } else {
            qCDebug(zero) << tr("Format not supported. Could not open file:") << fname;
            return false;
        }
    } else {
        qCDebug(zero) << tr("Could not open file in ReadOnly mode :") << fname;
        QMessageBox::critical(this, tr("Error!"), tr("Could not open file in ReadOnly mode : ") + fname + ".", QMessageBox::Ok, QMessageBox::NoButton);
        return false;
    }
    qCDebug(zero) << "Closing file.";
    fl.close();
    associateToWiredPanda(fname);
    setWindowTitle(tr("beWavedDolphin Simulator") + " [" + m_currentFile.fileName() + "]");
    return true;
}

void BewavedDolphin::load(QDataStream &ds)
{
    QString str;
    ds >> str;
    if (!str.startsWith("beWavedDolphin")) {
        throw(std::runtime_error(ERRORMSG(QObject::tr("Invalid file format. Starts with: ").toStdString() + str.toStdString())));
    }
    qint64 rows;
    qint64 cols;
    ds >> rows;
    ds >> cols;
    if (rows > m_input_ports) {
        rows = m_input_ports;
    }
    if ((cols < 2) || (cols > 2048)) {
        throw(std::runtime_error(ERRORMSG(QObject::tr("Invalid number of columns.").toStdString())));
    }
    setLength(cols, false);
    qCDebug(zero) << "Update table.";
    for (int col = 0; col < cols; ++col) {
        for (int row = 0; row < rows; ++row) {
            qint64 value;
            ds >> value;
            CreateElement(row, col, value, true);
        }
    }
    run();
}

void BewavedDolphin::load(QFile &fl)
{
    QByteArray content = fl.readAll();
    QList<QByteArray> wordList(content.split(','));
    int rows = wordList[0].toInt();
    int cols = wordList[1].toInt();
    if (rows > m_input_ports) {
        rows = m_input_ports;
    }
    if ((cols < 2) || (cols > 2048)) {
        throw(std::runtime_error(ERRORMSG(QObject::tr("Invalid number of columns.").toStdString())));
    }
    setLength(cols, false);
    qCDebug(zero) << "Update table.";
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int value = wordList[2 + col + row * cols].toInt();
            CreateElement(row, col, value, true);
        }
    }
    run();
}

void BewavedDolphin::on_actionShowValues_triggered()
{
    m_type = PlotType::Number;
    for (int row = 0; row < m_model->rowCount(); ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            m_model->setData(m_model->index(row, col), "", Qt::DecorationRole);
        }
    }
    for (int row = 0; row < m_input_ports; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            CreateElement(row, col, m_model->item(row, col)->text().toInt());
        }
    }
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionShowCurve_triggered()
{
    m_type = PlotType::Line;
    for (int row = 0; row < m_input_ports; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            CreateElement(row, col, m_model->item(row, col)->text().toInt());
        }
    }
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionExport_to_PNG_triggered()
{
    QString pngFile = QFileDialog::getSaveFileName(this, tr("Export to Image"), m_currentFile.absolutePath(), tr("PNG files (*.png)"));
    if (pngFile.isEmpty()) {
        return;
    }
    if (!pngFile.endsWith(".png", Qt::CaseInsensitive)) {
        pngFile.append(".png");
    }
    QRectF s = m_scene->sceneRect();
    QPixmap p(s.size().toSize());
    QPainter painter;
    painter.begin(&p);
    painter.setRenderHint(QPainter::Antialiasing);
    m_scene->render(&painter, QRectF(), s);
    painter.end();
    QImage img = p.toImage();
    img.save(pngFile);
}

void BewavedDolphin::on_actionExport_to_PDF_triggered()
{
    QString pdfFile = QFileDialog::getSaveFileName(this, tr("Export to PDF"), m_currentFile.absolutePath(), tr("PDF files (*.pdf)"));
    if (pdfFile.isEmpty()) {
        return;
    }
    if (!pdfFile.endsWith(".pdf", Qt::CaseInsensitive)) {
        pdfFile.append(".pdf");
    }
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Orientation::Landscape);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(pdfFile);
    QPainter p;
    if (!p.begin(&printer)) {
        QMessageBox::critical(this, tr("Error!"), tr("Could not print this circuit to PDF."), QMessageBox::Ok);
        return;
    }
    m_scene->render(&p, QRectF(), m_scene->sceneRect().adjusted(-64, -64, 64, 64));
    p.end();
}

void BewavedDolphin::on_actionAbout_triggered()
{
    QMessageBox::about(this,
                       "beWavedDolphin",
                       tr("<p>beWavedDolphin is a waveform simulator for the WiRedPanda software developed by the Federal University of São Paulo."
                          " This project was created in order to help students learn about logic circuits.</p>"
                          "<p>Software version: %1</p>"
                          "<p><strong>Creators:</strong></p>"
                          "<ul>"
                          "<li> Prof. Fábio Cappabianco, Ph.D. </li>"
                          "</ul>"
                          "<p> beWavedDolphin is currently maintained by Prof. Fábio Cappabianco, Ph.D. and Vinícius R. Miguel.</p>"
                          "<p> Please file a report at our GitHub page if bugs are found or if you wish for a new functionality to be implemented.</p>"
                          "<p><a href=\"http://gibis-unifesp.github.io/wiRedPanda/\">Visit our website!</a></p>")
                           .arg(QApplication::applicationVersion()));
}

void BewavedDolphin::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void BewavedDolphin::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    resizeScene();
}

void BewavedDolphin::resizeScene()
{
    m_signalTableView->resize((this->width() - 20) / m_scale, (this->height() - 102) / m_scale);
    QRectF rect = m_scene->itemsBoundingRect();
    m_scene->setSceneRect(rect);
}

