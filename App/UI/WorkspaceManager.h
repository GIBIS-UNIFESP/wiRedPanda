// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief WorkspaceManager: owns the circuit tabs and their file lifecycle.
 */

#pragma once

#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QString>

class MainWindowHost;
class QByteArray;
class QTabBar;
class QTabWidget;
class WorkSpace;

/**
 * \class WorkspaceManager
 * \brief Owns the document/tab model: the current tab, tab creation/closing/switching,
 * and the file open/save/reload/autosave workflow.
 *
 * \details Extracted from MainWindow via ownership inversion. It holds the current
 * WorkSpace and drives the QTabWidget's contents (it does not own the widget — the UI
 * does). UI feedback (status messages, palette refresh, IC tool-button state, dialog
 * parenting) is reached through MainWindowHost; chrome rebinding is the shell's job,
 * triggered by the currentTabChanged() signal. The public operations are kept callable
 * from MainWindow delegators so the CLI batch mode, the MCP server, and tests are
 * unaffected.
 */
class WorkspaceManager : public QObject
{
    Q_OBJECT

public:
    /// \param tab The tab widget whose contents this manager owns (non-owning).
    /// \param host Application context for status/palette/dialog-parent/IC-button feedback.
    WorkspaceManager(QTabWidget *tab, MainWindowHost &host, QObject *parent = nullptr);

    // --- Accessors (MainWindow's DolphinHost/MainWindowHost delegate here) ---

    [[nodiscard]] WorkSpace *currentTab() const { return m_currentTab; }
    [[nodiscard]] QFileInfo currentFile() const;
    /// Display name of the current tab (file name, numbered placeholder, or "[blob]" for an
    /// inline IC), without the unsaved "*" marker. Used for the window title.
    [[nodiscard]] QString currentTabName() const;
    [[nodiscard]] QDir currentDir() const;
    [[nodiscard]] QFileInfo icListFile() const;
    [[nodiscard]] QString dolphinFileName() const;
    void setDolphinFileName(const QString &fileName);

    // --- Headless operations (called by CLI batch mode, the MCP server, and tests) ---

    void createNewTab();
    void loadPandaFile(const QString &fileName);
    void openICInTab(const QString &blobName, int icElementId, const QByteArray &blob);
    void save(const QString &fileName = {});
    void loadAutosaveFiles();

    [[nodiscard]] bool hasModifiedFiles();
    bool closeFiles();
    int confirmSave(bool multiple);

public slots:
    /// Reacts to QTabWidget::currentChanged: updates the current tab and emits currentTabChanged.
    void onCurrentIndexChanged(int newIndex);
    /// Closes the tab at \a tabIndex (prompting to save if needed). Returns false if cancelled.
    bool closeTab(int tabIndex);

    // Interactive file-menu handlers (each guards its own body).
    void newTab();
    void openFile();
    void saveFile();
    void saveFileAs();
    void reloadFile();

    /// Updates the tab title / tooltip / recent files for the workspace that emitted fileChanged.
    void setCurrentFile(const QFileInfo &fileInfo);

signals:
    /// Emitted when the active tab changes (or becomes null); the shell rebinds the chrome.
    void currentTabChanged(WorkSpace *tab);
    /// Emitted when a file-backed tab is (re)named, to feed the recent-files list.
    void recentFileAdded(const QString &filePath);
    /// Emitted when the current tab's title or modified state changes, so the shell can
    /// refresh the window title.
    void titleChanged();

protected:
    /// Closes the tab under the cursor when its tab bar is middle-clicked.
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    int closeTabAnyway();
    [[nodiscard]] int findTabWithFile(const QString &fileName) const;

    /// Display name for \a ws given its \a fileInfo (file name, numbered placeholder, or
    /// "[blob]" for an inline IC), without the unsaved "*" marker. Shared by the tab text
    /// and the window title so they stay in step.
    [[nodiscard]] QString displayName(const WorkSpace *ws, const QFileInfo &fileInfo) const;

    /// Returns the lowest-unused "New Project" placeholder title (unnumbered, then " 2",
    /// " 3", …) not already shown by another open tab, so fresh tabs stay distinguishable.
    [[nodiscard]] QString nextUntitledTitle() const;

    /// Warns and offers to switch tabs if \a fileName is already open in another tab.
    /// Returns \c true if a conflict was found (and shown), \c false if the save should proceed.
    bool warnIfOpenInAnotherTab(const QString &fileName);

    /// Resolves \a fileName into a non-empty, ".panda"-suffixed absolute path for the
    /// current tab, prompting via a Save-As dialog if the tab has no path of its own yet
    /// (a brand-new project, or one recovered from an autosave file). Returns an empty
    /// string if there's nothing to resolve to and the user cancels the dialog.
    QString promptSavePath(const QString &fileName);

    QTabWidget *m_tab;
    /// The tab bar we install the middle-click filter on, cached at construction so the filter
    /// never dereferences m_tab (whose dynamic type is no longer QTabWidget while it is being
    /// destroyed, which a teardown event reaching the filter would otherwise trip on).
    QTabBar *m_tabBar = nullptr;
    MainWindowHost &m_host;
    WorkSpace *m_currentTab = nullptr;
    int m_tabIndex = -1;
};
