// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bewaveddolphin.h"
#include "ui_bewaveddolphin.h"

#include "clockdialog.h"
#include "common.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "graphicelementinput.h"
#include "inputrotary.h"
#include "lengthdialog.h"
#include "mainwindow.h"
#include "settings.h"
#include "simulationblocker.h"

#include <QAbstractItemView>
#include <QClipboard>
#include <QCloseEvent>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QMimeData>
#include <QPrinter>
#include <QSaveFile>
#include <QTextStream>
#include <cmath>
#include <iostream>

SignalModel::SignalModel(const int inputs, const int rows, const int columns, QObject *parent)
    : QStandardItemModel(rows, columns, parent)
    , m_inputCount(inputs)
{
}

Qt::ItemFlags SignalModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    return flags;
}

SignalDelegate::SignalDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

void SignalDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem itemOption(option);
    itemOption.rect.adjust(-4, 0, 0, 0);
    QItemDelegate::paint(painter, itemOption, index);
}

bool DolphinGraphicsView::canZoomOut() const
{
    return m_zoomLevel > 0;
}

void DolphinGraphicsView::wheelEvent(QWheelEvent *event)
{
    int zoomDirection = event->angleDelta().y();

    if (zoomDirection > 0 && canZoomIn()) {
        zoomIn();
    } else if (zoomDirection < 0 && canZoomOut()) {
        zoomOut();
    }

    centerOn(QPoint(0, 0));

    event->accept();
}

BewavedDolphin::BewavedDolphin(Scene *scene, const bool askConnection, MainWindow *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::BewavedDolphin)
    , m_mainWindow(parent)
    , m_externalScene(scene)
    , m_askConnection(askConnection)
{
    m_ui->setupUi(this);
    m_ui->retranslateUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::WindowModal);
    setWindowTitle(tr("beWavedDolphin Simulator"));

    resize(800, 500);

    restoreGeometry(Settings::value("beWavedDolphin/geometry").toByteArray());

    m_signalTableView->setItemDelegate(new SignalDelegate(this));

    m_scene->addWidget(m_signalTableView);

    m_view.setScene(m_scene);
    m_view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view.setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_ui->verticalLayout->addWidget(&m_view);

    m_ui->actionZoomOut->setEnabled(false);

    m_ui->mainToolBar->setToolButtonStyle(Settings::value("labelsUnderIcons").toBool() ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);

    QSlider *slider = new QSlider(Qt::Horizontal);
    QSize sliderSize(130, 30);
    slider->setFixedSize(sliderSize);
    slider->setRange(0, 64);

    QList<QAction*> actions = m_ui->mainToolBar->actions();

    for (int i = 0; i < actions.size(); ++i) {
        if (actions[i] == m_ui->actionZoomOut) {
            m_ui->mainToolBar->insertWidget(actions[i], slider);
            break;
        }
    }

    loadPixmaps();

    connect(&m_view,     &DolphinGraphicsView::zoomChanged, this, &BewavedDolphin::zoomChanged);
    connect(m_ui->actionAbout,         &QAction::triggered, this, &BewavedDolphin::on_actionAbout_triggered);
    connect(m_ui->actionAboutQt,       &QAction::triggered, this, &BewavedDolphin::on_actionAboutQt_triggered);
    connect(m_ui->actionClear,         &QAction::triggered, this, &BewavedDolphin::on_actionClear_triggered);
    connect(m_ui->actionCombinational, &QAction::triggered, this, &BewavedDolphin::on_actionCombinational_triggered);
    connect(m_ui->actionCopy,          &QAction::triggered, this, &BewavedDolphin::on_actionCopy_triggered);
    connect(m_ui->actionCut,           &QAction::triggered, this, &BewavedDolphin::on_actionCut_triggered);
    connect(m_ui->actionExit,          &QAction::triggered, this, &BewavedDolphin::on_actionExit_triggered);
    connect(m_ui->actionExportToPdf,   &QAction::triggered, this, &BewavedDolphin::on_actionExportToPdf_triggered);
    connect(m_ui->actionExportToPng,   &QAction::triggered, this, &BewavedDolphin::on_actionExportToPng_triggered);
    connect(m_ui->actionInvert,        &QAction::triggered, this, &BewavedDolphin::on_actionInvert_triggered);
    connect(m_ui->actionLoad,          &QAction::triggered, this, &BewavedDolphin::on_actionLoad_triggered);
    connect(m_ui->actionPaste,         &QAction::triggered, this, &BewavedDolphin::on_actionPaste_triggered);
    connect(m_ui->actionResetZoom,     &QAction::triggered, this, &BewavedDolphin::on_actionResetZoom_triggered);
    connect(m_ui->actionSave,          &QAction::triggered, this, &BewavedDolphin::on_actionSave_triggered);
    connect(m_ui->actionSaveAs,        &QAction::triggered, this, &BewavedDolphin::on_actionSaveAs_triggered);
    connect(m_ui->actionSetClockWave,  &QAction::triggered, this, &BewavedDolphin::on_actionSetClockWave_triggered);
    connect(m_ui->actionSetLength,     &QAction::triggered, this, &BewavedDolphin::on_actionSetLength_triggered);
    connect(m_ui->actionSetTo0,        &QAction::triggered, this, &BewavedDolphin::on_actionSetTo0_triggered);
    connect(m_ui->actionSetTo1,        &QAction::triggered, this, &BewavedDolphin::on_actionSetTo1_triggered);
    connect(m_ui->actionShowNumbers,   &QAction::triggered, this, &BewavedDolphin::on_actionShowNumbers_triggered);
    connect(m_ui->actionShowWaveforms, &QAction::triggered, this, &BewavedDolphin::on_actionShowWaveforms_triggered);
    connect(m_ui->actionZoomIn,        &QAction::triggered, this, &BewavedDolphin::on_actionZoomIn_triggered);
    connect(m_ui->actionZoomOut,       &QAction::triggered, this, &BewavedDolphin::on_actionZoomOut_triggered);
}

