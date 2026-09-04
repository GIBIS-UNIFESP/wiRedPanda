// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickDolphinController: QML-facing presenter for the Quick BeWavedDolphin port.
 */

#pragma once

#include <functional>
#include <memory>

#include <QFileInfo>
#include <QItemSelectionModel>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QTimer>
#include <QUndoStack>

#include "App/BeWavedDolphin/DolphinModelBuilder.h"
#include "App/BeWavedDolphin/SignalModel.h"

class CanvasItem;
class DolphinHost;
class GraphicElement;
class GraphicElementInput;
class QTextStream;
class QuickDolphinZoom;
class Simulation;
class WaveformSimulator;
namespace DolphinSerializer { struct WaveformData; }

/**
 * \class QuickDolphinController
 * \brief Quick-side port of BewavedDolphin's data/simulation half plus its window-action layer.
 *
 * \details Copy-and-adapt port (same precedent as every other Quick* controller in this rewrite),
 * mirroring BewavedDolphin::prepare()/loadNewTable()/run() -- but against CanvasItem instead of
 * Scene. Owns a real SignalModel (100% reused as-is), a WaveformSimulator (already Scene-free,
 * see WaveformSimulator.h's doc comment), and a QuickDolphinZoom. m_canvas is a QPointer, not a
 * raw CanvasItem*, for the exact reason documented on QuickElementEditor::m_canvas -- this class
 * outlives any single circuit tab. Unlike BewavedDolphin (one window, one shot, WA_DeleteOnClose),
 * this controller is a long-lived singleton (DolphinWindow.qml is a single static QML instance,
 * matching TruthTableDialog's own precedent) reused across multiple createWaveform() calls.
 */
class QuickDolphinController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("QuickDolphinController is only ever exposed via AppController.dolphin")

    // Declared as QObject*, not SignalModel*: SignalModel is part of wiredpanda_lib and has no
    // QML registration at all (see isInputRow()/cellValue()'s own doc comment on why it can't
    // get one) -- qmllint's static analysis of Q_PROPERTY types needs every declared type to be
    // resolvable, and warns ("unresolved-type") on SignalModel specifically, even though nothing
    // ever reads a member off this property in QML (SignalTable.qml only ever assigns it
    // straight into TableView.model, which accepts any QAbstractItemModel-derived QObject
    // opaquely). model()'s C++ return type stays SignalModel* for real callers; the implicit
    // SignalModel*->QObject* upcast in the READ call is what moc's generated code relies on.
    Q_PROPERTY(QObject *model READ model NOTIFY modelChanged FINAL)
    Q_PROPERTY(int inputRows READ inputRows NOTIFY modelChanged FINAL)
    Q_PROPERTY(qreal columnWidth READ columnWidth NOTIFY zoomChanged FINAL)
    Q_PROPERTY(qreal rowHeight READ rowHeight NOTIFY zoomChanged FINAL)
    Q_PROPERTY(qreal fontScale READ fontScale NOTIFY zoomChanged FINAL)
    Q_PROPERTY(bool canZoomIn READ canZoomIn NOTIFY zoomChanged FINAL)
    Q_PROPERTY(bool canZoomOut READ canZoomOut NOTIFY zoomChanged FINAL)
    Q_PROPERTY(bool numberMode READ numberMode NOTIFY numberModeChanged FINAL)
    Q_PROPERTY(QItemSelectionModel *selectionModel READ selectionModel NOTIFY modelChanged FINAL)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY hasSelectionChanged FINAL)
    Q_PROPERTY(int length READ length NOTIFY modelChanged FINAL)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY undoRedoStateChanged FINAL)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY undoRedoStateChanged FINAL)
    Q_PROPERTY(QString undoText READ undoText NOTIFY undoRedoStateChanged FINAL)
    Q_PROPERTY(QString redoText READ redoText NOTIFY undoRedoStateChanged FINAL)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged FINAL)
    Q_PROPERTY(QString currentFileName READ currentFileName NOTIFY currentFileChanged FINAL)

