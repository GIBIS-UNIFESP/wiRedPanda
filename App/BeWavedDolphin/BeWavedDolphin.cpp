// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/BeWavedDolphin.h"

#include <cmath>
#include <iostream>

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QPrinter>
#include <QSaveFile>
#include <QTextStream>

#include "App/BeWavedDolphin/Serializer.h"
#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/Settings.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/IO/Serialization.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/UI/ClockDialog.h"
#include "App/UI/LengthDialog.h"
#include "App/UI/MainWindow.h"

static constexpr int kMaxSimLength = 2048;

BewavedDolphin::BewavedDolphin(Scene *scene, const bool askConnection, MainWindow *parent)
    : QMainWindow(parent)
    , m_ui(std::make_unique<BewavedDolphinUi>())
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
    restoreGeometry(Settings::dolphinGeometry());

    // Create the waveform grid item and add it to the scene
    m_grid = new WaveformGrid();
    m_scene->addItem(m_grid);

    m_view.setScene(m_scene);
    m_view.setRedirectZoom(true);
    m_view.setBackgroundBrush(palette().window());
    m_ui->verticalLayout->addWidget(&m_view);

    m_ui->mainToolBar->setToolButtonStyle(Settings::labelsUnderIcons() ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);

    connect(&m_view,                   &WaveformView::scaleIn,         this, &BewavedDolphin::on_actionZoomIn_triggered);
    connect(&m_view,                   &WaveformView::scaleOut,        this, &BewavedDolphin::on_actionZoomOut_triggered);
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

    connect(m_grid, &WaveformGrid::selectionChanged, this, &BewavedDolphin::onSelectionChanged);
    connect(m_grid, &WaveformGrid::cellEdited,       this, &BewavedDolphin::onCellEdited);
}

BewavedDolphin::~BewavedDolphin()
{
    Settings::setDolphinGeometry(saveGeometry());
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

    qCDebug(zero) << "Resuming digital circuit main window after waveform simulation is finished.";
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
        throw PANDACEPTION("");
    }

    int rows = wordList.at(0).toInt();
    const int cols = wordList.at(1).toInt();

    if (rows > m_inputPorts) {
        rows = m_inputPorts;
    }

    if ((cols < 2) || (cols > kMaxSimLength)) {
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
            createElement(row, col, value, true, false);
        }
    }

    run();
}

void BewavedDolphin::prepare(const QString &fileName)
{
    qCDebug(zero) << "Updating window name with current: " << fileName;
    m_simulation = m_externalScene->simulation();
    loadElements();
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
    QStringList inputLabels;
    QStringList outputLabels;
    loadSignals(inputLabels, outputLabels);

    m_grid->load(inputLabels, outputLabels, m_length);

    on_actionClear_triggered();
}

void BewavedDolphin::onSelectionChanged()
{
    m_externalScene->clearSelection();

    for (auto [row, col] : m_grid->selectedCells()) {
        if (row < m_inputs.size()) {
            m_inputs.at(row)->setSelected(true);
        }
    }

    m_externalScene->view()->update();
}

void BewavedDolphin::onCellEdited(int /*row*/, int /*col*/, int /*newValue*/)
{
    m_edited = true;
    run();
}

