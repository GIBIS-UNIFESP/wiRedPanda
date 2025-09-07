// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "bewaveddolphin_ui.h"
#include "graphicsview.h"
#include "scene.h"

#include <QFileInfo>
#include <QItemDelegate>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QTableView>
#include <memory>

class GraphicsView;
class MainWindow;
class QItemSelection;
class QSaveFile;


enum class PlotType { Number, Line };

/*!
 * @class SignalModel
 * @brief Data model for digital signals in the waveform editor
 *
 * The SignalModel class extends QStandardItemModel to manage the data representation
 * of digital signals in the waveform editor table. It handles the editable/non-editable
 * state of cells based on whether they represent input or output signals.
 */
class SignalModel : public QStandardItemModel
{
    Q_OBJECT

public:
    SignalModel(const int inputs, const int rows, const int columns, QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    const int m_inputCount;
};

/*!
 * @class SignalDelegate
 * @brief Custom drawing delegate for waveform signal cells
 *
 * This class handles the visual representation of signal cells in the
 * waveform table and provide custom drawing for digital signal states.
 */
class SignalDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    SignalDelegate(QObject *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

/*!
 * @class DolphinGraphicsView
 * @brief Custom GraphicsView for the waveform display
 *
 * Extends GraphicsView to provide special zooming behavior and wheel
 * event handling specific to the waveform editor (like Quartus Prime zooming).
 */
class DolphinGraphicsView : public GraphicsView
{
    Q_OBJECT

public:
    explicit DolphinGraphicsView(QWidget *parent = nullptr);

    bool canZoomIn() const;
    bool canZoomOut() const;
    void resetZoom();
    void zoomIn();
    void zoomOut();

protected:
    void wheelEvent(QWheelEvent *event) override;
};

/*!
 * @class BewavedDolphin
 * @brief Main waveform editor window
 *
 * beWavedDolphin is the main class for the waveform editor component.
 * It provides functionality to create, edit, visualize, and export digital
 * waveforms, integrating with wiRedPanda circuit simulations.
 */
class BewavedDolphin : public QMainWindow
{
    Q_OBJECT

public:
    explicit BewavedDolphin(Scene *scene, const bool askConnection = true, MainWindow *parent = nullptr);
    ~BewavedDolphin() override;

    void createWaveform();
    void createWaveform(const QString &fileName);
    void print();
    void saveToTxt(QTextStream &stream);
    void show();

    // MCP interface methods for programmatic access
    bool exportToPng(const QString &filename);
    void createElement(const int row, const int col, const int value, const bool isInput = true, const bool changeNext = false);
    void prepare(const QString &fileName = {});
    void run();
    void setLength(const int simLength, const bool runSimulation = false);

    // Getters for MCP access to internal data
    QStandardItemModel* getModel() const { return m_model; }
    const QVector<GraphicElement *>& getOutputElements() const { return m_outputs; }
    const QVector<GraphicElementInput *>& getInputElements() const { return m_inputs; }
    int getLength() const { return m_length; }

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    Q_DISABLE_COPY(BewavedDolphin)

    bool checkSave();
    bool exportWaveformToPng(const QString &filename);
    int sectionFirstColumn(const QItemSelection &ranges);
    int sectionFirstRow(const QItemSelection &ranges);
    void associateToWiRedPanda(const QString &fileName);
    void copy(const QItemSelection &ranges, QDataStream &stream);
    void createOneElement(const int row, const int col, const bool isInput = true, const bool changeNext = true);
    void createZeroElement(const int row, const int col, const bool isInput = true, const bool changeNext = true);
    void cut(const QItemSelection &ranges, QDataStream &stream);
    void load(QDataStream &stream);
    void load(QFile &file);
    void load(const QString &fileName);
    void loadElements();
    void loadFromTerminal();
    void loadNewTable();
    void loadPixmaps();
    void loadSignals(QStringList &inputLabels, QStringList &outputLabels);
    void on_actionAboutQt_triggered();
    void on_actionAbout_triggered();
    void on_actionAutoCrop_triggered();
    void on_actionClear_triggered();
    void on_actionCombinational_triggered();
    void on_actionCopy_triggered();
    void on_actionCut_triggered();
    void on_actionExit_triggered();
    void on_actionExportToPdf_triggered();
    void on_actionExportToPng_triggered();
    void on_actionFitScreen_triggered();
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
    void on_actionShowNumbers_triggered();
    void on_actionShowWaveforms_triggered();
    void on_actionZoomIn_triggered();
    void on_actionZoomOut_triggered();
    void on_tableView_cellDoubleClicked();
    void on_tableView_selectionChanged();
    void paste(const QItemSelection &ranges, QDataStream &stream);
    void resizeScene();
    void restoreInputs();
    void save(QDataStream &stream);
    void save(QSaveFile &file);
    void save(const QString &fileName);
    void zoomChanged();

    std::unique_ptr<BewavedDolphin_Ui> m_ui;
    DolphinGraphicsView m_view;
    MainWindow *m_mainWindow = nullptr;
    PlotType m_type = PlotType::Line;
    QFileInfo m_currentFile;
    QGraphicsScene *m_scene = new QGraphicsScene(this);
    QPixmap m_fallingBlue;
    QPixmap m_fallingGreen;
    QPixmap m_highBlue;
    QPixmap m_highGreen;
    QPixmap m_lowBlue;
    QPixmap m_lowGreen;
    QPixmap m_risingBlue;
    QPixmap m_risingGreen;
    QStandardItemModel *m_model = nullptr;
    QTableView *m_signalTableView = new QTableView();
    QVector<GraphicElement *> m_outputs;
    QVector<GraphicElementInput *> m_inputs;
    QVector<Status> m_oldInputValues;
    Scene *m_externalScene = nullptr;
    Simulation *m_simulation = nullptr;
    bool m_edited = false;
    const bool m_askConnection;
    const double m_scaleFactor = 0.8;
    double m_scale = 1.25;
    int m_clockPeriod = 0;
    int m_inputPorts = 0;
    int m_length = 32;
};
