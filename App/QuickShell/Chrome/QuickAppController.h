// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickAppController: QML-facing root object tying the Quick chrome together.
 */

#pragma once

#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QString>

#include "App/QuickShell/Chrome/QuickExportController.h"
#include "App/QuickShell/Chrome/QuickMainWindowHost.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/QuickShell/Chrome/QuickWorkspaceManager.h"

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

    Q_PROPERTY(QuickWorkSpace *currentTab READ currentTab NOTIFY currentTabChanged)
    Q_PROPERTY(int tabCount READ tabCount NOTIFY tabsChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentTabChanged)
    Q_PROPERTY(QString windowTitle READ windowTitle NOTIFY windowTitleChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY undoRedoStateChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY undoRedoStateChanged)
    Q_PROPERTY(QString undoText READ undoText NOTIFY undoRedoStateChanged)
    Q_PROPERTY(QString redoText READ redoText NOTIFY undoRedoStateChanged)

public:
    explicit QuickAppController(QObject *parent = nullptr);

    // --- QuickMainWindowHost ---
    QuickWorkSpace *currentTab() const override;
    QFileInfo currentFile() const override;
    QDir currentDir() const override;
    void showStatusMessage(const QString &message, int timeout) override;

    // --- QML-facing accessors ---
    Q_INVOKABLE QuickWorkSpace *tabAt(int index) const { return m_workspaceManager.tabAt(index); }
    Q_INVOKABLE QString tabTitle(QuickWorkSpace *tab) const { return m_workspaceManager.tabTitle(tab); }
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

public slots:
    // --- File menu ---
    void newTab() { m_workspaceManager.newTab(); }
    void openFile() { m_workspaceManager.openFile(); }
    void saveFile() { m_workspaceManager.saveFile(); }
    void saveFileAs() { m_workspaceManager.saveFileAs(); }
    void reloadFile() { m_workspaceManager.reloadFile(); }
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

    // --- Simulation menu (Play/pause is deliberately not here: CanvasItem's Simulation has
    // no public indefinite-pause entry point of its own yet, only the RAII SimulationBlocker
    // used internally by topology-mutating commands -- a real gap, not an oversight, tracked
    // for whichever later pass needs it) ---
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

private:
    /// Returns the active tab's canvas, or nullptr. Shared by every Edit/Transform/Align
    /// action above -- mirrors MainWindow::connectSceneAction()'s "guarded by a current-tab
    /// check" pattern.
    class CanvasItem *activeCanvas() const;

    /// Disconnects the previous tab's undo-stack signal wiring and reconnects to the new
    /// current tab's, re-emitting undoRedoStateChanged()/windowTitleChanged() as needed.
    /// The CanvasItem-side counterpart of SceneUiBinder::bind()/unbind()'s undo/redo half --
    /// element-editor rebinding, zoom/mute/play action sync, and status-bar info are the
    /// other half, deferred to when their chrome exists (sub-steps 4/5/7).
    void bindCurrentTab();

    QuickWorkspaceManager m_workspaceManager;
    QuickExportController m_exportController;
    QList<QMetaObject::Connection> m_tabConnections;
};