void BewavedDolphin::loadSignals(QStringList &inputLabels, QStringList &outputLabels)
{
    QVector<Status> oldValues(m_inputPorts);
    int oldIndex = 0;

    for (auto *input : std::as_const(m_inputs)) {
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

    for (auto *output : std::as_const(m_outputs)) {
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
    SimulationBlocker simulationBlocker(m_simulation);

    for (int column = 0; column < m_grid->length(); ++column) {
        int row = 0;

        for (auto *input : std::as_const(m_inputs)) {
            const bool isRotary = qobject_cast<InputRotary *>(input);
            for (int port = 0; port < input->outputSize(); ++port) {
                const bool value = m_grid->cellValue(row++, column);

                if (isRotary && value) {
                    input->setOn(1, port);
                } else if (!isRotary) {
                    input->setOn(value, port);
                }
            }
        }

        m_simulation->update();

        row = m_inputPorts;

        for (auto *output : std::as_const(m_outputs)) {
            for (int port = 0; port < output->inputSize(); ++port) {
                const int value = static_cast<int>(output->inputPort(port)->status());
                m_grid->setCellValue(row, column, value);
                ++row;
            }
        }
    }

    restoreInputs();
}

void BewavedDolphin::restoreInputs()
{
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
    m_scene->setSceneRect(m_grid->boundingRect());
    m_view.fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
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
    case QMessageBox::Save:    on_actionSave_triggered(); return (!m_edited);
    case QMessageBox::Discard: return true;
    case QMessageBox::Cancel:  return false;
    default:                   return true;
    }
}

void BewavedDolphin::createElement(const int row, const int col, const int value, const bool /*isInput*/, const bool /*changeNext*/)
{
    setCellValue(row, col, value);
}

void BewavedDolphin::setCellValue(const int row, const int col, const int value)
{
    m_grid->setCellValue(row, col, value);
}

void BewavedDolphin::show()
{
    QMainWindow::show();
    resizeScene();
}

void BewavedDolphin::print()
{
    std::cout << m_grid->rowCount() << ",";
    std::cout << m_grid->length() << ",\n";

    for (int row = 0; row < m_grid->rowCount(); ++row) {
        for (int col = 0; col < m_grid->length(); ++col) {
            std::cout << m_grid->cellValue(row, col) << ",";
        }

        std::cout << "\n";
    }
}

void BewavedDolphin::saveToTxt(QTextStream &stream)
{
    on_actionCombinational_triggered();

    const int truthTableSize = static_cast<int>(std::pow(2, m_inputPorts));
    setLength(truthTableSize, false);

    for (int row = 0; row < m_inputs.size(); ++row) {
        for (int col = 0; col < m_grid->length(); ++col) {
            stream << m_grid->cellValue(row, col);
        }

        stream << " : \"" << m_grid->rowLabel(row) << "\"\n";
    }

    stream << "\n";

    for (int row = static_cast<int>(m_inputs.size()); row < m_grid->rowCount(); ++row) {
        for (int col = 0; col < m_grid->length(); ++col) {
            stream << m_grid->cellValue(row, col);
        }

        stream << " : \"" << m_grid->rowLabel(row) << "\"\n";
    }
}

bool BewavedDolphin::exportToPng(const QString &filename)
{
    try {
        resizeScene();
        return exportWaveformToPng(filename);
    } catch (...) {
        return false;
    }
}

void BewavedDolphin::on_actionSetTo0_triggered()
{
    for (auto [row, col] : m_grid->selectedCells()) {
        setCellValue(row, col, 0);
    }

    m_edited = true;
    run();
}

void BewavedDolphin::on_actionSetTo1_triggered()
{
    for (auto [row, col] : m_grid->selectedCells()) {
        setCellValue(row, col, 1);
    }

    m_edited = true;
    run();
}

void BewavedDolphin::on_actionInvert_triggered()
{
    for (auto [row, col] : m_grid->selectedCells()) {
        const int value = (m_grid->cellValue(row, col) + 1) % 2;
        setCellValue(row, col, value);
    }

    m_edited = true;
    run();
}

void BewavedDolphin::on_actionSetClockWave_triggered()
{
    if (!m_grid->hasSelection()) {
        throw PANDACEPTION("No cells selected.");
    }

    const int firstCol = m_grid->selectionFirstCol();

    ClockDialog dialog(m_clockPeriod, this);
    const int clockPeriod = dialog.frequency();

    if (clockPeriod < 0) {
        return;
    }

    m_clockPeriod = clockPeriod;
    const int halfClockPeriod = clockPeriod / 2;

    for (auto [row, col] : m_grid->selectedCells()) {
        const int value = ((col - firstCol) % clockPeriod < halfClockPeriod ? 0 : 1);
        setCellValue(row, col, value);
    }

    m_edited = true;
    run();
}

void BewavedDolphin::on_actionCombinational_triggered()
{
    const int truthTableSize = static_cast<int>(std::min(static_cast<double>(kMaxSimLength), std::pow(2, m_inputPorts)));

    if (m_length < truthTableSize) {
        setLength(truthTableSize, false);
    }

    int halfClockPeriod = 1;
    int clockPeriod     = 2;

    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = 0; col < m_grid->length(); ++col) {
            setCellValue(row, col, (col % clockPeriod < halfClockPeriod ? 0 : 1));
        }

        halfClockPeriod = std::min(clockPeriod, 524288);
        clockPeriod     = std::min(2 * clockPeriod, 1048576);
    }

    m_edited = true;
    run();
}

