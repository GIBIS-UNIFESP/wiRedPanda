// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickWorkspaceManager: CanvasItem-side port of App/UI/WorkspaceManager.h.
 */

#pragma once

#include <memory>
#include <vector>

#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QString>

#include "App/QuickShell/Chrome/DialogProvider.h"

class QuickWorkSpace;

/**
 * \class QuickWorkspaceManager
 * \brief Owns the document/tab model: the current tab, tab creation/closing/switching, and
 * the file open/save/reload/autosave workflow. CanvasItem-side port of WorkspaceManager.
 *
 * \details Copy-and-adapt port (same precedent as CanvasCommands/CanvasICRegistry/
 * QuickWorkSpace), not a modification of the production WorkspaceManager. The real
 * adaptation here isn't the file-operation logic (that ports close to unchanged) -- it's that
 * WorkspaceManager is built directly out of a QTabWidget (constructor takes one, every method
 * calls straight through to `m_tab->count()`/`widget(i)`/`addTab()`/`setCurrentIndex()`/...),
 * not just parented by one. This class replaces that with a plain ordered
 * `QVector<std::unique_ptr<QuickWorkSpace>>` plus a current index -- no QTabWidget-shaped
 * dependency to satisfy, and a tab's display title is computed on demand (tabTitle()) from
 * the QuickWorkSpace's own state rather than tracked as separately-mutable QTabWidget text
 * that has to be kept in sync by hand.
 *
 * Deliberately has no MainWindowHost-shaped dependency yet: every real WorkspaceManager call
 * site that needs one only does so for palette refresh (`palette()->updateICList()`/
 * `updateEmbeddedICList()`), status-bar messages, or IC-button visibility -- none of which
 * exist yet (Phase 4 sub-steps 3/4). Those call sites are marked with a comment at each
 * no-op point rather than silently dropped; wiring them up is those later sub-steps' job, not
 * a redesign of this class.
 */
class QuickWorkspaceManager : public QObject
{
    Q_OBJECT

public:
    explicit QuickWorkspaceManager(QObject *parent = nullptr);

    // --- Tab list access (no QTabWidget to lean on, see this class's doc comment) ---

    [[nodiscard]] int count() const { return static_cast<int>(m_tabs.size()); }
    [[nodiscard]] QuickWorkSpace *tabAt(int index) const;
    [[nodiscard]] int indexOf(QuickWorkSpace *tab) const;
    [[nodiscard]] int currentIndex() const { return m_currentIndex; }
    /// Switches the current tab and emits currentTabChanged(). Mirrors what setting
    /// QTabWidget::currentIndex used to trigger via onCurrentIndexChanged().
    void setCurrentIndex(int index);

    // --- Accessors (mirror WorkspaceManager's MainWindowHost-delegate surface) ---

    [[nodiscard]] QuickWorkSpace *currentTab() const { return m_currentTab; }
    [[nodiscard]] QFileInfo currentFile() const;
    /// Display title for \a tab (file name, numbered placeholder, or "[blob]" for an inline
    /// IC tab), without the unsaved "*" marker. Mirrors WorkspaceManager::displayName(), but
    /// callable for any tab (not just the current one) since there's no QTabWidget tab-text
    /// cache to read instead.
    [[nodiscard]] QString tabTitle(QuickWorkSpace *tab) const;
    [[nodiscard]] QString currentTabName() const;
    [[nodiscard]] QDir currentDir() const;
    [[nodiscard]] QFileInfo icListFile() const;
    [[nodiscard]] QString dolphinFileName() const;
    void setDolphinFileName(const QString &fileName);

    // --- Headless operations (mirror WorkspaceManager's, called by CLI batch/MCP/tests) ---

    void createNewTab();
    void loadPandaFile(const QString &fileName);
    void openICInTab(const QString &blobName, int icElementId, const QByteArray &blob);
    void save(const QString &fileName = {});
    void loadAutosaveFiles();

    [[nodiscard]] bool hasModifiedFiles();
    bool closeFiles();
    /// Asks "save before closing?"; \a multiple offers Yes-to-all/No-to-all too. Mirrors
    /// WorkspaceManager::confirmSave(), returning DialogButton instead of a raw
    /// QMessageBox::StandardButton int.
    DialogButton confirmSave(bool multiple);

public slots:
    /// Closes the tab at \a tabIndex (prompting to save if needed). Returns false if cancelled.
    bool closeTab(int tabIndex);

    // Interactive file-menu handlers (each guards its own body, mirroring WorkspaceManager's).
    void newTab();
    void openFile();
    void saveFile();
    void saveFileAs();
    void reloadFile();

    /// Updates recent-files/tooltip bookkeeping for the workspace that emitted fileChanged.
    /// Mirrors WorkspaceManager::setCurrentFile() (name collision with QuickWorkSpace's own
    /// private setCurrentFile() is fine, different classes/signatures).
    void onTabFileChanged(const QFileInfo &fileInfo);

signals:
    /// Emitted when the active tab changes (or becomes null); a later sub-step's chrome
    /// rebinds itself here, the same way SceneUiBinder does off WorkspaceManager's signal.
    void currentTabChanged(QuickWorkSpace *tab);
    /// Emitted when a file-backed tab is (re)named, to feed a future RecentFiles list.
    void recentFileAdded(const QString &filePath);
    /// Emitted when the current tab's title or modified state changes.
    void titleChanged();
    /// Emitted whenever the tab list itself changes shape (added/removed/reordered) -- no
    /// QTabWidget to derive this from automatically, so it's explicit. A later sub-step's QML
    /// tab bar rebuilds its view off this.
    void tabsChanged();

private:
    [[nodiscard]] int closeTabAnyway();
    [[nodiscard]] int findTabWithFile(const QString &fileName) const;
    [[nodiscard]] QString nextUntitledTitle() const;
    /// Warns and offers to switch tabs if \a fileName is already open in another tab. Returns
    /// true if a conflict was found (and shown), false if the save should proceed. Simplified
    /// from WorkspaceManager's version: the real one offers a "Switch to Tab" action button
    /// alongside Ok; DialogProvider's choice() has no room for a custom-labeled button yet
    /// (a small, deliberately deferred UX gap -- the conflict is still correctly detected and
    /// blocked, just without the one-click convenience switch).
    bool warnIfOpenInAnotherTab(const QString &fileName);
    /// Resolves \a fileName into a non-empty, ".panda"-suffixed absolute path for the current
    /// tab, prompting via a Save-As dialog if the tab has no path of its own yet. Returns an
    /// empty string if there's nothing to resolve to and the user cancels the dialog.
    QString promptSavePath(const QString &fileName);

    std::vector<std::unique_ptr<QuickWorkSpace>> m_tabs;
    QuickWorkSpace *m_currentTab = nullptr;
    int m_currentIndex = -1;
};
