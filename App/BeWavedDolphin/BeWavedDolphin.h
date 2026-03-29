// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QFileInfo>
#include <QMainWindow>

#include "App/BeWavedDolphin/BeWavedDolphinUI.h"
#include "App/BeWavedDolphin/WaveformGrid.h"
#include "App/BeWavedDolphin/WaveformView.h"
#include "App/Scene/Scene.h"

class MainWindow;
class QSaveFile;

class BewavedDolphin : public QMainWindow
{
    Q_OBJECT

public:
    explicit BewavedDolphin(Scene *scene, bool askConnection = true, MainWindow *parent = nullptr);
    ~BewavedDolphin() override;

    void createWaveform();
    void createWaveform(const QString &fileName);
    void print();
    void saveToTxt(QTextStream &stream);
    void show();

    // --- MCP Interface ---

    bool exportToPng(const QString &filename);
    void createElement(int row, int col, int value, bool isInput, bool changeNext);
    void prepare(const QString &fileName = {});
    void run();
    void setLength(int simLength, bool runSimulation = false);

    WaveformGrid *getGrid() const { return m_grid; }
    const QVector<GraphicElement *> &getOutputElements() const { return m_outputs; }
    const QVector<GraphicElementInput *> &getInputElements() const { return m_inputs; }
    int getLength() const { return m_length; }

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    Q_DISABLE_COPY(BewavedDolphin)

    // --- File I/O ---

    bool checkSave();
    bool exportWaveformToPng(const QString &filename);
    void associateToWiRedPanda(const QString &fileName);
    void load(QDataStream &stream);
    void load(QFile &file);
    void load(const QString &fileName);
    void save(QDataStream &stream);
    void save(QSaveFile &file);
    void save(const QString &fileName);

    // --- Signal Table Management ---

    void copy(QDataStream &stream);
    void cut(QDataStream &stream);
    void loadElements();
    void loadFromTerminal();
    void loadNewTable();
    void loadSignals(QStringList &inputLabels, QStringList &outputLabels);
    void paste(QDataStream &stream);
    void resizeScene();
    void restoreInputs();
    void setCellValue(int row, int col, int value);
    void zoomChanged();

    // --- Action Handlers ---

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
    void onSelectionChanged();
    void onCellEdited(int row, int col, int newValue);

    // --- Members ---

    std::unique_ptr<BewavedDolphinUi> m_ui;
    WaveformView m_view;
    MainWindow *m_mainWindow          = nullptr;
    QFileInfo m_currentFile;
    QGraphicsScene *m_scene           = new QGraphicsScene(this);
    WaveformGrid *m_grid              = nullptr;
    QVector<GraphicElement *> m_outputs;
    QVector<GraphicElementInput *> m_inputs;
    QVector<Status> m_oldInputValues;
    Scene *m_externalScene            = nullptr;
    Simulation *m_simulation          = nullptr;
    bool m_edited                     = false;
    const bool m_askConnection;
    int m_clockPeriod                 = 0;
    int m_inputPorts                  = 0;
    int m_length                      = 32;
};