void BewavedDolphin::on_actionSetLength_triggered()
{
    const int currentLength = m_length > 0 ? m_length : m_grid->length();
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
    m_grid->setLength(simLength);
    resizeScene();
    m_edited = true;

    if (runSimulation) {
        run();
    }
}

void BewavedDolphin::on_actionZoomOut_triggered()
{
    m_view.zoomOut();
    zoomChanged();
}

void BewavedDolphin::on_actionZoomIn_triggered()
{
    m_view.zoomIn();
    zoomChanged();
}

void BewavedDolphin::on_actionResetZoom_triggered()
{
    m_view.resetZoom();
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
    m_view.resetZoom();
    resizeScene();
    zoomChanged();
}

void BewavedDolphin::on_actionClear_triggered()
{
    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = 0; col < m_grid->length(); ++col) {
            setCellValue(row, col, 0);
        }
    }

    m_edited = true;
    run();
}

void BewavedDolphin::on_actionAutoCrop_triggered()
{
    setLength(static_cast<int>(std::pow(2, m_inputs.length())), true);
}

void BewavedDolphin::on_actionCopy_triggered()
{
    if (!m_grid->hasSelection()) {
        QApplication::clipboard()->clear();
        return;
    }

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writeDolphinHeader(stream);
    copy(stream);

    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-bewaveddolphin-waveform", itemData);
    QApplication::clipboard()->setMimeData(mimeData);
}

void BewavedDolphin::copy(QDataStream &stream)
{
    const int firstRow = m_grid->selectionFirstRow();
    const int firstCol = m_grid->selectionFirstCol();
    const auto cells = m_grid->selectedCells();
    stream << cells.size();

    for (auto [row, col] : cells) {
        const int data_ = m_grid->cellValue(row, col);
        stream << static_cast<qint64>(row - firstRow);
        stream << static_cast<qint64>(col - firstCol);
        stream << static_cast<qint64>(data_);
    }
}

void BewavedDolphin::on_actionCut_triggered()
{
    if (!m_grid->hasSelection()) {
        QApplication::clipboard()->clear();
        return;
    }

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writeDolphinHeader(stream);
    cut(stream);

    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-bewaveddolphin-waveform", itemData);
    QApplication::clipboard()->setMimeData(mimeData);
    m_edited = true;
}

void BewavedDolphin::cut(QDataStream &stream)
{
    copy(stream);
    on_actionSetTo0_triggered();
}

void BewavedDolphin::on_actionPaste_triggered()
{
    if (!m_grid->hasSelection()) {
        return;
    }

    const auto *mimeData = QApplication::clipboard()->mimeData();
    QByteArray itemData;

    if (mimeData->hasFormat("bdolphin/copydata")) {
        itemData = mimeData->data("bdolphin/copydata");
    }

    if (mimeData->hasFormat("application/x-bewaveddolphin-waveform")) {
        itemData = mimeData->data("application/x-bewaveddolphin-waveform");
    }

    if (!itemData.isEmpty()) {
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        Serialization::readDolphinHeader(stream);
        paste(stream);
        m_edited = true;
    }
}

