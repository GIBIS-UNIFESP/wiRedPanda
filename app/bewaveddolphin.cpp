#include "bewaveddolphin.h"

#include <cstring>

#include <QClipboard>
#include <QFileDialog>
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <QHeaderView>
#include <QMessageBox>
#include <QMimeData>
#include <QPrinter>
#include <QSaveFile>
#include <QSettings>
#include <QSizePolicy>
#include <QtDebug>

#include "clockDialog.h"
#include "elementmapping.h"
#include "input.h"
#include "lengthDialog.h"
#include "mainwindow.h"
#include "scstop.h"

#include "ui_bewaveddolphin.h"

SignalModel::SignalModel(int rows, int inputs, int columns, QObject *parent)
    : QStandardItemModel(rows, columns, parent)
    , inputs(inputs)
{
}

Qt::ItemFlags SignalModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags;
    if (index.row() >= inputs)
        flags = ~(Qt::ItemIsEditable | Qt::ItemIsSelectable);
    else
        flags = ~(Qt::ItemIsEditable | Qt::ItemIsSelectable) | Qt::ItemIsSelectable;
    return (flags);
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

BewavedDolphin::BewavedDolphin(Editor *editor, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::BewavedDolphin)
    , editor(editor)
    , mainWindow(dynamic_cast<MainWindow *>(parent))
    , type(PlotType::line)
{
    scale = 1.0;
    ui->setupUi(this);
    resize(800, 500);
    setWindowTitle("Bewaved Dolphin Simulator");
    setWindowFlags(Qt::Window);
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("BewavedDolphin");
    restoreGeometry(settings.value("geometry").toByteArray());
    settings.endGroup();
    gv = new GraphicsView(this);
    ui->verticalLayout->addWidget(gv);
    scene = new QGraphicsScene(this);
    // ui->graphicsView->setScene( scene );
    gv->setScene(scene);
    signalTableView = new QTableView;
    scene->installEventFilter(signalTableView);
    auto *delegate = new SignalDelegate(4, this);
    signalTableView->setItemDelegate(delegate);
    scene->addWidget(signalTableView);
    QList<QKeySequence> zoom_in_shortcuts;
    zoom_in_shortcuts << QKeySequence("Ctrl++") << QKeySequence("Ctrl+=");
    ui->actionZoom_In->setShortcuts(zoom_in_shortcuts);
    // connect( gv->gvzoom( ), &GraphicsViewZoom::zoomed, this, &BewavedDolphin::zoomChanged );
    gv->gvzoom()->set_zoom_factor_base(SCALE_FACTOR);
    drawPixMaps();
    edited = false;
}

BewavedDolphin::~BewavedDolphin()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("BewavedDolphin");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
    delete ui;
}

void BewavedDolphin::zoomChanged()
{
    ui->actionZoom_In->setEnabled(gv->gvzoom()->canZoomIn());
    ui->actionZoom_out->setEnabled(gv->gvzoom()->canZoomOut());
}

void BewavedDolphin::drawPixMaps()
{
    lowGreen = QPixmap(":/dolphin/low_green.png").scaled(100, 38);
    lowBlue = QPixmap(":/dolphin/low_blue.png").scaled(100, 38);
    highGreen = QPixmap(":/dolphin/high_green.png").scaled(100, 38);
    highBlue = QPixmap(":/dolphin/high_blue.png").scaled(100, 38);
    fallingGreen = QPixmap(":/dolphin/falling_green.png").scaled(100, 38);
    fallingBlue = QPixmap(":/dolphin/falling_blue.png").scaled(100, 38);
    risingGreen = QPixmap(":/dolphin/rising_green.png").scaled(100, 38);
    risingBlue = QPixmap(":/dolphin/rising_blue.png").scaled(100, 38);
}

void BewavedDolphin::closeEvent(QCloseEvent *e)
{
    e->ignore();
    if (checkSave())
        e->accept();
}

void BewavedDolphin::on_actionExit_triggered()
{
    close();
}

