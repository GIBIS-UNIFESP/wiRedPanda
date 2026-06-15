// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief BewavedDolphin waveform editor: digital signal creation, display, and export.
 */

#pragma once

#include <functional>
#include <memory>

#include <QFileInfo>
#include <QMainWindow>
#include <QPixmap>
#include <QStandardItemModel>
#include <QTableView>

#include "App/BeWavedDolphin/BeWavedDolphinUI.h"
#include "App/BeWavedDolphin/SignalDelegate.h"
#include "App/BeWavedDolphin/SignalModel.h"
#include "App/Scene/Scene.h"

class DolphinHost;
class WaveformSimulator;

namespace DolphinSerializer { struct WaveformData; }

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
    friend class TestBewavedDolphinGui;

public:
    /**
     * \brief Constructs the waveform editor.
     * \param scene         Circuit scene whose I/O elements will be mapped to signals.
     * \param askConnection If \c true, prompts the user to link to a .dolphin file.
     * \param host          Host application providing the circuit file context (a
     *                      MainWindow in the app; a stub in tests). May be \c nullptr.
     */
    explicit BewavedDolphin(Scene *scene, const bool askConnection = true, DolphinHost *host = nullptr);
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
     */
    void createElement(const int row, const int col, const int value);

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
    SignalModel* getModel() const { return m_model; }

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
    /// \reimp Intercepts the mouse wheel on the table viewport to zoom the columns.
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Q_DISABLE_COPY(BewavedDolphin)

    // --- File I/O ---

    /// Prompts the user to save unsaved changes; returns \c false if cancelled.
    bool checkSave();
    bool exportWaveformToPng(const QString &filename);
    /// Links the waveform editor to the .dolphin file at \a fileName.
    void associateToWiRedPanda(const QString &fileName);
    /// Opens the .dolphin/.csv file at \a fileName and loads its waveform data.
    void load(const QString &fileName);
    /// Applies parsed file \a fileData to the model (sets length, fills inputs, re-runs).
    void applyWaveformData(const DolphinSerializer::WaveformData &fileData);

    // --- Signal Table Management ---

    /// Collects input/output GraphicElements from the circuit scene.
    void loadElements();
    /// Populates signal rows from the external circuit's terminal elements.
    void loadFromTerminal();
    /// Creates a fresh table model with the current input/output configuration.
    void loadNewTable();
    /// Fills \a inputLabels and \a outputLabels from the circuit's I/O elements.
    void loadSignals(QStringList &inputLabels, QStringList &outputLabels);
    /// Applies \a valueFn to every selected cell, marks the waveform edited, and re-runs.
    void applyToSelectedCells(const std::function<int(int)> &valueFn);
    /// Fills the first m_inputPorts rows of \a model (over \a columns) with the
    /// Gray-code-style enumeration of all input combinations (the combinational truth table).
    void fillCombinationalInputs(SignalModel *model, int columns) const;
    /// Applies the current zoom factor to the table's row/column sizes and font.
    void applyZoom();
    /// Renders the full waveform to a pixmap using \a cellW x \a cellH cells, via a
    /// throwaway offscreen table bound to the same model (the live view is untouched).
    QPixmap renderWaveform(int cellW, int cellH) const;
    /// Updates the enabled state of the zoom-in/out actions for the current zoom.
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
    DolphinHost *m_host            = nullptr;         ///< Host app providing circuit file context.
    PlotType m_type                = PlotType::Line;  ///< Current display style (line vs. number).
    QFileInfo m_currentFile;                          ///< Path of the currently loaded .dolphin file.
    SignalModel *m_model           = nullptr;         ///< Table model (rows = signals, cols = time steps).
    QTableView *m_signalTableView  = new QTableView(); ///< The waveform table, shown as the central widget.
    SignalDelegate *m_delegate     = nullptr;         ///< Custom delegate rendering waveform cells.
    QVector<GraphicElement *> m_outputs;              ///< Output elements mapped to waveform rows.
    QVector<GraphicElementInput *> m_inputs;          ///< Input elements mapped to waveform rows.
    QVector<Status> m_oldInputValues;                 ///< Saved input values for restoreInputs().
    Scene *m_externalScene         = nullptr;         ///< The circuit scene being simulated.
    Simulation *m_simulation       = nullptr;         ///< Simulation engine used for waveform runs.
    std::unique_ptr<WaveformSimulator> m_simDriver;   ///< Drives the column-by-column simulation sweep.
    bool m_edited                  = false;           ///< True if the waveform has unsaved changes.
    const bool m_askConnection;                       ///< If true, prompt to link to a .dolphin file on open.
    int m_zoomLevel                = 0;               ///< Discrete column-width zoom step (0..6); Zoom In/Out/Reset only — row height and font stay fixed.
    double m_fitScale              = 1.0;             ///< Uniform scale from Fit Screen; scales column width, row height, and font together.
    int m_clockPeriod              = 0;               ///< Period used by "Set Clock Wave" (0 = auto).
    int m_inputPorts               = 0;               ///< Number of input ports in the circuit.
    int m_length                   = 32;              ///< Number of simulation time-step columns.
};