BewavedDolphin::~BewavedDolphin()
{
    Settings::setValue("beWavedDolphin/geometry", saveGeometry());
    delete m_ui;
}

void BewavedDolphin::loadPixmaps()
{
    m_lowGreen = QPixmap(":/dolphin/low_green.svg").scaled(100, 38);
    m_highGreen = QPixmap(":/dolphin/high_green.svg").scaled(100, 38);
    m_fallingGreen = QPixmap(":/dolphin/falling_green.svg").scaled(100, 38);
    m_risingGreen = QPixmap(":/dolphin/rising_green.svg").scaled(100, 38);

    m_lowBlue = QPixmap(":/dolphin/low_blue.svg").scaled(100, 38);
    m_highBlue = QPixmap(":/dolphin/high_blue.svg").scaled(100, 38);
    m_fallingBlue = QPixmap(":/dolphin/falling_blue.svg").scaled(100, 38);
    m_risingBlue = QPixmap(":/dolphin/rising_blue.svg").scaled(100, 38);
}

void BewavedDolphin::createWaveform(const QString &fileName)
{
    prepare(fileName);

    if (fileName.isEmpty()) {
        setWindowTitle(tr("beWavedDolphin Simulator"));
        run();
    } else {
        QFileInfo fileInfo(m_mainWindow->currentDir(), QFileInfo(fileName).fileName());

        if (!fileInfo.exists()) {
            m_ui->statusbar->showMessage(tr("File \"%1\" does not exist!").arg(fileName), 4000);
            return;
        }

        load(fileInfo.absoluteFilePath());
    }

    qCDebug(zero) << tr("Resuming digital circuit main window after waveform simulation is finished.");
    m_edited = false;
}

void BewavedDolphin::createWaveform()
{
    prepare();
    loadFromTerminal();
}