bool BewavedDolphin::checkSave()
{
    if (edited) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      tr("Wired Panda - Bewaved Dolphin"),
                                      tr("Save simulation before closing?"),
                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (reply == QMessageBox::Save) {
            on_actionSave_triggered();
            if (edited)
                return (false);
            else
                return (true);
        } else if (reply == QMessageBox::Discard)
            return (true);
        return (false);
    }
    return (true);
}

bool BewavedDolphin::loadElements()
{
    QVector<GraphicElement *> elements = editor->getScene()->getElements();
    inputs.clear();
    outputs.clear();
    if (elements.isEmpty()) {
        return (false);
    }
    elements = ElementMapping::sortGraphicElements(elements);
    for (GraphicElement *elm : qAsConst(elements)) {
        if (elm && (elm->type() == GraphicElement::Type)) {
            if (elm->elementGroup() == ElementGroup::INPUT) {
                inputs.append(elm);
            } else if (elm->elementGroup() == ElementGroup::OUTPUT) {
                outputs.append(elm);
            }
        }
    }
    std::stable_sort(inputs.begin(), inputs.end(), [](GraphicElement *elm1, GraphicElement *elm2) {
        return (QString::compare(elm1->getLabel().toUtf8(), elm2->getLabel().toUtf8(), Qt::CaseInsensitive) <= 0);
    });
    std::stable_sort(outputs.begin(), outputs.end(), [](GraphicElement *elm1, GraphicElement *elm2) {
        return (QString::compare(elm1->getLabel().toUtf8(), elm2->getLabel().toUtf8(), Qt::CaseInsensitive) <= 0);
    });

    if ((inputs.isEmpty()) || (outputs.isEmpty()))
        return (false);
    return (true);
}

void BewavedDolphin::CreateElement(int row, int col, int value, bool isInput, bool changePrevious)
{
    if (value == 0)
        return (CreateZeroElement(row, col, isInput, changePrevious));
    else
        return (CreateOneElement(row, col, isInput, changePrevious));
}

