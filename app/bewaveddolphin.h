/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef BEWAVEDDOLPHIN_H
#define BEWAVEDDOLPHIN_H

#include "editor.h"
#include "graphicsview.h"

#include <QFileInfo>
#include <QItemDelegate>
#include <QMainWindow>
#include <QPainter>
#include <QSaveFile>
#include <QStandardItemModel>
#include <QTableView>

namespace Ui
{
class BewavedDolphin;
}

enum class PlotType { number, line };

class SignalModel : public QStandardItemModel
{
public:
    SignalModel(int rows, int inputs, int columns, QObject *parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int inputs;
};

class SignalDelegate : public QItemDelegate
{
public:
    SignalDelegate(int margin, QObject *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    int m_margin;
};

class BewavedDolphin : public QMainWindow
{
    Q_OBJECT

public:
    explicit BewavedDolphin(Editor *editor, QWidget *parent = nullptr);
    ~BewavedDolphin() override;
    bool createWaveform(QString filename);
    void show();
    void print();

private slots:
    void on_actionExit_triggered();

    void on_actionSet_to_0_triggered();

    void on_actionSet_to_1_triggered();

    void on_actionInvert_triggered();

    void on_actionSet_clock_wave_triggered();

    void on_actionCombinational_triggered();

    void on_actionSet_Length_triggered();

    void on_actionZoom_out_triggered();

    void on_actionZoom_In_triggered();

    void on_actionReset_Zoom_triggered();

    void on_actionZoom_Range_triggered();

    void on_actionClear_triggered();

    void on_actionCopy_triggered();

    void on_actionPaste_triggered();

    void on_actionCut_triggered();

    void on_actionSave_as_triggered();

    void on_actionSave_triggered();

    void on_actionLoad_triggered();

    void on_actionShowValues_triggered();

    void on_actionShowCurve_triggered();

    void on_actionExport_to_PNG_triggered();

    void on_actionExport_to_PDF_triggered();

    void on_actionAbout_triggered();

    void on_actionAbout_Qt_triggered();

private:
    Ui::BewavedDolphin *ui;
    Editor *editor;
    MainWindow *mainWindow;
    SimulationController *sc;
    GraphicsView *gv;

    QFileInfo currentFile;

    QVector<GraphicElement *> inputs;
    QVector<GraphicElement *> outputs;
    QGraphicsScene *scene;
    QTableView *signalTableView;
    QStandardItemModel *model;
    PlotType type;
    bool edited;

    double scale;
    const double SCALE_FACTOR = 0.8;

    QPixmap lowGreen;
    QPixmap highGreen;
    QPixmap fallingGreen;
    QPixmap risingGreen;
    QPixmap lowBlue;
    QPixmap highBlue;
    QPixmap fallingBlue;
    QPixmap risingBlue;

    bool loadElements();
    void loadNewTable(QStringList &input_labels, QStringList &output_labels);
    QVector<char> loadSignals(QStringList &input_labels, QStringList &output_labels);
    void run();
    void setLength(int sim_length, bool run_simulation = true);
    void cut(const QItemSelection &ranges, QDataStream &ds);
    void copy(const QItemSelection &ranges, QDataStream &ds);
    void paste(QItemSelection &ranges, QDataStream &ds);
    int sectionFirstColumn(const QItemSelection &ranges);
    int sectionFirstRow(const QItemSelection &ranges);
    bool save(QString &fname);
    void save(QDataStream &ds);
    void save(QSaveFile &fl);
    bool load(QString fname);
    void load(QDataStream &ds);
    void load(QFile &fl);
    void drawPixMaps();
    void CreateElement(int row, int col, int value, bool isInput = true, bool changePrevious = true);
    void CreateZeroElement(int row, int col, bool isInput = true, bool changePrevious = true);
    void CreateOneElement(int row, int col, bool isInput = true, bool changePrevious = true);
    void zoomChanged();
    bool checkSave();

protected:
    void closeEvent(QCloseEvent *e) override;
};

#endif // BEWAVEDDOLPHIN_H
