// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickAppController: QML-facing root object tying the Quick chrome together.
 */

#pragma once

#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QRect>
#include <QString>
#include <QStringList>

#include "App/Core/Enums.h"
#include "App/IO/RecentFiles.h"
#include "App/QuickShell/Chrome/QuickElementEditor.h"
#include "App/QuickShell/Chrome/QuickElementPalette.h"
#include "App/QuickShell/Chrome/QuickExportController.h"
#include "App/QuickShell/Chrome/QuickICPreview.h"
#include "App/QuickShell/Chrome/QuickMainWindowHost.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/QuickShell/Chrome/QuickWorkspaceManager.h"

class CanvasItem;

/// One entry in examplesList(). A real QML value type -- not QVariantMap -- so a
/// delegate's `required property exampleEntry modelData` gives qmlcachegen a concrete
/// field layout to compile against. See project_qml_aot_compilation_fusion_style_pin
/// memory.
class ExampleEntry
{
    Q_GADGET
    QML_VALUE_TYPE(exampleEntry)

    Q_PROPERTY(QString title READ title FINAL)
    Q_PROPERTY(QString path READ path FINAL)

public:
    ExampleEntry() = default;
    ExampleEntry(QString title, QString path)
        : m_title(std::move(title))
        , m_path(std::move(path))
    {
    }

    [[nodiscard]] QString title() const { return m_title; }
    [[nodiscard]] QString path() const { return m_path; }

private:
    QString m_title;
    QString m_path;
};

/**
 * \class QuickAppController
 * \brief Owns the Quick chrome's document model and export workflow, exposes them to QML,
 * and re-binds reactive UI state (undo/redo, window title) to whichever tab is current --
 * the CanvasItem-side counterpart of MainWindow + SceneUiBinder's tab-rebinding half.
 *
 * \details Implements QuickMainWindowHost directly on itself (same multi-inheritance pattern
 * QuickDialogProvider already uses for DialogProvider/FileDialogProvider) rather than via a
 * separate object, since there's exactly one instance of this class per running app. Not
 * every MainWindow/SceneUiBinder responsibility is here yet -- see this class's individual
 * method doc comments for what's deliberately deferred to later Phase 4 sub-steps (element
 * editor rebinding needs sub-step 5's property panel to exist; View-menu toggles, Theme/
 * Language menus, and the Learn menu need sub-steps 4/8/Phase 5's chrome).
 */
class QuickAppController : public QObject, public QuickMainWindowHost
{
    Q_OBJECT

    // FINAL on every property here: QuickAppController is never subclassed, so QML's
    // shadow-check (qqmljsshadowcheck.cpp) can trust these types instead of treating
    // any further lookup through them as unresolvable "var" -- see
    // project_qml_aot_compilation_fusion_style_pin memory for how this was verified.
    Q_PROPERTY(QuickWorkSpace *currentTab READ currentTab NOTIFY currentTabChanged FINAL)
    Q_PROPERTY(int tabCount READ tabCount NOTIFY tabsChanged FINAL)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentTabChanged FINAL)
    Q_PROPERTY(QString windowTitle READ windowTitle NOTIFY windowTitleChanged FINAL)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY undoRedoStateChanged FINAL)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY undoRedoStateChanged FINAL)
    Q_PROPERTY(QString undoText READ undoText NOTIFY undoRedoStateChanged FINAL)
    Q_PROPERTY(QString redoText READ redoText NOTIFY undoRedoStateChanged FINAL)
    Q_PROPERTY(QStringList recentFiles READ recentFiles NOTIFY recentFilesChanged FINAL)
    Q_PROPERTY(bool simulationRunning READ isSimulationRunning WRITE setSimulationRunning NOTIFY simulationRunningChanged FINAL)
    Q_PROPERTY(bool backgroundSimulationEnabled READ isBackgroundSimulationEnabled WRITE setBackgroundSimulationEnabled NOTIFY backgroundSimulationEnabledChanged FINAL)
    Q_PROPERTY(QuickElementPalette *elementPalette READ elementPalette CONSTANT FINAL)
    Q_PROPERTY(QuickElementEditor *elementEditor READ elementEditor CONSTANT FINAL)
    Q_PROPERTY(QuickICPreview *icPreview READ icPreview CONSTANT FINAL)