void BewavedDolphin::CreateZeroElement(int row, int col, bool isInput, bool changePrevious)
{
    COMMENT("Getting previous value to check previous cell refresh requirement.", 3);
    // QMessageBox::warning( this, "Zero: row, col", ( row + 48 ) + QString( " " ) + ( col + 48 ) );
    int previous_value;
    if (model->item(row, col) == nullptr)
        previous_value = -1;
    else if (model->item(row, col)->text().toInt() == 0)
        previous_value = 0;
    else
        previous_value = 1;
    COMMENT("Changing current item.", 3);
    auto index = model->index(row, col);
    model->setData(index, "0", Qt::DisplayRole);
    model->setData(index, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (type == PlotType::line) {
        model->setData(index, Qt::AlignLeft, Qt::TextAlignmentRole);
        if ((col != model->columnCount() - 1) && (model->item(row, col + 1) != nullptr) && (model->item(row, col + 1)->text().toInt() == 1))
            model->setData(index, risingGreen, Qt::DecorationRole);
        else
            model->setData(index, lowGreen, Qt::DecorationRole);
    } else
        model->setData(index, Qt::AlignCenter, Qt::TextAlignmentRole);
    COMMENT("Changing previous item, if needed.", 3);
    if ((changePrevious) && (col != 0) && (previous_value == 1))
        CreateElement(row, col - 1, model->item(row, col - 1)->text().toInt(), isInput, false);
}

void BewavedDolphin::CreateOneElement(int row, int col, bool isInput, bool changePrevious)
{
    COMMENT("Getting previous value to check previous cell refresh requirement.", 3);
    // QMessageBox::warning( this, "One: row, col", ( row + 48 ) + QString( " " ) + ( col + 48 ) );
    int previous_value;
    if (model->item(row, col) == nullptr)
        previous_value = -1;
    else if (model->item(row, col)->text().toInt() == 0)
        previous_value = 0;
    else
        previous_value = 1;
    COMMENT("Changing current item.", 3);
    auto index = model->index(row, col);
    model->setData(index, "1", Qt::DisplayRole);
    if (type == PlotType::line) {
        model->setData(index, Qt::AlignLeft, Qt::TextAlignmentRole);
        if ((col != model->columnCount() - 1) && (model->item(row, col + 1) != nullptr) && (model->item(row, col + 1)->text().toInt() == 0))
            model->setData(index, fallingGreen, Qt::DecorationRole);
        else
            model->setData(index, highGreen, Qt::DecorationRole);
    } else
        model->setData(index, Qt::AlignCenter, Qt::TextAlignmentRole);
    COMMENT("Changing previous item, if needed.", 3);
    if ((changePrevious) && (col != 0) && (previous_value == 0))
        CreateElement(row, col - 1, model->item(row, col - 1)->text().toInt(), isInput, false);
}

void BewavedDolphin::run()
{
    for (int itr = 0; itr < model->columnCount(); ++itr) {
        COMMENT("itr:" << itr, 3);
        for (int in = 0; in < inputs.size(); ++in) {
            int val = model->item(in, itr)->text().toInt();
            dynamic_cast<Input *>(inputs[in])->setOn(val);
        }
        COMMENT("Updating the values of the circuit logic based on current input values.", 3);
        sc->update();
        sc->updateAll();
        COMMENT("Setting the computed output values to the waveform results.", 3);
        int counter = inputs.size();
        for (int out = 0; out < outputs.size(); ++out) {
            int inSz = outputs[out]->inputSize();
            for (int port = inSz - 1; port >= 0; --port) {
                int value = static_cast<int>(outputs[out]->input(port)->value());
                COMMENT("out value: " << value, 0);
                CreateElement(counter, itr, value, false);
                counter++;
            }
        }
    }
    signalTableView->viewport()->update();
}

void BewavedDolphin::loadNewTable(QStringList &input_labels, QStringList &output_labels)
{
    int iterations = 32;
    COMMENT("Num iter = " << iterations, 0);
    COMMENT("Update table.", 0);
    model = new SignalModel(inputs.size() + output_labels.size(), inputs.size(), iterations, this);
    signalTableView->setModel(model);
    model->setVerticalHeaderLabels(input_labels + output_labels);
    signalTableView->setAlternatingRowColors(true);
    signalTableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    signalTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    COMMENT("Inputs: " << input_labels.size() << ", outputs: " << output_labels.size(), 0);
    edited = false;
    on_actionClear_triggered();
}

QVector<char> BewavedDolphin::loadSignals(QStringList &input_labels, QStringList &output_labels)
{
    QVector<char> oldValues(inputs.size());
    for (int in = 0; in < inputs.size(); ++in) {
        QString label = inputs[in]->getLabel();
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(inputs[in]->elementType());
        }
        input_labels.append(label);
        oldValues[in] = inputs[in]->output()->value();
    }
    COMMENT(
        "Getting the name of the outputs. If no label is given, the element type is used as a name. Bug here? What if there are 2 outputs without name or two "
        "identical labels?",
        0);
    for (int out = 0; out < outputs.size(); ++out) {
        QString label = outputs[out]->getLabel();
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(outputs[out]->elementType());
        }
        for (int port = 0; port < outputs[out]->inputSize(); ++port) {
            if (outputs[out]->inputSize() > 1) {
                output_labels.append(QString("%1_%2").arg(label).arg(port));
            } else {
                output_labels.append(label);
            }
        }
    }
    return (oldValues);
}

bool BewavedDolphin::createWaveform(QString filename)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    COMMENT("Getting digital circuit simulator.", 0);
    sc = editor->getSimulationController();
    COMMENT("Creating class to pause main window simulator while creating waveform.", 0);
    SCStop scst(sc);
    COMMENT("Loading elements. All elements initially in elements vector. Then, inputs and outputs are extracted from it.", 0);
    if (!loadElements()) {
        QMessageBox::warning(parentWidget(), tr("Error"), tr("Could not load enough elements for the simulation."));
        return (false);
    }
    QStringList input_labels;
    QStringList output_labels;
    COMMENT(
        "Getting initial value from inputs and writing them to oldvalues. Used to save current state of inputs and restore it after simulation. Not saving "
        "memory states though...",
        0);
    COMMENT(
        "Also getting the name of the inputs. If no label is given, the element type is used as a name. Bug here? What if there are 2 inputs without name or "
        "two identical labels?",
        0);
    QVector<char> oldValues = loadSignals(input_labels, output_labels);
    COMMENT("Loading initial data into the table.", 0);
    loadNewTable(input_labels, output_labels);
    if (filename != "none") {
        if (!load(filename))
            return (false);
    }
    COMMENT("Restoring the old values to the inputs, prior to simulaton.", 0);
    for (int in = 0; in < inputs.size(); ++in) {
        dynamic_cast<Input *>(inputs[in])->setOn(oldValues[in]);
    }
    COMMENT("Resuming digital circuit main window after waveform simulation is finished.", 0);
    scst.release();
    edited = false;
    return (true);
}

