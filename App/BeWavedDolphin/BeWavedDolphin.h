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

    bool checkSave();
    void associateToWiRedPanda(const QString &fileName);
    void load(QDataStream &stream);
    void load(QFile &file);
    void load(const QString &fileName);
    void save(QDataStream &stream);
    void save(QSaveFile &file);
    void save(const QString &fileName);

    // --- Signal Table Management ---

    void loadElements();
    void loadFromTerminal();
    void loadNewTable();
    void loadPixmaps();
    void loadSignals(QStringList &inputLabels, QStringList &outputLabels);
    void createElement(const int row, const int col, const int value, const bool isInput = true, const bool changeNext = true);
    void createOneElement(const int row, const int col, const bool isInput = true, const bool changeNext = true);
    void createZeroElement(const int row, const int col, const bool isInput = true, const bool changeNext = true);

    // --- Clipboard ---

    int sectionFirstColumn(const QItemSelection &ranges);
    int sectionFirstRow(const QItemSelection &ranges);
    void copy(const QItemSelection &ranges, QDataStream &stream);
    void cut(const QItemSelection &ranges, QDataStream &stream);
    void paste(const QItemSelection &ranges, QDataStream &stream);

    // --- Simulation State ---

    void prepare(const QString &fileName = {});
    void restoreInputs();
    void run();
    void setLength(const int simLength, const bool runSimulation);

    // --- Scene / View ---

    void resizeScene();
    void zoomChanged();

    // --- Table View Event Handlers ---

    void on_tableView_cellDoubleClicked();
    void on_tableView_selectionChanged();

    // --- Menu Actions: File ---

    void on_actionLoad_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionExportToPdf_triggered();
    void on_actionExportToPng_triggered();
    void on_actionExit_triggered();

    // --- Menu Actions: Edit ---

    void on_actionAutoCrop_triggered();
    void on_actionClear_triggered();
    void on_actionCombinational_triggered();
    void on_actionCopy_triggered();
    void on_actionCut_triggered();
    void on_actionInvert_triggered();
    void on_actionPaste_triggered();
    void on_actionSetClockWave_triggered();
    void on_actionSetLength_triggered();
    void on_actionSetTo0_triggered();
    void on_actionSetTo1_triggered();

    // --- Menu Actions: View ---

    void on_actionFitScreen_triggered();
    void on_actionResetZoom_triggered();
    void on_actionShowNumbers_triggered();
    void on_actionShowWaveforms_triggered();
    void on_actionZoomIn_triggered();
    void on_actionZoomOut_triggered();

    // --- Menu Actions: About ---

    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();

    // --- Members ---

    std::unique_ptr<BewavedDolphinUi> m_ui;
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

