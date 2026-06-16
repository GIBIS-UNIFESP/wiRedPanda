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

private:
    int closeTabAnyway();
    [[nodiscard]] int findTabWithFile(const QString &fileName) const;

    QTabWidget *m_tab;
    MainWindowHost &m_host;
    WorkSpace *m_currentTab = nullptr;
    int m_tabIndex = -1;
};
