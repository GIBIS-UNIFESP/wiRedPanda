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
#include <QUndoStack>

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
#include "App/Scene/Scene.h"

class DolphinHost;
class DolphinZoom;
class ExerciseOverlay;
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
class BewavedDolphin : public BewavedDolphinBase
{
    Q_OBJECT
    friend class TestBewavedDolphinGui;

public:
    /**
     * \brief Constructs the waveform editor.
     * \param scene         Circuit scene whose I/O elements will be mapped to signals.
     * \param askConnection If \c true, closing prompts to save unsaved changes (checkSave()).
     * \param host          Host application providing the circuit file context (a
     *                      MainWindow in the app; a stub in tests). May be \c nullptr.
     * \param parent        Widget to parent this window to, so its Qt::WindowModal setting
     *                      (see the constructor body) actually blocks it — separate from
     *                      \a host since tests pass a non-QWidget stub host. May be \c nullptr.
     */
    explicit BewavedDolphin(Scene *scene, const bool askConnection = true, DolphinHost *host = nullptr,
                            QWidget *parent = nullptr);
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
    void setCellValue(const int row, const int col, const int value);

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

    /// A labelled waveform signal: a row label and its per-column 0/1 values.
    struct Signal {
        QString label;        ///< The signal's display label.
        QVector<int> values;  ///< Per-column logic values.
    };

    /// A read-only snapshot of the computed waveform, for automation/export consumers.
    struct WaveformSnapshot {
        QList<Signal> inputs;   ///< Input signals (one per input element).
        QList<Signal> outputs;  ///< Output signals (one per output element).
    };

    /// Returns the input/output signals over the first \a duration columns (MCP access).
    WaveformSnapshot snapshot(int duration) const;

    /// Returns the input row index whose element label equals \a label, or -1 (MCP access).
    int inputRow(const QString &label) const;

    // --- Tour / Exercise overlay support ---

    /// Registers \a overlay so it is repositioned whenever the window is resized.
    /// Pass \c nullptr to detach. Does not take ownership.
    void setExerciseOverlay(ExerciseOverlay *overlay);

    /// Returns the waveform table view (for tour target resolution).
    QTableView *signalTableView() const { return m_signalTableView; }
    /// Returns the main toolbar (for tour target resolution).
    QToolBar   *mainToolBar()        const;
    /// Returns the combinational action (for tour button spotlighting).
    QAction    *actionCombinational() const;
    /// Triggers the combinational input-pattern generator (for tour automation).
    void triggerCombinational();

protected:
    // --- Qt event overrides ---

    /// \reimp
    void closeEvent(QCloseEvent *event) override;
    /// \reimp
    void resizeEvent(QResizeEvent *event) override;
    /// \reimp Intercepts the mouse wheel on the table viewport to zoom the columns.
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Q_DISABLE_COPY(BewavedDolphin)

#ifdef USE_KDE_FRAMEWORKS
    void setupKdeActions();
#endif

    // --- Internal accessors (test-only; TestBewavedDolphinGui is a friend) ---

    /// Returns the underlying table model.
    const SignalModel* model() const { return m_model; }
    /// Returns the output element vector.
    const QVector<GraphicElement *>& outputElements() const { return m_outputs; }
    /// Returns the input element vector.
    const QVector<GraphicElementInput *>& inputElements() const { return m_inputs; }
    /// Returns the current simulation length (number of columns).
    int length() const { return m_length; }

    // --- File I/O ---

    /// Prompts the user to save unsaved changes; returns \c false if cancelled.
    bool checkSave();
    /// Links the waveform editor to the .dolphin file at \a fileName.
    void associateToWiRedPanda(const QString &fileName);
    /// Opens the .dolphin/.csv file at \a fileName and loads its waveform data.
    void load(const QString &fileName);
    /// Applies parsed file \a fileData to the model (sets length, fills inputs, re-runs).
    void applyWaveformData(const DolphinSerializer::WaveformData &fileData);

    /// Returns \c false if any element in m_inputs/m_outputs was deleted from the live scene
    /// since prepare() snapshotted them (e.g. via the main canvas or an MCP client — neither
    /// path is blocked by this window's modality, which only affects interactive GUI input).
    /// run()/saveToTxt() must check this before dereferencing those pointers via sweep().
    bool elementsStillLive() const;

    // --- Signal Table Management ---

    /// Populates signal rows from the external circuit's terminal elements.
    void loadFromTerminal();
    /// Builds the table model + wires the view for the given input/output row labels
    /// (produced by DolphinModelBuilder), snapshots input states, and runs the first sweep.
    void loadNewTable(const QStringList &inputLabels, const QStringList &outputLabels);
    /// Applies \a valueFn to every selected cell, marks the waveform edited, and re-runs.
    /// Undoable -- pushes a SetCellsCommand (see DolphinCommands.h) after applying.
    void applyToSelectedCells(const std::function<int(int)> &valueFn);

    /// Snapshots \a indexes' current (row, col) positions and values, for constructing a
    /// SetCellsCommand right before an edit that will change them.
    std::pair<QList<QPair<int, int>>, QList<int>> snapshotCells(const QModelIndexList &indexes) const;

    /// Returns every (row, col) index in the rectangular range [0, rows) x [0, cols), for
    /// snapshotting a whole-grid edit (clear, combinational fill) with snapshotCells().
    QModelIndexList allCellIndexes(int rows, int cols) const;
    /// Applies the current zoom factor to the table's row/column sizes and font.
    void applyZoom();
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
    QUndoStack m_undoStack;                           ///< Undo/redo history for waveform cell edits (#19).
    DolphinHost *m_host            = nullptr;         ///< Host app providing circuit file context.
    QFileInfo m_currentFile;                          ///< Path of the currently loaded .dolphin file.
    SignalModel *m_model           = nullptr;         ///< Table model (rows = signals, cols = time steps). Created once by loadNewTable() and owned via QObject parenting (this); freed when the window closes.
    QTableView *m_signalTableView  = new QTableView(); ///< The waveform table, shown as the central widget.
    SignalDelegate *m_delegate     = nullptr;         ///< Custom delegate rendering waveform cells.
    QVector<GraphicElement *> m_outputs;              ///< Output elements mapped to waveform rows.
    QVector<GraphicElementInput *> m_inputs;          ///< Input elements mapped to waveform rows.
    QVector<Status> m_oldInputValues;                 ///< Saved input values for restoreInputs().
    Scene *m_externalScene         = nullptr;         ///< The circuit scene being simulated.
    Simulation *m_simulation       = nullptr;         ///< Simulation engine used for waveform runs.
    std::unique_ptr<WaveformSimulator> m_simDriver;   ///< Drives the column-by-column simulation sweep.
    std::unique_ptr<DolphinZoom> m_zoom;              ///< Owns the table's zoom state + view metrics.
    bool m_edited                  = false;           ///< True if the waveform has unsaved changes.
    const bool m_askConnection;                       ///< If true, closing consults checkSave()'s save-changes prompt.
    int m_clockPeriod              = 0;               ///< Period used by "Set Clock Wave" (0 = auto).
    int m_inputPorts               = 0;               ///< Number of input ports in the circuit.
    int m_length                   = 32;              ///< Number of simulation time-step columns.
    ExerciseOverlay *m_exerciseOverlay = nullptr;     ///< Non-owning; repositioned on resize.
};