void BewavedDolphin::show()
{
    QMainWindow::show();
    COMMENT("Getting table dimensions.", 0);
    int width = signalTableView->horizontalHeader()->length() + signalTableView->columnWidth(0);
    int hight = signalTableView->verticalHeader()->length() + signalTableView->rowHeight(0);
    signalTableView->resize(width, hight);
}

void BewavedDolphin::print()
{
    std::cout << std::to_string(model->rowCount()).c_str() << ",";
    std::cout << std::to_string(model->columnCount()).c_str() << ",\n";
    for (int row = 0; row < model->rowCount(); ++row) {
        std::cout << model->verticalHeaderItem(row)->text().toStdString() << ": ";
        for (int col = 0; col < model->columnCount(); ++col) {
            QString val = model->item(row, col)->text();
            std::cout << val.toStdString().c_str() << ",";
        }
        std::cout << "\n";
    }
}

void BewavedDolphin::on_actionSet_to_0_triggered()
{
    COMMENT("Pressed 0!", 0);
    // auto itemList = signalTableView->selectedItems( );
    auto itemList = signalTableView->selectionModel()->selectedIndexes();
    for (auto item : qAsConst(itemList)) {
        int row = item.row();
        int col = item.column();
        COMMENT("Editing value.", 0);
        CreateZeroElement(row, col);
    }
    edited = true;
    COMMENT("Running simulation", 0);
    run();
}

void BewavedDolphin::on_actionSet_to_1_triggered()
{
    COMMENT("Pressed 0!", 0);
    auto itemList = signalTableView->selectionModel()->selectedIndexes();
    for (auto item : qAsConst(itemList)) {
        int row = item.row();
        int col = item.column();
        COMMENT("Editing value.", 0);
        CreateOneElement(row, col);
    }
    edited = true;
    COMMENT("Running simulation", 0);
    run();
}

void BewavedDolphin::on_actionInvert_triggered()
{
    COMMENT("Pressed Not!", 0);
    auto itemList = signalTableView->selectionModel()->selectedIndexes();
    for (auto item : qAsConst(itemList)) {
        int row = item.row();
        int col = item.column();
        int value = model->index(row, col, QModelIndex()).data().toInt();
        value = (value + 1) % 2;
        COMMENT("Editing value.", 0);
        CreateElement(row, col, value);
    }
    edited = true;
    COMMENT("Running simulation", 0);
    run();
}

int BewavedDolphin::sectionFirstColumn(const QItemSelection &ranges)
{
    int first_col = model->columnCount() - 1;
    for (const auto &range : ranges) {
        if (range.left() < first_col)
            first_col = range.left();
    }
    return (first_col);
}

int BewavedDolphin::sectionFirstRow(const QItemSelection &ranges)
{
    int first_row = model->rowCount() - 1;
    for (const auto &range : ranges) {
        if (range.top() < first_row)
            first_row = range.top();
    }
    return (first_row);
}

