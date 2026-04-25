// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief BewavedDolphin waveform editor: digital signal creation, display, and export.
 */

#pragma once

#include <functional>
#include <memory>

#include <QFileInfo>
#include <QStandardItemModel>
#include <QTableView>

#ifdef USE_KDE_FRAMEWORKS
#include <KXmlGuiWindow>
using BewavedDolphinBase = KXmlGuiWindow;
#else
#include <QMainWindow>
using BewavedDolphinBase = QMainWindow;
#endif

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
class BewavedDolphin : public BewavedDolphinBase
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

#ifdef USE_KDE_FRAMEWORKS
    void setupKdeActions();
#endif

    // --- File I/O ---

    /// Prompts the user to save unsaved changes; returns \c false if cancelled.
    bool checkSave();
    bool exportWaveformToPng(const QString &filename);
    /// Links the waveform editor to the .dolphin file at \a fileName.
    void associateToWiRedPanda(const QString &fileName);
    /// Loads waveform data from a binary \a stream.
    void load(QDataStream &stream);
    /// Opens \a file and loads waveform data from it.
    void load(QFile &file);
    /// Opens the .dolphin file at \a fileName and loads waveform data.
    void load(const QString &fileName);
    /// Writes the current waveform data to a binary \a stream.
    void save(QDataStream &stream);
    /// Writes the waveform to \a file (atomic save via QSaveFile).
    void save(QSaveFile &file);
    /// Saves the waveform to the .dolphin file at \a fileName.
    void save(const QString &fileName);

    // --- Signal Table Management ---

    /// Returns the first column index in the selected \a ranges.
    int sectionFirstColumn(const QItemSelection &ranges);
    /// Returns the first row index in the selected \a ranges.
    int sectionFirstRow(const QItemSelection &ranges);
    /// Serializes the selected \a ranges to \a stream (clipboard copy).
    void copy(const QItemSelection &ranges, QDataStream &stream);
    /// Serializes and clears the selected \a ranges (clipboard cut).
    void cut(const QItemSelection &ranges, QDataStream &stream);
    /// Collects input/output GraphicElements from the circuit scene.
    void loadElements();
    /// Populates signal rows from the external circuit's terminal elements.
    void loadFromTerminal();
    /// Creates a fresh table model with the current input/output configuration.
    void loadNewTable();
    /// Fills \a inputLabels and \a outputLabels from the circuit's I/O elements.
    void loadSignals(QStringList &inputLabels, QStringList &outputLabels);
    /// Deserializes clipboard data from \a stream into the selected \a ranges.
    void paste(const QItemSelection &ranges, QDataStream &stream);
    /// Resizes the internal QGraphicsScene to fit the current table dimensions.
    void resizeScene();
    /// Restores original input element values after a simulation run.
    void restoreInputs();
    /// Sets a single cell at (\a row, \a col) to \a value with edge-update control.
    void setCellValue(int row, int col, int value, bool isInput, bool changeNext);
    /// Applies \a valueFn to every selected cell, marks the waveform edited, and re-runs.
    void applyToSelectedCells(const std::function<int(int)> &valueFn);
    /// Applies \a widthFn to every column width, then resizes the scene and updates zoom state.
    void adjustColumnWidths(const std::function<int(int)> &widthFn);
    /// Adjusts the table view widget to \a width columns and \a height rows.
    void setTableViewSize(int width, int height);
    /// Slot: applies the current zoom scale to the waveform view.
    void zoomChanged();

    // --- Action Handlers (menu / toolbar callbacks) ---

    void on_actionAboutQt_triggered();           ///< Shows the Qt About dialog.
    void on_actionAbout_triggered();             ///< Shows the BeWavedDolphin About dialog.
    void on_actionAutoCrop_triggered();          ///< Removes trailing zero-valued columns.
    void on_actionClear_triggered();             ///< Sets all selected input cells to 0.
    void on_actionCombinational_triggered();     ///< Generates all combinational input patterns.
    void on_actionCopy_triggered();              ///< Copies selected cells to the clipboard.
    void on_actionCut_triggered();               ///< Cuts selected cells to the clipboard.
    void on_actionExit_triggered();              ///< Closes the waveform editor.
    void on_actionExportToPdf_triggered();       ///< Exports the waveform to a PDF file.
    void on_actionExportToPng_triggered();       ///< Exports the waveform to a PNG file.
    void on_actionFitScreen_triggered();         ///< Scales the view to fit the window.
    void on_actionInvert_triggered();            ///< Inverts the selected input cells.
    void on_actionLoad_triggered();              ///< Opens a .dolphin file.
    void on_actionPaste_triggered();             ///< Pastes clipboard data into the selection.
    void on_actionResetZoom_triggered();         ///< Resets the waveform zoom to 100%.
    void on_actionSaveAs_triggered();            ///< Saves the waveform to a new .dolphin file.
    void on_actionSave_triggered();              ///< Saves the waveform to the current file.
    void on_actionSetClockWave_triggered();      ///< Fills the selected row with a clock pattern.
    void on_actionSetLength_triggered();         ///< Opens a dialog to change the simulation length.
    void on_actionSetTo0_triggered();            ///< Sets all selected input cells to 0.
    void on_actionSetTo1_triggered();            ///< Sets all selected input cells to 1.
    void on_actionShowNumbers_triggered();       ///< Switches the display to numeric mode.
    void on_actionShowWaveforms_triggered();     ///< Switches the display to waveform mode.
    void on_actionZoomIn_triggered();            ///< Increases the waveform zoom level.
    void on_actionZoomOut_triggered();           ///< Decreases the waveform zoom level.
    void on_tableView_cellDoubleClicked();       ///< Toggles the double-clicked input cell value.
    void on_tableView_selectionChanged();        ///< Updates toolbar button state for the new selection.

    // --- Members ---

    std::unique_ptr<BewavedDolphinUi> m_ui;          ///< Auto-generated UI descriptor.
    WaveformView m_view;                              ///< Graphics view for the waveform scene.
    MainWindow *m_mainWindow       = nullptr;         ///< Owner main window (for simulation restore).
    PlotType m_type                = PlotType::Line;  ///< Current display style (line vs. number).
    QFileInfo m_currentFile;                          ///< Path of the currently loaded .dolphin file.
    QGraphicsScene *m_scene        = new QGraphicsScene(this); ///< Scene backing the waveform view.
    QStandardItemModel *m_model    = nullptr;         ///< Table model (rows = signals, cols = time steps).
    QTableView *m_signalTableView  = new QTableView(); ///< Hidden table view used for selection management.
    SignalDelegate *m_delegate     = nullptr;         ///< Custom delegate rendering waveform cells.
    QVector<GraphicElement *> m_outputs;              ///< Output elements mapped to waveform rows.
    QVector<GraphicElementInput *> m_inputs;          ///< Input elements mapped to waveform rows.
    QVector<Status> m_oldInputValues;                 ///< Saved input values for restoreInputs().
    Scene *m_externalScene         = nullptr;         ///< The circuit scene being simulated.
    Simulation *m_simulation       = nullptr;         ///< Simulation engine used for waveform runs.
    bool m_edited                  = false;           ///< True if the waveform has unsaved changes.
    const bool m_askConnection;                       ///< If true, prompt to link to a .dolphin file on open.
    const double m_scaleFactor     = 0.8;             ///< Multiplier per zoom-out step.
    double m_scale                 = 1.25;            ///< Current zoom scale factor.
    int m_clockPeriod              = 0;               ///< Period used by "Set Clock Wave" (0 = auto).
    int m_inputPorts               = 0;               ///< Number of input ports in the circuit.
    int m_length                   = 32;              ///< Number of simulation time-step columns.
};

