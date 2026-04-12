// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Main application window providing menus, toolbars, and tab management.
 */

#pragma once

#include <memory>

#include <QDir>
#include <QMainWindow>
#include <QSpacerItem>
#include <QUrl>

#include "App/UI/MainWindowUI.h"

class ElementLabel;
class ElementPalette;
class IC;
class LanguageManager;
class QShortcut;
class RecentFiles;
class WorkSpace;

/**
 * \class MainWindow
 * \brief The top-level application window hosting the tab bar, menus, element palette, and editor.
 *
 * \details MainWindow orchestrates the full editing workflow: it owns the tab widget (each tab is a
 * WorkSpace), connects undo/redo stacks, handles file open/save/export, manages translations,
 * and integrates the element palette, element editor, and IC list panel.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /**
     * \brief Constructs the MainWindow and optionally opens \a fileName on start.
     * \param fileName Optional .panda file to load at startup.
     * \param parent   Optional parent widget.
     */
    explicit MainWindow(const QString &fileName = {}, QWidget *parent = nullptr);
    ~MainWindow() override;

    /// Shows the window and initializes child widget state.
    void show();

    // --- Tab Management ---

    /// Creates a new empty circuit tab.
    void createNewTab();

    /// Returns the currently visible WorkSpace tab, or nullptr.
    WorkSpace *currentTab() const;

    /// Closes all tabs, prompting to save if needed. Returns \c false if cancelled.
    bool closeFiles();

    // --- File Operations ---

    /**
     * \brief Saves the current circuit to \a fileName (or prompts if empty).
     * \param fileName Absolute path; empty triggers a Save-As dialog.
     */
    void save(const QString &fileName = {});

    /**
     * \brief Loads a .panda circuit file into the current or a new tab.
     * \param fileName Absolute path to the .panda file.
     */
    void loadPandaFile(const QString &fileName);

    /// Opens an embedded IC blob for editing in a new tab.
    void openICInTab(const QString &blobName, int icElementId, const QByteArray &blob);

    /**
     * \brief Prompts the user to save unsaved changes before closing.
     * \param multiple \c true when multiple tabs may be affected.
     * \return QMessageBox button code: Save, Discard, or Cancel.
     */
    int confirmSave(const bool multiple);

    /// Returns the QFileInfo of the currently active .panda file.
    QFileInfo currentFile() const;

    /// Returns the directory of the currently active .panda file.
    QDir currentDir() const;

    /**
     * \brief Updates the tracked current file to \a fileInfo.
     * \param fileInfo New file info (used after load or save-as).
     */
    void setCurrentFile(const QFileInfo &fileInfo);

    // --- Export ---

    /**
     * \brief Generates Arduino sketch code for the current circuit.
     * \param fileName Output .ino file path.
     */
    void exportToArduino(QString fileName);

    /**
     * \brief Generates SystemVerilog code for the current circuit.
     * \param fileName Output .sv file path.
     */
    void exportToSystemVerilog(QString fileName);

    /**
     * \brief Saves the BeWavedDolphin waveform session to \a fileName.
     * \param fileName Output .dolphin file path.
     */
    void exportToWaveFormFile(const QString &fileName);

    /// Exports the current waveform simulation to standard output (terminal).
    void exportToWaveFormTerminal();

    // --- BeWavedDolphin Integration ---

    /// Returns the BeWavedDolphin file name associated with the current tab.
    QString dolphinFileName();

    /**
     * \brief Associates \a fileName as the BeWavedDolphin file for the current tab.
     * \param fileName Path to the .dolphin file.
     */
    void setDolphinFileName(const QString &fileName);

    // --- Translation ---

    /**
     * \brief Loads and installs the Qt translation for \a language.
     * \param language BCP-47 language code.
     */
    void loadTranslation(const QString &language);

    /// Populates the Language submenu with available translations.
    void populateLanguageMenu();

    // --- Palette ---

    /**
     * \brief Fills \a layout with ElementLabel buttons from \a names, preceded by \a spacer.
     * \param spacer   Spacer item to insert before the buttons.
     * \param names    Element type names to add.
     * \param layout   Target layout.
     */
    void populateMenu(QSpacerItem *spacer, const QStringList &names, QLayout *layout);

    // --- View / Rendering ---

    /// Re-applies all translatable strings to the UI after a language change.
    void retranslateUi();

    /**
     * \brief Enables or disables fast (non-antialiased) rendering for all views.
     * \param fastMode \c true to use fast rendering.
     */
    void setFastMode(const bool fastMode);

    // --- Qt Event Override ---

    /// \reimp
    bool event(QEvent *event) override;

signals:
    /**
     * \brief Emitted when a file is successfully loaded or saved.
     * \param fileName Absolute path of the affected file.
     */
    void addRecentFile(const QString &fileName);

protected:
    // --- Protected Interface ---

    /// \reimp
    void closeEvent(QCloseEvent *event) override;

