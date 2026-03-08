// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief BeWavedDolphin waveform editor: digital signal creation, display, and export.
 */

#pragma once

#include <memory>

#include <QFileInfo>
#include <QItemDelegate>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QTableView>

#include "App/BeWavedDolphin/BeWavedDolphinUI.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"

class GraphicsView;
class MainWindow;
class QItemSelection;
class QSaveFile;

/// Controls how signal cells are rendered in the waveform table.
enum class PlotType {
    Number, ///< Cells display the numeric value (0/1).
    Line    ///< Cells display a waveform-style rising/falling edge graphic.
};

/**
 * \class SignalModel
 * \brief QStandardItemModel subclass that makes output rows read-only in the waveform table.
 *
 * \details Input signal cells are editable; output signal cells are read-only since their
 * values are computed by the simulation.
 */
class SignalModel : public QStandardItemModel
{
    Q_OBJECT

public:
    /**
     * \brief Constructs the model.
     * \param inputs  Number of input signal rows (these are editable).
     * \param rows    Total number of signal rows.
     * \param columns Number of time-step columns.
     * \param parent  Optional parent object.
     */
    SignalModel(const int inputs, const int rows, const int columns, QObject *parent = nullptr);

    /// \reimp
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    const int m_inputCount; ///< Number of input rows; rows above this index are editable.
};

/**
 * \class SignalDelegate
 * \brief Item delegate that draws digital waveform graphics inside table cells.
 *
 * \details Replaces the default text rendering with pixmaps representing logic-high,
 * logic-low, rising-edge, and falling-edge states in the chosen PlotType style.
 */
class SignalDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    /// Constructs the delegate with \a parent.
    SignalDelegate(QObject *parent);

    /// \reimp
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

/**
 * \class DolphinGraphicsView
 * \brief GraphicsView subclass with waveform-editor zoom behaviour.
 *
 * \details Overrides the wheel event to zoom the entire scene (like Quartus Prime) rather
 * than scroll, and provides a discrete zoom-level range suitable for waveform display.
 */
class DolphinGraphicsView : public GraphicsView
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the view with \a parent.
    explicit DolphinGraphicsView(QWidget *parent = nullptr);

    // --- Zoom ---

    /// Returns \c true if zooming in further is possible.
    bool canZoomIn() const;

    /// Returns \c true if further zoom-out is possible.
    bool canZoomOut() const;

    /// Resets zoom to the default scale.
    void resetZoom();

    /// Increases zoom by one step.
    void zoomIn();

    /// Decreases zoom by one step.
    void zoomOut();

protected:
    // --- Qt event overrides ---

    /// \reimp Redirects wheel events to zoom instead of scroll.
    void wheelEvent(QWheelEvent *event) override;
};

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
    // --- Lifecycle ---

    /**
     * \brief Constructs the waveform editor.
     * \param scene External scene to read circuit elements from.
     * \param askConnection Whether to prompt when the file association is ambiguous.
     * \param parent Parent window.
     */
    explicit BewavedDolphin(Scene *scene, const bool askConnection = true, MainWindow *parent = nullptr);
    ~BewavedDolphin() override;

    // --- Waveform Initialization ---

    /// Creates a fresh empty waveform table from the current circuit.
    void createWaveform();

    /**
     * \brief Initializes the waveform from \a fileName if it matches the current circuit.
     * \param fileName Path to an existing .dolphin file.
     */
    void createWaveform(const QString &fileName);

    // --- Display ---

    /// Shows the waveform editor window.
    void show();

    // --- Export ---

    /// Prints the waveform to the system printer.
    void print();
    /// Writes the waveform data as plain text to \a stream.
    void saveToTxt(QTextStream &stream);

protected:
    // --- Qt event overrides ---

    /// \reimp
    void closeEvent(QCloseEvent *event) override;
    /// \reimp
    void resizeEvent(QResizeEvent *event) override;