void BewavedDolphin::on_actionSet_clock_wave_triggered()
{
    COMMENT("Getting first column.", 0);
    QItemSelection ranges = signalTableView->selectionModel()->selection();
    int first_col = sectionFirstColumn(ranges);
    COMMENT("Setting the signal according it its column and clock period.", 0);
    clockDialog dialog(this);
    int clock_period = dialog.getFrequency();
    if (clock_period < 0)
        return;
    int half_clock_period = clock_period / 2;
    auto itemList = signalTableView->selectionModel()->selectedIndexes();
    for (auto item : qAsConst(itemList)) {
        int row = item.row();
        int col = item.column();
        int value = ((col - first_col) % clock_period < half_clock_period ? 0 : 1);
        COMMENT("Editing value.", 0)
        CreateElement(row, col, value);
    }
    edited = true;
    COMMENT("Running simulation", 0);
    run();
}

void BewavedDolphin::on_actionCombinational_triggered()
{
    COMMENT("Setting the signal according it its column and clock period.", 0);
    int clock_period = 2;
    int half_clock_period = 1;
    for (int row = 0; row < inputs.size(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            int value = (col % clock_period < half_clock_period ? 0 : 1);
            CreateElement(row, col, value);
        }
        clock_period *= 2;
        half_clock_period *= 2;
    }
    edited = true;
    COMMENT("Running simulation", 0);
    run();
}

void BewavedDolphin::on_actionSet_Length_triggered()
{
    COMMENT("Setting the simulation length.", 0);
    lengthDialog dialog(this);
    int sim_length = dialog.getFrequency();
    if (sim_length < 0) {
        return;
    }
    setLength(sim_length);
}

void BewavedDolphin::setLength(int sim_length, bool run_simulation)
{
    if (sim_length <= model->columnCount()) {
        COMMENT("Reducing or keeping the simulation length.", 0);
        model->setColumnCount(sim_length);
        int width = signalTableView->horizontalHeader()->length() + signalTableView->columnWidth(0);
        int hight = signalTableView->verticalHeader()->length() + signalTableView->rowHeight(0);
        signalTableView->resize(width, hight);
        QRectF rect = scene->itemsBoundingRect();
        scene->setSceneRect(rect);
        edited = true;
        return;
    }
    COMMENT("Increasing the simulation length.", 0);
    int old_length = model->columnCount();
    model->setColumnCount(sim_length);
    for (int row = 0; row < inputs.size(); ++row) {
        for (int col = old_length; col < model->columnCount(); ++col) {
            CreateZeroElement(row, col, true, false);
        }
    }
    int width = signalTableView->horizontalHeader()->length() + signalTableView->columnWidth(0);
    int hight = signalTableView->verticalHeader()->length() + signalTableView->rowHeight(0);
    signalTableView->resize(width, hight);
    QRectF rect = scene->itemsBoundingRect();
    scene->setSceneRect(rect);
    edited = true;
    COMMENT("Running simulation", 0);
    if (run_simulation) {
        run();
    }
}

void BewavedDolphin::on_actionZoom_out_triggered()
{
    // gv->gvzoom( )->zoomOut( );
    scale *= SCALE_FACTOR;
    gv->scale(SCALE_FACTOR, SCALE_FACTOR);
}

void BewavedDolphin::on_actionZoom_In_triggered()
{
    // gv->gvzoom( )->zoomIn( );
    scale /= SCALE_FACTOR;
    gv->scale(1.0 / SCALE_FACTOR, 1.0 / SCALE_FACTOR);
}

void BewavedDolphin::on_actionReset_Zoom_triggered()
{
    // gv->gvzoom( )->resetZoom( );
    gv->scale(1.0 / scale, 1.0 / scale);
    scale = 1.0;
}

void BewavedDolphin::on_actionZoom_Range_triggered()
{
    gv->scale(1.0 / scale, 1.0 / scale);
    double w_scale = static_cast<double>(gv->width()) / (signalTableView->horizontalHeader()->length() + signalTableView->columnWidth(0));
    double h_scale = static_cast<double>(gv->height()) / (signalTableView->verticalHeader()->length() + signalTableView->rowHeight(0));
    scale = std::min(w_scale, h_scale);
    gv->scale(1.0 * scale, 1.0 * scale);
}

void BewavedDolphin::on_actionClear_triggered()
{
    for (int row = 0; row < inputs.size(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            CreateZeroElement(row, col);
        }
    }
    COMMENT("Running simulation", 0);
    edited = true;
    run();
}