void BewavedDolphin::paste(QDataStream &stream)
{
    const int firstCol = m_grid->selectionFirstCol();
    const int firstRow = m_grid->selectionFirstRow();
    quint64 itemListSize; stream >> itemListSize;

    for (int i = 0; i < static_cast<int>(itemListSize); ++i) {
        quint64 row;   stream >> row;
        quint64 col;   stream >> col;
        quint64 data_; stream >> data_;
        const int newRow = static_cast<int>(static_cast<quint64>(firstRow) + row);
        const int newCol = static_cast<int>(static_cast<quint64>(firstCol) + col);

        if ((newRow < m_inputPorts) && (newCol < m_grid->length())) {
            setCellValue(newRow, newCol, static_cast<int>(data_));
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
    associateToWiRedPanda(fileName);
    setWindowTitle(tr("beWavedDolphin Simulator") + " [" + m_currentFile.fileName() + "]");
    m_ui->statusbar->showMessage(tr("Saved file successfully."), 4000);
    m_edited = false;
}

void BewavedDolphin::save(const QString &fileName)
{
    QSaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        throw PANDACEPTION("Error opening file: %1", file.errorString());
    }

    if (fileName.endsWith(".dolphin")) {
        QDataStream stream(&file);
        Serialization::writeDolphinHeader(stream);
        save(stream);
    } else {
        save(file);
    }

    if (!file.commit()) {
        throw PANDACEPTION("Error saving file: %1", file.errorString());
    }
}

void BewavedDolphin::save(QDataStream &stream)
{
    DolphinSerializer::saveBinary(stream, m_grid, m_inputPorts);
}

void BewavedDolphin::save(QSaveFile &file)
{
    DolphinSerializer::saveCSV(file, m_grid);
}

void BewavedDolphin::associateToWiRedPanda(const QString &fileName)
{
    if ((m_mainWindow->dolphinFileName() != fileName) && Application::interactiveMode) {
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

    if (!file.open(QIODevice::ReadOnly)) {
        throw PANDACEPTION("Could not open file in ReadOnly mode: %1", file.errorString());
    }

    if (fileName.endsWith(".dolphin")) {
        QDataStream stream(&file);
        Serialization::readDolphinHeader(stream);
        load(stream);
        m_currentFile = QFileInfo(fileName);
    } else if (fileName.endsWith(".csv")) {
        load(file);
        m_currentFile = QFileInfo(fileName);
    } else {
        throw PANDACEPTION("Format not supported. Could not open file: %1", fileName);
    }

    file.close();
    associateToWiRedPanda(fileName);
    setWindowTitle(tr("beWavedDolphin Simulator") + " [" + m_currentFile.fileName() + "]");
}

void BewavedDolphin::load(QDataStream &stream)
{
    const auto loadedData = DolphinSerializer::loadBinary(stream, m_inputPorts);
    setLength(loadedData.columns, false);

    for (int row = 0; row < loadedData.inputPorts; ++row) {
        for (int col = 0; col < loadedData.columns; ++col) {
            setCellValue(row, col, loadedData.values[row * loadedData.columns + col]);
        }
    }

    run();
}

void BewavedDolphin::load(QFile &file)
{
    const auto loadedData = DolphinSerializer::loadCSV(file, m_inputPorts);
    setLength(loadedData.columns, false);

    for (int row = 0; row < loadedData.inputPorts; ++row) {
        for (int col = 0; col < loadedData.columns; ++col) {
            setCellValue(row, col, loadedData.values[row * loadedData.columns + col]);
        }
    }

    run();
}

void BewavedDolphin::on_actionShowNumbers_triggered()
{
    m_grid->setPlotType(PlotType::Number);
    run();
}

void BewavedDolphin::on_actionShowWaveforms_triggered()
{
    m_grid->setPlotType(PlotType::Line);
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

    exportWaveformToPng(pngFile);
}

bool BewavedDolphin::exportWaveformToPng(const QString &filename)
{
    const QRectF sceneRect = m_scene->sceneRect();
    QPixmap pixmap(sceneRect.size().toSize());

    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    m_scene->render(&painter, QRectF(), sceneRect);
    painter.end();

    return pixmap.toImage().save(filename);
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
        throw PANDACEPTION("Could not print this circuit to PDF.");
    }

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