private:
    Q_DISABLE_COPY(MainWindow)

    // --- Tab Lifecycle ---

    bool closeTab(const int tabIndex);
    int closeTabAnyway();
    void tabChanged(const int newTabIndex);
    void addUndoRedoMenu();
    void removeUndoRedoMenu();
    void disconnectTab();
    void connectTab();

    /// Returns the file info to use for populating the file-based IC palette.
    /// For inline IC tabs this returns the parent workspace's file info.
    QFileInfo icListFile() const;

    /// Connects \a action to \a method on the current tab's scene (guarded by m_currentTab check).
    void connectSceneAction(QAction *action, void (Scene::*method)());

    // --- File Helpers ---

    /// Appends \a extension (e.g. ".panda") to \a fileName if not already present.
    static void ensureFileExtension(QString &fileName, const QString &extension);

    /// Returns the first selected IC element, or nullptr if none is selected or
    /// if there is no current tab or the first selected element is not an IC.
    IC *getSelectedIC() const;

    /// Shows or hides the IC management buttons (Add, Remove, MakeSelfContained).
    void setICButtonsVisible(bool visible);

    bool hasModifiedFiles();
    void loadAutosaveFiles();
    void createRecentFileActions();
    void updateRecentFileActions();
    void openRecentFile();
    void removeICFile(const QString &icFileName);
    QString resolveUniqueBlobName(const QString &initialName, Scene *scene);
    void embedSelectedIC();
    void extractSelectedIC();
    void makeSelfContained();
    void embedICByFile(const QString &fileName);
    void extractICByBlobName(const QString &blobName);
    void showUpdateDialog(const QString &latestVersion, const QUrl &releaseUrl);

    // --- Settings & Theme ---

    void updateSettings();
    void updateTheme();
    void zoomChanged();
    void aboutThisVersion();

    // --- Action Handlers ---

    static void on_actionDarkTheme_triggered();
    static void on_actionLightTheme_triggered();
    static void on_actionSystemTheme_triggered();
    void on_actionAboutQt_triggered();
    void on_actionAbout_triggered();
    void on_actionExit_triggered();
    void on_actionExportToArduino_triggered();
    void on_actionExportToSystemVerilog_triggered();
    void on_actionExportToImage_triggered();
    void on_actionExportToPdf_triggered();
    void on_actionFastMode_triggered(const bool checked);
    void on_actionFlipHorizontally_triggered();
    void on_actionFlipVertically_triggered();
    void on_actionFullscreen_triggered();
    void on_actionGates_triggered(const bool checked);
    void on_actionLabelsUnderIcons_triggered(const bool checked);
    void on_actionMute_triggered(const bool checked);
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionPlay_toggled(const bool checked);
    void on_actionReloadFile_triggered();
    void on_actionReportTranslationError_triggered();
    void on_actionResetZoom_triggered() const;
    void on_actionRestart_triggered();
    void on_actionRotateLeft_triggered();
    void on_actionRotateRight_triggered();
    void on_actionSaveAs_triggered();
    void on_actionSave_triggered();
    void on_actionSelectAll_triggered();
    void on_actionShortcuts_and_Tips_triggered();
    void on_actionWaveform_triggered();
    void on_actionWires_triggered(const bool checked);
    void on_actionZoomIn_triggered() const;
    void on_actionZoomOut_triggered() const;
    void on_pushButtonAddIC_clicked();
    void on_pushButtonRemoveIC_clicked();

#ifdef Q_OS_WASM
    /// Emscripten beforeunload callback — saves window geometry before the browser tab closes.
    static const char *onBeforeUnload(int eventType, const void *reserved, void *userData);
#endif

    // --- Constructor Setup ---

    /// Initialises language/translation: detects system locale, loads .qm file, populates the Language menu.
    void setupLanguage();
    /// Restores window and splitter geometry from the previous session.
    void setupGeometry();
    /// Creates the theme action group, connects ThemeManager, applies fast-mode / label-under-icons state.
    void setupTheme();
    /// Constructs RecentFiles and wires its update signals.
    void setupRecentFiles();
    /// Populates the Examples menu from the Examples/ directory.
    void setupExamplesMenu();
    /// Registers all QShortcut objects (requires m_currentTab to be valid).
    void setupShortcuts();
    /// Connects all QAction and widget signals to their slots.
    void setupConnections();

    // --- Members ---

    std::unique_ptr<MainWindowUi> m_ui;

    ElementPalette  *m_palette         = nullptr;
    LanguageManager *m_languageManager = nullptr;
    RecentFiles     *m_recentFiles     = nullptr;

    WorkSpace *m_currentTab = nullptr;
    int m_tabIndex = -1;

    // Scene-level shortcuts created once in setupShortcuts(), reconnected on tab switch.
    QShortcut *m_prevMainPropShortcut  = nullptr;
    QShortcut *m_nextMainPropShortcut  = nullptr;
    QShortcut *m_prevSecndPropShortcut = nullptr;
    QShortcut *m_nextSecndPropShortcut = nullptr;
    QShortcut *m_changePrevElmShortcut = nullptr;
    QShortcut *m_changeNextElmShortcut = nullptr;
};