public:
    explicit QuickDolphinController(QObject *parent = nullptr);
    ~QuickDolphinController() override;

    [[nodiscard]] SignalModel *model() const { return m_model; }
    [[nodiscard]] int inputRows() const { return m_model ? m_model->inputRows() : 0; }

    /// \copydoc SignalModel::isInputRow
    /// Wraps the model's own method instead of exposing SignalModel to QML directly: SignalModel
    /// is part of wiredpanda_lib, and qt_add_qml_module(wiredpanda ...)'s type scanner only
    /// walks wiredpanda's own listed sources -- a wiredpanda_lib class can't be made
    /// visible to it, whether via its own QML_ELEMENT or a QML_FOREIGN struct here. QuickDolphinController
    /// itself has no such problem (it's a real wiredpanda type), so SignalTable.qml's cell
    /// delegate calls through here instead of `root.controller.model.isInputRow(...)`.
    Q_INVOKABLE bool isInputRow(int row) const { return m_model && m_model->isInputRow(row); }
    /// \copydoc SignalModel::value
    /// Same reasoning as isInputRow() above.
    Q_INVOKABLE int cellValue(int row, int col) const { return m_model ? m_model->value(row, col) : 0; }

    [[nodiscard]] qreal columnWidth() const;
    [[nodiscard]] qreal rowHeight() const;
    [[nodiscard]] qreal fontScale() const;
    [[nodiscard]] bool canZoomIn() const;
    [[nodiscard]] bool canZoomOut() const;
    [[nodiscard]] bool numberMode() const { return m_numberMode; }
    /// A real QItemSelectionModel targeting the live model. Qt itself registers QItemSelectionModel
    /// as a QML_FOREIGN type (QtQmlModels' qqmlmodelsmodule_p.h, used for exactly this
    /// TableView.selectionModel scenario), so the property can be statically typed instead of
    /// needing SignalModel's own QObject*-erasure workaround.
    [[nodiscard]] QItemSelectionModel *selectionModel() const { return m_selectionModel; }
    [[nodiscard]] bool hasSelection() const { return m_selectionModel && m_selectionModel->hasSelection(); }
    [[nodiscard]] int length() const { return m_model ? m_model->columnCount() : m_length; }
    [[nodiscard]] bool canUndo() const { return m_undoStack.canUndo(); }
    [[nodiscard]] bool canRedo() const { return m_undoStack.canRedo(); }
    [[nodiscard]] QString undoText() const { return m_undoStack.undoText(); }
    [[nodiscard]] QString redoText() const { return m_undoStack.redoText(); }
    [[nodiscard]] QString statusMessage() const { return m_statusMessage; }
    [[nodiscard]] QString currentFileName() const { return m_currentFile.fileName(); }

    /// Links the waveform editor to its host app (circuit file context, dolphin-file
    /// association). Called once from QuickAppController's own constructor body
    /// (App/QuickShell/Chrome/QuickAppController -- the real, only DolphinHost implementation).
    void setHost(DolphinHost *host) { m_host = host; }

    /// Builds a fresh waveform from \a canvas's current circuit (mirrors BewavedDolphin::
    /// prepare() + loadNewTable()'s blank-table path), then, if \a fileName is non-empty,
    /// resolves and loads it on top (mirrors BewavedDolphin::createWaveform(const QString &)'s
    /// file-load branch, incl. its absolute-path-first-then-host-relative-directory fallback).
    /// Throws (Pandaception) on an empty circuit or one missing inputs/outputs, same as
    /// DolphinModelBuilder::collect() always has -- QuickAppController::openWaveform() (this
    /// method's real launch site) is expected to catch and report it the same way
    /// MainWindow::on_actionWaveform_triggered()'s Application::guardedSlot wrapping does today.
    void createWaveform(CanvasItem *canvas, const QString &fileName = {});

    /// Re-runs the simulation sweep across the current model, mirrors BewavedDolphin::run().
    Q_INVOKABLE void run();
    /// Sets one cell value and re-runs, mirrors BewavedDolphin::setCellValue() (MCP entry point --
    /// interactive double-click editing goes through invertSelected()/SetCellsCommand instead, see
    /// SignalTable.qml's cell delegate).
    Q_INVOKABLE void setCellValue(int row, int col, int value);

    /// Click-to-select on the signal table's cell delegate (SignalTable.qml has no built-in
    /// mouse-driven selection -- QtQuick.Controls' TableView, unlike QTableView, doesn't wire one
    /// up itself). Mirrors QAbstractItemView::ExtendedSelection's own plain/Ctrl/Shift-click
    /// semantics exactly, since that's BewavedDolphin's un-overridden default: plain click
    /// replaces the selection with just this cell (and becomes the new anchor for a following
    /// Shift-click); Ctrl toggles this cell in/out of the existing selection; Shift selects the
    /// rectangular range from the last plain-click anchor to this cell.
    Q_INVOKABLE void selectCell(int row, int column, Qt::KeyboardModifiers modifiers);

    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();
    Q_INVOKABLE void resetZoom();
    /// \copydoc QuickDolphinZoom::fitScreen
    Q_INVOKABLE void fitScreen(qreal viewportWidth, qreal viewportHeight, qreal headerWidth, qreal headerHeight);

    /// Switches the cell delegate to numeric text ("0"/"1"), mirrors BewavedDolphin::
    /// on_actionShowNumbers_triggered(). A pure view concern (the model's values never change),
    /// same as zoom -- no undo/file dependency.
    Q_INVOKABLE void showNumbers();
    /// Switches the display to waveform mode.
    Q_INVOKABLE void showWaveforms();

    // --- Undo/redo ---

    Q_INVOKABLE void undo() { m_undoStack.undo(); }
    Q_INVOKABLE void redo() { m_undoStack.redo(); }

    // --- Edit actions (mirror BewavedDolphin's own on_action*_triggered handlers) ---

    /// Sets all selected input cells to 0.
    Q_INVOKABLE void setSelectedTo0();
    /// Sets all selected input cells to 1.
    Q_INVOKABLE void setSelectedTo1();
    /// Inverts the selected input cells.
    Q_INVOKABLE void invertSelected();
    /// Clears every input cell, not just the selection -- same distinction as the original
    /// (allCellIndexes(m_inputPorts, ...) vs. applyToSelectedCells()).
    Q_INVOKABLE void clearInputs();
    /// Generates all combinational input patterns.
    Q_INVOKABLE void combinational();
    /// Removes trailing zero-valued columns.
    Q_INVOKABLE void autoCrop();
    /// Applies a clock period to the current selection, mirrors the tail of BewavedDolphin::
    /// on_actionSetClockWave_triggered() (from ClockDialog::exec() returning Accepted onward).
    Q_INVOKABLE void applyClockWave(int period);
    /// Sets the simulation length (number of columns) and rebuilds the waveform accordingly.
    /// Always re-runs (mirrors on_actionSetLength_triggered()'s runSimulation=true; AutoCrop's
    /// own runSimulation=true call goes through autoCrop() above, not this directly).
    Q_INVOKABLE void setLength(int newLength);

    // --- Clipboard (mirrors on_actionCut/Copy/Paste_triggered) ---

    Q_INVOKABLE void cut();
    Q_INVOKABLE void copy();
    /// Paste is not undoable in the original either (on_actionPaste_triggered() never pushes a
    /// SetCellsCommand) -- preserved as-is, not a gap this port introduces.
    Q_INVOKABLE void paste();

    // --- File I/O + export ---
    // Mirror BewavedDolphin's own on_action{Save,SaveAs,Load,ExportToPng,ExportToPdf}_triggered
    // handlers, own dialog and all -- FileDialogs::provider() already works unmodified under
    // wiredpanda (QuickDialogProvider ignores the QWidget* parent argument these take,
    // same precedent QuickExportController's own export actions already established).

    /// Saves the waveform to the current file.
    Q_INVOKABLE void save();
    /// Saves the waveform to a new .dolphin file.
    Q_INVOKABLE void saveAs();
    /// Opens a .dolphin file.
    Q_INVOKABLE void load();
    /// Exports the waveform to a PNG file.
    Q_INVOKABLE void exportToPng();
    /// Exports the waveform to a PDF file.
    Q_INVOKABLE void exportToPdf();

    /// Prompts the user to save unsaved changes; returns \c false if cancelled.
    /// Uses Dialogs::provider()->choice() (Yes/No/Cancel) instead of QMessageBox::question --
    /// "Yes"/"No" read naturally against this method's own "Save simulation before closing?"
    /// question text, the same button-label substitution QuickWorkspaceManager's own close-tab
    /// prompts already use for the analogous Widgets QMessageBox::Save/Discard/Cancel triple.
    Q_INVOKABLE bool checkSave();

    // --- MCP-only accessors (App/BeWavedDolphin/BeWavedDolphin.h's own "MCP Interface" section) ---

    /// A labelled waveform signal: a row label and its per-column 0/1 values.
    struct Signal {
        QString label;
        QVector<int> values;
    };
    /// A read-only snapshot of the computed waveform, for automation/export consumers.
    struct WaveformSnapshot {
        QList<Signal> inputs;
        QList<Signal> outputs;
    };

    /// A read-only snapshot of the computed waveform, for automation/export consumers.
    [[nodiscard]] WaveformSnapshot snapshot(int duration) const;
    /// Returns the row index of the input signal labelled \a label.
    [[nodiscard]] int inputRow(const QString &label) const;
    /// Writes the current waveform as plain text, one row per signal.
    void saveToTxt(QTextStream &stream) const;

