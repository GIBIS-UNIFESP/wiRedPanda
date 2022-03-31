/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QFileInfo>
#include <QItemDelegate>
#include <QMainWindow>
#include <QStandardItemModel>

class QSaveFile;
class QItemSelection;
class Editor;
class GraphicsView;
class MainWindow;
class GraphicElement;
class QGraphicsScene;
class QPainter;
class QTableView;
class SimulationController;

namespace Ui
{
class BewavedDolphin;
}

enum class PlotType { Number, Line };

class SignalModel : public QStandardItemModel
{
    Q_OBJECT

public:
    SignalModel(int rows, int inputs, int columns, QObject *parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    int m_inputs;
};

class SignalDelegate : public QItemDelegate
{
    Q_OBJECT

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
    bool createWaveform(const QString &filename);
    void show();
    void print();

private slots:
<<<<<<< HEAD:app/BewavedDolphin.h
    
    
    /* Menu File */
    void actionLoadTriggered();
    void actionSaveTriggered();
    void actionSaveAsTriggered();
    void actionExportToPdfTriggered();
    void actionExportToPngTriggered();
    void actionExitTriggered();

    /* Menu Edit*/
    void actionCutTriggered();
    void actionCopyTriggered();
    void actionPasteTriggered();
    void actionClearTriggered();
    void actionCombinationalTriggered();
    void actionSetTo_0_Triggered();
    void actionSetTo_1_Triggered();
    void actionInvertTriggered();
    void actionSetClockWaveTriggered();

    /* Menu View */
    void actionZoomOutTriggered();
    void actionZoomInTriggered();
    void actionResetZoomTriggered();
    void actionZoomRangeTriggered();
    void actionSetLengthTriggered();
    void actionShowValuesTriggered();
    void actionShowCurveTriggered();

    
    /* Menu Help */
    void actionAboutTriggered();
    void actionAboutQtTriggered();
=======
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
>>>>>>> master:app/bewaveddolphin.h

private:
    Ui::BewavedDolphin *m_ui;
    Editor *m_editor;
    MainWindow *m_mainWindow;
    SimulationController *m_sc;
    GraphicsView *m_gv;

    QFileInfo m_currentFile;

    QVector<GraphicElement *> m_inputs;
    QVector<GraphicElement *> m_outputs;
    QVector<char> m_oldInputValues;
    int m_input_ports;
    QGraphicsScene *m_scene;
    QTableView *m_signalTableView;
    QStandardItemModel *m_model;
    PlotType m_type;
    bool m_edited;

    double m_scale;
    const double m_SCALE_FACTOR = 0.8;

    QPixmap m_lowGreen;
    QPixmap m_highGreen;
    QPixmap m_fallingGreen;
    QPixmap m_risingGreen;
    QPixmap m_lowBlue;
    QPixmap m_highBlue;
    QPixmap m_fallingBlue;
    QPixmap m_risingBlue;

    bool loadElements();
    void loadNewTable(const QStringList &input_labels, const QStringList &output_labels);
    QVector<char> loadSignals(QStringList &input_labels, QStringList &output_labels);
    void run();
    void setLength(int sim_length, bool run_simulation = true);
    void cut(const QItemSelection &ranges, QDataStream &ds);
    void copy(const QItemSelection &ranges, QDataStream &ds);
    void paste(QItemSelection &ranges, QDataStream &ds);
    int sectionFirstColumn(const QItemSelection &ranges);
    int sectionFirstRow(const QItemSelection &ranges);
    bool save(const QString &fname);
    void save(QDataStream &ds);
    void save(QSaveFile &fl);
    bool load(const QString &fname);
    void load(QDataStream &ds);
    void load(QFile &fl);
    void drawPixMaps();
    void CreateElement(int row, int col, int value, bool isInput = true, bool changePrevious = true);
    void CreateZeroElement(int row, int col, bool isInput = true, bool changePrevious = true);
    void CreateOneElement(int row, int col, bool isInput = true, bool changePrevious = true);
    void zoomChanged();
    bool checkSave();

    void associateToWiredPanda(const QString &fname);
    void restoreInputs();
    void resizeScene();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *e) override;
};
