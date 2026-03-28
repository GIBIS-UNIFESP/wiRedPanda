// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief BewavedDolphin waveform editor: digital signal creation, display, and export.
 */

#pragma once

#include <memory>

#include <QFileInfo>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QTableView>

#include "App/BeWavedDolphin/BeWavedDolphinUI.h"
#include "App/BeWavedDolphin/SignalDelegate.h"
#include "App/BeWavedDolphin/SignalModel.h"
#include "App/BeWavedDolphin/WaveformView.h"
#include "App/Scene/Scene.h"

class MainWindow;
class QItemSelection;
class QSaveFile;

/**
 * \class BewavedDolphin
 * \brief Waveform editor main window for creating and analyzing digital signal sequences.
 *
 * \details BewavedDolphin displays a table where rows represent circuit I/O signals and
 * columns represent simulation time steps.  Users can set input values and then run the
 * associated wiRedPanda circuit to observe output responses.  The waveform can be saved
 * to a .dolphin file, printed, or exported to PDF/PNG.
 *
 * A subset of methods is also accessible via the MCP server for automated testing.
 */
class BewavedDolphin : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * \brief Constructs the waveform editor.
     * \param scene         Circuit scene whose I/O elements will be mapped to signals.
     * \param askConnection If \c true, prompts the user to link to a .dolphin file.
     * \param parent        Owner MainWindow (used to restore simulation state after run).
     */
    explicit BewavedDolphin(Scene *scene, const bool askConnection = true, MainWindow *parent = nullptr);
    ~BewavedDolphin() override;

    /// Initializes a blank waveform from the current scene's I/O elements.
    void createWaveform();

    /**
     * \brief Initializes the waveform from \a fileName if it matches the current circuit.
     * \param fileName Path to an existing .dolphin file.
     */
    void createWaveform(const QString &fileName);

    /// Prints the waveform table to the system printer.
    void print();

    /**
     * \brief Exports the waveform data as plain text to \a stream.
     * \param stream Destination text stream.
     */
    void saveToTxt(QTextStream &stream);

    /// Shows the window and loads the initial waveform.
    void show();

    // --- MCP Interface ---

    /**
     * \brief Exports the waveform scene to a PNG image file.
     * \param filename Output file path.
     * \return \c true on success.
     */
    bool exportToPng(const QString &filename);

    /**
     * \brief Sets a single cell value in the waveform table.
     * \param row        Signal row index.
     * \param col        Time-step column index.
     * \param value      Logical value (0 or 1).
     * \param isInput    \c true if the row is an input signal.
     * \param changeNext \c true to also update the subsequent cell's edge pixmap.
     */
    void createElement(const int row, const int col, const int value, const bool isInput, const bool changeNext);

    /**
     * \brief Prepares the waveform from \a fileName (or blank if empty).
     * \param fileName Optional .dolphin file to load.
     */
    void prepare(const QString &fileName = {});

    /// Runs the simulation for all input combinations and fills output rows.
    void run();

    /**
     * \brief Sets the number of time-step columns.
     * \param simLength     Number of columns.
     * \param runSimulation If \c true, immediately re-runs the simulation.
     */
    void setLength(const int simLength, const bool runSimulation = false);

    // --- MCP Accessors ---

    /// Returns the underlying table model (MCP access).
    QStandardItemModel* getModel() const { return m_model; }

    /// Returns the output element vector (MCP access).
    const QVector<GraphicElement *>& getOutputElements() const { return m_outputs; }

    /// Returns the input element vector (MCP access).
    const QVector<GraphicElementInput *>& getInputElements() const { return m_inputs; }

    /// Returns the current simulation length (number of columns).
    int getLength() const { return m_length; }

protected:
    // --- Qt event overrides ---

    /// \reimp
    void closeEvent(QCloseEvent *event) override;
    /// \reimp
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

    int sectionFirstColumn(const QItemSelection &ranges);
    int sectionFirstRow(const QItemSelection &ranges);
    void copy(const QItemSelection &ranges, QDataStream &stream);
    void cut(const QItemSelection &ranges, QDataStream &stream);
    void loadElements();
    void loadFromTerminal();
    void loadNewTable();
    void loadSignals(QStringList &inputLabels, QStringList &outputLabels);
    void paste(const QItemSelection &ranges, QDataStream &stream);
    void resizeScene();
    void restoreInputs();
    void setCellValue(int row, int col, int value, bool isInput, bool changeNext);
    void setTableViewSize(int width, int height);
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
    void on_tableView_cellDoubleClicked();
    void on_tableView_selectionChanged();

    // --- Members ---

    std::unique_ptr<BewavedDolphinUi> m_ui;
    WaveformView m_view;
    MainWindow *m_mainWindow       = nullptr;
    PlotType m_type                = PlotType::Line;
    QFileInfo m_currentFile;
    QGraphicsScene *m_scene        = new QGraphicsScene(this);
    QStandardItemModel *m_model    = nullptr;
    QTableView *m_signalTableView  = new QTableView();
    SignalDelegate *m_delegate     = nullptr;
    QVector<GraphicElement *> m_outputs;
    QVector<GraphicElementInput *> m_inputs;
    QVector<Status> m_oldInputValues;
    Scene *m_externalScene         = nullptr;
    Simulation *m_simulation       = nullptr;
    bool m_edited                  = false;
    const bool m_askConnection;
    const double m_scaleFactor     = 0.8;
    double m_scale                 = 1.25;
    int m_clockPeriod              = 0;
    int m_inputPorts               = 0;
    int m_length                   = 32;
};