void BewavedDolphin::loadFromTerminal()
{
    QTextStream cin(stdin);
    QString str = cin.readLine();
    const auto wordList(str.split(','));

    if (wordList.size() < 2) {
        throw Pandaception("");
    }

    int rows = wordList.at(0).toInt();
    const int cols = wordList.at(1).toInt();

    if (rows > m_inputPorts) {
        rows = m_inputPorts;
    }

    if ((cols < 2) || (cols > 2048)) {
        throw Pandaception("");
    }

    setLength(cols, false);

    for (int row = 0; row < rows; ++row) {
        str = cin.readLine();
        const auto wordList2(str.split(','));

        if (wordList2.size() < cols) {
            throw Pandaception("");
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
    qCDebug(zero) << tr("Updating window name with current: ") << fileName;
    m_simulation = m_externalScene->simulation();

    qCDebug(zero) << tr("Loading elements. All elements initially in elements vector. Then, inputs and outputs are extracted from it.");
    loadElements();

    qCDebug(zero) << tr("Loading initial data into the table.");
    loadNewTable();
}

void BewavedDolphin::loadElements()
{
    m_inputs.clear();
    m_outputs.clear();
    m_inputPorts = 0;

    const auto elements = Common::sortGraphicElements(m_externalScene->elements());

    if (elements.isEmpty()) {
        throw Pandaception(tr("Could not load enough elements for the simulation."));
    }

    for (auto *elm : elements) {
        if (!elm || (elm->type() != GraphicElement::Type)) {
            continue;
        }

        if (elm->elementGroup() == ElementGroup::Input) {
            m_inputs.append(qobject_cast<GraphicElementInput *>(elm));
            m_inputPorts += elm->outputSize();
        }

        if (elm->elementGroup() == ElementGroup::Output) {
            m_outputs.append(elm);
        }
    }

    std::stable_sort(m_inputs.begin(), m_inputs.end(), [](const auto &elm1, const auto &elm2) {
        return QString::compare(elm1->label(), elm2->label(), Qt::CaseInsensitive) < 0;
    });

    std::stable_sort(m_outputs.begin(), m_outputs.end(), [](const auto &elm1, const auto &elm2) {
        return QString::compare(elm1->label(), elm2->label(), Qt::CaseInsensitive) < 0;
    });

    if (m_inputs.isEmpty() || m_outputs.isEmpty()) {
        throw Pandaception(tr("Could not load enough elements for the simulation."));
    }
}

void BewavedDolphin::loadNewTable()
{
    qCDebug(zero) << tr("Getting initial value from inputs and writing them to oldvalues. Used to save current state of inputs and restore it after simulation. Not saving memory states though...");
    qCDebug(zero) << tr("Also getting the name of the inputs. If no label is given, the element type is used as a name.");
    QStringList inputLabels;
    QStringList outputLabels;
    loadSignals(inputLabels, outputLabels);

    // ---------------------------------------

    qCDebug(zero) << tr("Num iter = ") << m_length;

    m_model = new SignalModel(inputLabels.size(), inputLabels.size() + outputLabels.size(), m_length, this);
    m_signalTableView->setModel(m_model);

    m_model->setVerticalHeaderLabels(inputLabels + outputLabels);

    m_signalTableView->setAlternatingRowColors(true);
    m_signalTableView->setShowGrid(false);

    m_signalTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    m_signalTableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);

    m_signalTableView->horizontalHeader()->setDefaultSectionSize(1);

    qCDebug(zero) << tr("Inputs: ") << inputLabels.size() << tr(", outputs: ") << outputLabels.size();

    on_actionClear_triggered();

    connect(m_signalTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &BewavedDolphin::on_tableView_selectionChanged);
    connect(m_signalTableView,                   &QAbstractItemView::doubleClicked,      this, &BewavedDolphin::on_tableView_cellDoubleClicked);
}

void BewavedDolphin::on_tableView_cellDoubleClicked() {
    const auto indexes = m_signalTableView->selectionModel()->selectedIndexes();

    for (auto index : indexes) {
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

    for (auto index : indexes) {
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

    for (auto *input : qAsConst(m_inputs)) {
        QString label = input->label();

        if (label.isEmpty()) {
            label = ElementFactory::translatedName(input->elementType());
        }

        for (int port = 0; port < input->outputSize(); ++port) {
            if (input->outputSize() > 1) {
                inputLabels.append(label + "[" + QString::number(port) + "]");
            } else {
                inputLabels.append(label);
            }

            oldValues[oldIndex] = input->outputPort(port)->status();
            ++oldIndex;
        }
    }

    qCDebug(zero) << tr("Getting the name of the outputs. If no label is given, element type is used as a name.");

    for (auto *output : qAsConst(m_outputs)) {
        QString label = output->label();

        if (label.isEmpty()) {
            label = ElementFactory::translatedName(output->elementType());
        }

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
    run2();
    //run2();
}

void BewavedDolphin::run2()
{
    qCDebug(zero) << tr("Creating class to pause main window simulator while creating waveform.");
    SimulationBlocker simulationBlocker(m_simulation);

    for (int column = 0; column < m_model->columnCount(); ++column) {
        qCDebug(four) << tr("Itr: ") << column << tr(", inputs: ") << m_inputs.size();
        int row = 0;

        for (auto *input : qAsConst(m_inputs)) {
            if (dynamic_cast<InputRotary*>(input)) {
                for (int port = 0; port < input->outputSize(); ++port) {
                    if (m_model->index(row, column).data().toInt()) {
                        input->setOn(1, port);
                    }

                    ++row;
                }
            } else {
                for (int port = 0; port < input->outputSize(); ++port) {
                    const bool value = static_cast<bool>(m_model->index(row, column).data().toInt());
                    input->setOn(value, port);
                    ++row;
                }
            }
        }

        qCDebug(four) << tr("Updating the values of the circuit logic based on current input values.");
        m_simulation->update();
        m_simulation->update();
        m_simulation->update();

        qCDebug(four) << tr("Setting the computed output values to the waveform results.");
        row = m_inputPorts;

        for (auto *output : qAsConst(m_outputs)) {
            for (int port = 0; port < output->inputSize(); ++port) {
                const int value = static_cast<int>(output->inputPort(port)->status());
                createElement(row, column, value, false);
                row++;
            }
        }
    }

    qCDebug(three) << tr("Setting inputs back to old values.");
    restoreInputs();
}

void BewavedDolphin::restoreInputs()
{
    qCDebug(zero) << tr("Restoring old values to inputs, prior to simulation.");

    for (int index = 0; index < m_inputs.size(); ++index) {
        for (int port = 0; port < m_inputs.value(index)->outputSize(); ++port) {
            auto *input = m_inputs.at(index);
            const bool oldValue = static_cast<bool>(m_oldInputValues.at(index));

            if (m_inputs.value(index)->outputSize() > 1) {
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
    const int newHeight = m_ui->centralwidget->height() - 2;

    if (newWidth > 4000 or newHeight > 4000) {
        on_actionResetZoom_triggered();
        throw Pandaception(tr("Waveform would be too big! Resetting zoom."));
    }

    m_signalTableView->resize(newWidth / (m_scale * 0.8),
                              newHeight / (m_scale * 0.8));
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
                tr("WiRedPanda - beWavedDolphin"),
                tr("Save simulation before closing?"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch(reply){
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

    qCDebug(three) << tr("Getting current value to check if need to refresh next cell");
    const int currentValue = index.data().toInt();

    qCDebug(three) << tr("Changing current item.");
    m_model->setData(index, 0, Qt::DisplayRole);

    if (m_type == PlotType::Number) {
        m_model->setData(index, Qt::AlignCenter, Qt::TextAlignmentRole);
    }

    if (m_type == PlotType::Line) {
        m_model->setData(index, Qt::AlignLeft, Qt::TextAlignmentRole);

        const auto previousIndex = index.siblingAtColumn(col - 1);
        const bool hasPreviousItem = previousIndex.isValid();
        const bool isPreviousHigh = hasPreviousItem ? previousIndex.data().toInt() == 1 : false;

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

        const auto nextIndex = m_model->index(row, col + 1);

        if (nextIndex.isValid() && (currentValue == 1)) {
            qCDebug(three) << tr("Changing next item.");
            createElement(row, col + 1, nextIndex.data().toInt(), isInput, false);
        }
    }
}

void BewavedDolphin::createOneElement(const int row, const int col, const bool isInput, const bool changeNext)
{
    const auto index = m_model->index(row, col);

    qCDebug(three) << tr("Getting current value to check if need to refresh next cell");
    const int currentValue = index.data().toInt();

    qCDebug(three) << tr("Changing current item.");
    m_model->setData(index, 1, Qt::DisplayRole);

    if (m_type == PlotType::Number) {
        m_model->setData(index, Qt::AlignCenter, Qt::TextAlignmentRole);
    }

    if (m_type == PlotType::Line) {
        m_model->setData(index, Qt::AlignLeft, Qt::TextAlignmentRole);

        const auto previousIndex = index.siblingAtColumn(col - 1);
        const bool hasPreviousItem = previousIndex.isValid();
        const bool isPreviousLow = hasPreviousItem ? previousIndex.data().toInt() == 0 : false;

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

        const auto nextIndex = m_model->index(row, col + 1);

        if (nextIndex.isValid() && (currentValue == 0)) {
            qCDebug(three) << tr("Changing next item.");
            createElement(row, col + 1, nextIndex.data().toInt(), isInput, false);
        }
    }
}

void BewavedDolphin::show()
{
    QMainWindow::show();
    qCDebug(zero) << tr("Getting table dimensions.");
    resizeScene();
}

void BewavedDolphin::print()
{
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
    on_actionCombinational_triggered();

    const int truthTableSize = std::pow(2, m_inputPorts);
    setLength(truthTableSize, false);

    for (int row = 0; row < m_inputs.size(); ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            stream << m_model->item(row, col)->text();
        }

        QString label = m_model->verticalHeaderItem(row)->text();
        stream << " : \"" << label << "\"\n";
    }

    stream << "\n";

    for (int row = m_inputs.size(); row < m_model->rowCount(); ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            stream << m_model->item(row, col)->text();
        }

        QString label = m_model->verticalHeaderItem(row)->text();
        stream << " : \"" << label << "\"\n";
    }
}

void BewavedDolphin::on_actionSetTo0_triggered()
{
    qCDebug(zero) << tr("Pressed 0.");
    const auto itemList = m_signalTableView->selectionModel()->selectedIndexes();

    for (const auto &item : itemList) {
        const int row = item.row();
        const int col = item.column();
        qCDebug(zero) << tr("Editing value.");
        createZeroElement(row, col);
    }

    m_edited = true;
    qCDebug(zero) << tr("Running simulation.");
    run();
}

void BewavedDolphin::on_actionSetTo1_triggered()
{
    qCDebug(zero) << tr("Pressed 0.");
    const auto itemList = m_signalTableView->selectionModel()->selectedIndexes();

    for (const auto &item : itemList) {
        const int row = item.row();
        const int col = item.column();
        qCDebug(zero) << tr("Editing value.");
        createOneElement(row, col);
    }

    m_edited = true;
    qCDebug(zero) << tr("Running simulation.");
    run();
}

void BewavedDolphin::on_actionInvert_triggered()
{
    qCDebug(zero) << tr("Pressed Not.");
    const auto itemList = m_signalTableView->selectionModel()->selectedIndexes();

    for (const auto &item : itemList) {
        const int row = item.row();
        const int col = item.column();
        int value = m_model->index(row, col, QModelIndex()).data().toInt();
        value = (value + 1) % 2;
        qCDebug(zero) << tr("Editing value.");
        createElement(row, col, value);
    }

    m_edited = true;
    qCDebug(zero) << tr("Running simulation.");
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
    qCDebug(zero) << tr("Getting first column.");
    const auto ranges = m_signalTableView->selectionModel()->selection();

    if (ranges.isEmpty()) {
        throw Pandaception(tr("No cells selected."));
    }

    const int firstCol = sectionFirstColumn(ranges);

    qCDebug(zero) << tr("Setting the signal according to its column and clock period.");
    ClockDialog dialog(m_clockPeriod, this);
    const int clockPeriod = dialog.frequency();

    if (clockPeriod < 0) {
        return;
    }

    m_clockPeriod = clockPeriod;

    const int halfClockPeriod = clockPeriod / 2;
    const auto itemList = m_signalTableView->selectionModel()->selectedIndexes();

    for (const auto &item : itemList) {
        const int row = item.row();
        const int col = item.column();
        const int value = ((col - firstCol) % clockPeriod < halfClockPeriod ? 0 : 1);
        qCDebug(zero) << tr("Editing value.");
        createElement(row, col, value);
    }

    m_edited = true;
    qCDebug(zero) << tr("Running simulation.");
    run();
}

void BewavedDolphin::on_actionCombinational_triggered()
{
    const int truthTableSize = std::pow(2, m_inputPorts);

    if (m_length < truthTableSize) {
        setLength(truthTableSize, false);
    }

    qCDebug(zero) << tr("Setting the signal according to its columns and clock period.");
    int halfClockPeriod = 1;
    int clockPeriod = 2;

    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            const int value = (col % clockPeriod < halfClockPeriod ? 0 : 1);
            createElement(row, col, value);
        }

        halfClockPeriod = std::min(clockPeriod, 524'288);
        clockPeriod = std::min(2 * clockPeriod, 1'048'576);
    }

    m_edited = true;
    qCDebug(zero) << tr("Running simulation.");
    run();
}

void BewavedDolphin::on_actionSetLength_triggered()
{
    qCDebug(zero) << tr("Setting the simulation length.");
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
        qCDebug(zero) << tr("Reducing or keeping the simulation length.");
        m_model->setColumnCount(simLength);
        resizeScene();
        m_edited = true;
        return;
    }

    qCDebug(zero) << tr("Increasing the simulation length.");
    const int oldLength = m_model->columnCount();
    m_model->setColumnCount(simLength);

    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = oldLength; col < m_model->columnCount(); ++col) {
            createZeroElement(row, col, true, false);
        }
    }

    resizeScene();
    m_edited = true;
    qCDebug(zero) << tr("Running simulation.");

    if (runSimulation) {
        run();
    }
}

void BewavedDolphin::on_actionZoomOut_triggered()
{
    m_scale *= m_scaleFactor;
    m_ui->graphicsView->zoomOut();
    resizeScene();
    zoomChanged();
}

void BewavedDolphin::on_actionZoomIn_triggered()
{
    m_scale /= m_scaleFactor;
    m_ui->graphicsView->zoomIn();
    resizeScene();
    zoomChanged();
}

void BewavedDolphin::on_actionResetZoom_triggered()
{
    m_ui->graphicsView->resetZoom();
    m_scale = 1.25;
    resizeScene();
    zoomChanged();
}

void BewavedDolphin::zoomChanged()
{
    m_ui->actionZoomIn->setEnabled(m_view.canZoomIn());
    m_ui->actionZoomOut->setEnabled(m_view.canZoomOut());
}

void BewavedDolphin::on_actionClear_triggered()
{
    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            createZeroElement(row, col);
        }
    }

    m_edited = true;
    qCDebug(zero) << tr("Running simulation.");
    run();
}

void BewavedDolphin::on_actionCopy_triggered()
{
    const auto ranges = m_signalTableView->selectionModel()->selection();

    if (ranges.isEmpty()) {
        QApplication::clipboard()->clear();
        return;
    }

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    copy(ranges, stream);

    auto *mimeData = new QMimeData();
    mimeData->setData("bdolphin/copydata", itemData);

    QApplication::clipboard()->setMimeData(mimeData);
}

void BewavedDolphin::copy(const QItemSelection &ranges, QDataStream &stream)
{
    qCDebug(zero) << tr("Serializing data into data stream.");
    const int firstRow = sectionFirstRow(ranges);
    const int firstCol = sectionFirstColumn(ranges);
    const auto itemList = m_signalTableView->selectionModel()->selectedIndexes();
    stream << static_cast<qint64>(itemList.size());

    for (const auto &item : itemList) {
        const int row = item.row();
        const int col = item.column();
        const int data_ = m_model->index(row, col).data().toInt();
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
    stream.setVersion(QDataStream::Qt_5_12);
    cut(ranges, stream);

    auto *mimeData = new QMimeData();
    mimeData->setData("bdolphin/copydata", itemData);

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

    if (mimeData->hasFormat("bdolphin/copydata")) {
        QByteArray itemData = mimeData->data("bdolphin/copydata");
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_12);
        paste(ranges, stream);
    }

    m_edited = true;
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
        const int newRow = static_cast<int>(firstRow + row);
        const int newCol = static_cast<int>(firstCol + col);

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

    if (!fileName.endsWith(".dolphin") && !fileName.endsWith(".csv")) {
        if (fileDialog.selectedNameFilter().contains("dolphin")) {
            fileName.append(".dolphin");
        } else {
            fileName.append(".csv");
        }
    }

    save(fileName);
    m_currentFile = QFileInfo(fileName);
    associateToWiredPanda(fileName);
    setWindowTitle(tr("beWavedDolphin Simulator") + " [" + m_currentFile.fileName() + "]");
    m_ui->statusbar->showMessage(tr("Saved file successfully."), 4000);
    m_edited = false;
}

void BewavedDolphin::save(const QString &fileName)
{
    QSaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        throw Pandaception(tr("Error opening file: ") + file.errorString());
    }

    if (fileName.endsWith(".dolphin")) {
        qCDebug(zero) << tr("Saving dolphin file.");
        QDataStream stream(&file);
        stream.setVersion(QDataStream::Qt_5_12);
        save(stream);
    } else {
        qCDebug(zero) << tr("Saving CSV file.");
        save(file);
    }

    if (!file.commit()) {
        throw Pandaception(tr("Error saving file: ") + file.errorString());
    }
}