void BewavedDolphin::on_actionCopy_triggered()
{
    QItemSelection ranges = signalTableView->selectionModel()->selection();
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
    QItemSelection ranges = signalTableView->selectionModel()->selection();
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
    edited = true;
}

void BewavedDolphin::on_actionPaste_triggered()
{
    QItemSelection ranges = signalTableView->selectionModel()->selection();
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
    edited = true;
}

void BewavedDolphin::cut(const QItemSelection &ranges, QDataStream &ds)
{
    copy(ranges, ds);
    on_actionSet_to_0_triggered();
}

void BewavedDolphin::copy(const QItemSelection &ranges, QDataStream &ds)
{
    COMMENT("Serializing data into data stream.", 0);
    int first_row = sectionFirstRow(ranges);
    int first_col = sectionFirstColumn(ranges);
    auto itemList = signalTableView->selectionModel()->selectedIndexes();
    ds << static_cast<qint64>(itemList.size());
    for (auto item : qAsConst(itemList)) {
        int row = item.row();
        int col = item.column();
        int data = model->item(row, col)->text().toInt();
        ds << static_cast<qint64>(row - first_row);
        ds << static_cast<qint64>(col - first_col);
        ds << static_cast<qint64>(data);
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
        quint64 data;
        ds >> row;
        ds >> col;
        ds >> data;
        int new_row = static_cast<int>(first_row + row);
        int new_col = static_cast<int>(first_col + col);
        if ((new_row < inputs.size()) && (new_col < model->columnCount())) {
            CreateElement(new_row, new_col, data);
        }
    }
    run();
}

void BewavedDolphin::on_actionSave_as_triggered()
{
    QString fname = currentFile.absoluteFilePath();
    QString path = mainWindow->getCurrentFile().dir().absolutePath();
    if (!currentFile.fileName().isEmpty()) {
        path = currentFile.absoluteFilePath();
    }
    auto *selected_filter = new QString(100);
    fname = QFileDialog::getSaveFileName(this, tr("Save File as ..."), path, tr("Dolphin files (*.dolphin);;CSV files (*.csv)"), selected_filter);
    if (fname.isEmpty()) {
        return;
    }
    if ((!fname.endsWith(".dolphin")) && (!fname.endsWith(".csv"))) {
        if (selected_filter->contains("dolphin"))
            fname.append(".dolphin");
        else
            fname.append(".csv");
    }
    if (save(fname)) {
        currentFile = fname;
        if (mainWindow->getDolphinFilename() != fname) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this,
                                          tr("Wired Panda - Bewaved Dolphin"),
                                          tr("Do you want to link this Bewaved Dolphin file to your current Wired Panda file and save it?"),
                                          QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                mainWindow->setDolphinFilename(fname);
                mainWindow->save();
            }
        }
        setWindowTitle("Bewaved Dolphin Simulator [" + currentFile.fileName() + "]");
        ui->statusbar->showMessage(tr("Saved file sucessfully."), 2000);
        edited = false;
    } else {
        ui->statusbar->showMessage(tr("Could not save file: ") + fname + ".", 2000);
    }
    delete selected_filter;
}

void BewavedDolphin::on_actionSave_triggered()
{
    if (currentFile.fileName().isEmpty()) {
        on_actionSave_as_triggered();
        return;
    }
    QString fname = currentFile.absoluteFilePath();
    if (save(fname)) {
        ui->statusbar->showMessage(tr("Saved file sucessfully."), 2000);
        edited = false;
    } else {
        ui->statusbar->showMessage(tr("Could not save file: ") + fname + ".", 2000);
    }
}

void BewavedDolphin::on_actionLoad_triggered()
{
    QDir defaultDirectory;
    if (currentFile.exists()) {
        defaultDirectory = currentFile.dir();
    } else {
        if (mainWindow->getCurrentFile().exists()) {
            mainWindow->getCurrentFile().dir();
        } else {
            defaultDirectory = QDir::home();
        }
    }
    QString fname = QFileDialog::getOpenFileName(this,
                                                 tr("Open File"),
                                                 defaultDirectory.absolutePath(),
                                                 tr("All supported files (*.dolphin *.csv);;Dolphin files (*.dolphin);;CSV files (*.csv)"));
    if (fname.isEmpty()) {
        return;
    }
    load(fname);
    edited = false;
    ui->statusbar->showMessage(tr("File loaded successfully."), 2000);
}