private:
    Q_DISABLE_COPY(BewavedDolphin)

    // --- File I/O ---

    /// Prompts the user to save unsaved changes; returns \c false if cancelled.
    bool checkSave();
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

    /// Collects input/output GraphicElements from the circuit scene.
    void loadElements();
    /// Populates signal rows from the external circuit's terminal elements.
    void loadFromTerminal();
    /// Creates a fresh table model with the current input/output configuration.
    void loadNewTable();
    void loadPixmaps();
    /// Fills \a inputLabels and \a outputLabels from the circuit's I/O elements.
    void loadSignals(QStringList &inputLabels, QStringList &outputLabels);
    void createElement(const int row, const int col, const int value, const bool isInput = true, const bool changeNext = true);
    void createOneElement(const int row, const int col, const bool isInput = true, const bool changeNext = true);
    void createZeroElement(const int row, const int col, const bool isInput = true, const bool changeNext = true);

    // --- Clipboard ---

    /// Returns the first column index in the selected \a ranges.
    int sectionFirstColumn(const QItemSelection &ranges);
    /// Returns the first row index in the selected \a ranges.
    int sectionFirstRow(const QItemSelection &ranges);
    /// Serializes the selected \a ranges to \a stream (clipboard copy).
    void copy(const QItemSelection &ranges, QDataStream &stream);
    /// Serializes and clears the selected \a ranges (clipboard cut).
    void cut(const QItemSelection &ranges, QDataStream &stream);
    /// Deserializes clipboard data from \a stream into the selected \a ranges.
    void paste(const QItemSelection &ranges, QDataStream &stream);

    // --- Simulation State ---

    void prepare(const QString &fileName = {});
    /// Restores original input element values after a simulation run.
    void restoreInputs();
    void run();
    void setLength(const int simLength, const bool runSimulation);

    // --- Scene / View ---

    /// Resizes the internal QGraphicsScene to fit the current table dimensions.
    void resizeScene();
    /// Slot: applies the current zoom scale to the waveform view.
    void zoomChanged();

    // --- Table View Event Handlers ---

    void on_tableView_cellDoubleClicked();       ///< Toggles the double-clicked input cell value.
    void on_tableView_selectionChanged();        ///< Updates toolbar button state for the new selection.

    // --- Menu Actions: File ---

    void on_actionLoad_triggered();              ///< Opens a .dolphin file.
    void on_actionSave_triggered();              ///< Saves the waveform to the current file.
    void on_actionSaveAs_triggered();            ///< Saves the waveform to a new .dolphin file.
    void on_actionExportToPdf_triggered();       ///< Exports the waveform to a PDF file.
    void on_actionExportToPng_triggered();       ///< Exports the waveform to a PNG file.
    void on_actionExit_triggered();              ///< Closes the waveform editor.

    // --- Menu Actions: Edit ---

    void on_actionAutoCrop_triggered();          ///< Removes trailing zero-valued columns.
    void on_actionClear_triggered();             ///< Sets all selected input cells to 0.
    void on_actionCombinational_triggered();     ///< Generates all combinational input patterns.
    void on_actionCopy_triggered();              ///< Copies selected cells to the clipboard.
    void on_actionCut_triggered();               ///< Cuts selected cells to the clipboard.
    void on_actionInvert_triggered();            ///< Inverts the selected input cells.
    void on_actionPaste_triggered();             ///< Pastes clipboard data into the selection.
    void on_actionSetClockWave_triggered();      ///< Fills the selected row with a clock pattern.
    void on_actionSetLength_triggered();         ///< Opens a dialog to change the simulation length.
    void on_actionSetTo0_triggered();            ///< Sets all selected input cells to 0.
    void on_actionSetTo1_triggered();            ///< Sets all selected input cells to 1.

    // --- Menu Actions: View ---

    void on_actionFitScreen_triggered();         ///< Scales the view to fit the window.
    void on_actionResetZoom_triggered();         ///< Resets the waveform zoom to 100%.
    void on_actionShowNumbers_triggered();       ///< Switches the display to numeric mode.
    void on_actionShowWaveforms_triggered();     ///< Switches the display to waveform mode.
    void on_actionZoomIn_triggered();            ///< Increases the waveform zoom level.
    void on_actionZoomOut_triggered();           ///< Decreases the waveform zoom level.

    // --- Menu Actions: About ---

    void on_actionAbout_triggered();             ///< Shows the BeWavedDolphin About dialog.
    void on_actionAboutQt_triggered();           ///< Shows the Qt About dialog.

    // --- Members ---

    std::unique_ptr<BewavedDolphinUi> m_ui;          ///< Auto-generated UI descriptor.
    DolphinGraphicsView m_view;                      ///< Graphics view for the waveform scene.
    MainWindow *m_mainWindow = nullptr;              ///< Owner main window (for simulation restore).
    PlotType m_type = PlotType::Line;                ///< Current display style (line vs. number).
    QFileInfo m_currentFile;                         ///< Path of the currently loaded .dolphin file.
    QGraphicsScene *m_scene = new QGraphicsScene(this); ///< Scene backing the waveform view.
    QPixmap m_fallingBlue;
    QPixmap m_fallingGreen;
    QPixmap m_highBlue;
    QPixmap m_highGreen;
    QPixmap m_lowBlue;
    QPixmap m_lowGreen;
    QPixmap m_risingBlue;
    QPixmap m_risingGreen;
    QStandardItemModel *m_model = nullptr;           ///< Table model (rows = signals, cols = time steps).
    QTableView *m_signalTableView = new QTableView(); ///< Hidden table view used for selection management.
    QVector<GraphicElement *> m_outputs;              ///< Output elements mapped to waveform rows.
    QVector<GraphicElementInput *> m_inputs;          ///< Input elements mapped to waveform rows.
    QVector<Status> m_oldInputValues;                 ///< Saved input values for restoreInputs().
    Scene *m_externalScene = nullptr;                ///< The circuit scene being simulated.
    Simulation *m_simulation = nullptr;              ///< Simulation engine used for waveform runs.
    bool m_edited = false;                           ///< True if the waveform has unsaved changes.
    const bool m_askConnection;                      ///< If true, prompt to link to a .dolphin file on open.
    const double m_scaleFactor = 0.8;                ///< Multiplier per zoom-out step.
    double m_scale = 1.25;                           ///< Current zoom scale factor.
    int m_clockPeriod = 0;                           ///< Period used by "Set Clock Wave" (0 = auto).
    int m_inputPorts = 0;                            ///< Number of input ports in the circuit.
    int m_length = 32;                               ///< Number of simulation time-step columns.
};