public:
    explicit QuickAppController(QObject *parent = nullptr);

    // --- QuickMainWindowHost ---
    QuickWorkSpace *currentTab() const override;
    QFileInfo currentFile() const override;
    QDir currentDir() const override;
    void showStatusMessage(const QString &message, int timeout) override;

    // --- QML-facing accessors ---
    Q_INVOKABLE QuickWorkSpace *tabAt(int index) const { return m_workspaceManager.tabAt(index); }
    [[nodiscard]] int tabCount() const { return m_workspaceManager.count(); }
    [[nodiscard]] int currentIndex() const { return m_workspaceManager.currentIndex(); }
    void setCurrentIndex(int index) { m_workspaceManager.setCurrentIndex(index); }
    [[nodiscard]] QString windowTitle() const;
    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;
    [[nodiscard]] QString undoText() const;
    [[nodiscard]] QString redoText() const;

    /// Rich-text body of the "Shortcuts and Tips" dialog. Mirrors MainWindow::shortcutsHelpHtml(),
    /// scoped to the shortcuts this sub-step actually wires (the rest of MainWindow's version is
    /// generated from its own QAction set, which doesn't exist here).
    Q_INVOKABLE QString shortcutsHelpHtml() const;

    // Not const: RecentFiles::recentFiles() isn't const-qualified upstream either.
    [[nodiscard]] QStringList recentFiles() { return m_recentFiles.recentFiles(); }

    /// Bundled example .panda files, prettified for display. Mirrors
    /// MainWindow::setupExamplesMenu()'s title-prettification logic exactly (word-split on
    /// '-'/'_', capitalize each word). Each entry is an ExampleEntry; QML iterates this
    /// directly as a Repeater model. Not reactive (no NOTIFY) since the bundled example set
    /// never changes at runtime, unlike recentFiles.
    Q_INVOKABLE QList<ExampleEntry> examplesList() const;

    /// Restores the persisted window geometry (Settings::quickWindowGeometry()), or an
    /// invalid/empty QRect if none was ever saved -- QML checks width/height before applying.
    Q_INVOKABLE QRect restoreWindowGeometry() const;
    Q_INVOKABLE void saveWindowGeometry(int x, int y, int width, int height);

    /// Mirrors MainWindow::closeEvent()'s logic: confirms exit (a plain yes/no) if nothing is
    /// modified, or runs the per-tab close-and-save-prompt flow (closeFiles()) otherwise.
    /// Returns true if it's OK to actually close the window.
    Q_INVOKABLE bool confirmClose();

    [[nodiscard]] bool isSimulationRunning() const { return m_simulationRunning; }
    /// The user's Play/pause intent (mirrors actionPlay's checked state, which starts checked
    /// in MainWindow.cpp). Starts/stops the *current* tab's real Simulation via the same
    /// Simulation::start()/stop() entry points MainWindow::on_actionPlay_toggled() uses --
    /// these are already public, unlike an earlier draft of this plan assumed.
    void setSimulationRunning(bool running);

    [[nodiscard]] bool isBackgroundSimulationEnabled() const { return m_backgroundSimulationEnabled; }
    /// Mirrors actionBackground_Simulation: when false (the default), losing window focus
    /// pauses the current tab's simulation; when true, it keeps running (e.g. for demoing a
    /// circuit on a second display).
    void setBackgroundSimulationEnabled(bool enabled);

    /// Mirrors MainWindow::event()'s QEvent::WindowActivate/WindowDeactivate handling. Called
    /// from Main.qml's ApplicationWindow.onActiveChanged. Pauses/resumes the current tab's
    /// simulation without touching simulationRunning itself, so the user's Play intent
    /// survives a temporary focus loss -- unless backgroundSimulationEnabled opts out of
    /// pausing on deactivate.
    Q_INVOKABLE void handleWindowActiveChanged(bool active);

    [[nodiscard]] QuickElementPalette *elementPalette() { return &m_palette; }
    [[nodiscard]] QuickElementEditor *elementEditor() { return &m_elementEditor; }
    [[nodiscard]] QuickICPreview *icPreview() { return &m_icPreview; }

    /// Adds one element from a palette entry to the current tab's canvas. \a type/\a
    /// icFileName/\a isEmbedded mirror QuickElementPalette's entry fields exactly (QML passes
    /// them straight through from whichever entry was dragged or double-clicked); \a x/\a y
    /// are canvas-local coordinates (== canvasHost-local, since CanvasItem has no pan/zoom
    /// transform yet). No-op if there is no current tab. Mirrors the connect(m_palette,
    /// &ElementPalette::addElementRequested, ...) lambda in MainWindow's constructor.
    Q_INVOKABLE void addElementToCurrentTab(int type, const QString &icFileName, bool isEmbedded, qreal x, qreal y);

    /// Returns true if the system clipboard holds a pasteable wiRedPanda payload. Mirrors
    /// Scene::contextMenu()'s ClipboardManager::canPaste(QApplication::clipboard()->mimeData())
    /// check, backing the empty-canvas context menu's Paste item.
    Q_INVOKABLE static bool canPaste();