bool BewavedDolphin::save(QString &fname)
{
    QSaveFile fl(fname);
    if (fl.open(QFile::WriteOnly)) {
        if (fname.endsWith(".dolphin")) {
            COMMENT("Saving dolphin file.", 0);
            QDataStream ds(&fl);
            try {
                save(ds);
            } catch (std::runtime_error &e) {
                ui->statusbar->showMessage(tr("Could not save file: ") + e.what(), 2000);
                return (false);
            }
        } else {
            COMMENT("Saving CSV file.", 0);
            try {
                save(fl);
            } catch (std::runtime_error &e) {
                ui->statusbar->showMessage(tr("Could not save file: ") + e.what(), 2000);
                return (false);
            }
        }
    }
    return (fl.commit());
}

void BewavedDolphin::save(QDataStream &ds)
{
    ds << QString("Bewaved Dolphin 1.0");
    COMMENT("Serializing data into data stream.", 0);
    ds << static_cast<qint64>(inputs.size());
    ds << static_cast<qint64>(model->columnCount());
    for (int itr = 0; itr < model->columnCount(); ++itr) {
        for (int in = 0; in < inputs.size(); ++in) {
            int val = model->item(in, itr)->text().toInt();
            ds << static_cast<qint64>(val);
        }
    }
}

void BewavedDolphin::save(QSaveFile &fl)
{
    fl.write(std::to_string(model->rowCount()).c_str());
    fl.write(",");
    fl.write(std::to_string(model->columnCount()).c_str());
    fl.write(",\n");
    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            QString val = model->item(row, col)->text();
            fl.write(val.toStdString().c_str());
            fl.write(",");
        }
        fl.write("\n");
    }
}

bool BewavedDolphin::load(QString fname)
{
    QFile fl(fname);
    if (!fl.exists()) {
        QMessageBox::warning(this, tr("Error!"), tr("File \"%1\" does not exists!").arg(fname), QMessageBox::Ok, QMessageBox::NoButton);
        return (false);
    }
    COMMENT("File exists.", 0);
    if (fl.open(QFile::ReadOnly)) {
        if (fname.endsWith(".dolphin")) {
            COMMENT("Dolphin file opened.", 0);
            QDataStream ds(&fl);
            COMMENT("Current file set.", 0);
            try {
                COMMENT("Loading in editor.", 0);
                load(ds);
                COMMENT("Finished updating chanched by signal.", 0);
                currentFile = QFileInfo(fname);
            } catch (std::runtime_error &e) {
                std::cerr << tr("Error loading project: ").toStdString() << e.what() << std::endl;
                QMessageBox::warning(this, tr("Error!"), tr("Could not open file.\nError: %1").arg(e.what()), QMessageBox::Ok, QMessageBox::NoButton);
                return (false);
            }
        } else if (fname.endsWith(".csv")) {
            COMMENT("CSV file opened.", 0);
            try {
                COMMENT("Loading in editor.", 0);
                load(fl);
                COMMENT("Finished updating chanched by signal.", 0);
                currentFile = QFileInfo(fname);
            } catch (std::runtime_error &e) {
                std::cerr << tr("Error loading project: ").toStdString() << e.what() << std::endl;
                QMessageBox::warning(this, tr("Error!"), tr("Could not open file.\nError: %1").arg(e.what()), QMessageBox::Ok, QMessageBox::NoButton);
                return (false);
            }
        } else {
            std::cerr << tr("Format not supported. Could not open file : ").toStdString() << fname.toStdString() << "." << std::endl;
            return (false);
        }
    } else {
        std::cerr << tr("Could not open file in ReadOnly mode : ").toStdString() << fname.toStdString() << "." << std::endl;
        QMessageBox::warning(this, tr("Error!"), tr("Could not open file in ReadOnly mode : ") + fname + ".", QMessageBox::Ok, QMessageBox::NoButton);
        return (false);
    }
    COMMENT("Closing file.", 0);
    fl.close();
    setWindowTitle("Bewaved Dolphin Simulator [" + currentFile.fileName() + "]");
    return (true);
}