void BewavedDolphin::save(QDataStream &stream)
{
    stream << QString("beWavedDolphin 1.0");
    qCDebug(zero) << tr("Serializing data into data stream.");
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

void BewavedDolphin::associateToWiredPanda(const QString &fileName)
{
    if ((m_mainWindow->dolphinFileName() != fileName) && GlobalProperties::verbose) {
        const auto reply =
            QMessageBox::question(
                this,
                tr("WiRedPanda - beWavedDolphin"),
                tr("Do you want to link this beWavedDolphin file to your current WiRedPanda file and save it?"),
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
        throw Pandaception(tr("File \"%1\" does not exist!").arg(fileName));
    }

    qCDebug(zero) << tr("File exists.");

    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(zero) << tr("Could not open file in ReadOnly mode: ") << file.errorString();
        throw Pandaception(tr("Could not open file in ReadOnly mode: ") + file.errorString() + ".");
    }

    if (fileName.endsWith(".dolphin")) {
        qCDebug(zero) << tr("Dolphin file opened.");
        QDataStream stream(&file);
        stream.setVersion(QDataStream::Qt_5_12);
        qCDebug(zero) << tr("Loading in editor.");
        load(stream);
        qCDebug(zero) << tr("Current file set.");
        m_currentFile = QFileInfo(fileName);
    } else if (fileName.endsWith(".csv")) {
        qCDebug(zero) << tr("CSV file opened.");
        qCDebug(zero) << tr("Loading in editor.");
        load(file);
        qCDebug(zero) << tr("Current file set.");
        m_currentFile = QFileInfo(fileName);
    } else {
        qCDebug(zero) << tr("Format not supported. Could not open file: ") << fileName;
        throw Pandaception(tr("Format not supported. Could not open file: ") + fileName);
    }

    qCDebug(zero) << tr("Closing file.");
    file.close();
    associateToWiredPanda(fileName);
    setWindowTitle(tr("beWavedDolphin Simulator") + " [" + m_currentFile.fileName() + "]");
}

void BewavedDolphin::load(QDataStream &stream)
{
    QString str; stream >> str;

    if (!str.startsWith("beWavedDolphin")) {
        throw Pandaception(tr("Invalid file format. Starts with: ") + str);
    }

    qint64 rows; stream >> rows;
    qint64 cols; stream >> cols;

    if (rows > m_inputPorts) {
        rows = m_inputPorts;
    }

    if ((cols < 2) || (cols > 2048)) {
        throw Pandaception(tr("Invalid number of columns."));
    }

    setLength(static_cast<int>(cols), false);
    qCDebug(zero) << tr("Update table.");

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
    const QByteArray content = file.readAll();
    const auto wordList(content.split(','));
    int rows = wordList.at(0).toInt();
    const int cols = wordList.at(1).toInt();

    if (rows > m_inputPorts) {
        rows = m_inputPorts;
    }

    if ((cols < 2) || (cols > 2048)) {
        throw Pandaception(tr("Invalid number of columns."));
    }

    setLength(cols, false);

    qCDebug(zero) << tr("Update table.");

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

    for (int row = 0; row < m_model->rowCount(); ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            m_model->setData(m_model->index(row, col), QVariant(), Qt::DecorationRole);
        }
    }

    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            createElement(row, col, m_model->index(row, col).data().toInt());
        }
    }

    qCDebug(zero) << tr("Running simulation.");
    run();
}

void BewavedDolphin::on_actionShowWaveforms_triggered()
{
    m_type = PlotType::Line;

    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            createElement(row, col, m_model->index(row, col).data().toInt());
        }
    }

    qCDebug(zero) << tr("Running simulation.");
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
    QPixmap pixmap(sceneRect.size().toSize());

    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
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

    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Orientation::Landscape);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(pdfFile);

    QPainter painter;

    if (!painter.begin(&printer)) {
        throw Pandaception(tr("Could not print this circuit to PDF."));
    }

    m_scene->render(&painter, QRectF(), m_scene->sceneRect().adjusted(-64, -64, 64, 64));
    painter.end();
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

void BewavedDolphin::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this);
}