public slots:
    // --- File menu ---
    void newTab() { m_workspaceManager.newTab(); }
    void openFile() { m_workspaceManager.openFile(); }
    void saveFile() { m_workspaceManager.saveFile(); }
    void saveFileAs() { m_workspaceManager.saveFileAs(); }
    void reloadFile() { m_workspaceManager.reloadFile(); }
    void openRecentFile(const QString &path) { m_workspaceManager.loadPandaFile(path); }
    /// Closes the tab at \a index (prompting to save if needed). Mirrors the MainWindow tab
    /// close button / Ctrl+W path -- there's no tab-bar close button yet (sub-step 4-adjacent
    /// chrome), but the underlying operation is real and exercised here.
    bool closeTab(int index) { return m_workspaceManager.closeTab(index); }

    // --- Edit menu (the six scene-property shortcuts -- [ ] { } < > -- are NOT here: they're
    // already implemented directly in CanvasItem::keyPressEvent() since Phase 3, see the plan's
    // Phase 4 sub-step 3 note) ---
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void duplicateSelection();
    void deleteSelection();
    void selectAll();

    // --- Transform menu ---
    void rotateRight();
    void rotateLeft();
    void flipHorizontal();
    void flipVertical();

    // --- Align / Distribute menu ---
    void alignLeft();
    void alignRight();
    void alignTop();
    void alignBottom();
    void alignHorizontalCenter();
    void alignVerticalCenter();
    void distributeHorizontally();
    void distributeVertically();

    // --- Simulation menu ---
    void mute(bool muted);
    void restartSimulation();

    // --- Export menu ---
    void exportArduino() { m_exportController.exportArduinoDialog(); }
    void exportSystemVerilog() { m_exportController.exportSystemVerilogDialog(); }

signals:
    void currentTabChanged();
    void tabsChanged();
    void windowTitleChanged();
    void undoRedoStateChanged();
    void recentFilesChanged();
    void simulationRunningChanged();
    void backgroundSimulationEnabledChanged();

private:
    /// Returns the active tab's canvas, or nullptr. Shared by every Edit/Transform/Align
    /// action above -- mirrors MainWindow::connectSceneAction()'s "guarded by a current-tab
    /// check" pattern.
    CanvasItem *activeCanvas() const;

    /// Disconnects the previous tab's undo-stack signal wiring and reconnects to the new
    /// current tab's, re-emitting undoRedoStateChanged()/windowTitleChanged() as needed. Also
    /// stops the previously-bound tab's simulation and starts the new one's if
    /// simulationRunning is set, mirroring SceneUiBinder::unbind()/bind()'s play-state half,
    /// and rebinds m_elementEditor/m_icPreview to the new canvas (QuickElementEditor::setCanvas()
    /// itself refreshes from the new canvas's current selection; QuickICPreview::setCanvas()
    /// hides any popup left pending/visible from the previous tab). The CanvasItem-side
    /// counterpart of SceneUiBinder::bind()/unbind() -- zoom/mute action sync and status-bar
    /// info are the remaining pieces, deferred to when their chrome exists (sub-step 7).
    void bindCurrentTab();

    /// Literal start()/stop() call on the current tab's Simulation, mirroring
    /// MainWindow::on_actionPlay_toggled()'s body. Shared by setSimulationRunning() (the user
    /// toggling Play) and handleWindowActiveChanged() (a temporary focus-driven pause/resume
    /// that must NOT alter m_simulationRunning itself).
    void applySimulationRunningState(bool running);

    QuickWorkspaceManager m_workspaceManager;
    QuickExportController m_exportController;
    QuickElementPalette m_palette;
    QuickElementEditor m_elementEditor;
    QuickICPreview m_icPreview;
    RecentFiles m_recentFiles;
    QList<QMetaObject::Connection> m_tabConnections;
    bool m_simulationRunning = true;
    bool m_backgroundSimulationEnabled = false;
    /// QPointer, not a raw CanvasItem*: QuickWorkspaceManager::closeTab() erases the closed
    /// QuickWorkSpace (and therefore its CanvasItem) before emitting currentTabChanged for
    /// whatever tab becomes current next, so the previously-bound canvas may already be
    /// destroyed by the time bindCurrentTab() runs.
    QPointer<CanvasItem> m_boundCanvas;
};

/**
 * \brief Exposes the single, C++-constructed \c QuickAppController instance (see Main.cpp) to
 * QML as the \c AppController singleton, replacing the earlier context-property approach --
 * context properties are invisible to qmllint/the QML Language Server, which the Qt docs
 * identify as their central drawback. Uses the \c QML_FOREIGN wrapper pattern Qt documents for
 * exposing a pre-existing, externally-owned instance (rather than one the engine constructs
 * itself via create()): \c s_instance is set once, before the QML engine loads Main.qml.
 */
struct AppControllerForeign
{
    Q_GADGET
    QML_FOREIGN(QuickAppController)
    QML_SINGLETON
    QML_NAMED_ELEMENT(AppController)

public:
    inline static QuickAppController *s_instance = nullptr;

    static QuickAppController *create(QQmlEngine *, QJSEngine *)
    {
        Q_ASSERT(s_instance);
        // Same rule as QuickAppController's own m_palette/m_elementEditor (see this class's
        // constructor): a QObject with no parent becomes eligible for JavaScriptOwnership --
        // and GC deletion -- the moment QML first sees it. s_instance is a static-storage
        // object Main.cpp owns for the whole process lifetime, so it must stay C++-owned.
        QQmlEngine::setObjectOwnership(s_instance, QQmlEngine::CppOwnership);
        return s_instance;
    }
};
