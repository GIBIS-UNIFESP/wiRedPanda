// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QDir>
#include <QMainWindow>
#include <QSpacerItem>
#include <QTranslator>

#include "App/UI/MainWindowUI.h"

class ElementLabel;
class RecentFiles;
class WorkSpace;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit MainWindow(const QString &fileName = {}, QWidget *parent = nullptr);
    ~MainWindow() override;

    //! Sets the main window as visible, as well as its child widgets. Cleans the editor.
    void show();

    // --- Tab Management ---

    //! Creates a new tab with the given tab_name. Used by new and load actions.
    void createNewTab();

    WorkSpace *currentTab() const;
    bool closeFiles();

    // --- File Operations ---

    //! Returns the file name of the currently loaded Panda file.
    QFileInfo currentFile() const;

    //! Returns the dir name of the currently loaded Panda file.
    QDir currentDir() const;

    //! Sets the current file to the given value.
    //! Mostly used by `loadPandaFile` and clearing functions
    void setCurrentFile(const QFileInfo &fileInfo);

    //! Saves the project to a .panda file. Removes the autosave file in the process.
    void save(const QString &fileName = {});

    //! Loads a .panda file
    void loadPandaFile(const QString &fileName);

    //! Opens a message box asking the user if he wishes to save his progress
    int confirmSave(const bool multiple = true);

    // --- Export ---

    //! Exports the current simulation to an
    void exportToArduino(QString fileName);

    //! Saves the current beWavedDolphin (waveform simulator) file
    void exportToWaveFormFile(const QString &fileName);

    void exportToWaveFormTerminal();

    // --- BeWavedDolphin Integration ---

    QString dolphinFileName();
    void setDolphinFileName(const QString &fileName);

    // --- Translation ---

    void loadTranslation(const QString &language);
    QString getLanguageDisplayName(const QString &langCode) const;
    QString getLanguageFlagIcon(const QString &langCode) const;
    QStringList getAvailableLanguages() const;
    void populateLanguageMenu();
    void retranslateUi();

    // --- Palette ---

    void populateMenu(QSpacerItem *spacer, const QStringList &names, QLayout *layout);

    // --- View / Rendering ---

    void setFastMode(const bool fastMode);

    // --- Qt Event Override ---

    bool event(QEvent *event) override;

signals:
    // --- Signals ---

    void addRecentFile(const QString &fileName);

protected:
    // --- Protected Interface ---

    void closeEvent(QCloseEvent *event) override;

private:
    Q_DISABLE_COPY(MainWindow)

    // --- Tab Lifecycle ---

    //! Adds undo and redo of selected tab into the UI menu.
    void addUndoRedoMenu();

    //! Removes undo and redo of current tab from the UI menu.
    void removeUndoRedoMenu();

    //! Function used to disconnect elements of current tab, to safely change or close a tab.
    void disconnectTab();

    //! Function called as a tab is selected. The tab is connected to the UI.
    void connectTab();

    void tabChanged(const int newTabIndex);
    bool closeTab(const int tabIndex);
    int closeTabAnyway();
    bool hasModifiedFiles();

    // --- File Helpers ---

    void loadAutosaveFiles();
    void createRecentFileActions();
    void updateRecentFileActions();
    void openRecentFile();
    void removeICFile(const QString &icFileName);

    // --- Settings & Theme ---

    void updateSettings();
    void updateTheme();
    static void on_actionDarkTheme_triggered();
    static void on_actionLightTheme_triggered();

    // --- Action Handlers ---

    // File menu
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionReloadFile_triggered();
    void on_actionExit_triggered();

    // Export actions
    void on_actionExportToArduino_triggered();
    void on_actionExportToImage_triggered();
    void on_actionExportToPdf_triggered();

    // Edit actions
    void on_actionSelectAll_triggered();
    void on_actionFlipHorizontally_triggered();
    void on_actionFlipVertically_triggered();
    void on_actionRotateLeft_triggered();
    void on_actionRotateRight_triggered();

    // View actions
    void on_actionZoomIn_triggered() const;
    void on_actionZoomOut_triggered() const;
    void on_actionResetZoom_triggered() const;
    void on_actionFullscreen_triggered();
    void on_actionFastMode_triggered(const bool checked);
    void on_actionGates_triggered(const bool checked);
    void on_actionWires_triggered(const bool checked);
    void on_actionLabelsUnderIcons_triggered(const bool checked);
    void zoomChanged();

    // Simulation actions
    void on_actionPlay_toggled(const bool checked);
    void on_actionWaveform_triggered();
    void on_actionMute_triggered(const bool checked);
    void on_actionRestart_triggered();

    // Help/about actions
    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();
    void on_actionShortcuts_and_Tips_triggered();
    void on_actionReportTranslationError_triggered();
    void aboutThisVersion();

    // IC panel actions
    void on_pushButtonAddIC_clicked();
    void on_pushButtonRemoveIC_clicked();

    // Search actions
    void on_lineEditSearch_returnPressed();
    void on_lineEditSearch_textChanged(const QString &text);

    // Internal helpers
    void backgroundSimulation();
    //! Returns the index of a tab by its widget object name, or -1 if not found
    int getTabIndex(const QString &objectName) const;
    void populateLeftMenu();
    void updateICList();

    // --- Members ---

    // Core UI
    std::unique_ptr<MainWindowUi> m_ui;

    // Translators
    QTranslator *m_pandaTranslator = nullptr;
    QTranslator *m_qtTranslator = nullptr;

    // Sub-controllers
    RecentFiles *m_recentFiles = nullptr;

    // Tab state
    QFileInfo m_currentFile;
    WorkSpace *m_currentTab = nullptr;
    int m_tabIndex = -1;
    int m_lastTabIndex = -1;
};