signals:
    void modelChanged();
    void zoomChanged();
    void numberModeChanged();
    void hasSelectionChanged();
    void undoRedoStateChanged();
    void statusMessageChanged();
    void currentFileChanged();

private:
    void loadNewTable(const QStringList &inputLabels, const QStringList &outputLabels);
    /// Shows a transient status message, mirrors QuickAppController::showStatusMessage()'s
    /// identical single-shot-timer contract.
    void showStatusMessage(const QString &message, int timeout);
    /// Opens the .dolphin/.csv file at \a fileName and loads its waveform data.
    void loadFile(const QString &fileName);
    /// Applies parsed file \a fileData to the model (sets length, fills inputs, re-runs).
    void applyWaveformData(const DolphinSerializer::WaveformData &fileData);
    /// Shared tail of setSelectedTo0/1/invertSelected: snapshots the selection, applies \a
    /// valueFn, and pushes the resulting SetCellsCommand. Mirrors BewavedDolphin::
    /// applyToSelectedCells() exactly.
    void applyToSelectedCells(const std::function<int(int)> &valueFn);
    /// The real two-argument worker -- setLength(int) above always passes runSimulation=true
    /// (on_actionSetLength_triggered()'s own call); combinational()/autoCrop() call this
    /// directly with the original's own true per-caller runSimulation argument.
    void setLengthInternal(int newLength, bool runSimulation);
    /// Links the waveform editor to the .dolphin file at \a fileName.
    void associateToWiRedPanda(const QString &fileName);
    /// Returns \c false if any element in m_inputs/m_outputs was deleted from the live scene
    /// since prepare() snapshotted them (e.g. via the main canvas or an MCP client -- neither
    /// path is blocked by this window's modality, which only affects interactive GUI input).
    /// run()/saveToTxt() must check this before dereferencing those pointers via sweep().
    [[nodiscard]] bool elementsStillLive() const;

    QPointer<CanvasItem> m_canvas;
    Simulation *m_simulation = nullptr;
    std::unique_ptr<WaveformSimulator> m_simDriver;
    SignalModel *m_model = nullptr; // owned via QObject parenting (this)
    QItemSelectionModel *m_selectionModel = nullptr; // owned via QObject parenting (this)
    QModelIndex m_selectionAnchor; // set by selectCell()'s plain-click branch; anchors Shift-click ranges
    QUndoStack m_undoStack;
    QVector<GraphicElement *> m_outputs;
    QVector<GraphicElementInput *> m_inputs;
    int m_inputPorts = 0;
    /// The table's row layout, one entry per row: element, port, side and per-port label.
    /// Single source of truth shared with the sweep and the snapshot API; see
    /// DolphinModelBuilder::Row. v2's WaveformSimulator::sweep() takes this directly (not the
    /// inputs/outputs/inputPorts tuple the original branch's port used at the point it was
    /// authored) -- see WaveformSimulator.h's own doc comment.
    QVector<DolphinModelBuilder::Row> m_rows;
    int m_length = 32;
    bool m_numberMode = false;
    std::unique_ptr<QuickDolphinZoom> m_zoom;
    QFileInfo m_currentFile;
    QString m_statusMessage;
    QTimer m_statusMessageTimer; // single-shot; restarted on every showStatusMessage() call
    DolphinHost *m_host = nullptr; // non-owning; set once via setHost()
    bool m_edited = false; // true if the waveform has unsaved changes -- drives checkSave()
};
