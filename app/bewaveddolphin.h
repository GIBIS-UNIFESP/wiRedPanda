/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicsview.h"
#include "scene.h"

#include <QFileInfo>
#include <QItemDelegate>
#include <QMainWindow>
#include <QStandardItemModel>

class GraphicsView;
class QItemSelection;
class QSaveFile;
class QTableView;

namespace Ui
{
class BewavedDolphin;
}

enum class PlotType { Number, Line };

class SignalModel : public QStandardItemModel
{
    Q_OBJECT

public:
    SignalModel(const int rows, const int inputs, const int columns, QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    int m_inputs;
};

class SignalDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    SignalDelegate(const int margin, QObject *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    int m_margin;
};

class BewavedDolphin : public QMainWindow
{
    Q_OBJECT

public:
    explicit BewavedDolphin(Scene *scene, const bool askConnection = true, MainWindow *parent = nullptr);
    ~BewavedDolphin() override;

    void createWaveform(const QString &fileName);
    void print();
    void show();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    QVector<char> loadSignals(QStringList &inputLabels, QStringList &outputLabels);
    bool checkSave();
    bool load(const QString &fileName);
    bool loadElements();
    bool save(const QString &fileName);
    int sectionFirstColumn(const QItemSelection &ranges);
    int sectionFirstRow(const QItemSelection &ranges);
    void associateToWiredPanda(const QString &fileName);
    void copy(const QItemSelection &ranges, QDataStream &stream);
    void createElement(const int row, const int col, const int value, const bool isInput = true, const bool changePrevious = true);
    void createOneElement(const int row, const int col, const bool isInput = true, const bool changePrevious = true);
    void createZeroElement(const int row, const int col, const bool isInput = true, const bool changePrevious = true);
    void cut(const QItemSelection &ranges, QDataStream &stream);
    void drawPixMaps();
    void load(QDataStream &stream);
    void load(QFile &file);
    void loadNewTable(const QStringList &inputLabels, const QStringList &outputLabels);
    void on_actionAboutQt_triggered();
    void on_actionAbout_triggered();
    void on_actionClear_triggered();
    void on_actionCombinational_triggered();
    void on_actionCopy_triggered();
    void on_actionCut_triggered();
    void on_actionExit_triggered();
    void on_actionExportToPdf_triggered();
    void on_actionExportToPng_triggered();
    void on_actionInvert_triggered();
    void on_actionLoad_triggered();
    void on_actionPaste_triggered();
    void on_actionResetZoom_triggered();
    void on_actionSaveAs_triggered();
    void on_actionSave_triggered();
    void on_actionSetClockWave_triggered();
    void on_actionSetLength_triggered();
    void on_actionSetTo0_triggered();
    void on_actionSetTo1_triggered();
    void on_actionShowCurve_triggered();
    void on_actionShowValues_triggered();
    void on_actionZoomIn_triggered();
    void on_actionZoomOut_triggered();
    void on_actionZoomRange_triggered();
    void paste(QItemSelection &ranges, QDataStream &stream);
    void resizeScene();
    void restoreInputs();
    void run();
    void save(QDataStream &stream);
    void save(QSaveFile &file);
    void setLength(const int simLength, const bool runSimulation = true);
    void zoomChanged();

    Ui::BewavedDolphin *m_ui;
    GraphicsView m_view;
    MainWindow *m_mainWindow = nullptr;
    PlotType m_type = PlotType::Line;
    QFileInfo m_currentFile;
    QGraphicsScene *m_scene = nullptr;
    QPixmap m_fallingBlue;
    QPixmap m_fallingGreen;
    QPixmap m_highBlue;
    QPixmap m_highGreen;
    QPixmap m_lowBlue;
    QPixmap m_lowGreen;
    QPixmap m_risingBlue;
    QPixmap m_risingGreen;
    QStandardItemModel *m_model = nullptr;
    QTableView *m_signalTableView = nullptr;
    QVector<GraphicElement *> m_inputs;
    QVector<GraphicElement *> m_outputs;
    QVector<char> m_oldInputValues;
    Scene *m_externalScene = nullptr;
    SimulationController *m_simController = nullptr;
    bool m_edited = false;
    const bool m_askConnection;
    const double m_SCALE_FACTOR = 0.8;
    double m_scale = 1.0;
    int m_inputPorts = 0;
};