void BewavedDolphin::load(QDataStream &ds)
{
    QString str;
    ds >> str;
    if (!str.startsWith("Bewaved Dolphin")) {
        throw(std::runtime_error(ERRORMSG("Invalid file format. Starts with: " + str.toStdString())));
    }
    qint64 rows;
    qint64 cols;
    ds >> rows;
    ds >> cols;
    if (rows > inputs.size()) {
        rows = inputs.size();
    }
    if ((cols < 2) || (cols > 2048)) {
        throw(std::runtime_error(ERRORMSG("Invalid number of columns.")));
    }
    setLength(cols, false);
    COMMENT("Update table.", 0);
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
    if (rows > inputs.size()) {
        rows = inputs.size();
    }
    if ((cols < 2) || (cols > 2048)) {
        throw(std::runtime_error(ERRORMSG("Invalid number of columns.")));
    }
    setLength(cols, false);
    COMMENT("Update table.", 0);
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
    type = PlotType::number;
    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            model->setData(model->index(row, col), "", Qt::DecorationRole);
        }
    }
    for (int row = 0; row < inputs.size(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            CreateElement(row, col, model->item(row, col)->text().toInt());
        }
    }
    COMMENT("Running simulation", 0);
    run();
}

void BewavedDolphin::on_actionShowCurve_triggered()
{
    type = PlotType::line;
    for (int row = 0; row < inputs.size(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            CreateElement(row, col, model->item(row, col)->text().toInt());
        }
    }
    COMMENT("Running simulation", 0);
    run();
}

void BewavedDolphin::on_actionExport_to_PNG_triggered()
{
    QString pngFile = QFileDialog::getSaveFileName(this, tr("Export to Image"), currentFile.absolutePath(), tr("PNG files (*.png)"));
    if (pngFile.isEmpty()) {
        return;
    }
    if (!pngFile.endsWith(".png", Qt::CaseInsensitive)) {
        pngFile.append(".png");
    }
    QRectF s = scene->sceneRect();
    QPixmap p(s.size().toSize());
    QPainter painter;
    painter.begin(&p);
    painter.setRenderHint(QPainter::Antialiasing);
    scene->render(&painter, QRectF(), s);
    painter.end();
    QImage img = p.toImage();
    img.save(pngFile);
}

void BewavedDolphin::on_actionExport_to_PDF_triggered()
{
    QString pdfFile = QFileDialog::getSaveFileName(this, tr("Export to PDF"), currentFile.absolutePath(), tr("PDF files (*.pdf)"));
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
        QMessageBox::warning(this, tr("ERROR"), tr("Could not print this circuit to PDF."), QMessageBox::Ok);
        return;
    }
    scene->render(&p, QRectF(), scene->sceneRect().adjusted(-64, -64, 64, 64));
    p.end();
}

void BewavedDolphin::on_actionAbout_triggered()
{
    QMessageBox::about(this,
                       "beWaved Dolphin",
                       tr("<p>beWaved Dolphin is a waveform simulator for the weRed Panda software developed by the Federal University of São Paulo."
                          " This project was created in order to help students to learn about logic circuits.</p>"
                          "<p>Software version: %1</p>"
                          "<p><strong>Creators:</strong></p>"
                          "<ul>"
                          "<li> Prof. Fábio Cappabianco, Ph.D. </li>"
                          "</ul>"
                          "<p> beWaved Dolphin is currently maintained by Prof. Fábio Cappabianco, Ph.D. and Vinícius R. Miguel.</p>"
                          "<p> Please file a report at our GitHub page if bugs are found or if you wish for a new functionality to be implemented.</p>"
                          "<p><a href=\"http://gibis-unifesp.github.io/wiRedPanda/\">Visit our website!</a></p>")
                           .arg(QApplication::applicationVersion()));
}

void BewavedDolphin::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}
