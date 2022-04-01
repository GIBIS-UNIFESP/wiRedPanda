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
    void print();
    void show();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *e) override;

private:
    QVector<char> loadSignals(QStringList &input_labels, QStringList &output_labels);
    bool checkSave();
    bool load(const QString &fname);
    bool loadElements();
    bool save(const QString &fname);
    int sectionFirstColumn(const QItemSelection &ranges);
    int sectionFirstRow(const QItemSelection &ranges);
    void CreateElement(int row, int col, int value, bool isInput = true, bool changePrevious = true);
    void CreateOneElement(int row, int col, bool isInput = true, bool changePrevious = true);
    void CreateZeroElement(int row, int col, bool isInput = true, bool changePrevious = true);
    void associateToWiredPanda(const QString &fname);
    void copy(const QItemSelection &ranges, QDataStream &ds);
    void cut(const QItemSelection &ranges, QDataStream &ds);
    void drawPixMaps();
    void load(QDataStream &ds);
    void load(QFile &fl);
    void loadNewTable(const QStringList &input_labels, const QStringList &output_labels);
    void on_actionAbout_Qt_triggered();
    void on_actionAbout_triggered();
    void on_actionClear_triggered();
    void on_actionCombinational_triggered();
    void on_actionCopy_triggered();
    void on_actionCut_triggered();
    void on_actionExit_triggered();
    void on_actionExport_to_PDF_triggered();
    void on_actionExport_to_PNG_triggered();
    void on_actionInvert_triggered();
    void on_actionLoad_triggered();
    void on_actionPaste_triggered();
    void on_actionReset_Zoom_triggered();
    void on_actionSave_as_triggered();
    void on_actionSave_triggered();
    void on_actionSet_Length_triggered();
    void on_actionSet_clock_wave_triggered();
    void on_actionSet_to_0_triggered();
    void on_actionSet_to_1_triggered();
    void on_actionShowCurve_triggered();
    void on_actionShowValues_triggered();
    void on_actionZoom_In_triggered();
    void on_actionZoom_Range_triggered();
    void on_actionZoom_out_triggered();
    void paste(QItemSelection &ranges, QDataStream &ds);
    void resizeScene();
    void restoreInputs();
    void run();
    void save(QDataStream &ds);
    void save(QSaveFile &fl);
    void setLength(int sim_length, bool run_simulation = true);
    void zoomChanged();

    Ui::BewavedDolphin *m_ui;
    Editor *m_editor;
    GraphicsView *m_gv;
    MainWindow *m_mainWindow;
    PlotType m_type;
    QFileInfo m_currentFile;
    QGraphicsScene *m_scene;
    QPixmap m_fallingBlue;
    QPixmap m_fallingGreen;
    QPixmap m_highBlue;
    QPixmap m_highGreen;
    QPixmap m_lowBlue;
    QPixmap m_lowGreen;
    QPixmap m_risingBlue;
    QPixmap m_risingGreen;
    QStandardItemModel *m_model;
    QTableView *m_signalTableView;
    QVector<GraphicElement *> m_inputs;
    QVector<GraphicElement *> m_outputs;
    QVector<char> m_oldInputValues;
    SimulationController *m_sc;
    bool m_edited;
    const double m_SCALE_FACTOR = 0.8;
    double m_scale;
    int m_input_ports;
};
