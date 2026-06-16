// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Main application window providing menus, toolbars, and tab management.
 */

#pragma once

#include <memory>

#include <QDir>
#include <QMainWindow>
#include <QPointer>
#include <QSpacerItem>

#include "App/BeWavedDolphin/DolphinHost.h"
#include "App/UI/MainWindowHost.h"
#include "App/UI/MainWindowUI.h"

class ElementLabel;
class ElementPalette;
class ExportController;
class ICController;
class ICPreviewPopup;
class LanguageManager;
class RecentFiles;
class SceneUiBinder;
class WorkSpace;

/**
 * \class MainWindow
 * \brief The top-level application window hosting the tab bar, menus, element palette, and editor.
 *
 * \details MainWindow orchestrates the full editing workflow: it owns the tab widget (each tab is a
 * WorkSpace), connects undo/redo stacks, handles file open/save/export, manages translations,
 * and integrates the element palette, element editor, and IC list panel.
 */
class MainWindow : public QMainWindow, public DolphinHost, public MainWindowHost
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

    /// Returns the shared IC-hover preview popup.  Owned by this MainWindow as
    /// a Qt child; auto-nulls (via QPointer) if destroyed before this window.
    ICPreviewPopup *icPreviewPopup() const;

    // --- Tab Management ---

    /// Creates a new empty circuit tab.
    void createNewTab();

    /// Returns the currently visible WorkSpace tab, or nullptr.
    WorkSpace *currentTab() const override;

    /// Closes all tabs, prompting to save if needed. Returns \c false if cancelled.
    bool closeFiles();

    // --- File Operations ---

    /**
     * \brief Saves the current circuit to \a fileName (or prompts if empty).
     * \param fileName Absolute path; empty triggers a Save-As dialog.
     */
    void save(const QString &fileName = {}) override;

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
    QFileInfo currentFile() const override;

    /// Returns the directory of the currently active .panda file.
    QDir currentDir() const override;

    /// \reimp MainWindowHost — parent widget for controller-spawned dialogs.
    QWidget *widget() override;

    /// \reimp MainWindowHost — this window's DolphinHost facet.
    DolphinHost *dolphinHost() override;

    /// \reimp MainWindowHost — the element palette panel.
    ElementPalette *palette() const override;

    /// \reimp MainWindowHost — shows \a message in the status bar for \a timeout ms.
    void showStatusMessage(const QString &message, int timeout) override;

    /// \reimp MainWindowHost — saves the current tab (triggers the Save action).
    void requestSave() override;

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
    QString dolphinFileName() override;

    /**
     * \brief Associates \a fileName as the BeWavedDolphin file for the current tab.
     * \param fileName Path to the .dolphin file.
     */
    void setDolphinFileName(const QString &fileName) override;

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
    void disconnectTab();
    void connectTab();

    /// Returns the file info to use for populating the file-based IC palette.
    /// For inline IC tabs this returns the parent workspace's file info.
    QFileInfo icListFile() const override;

    /// Connects \a action to \a method on the current tab's scene (guarded by m_currentTab check).
    void connectSceneAction(QAction *action, void (Scene::*method)());

    // --- File Helpers ---

    /// Appends \a extension (e.g. ".panda") to \a fileName if not already present.
    static void ensureFileExtension(QString &fileName, const QString &extension);

    /// Shows or hides the IC management buttons (Add, Remove, MakeSelfContained).
    void setICButtonsVisible(bool visible);

    /// Updates the enabled state of Add IC based on current tab's saved-file status.
    void refreshICButtonsEnabled();

    bool hasModifiedFiles();
    void loadAutosaveFiles();
    void createRecentFileActions();
    void updateRecentFileActions();
    void openRecentFile();

    // --- Settings & Theme ---

    void updateSettings();
    void updateTheme();
    void aboutThisVersion();

    // --- Action Handlers ---

    static void on_actionDarkTheme_triggered();
    static void on_actionLightTheme_triggered();
    static void on_actionSystemTheme_triggered();
    void on_actionAboutQt_triggered();
    void on_actionAbout_triggered();
    void on_actionExit_triggered();
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
    /// Registers the global search shortcut (scene-property shortcuts live in SceneUiBinder).
    void setupShortcuts();
    /// Connects all QAction and widget signals to their slots.
    void setupConnections();

    // --- Members ---

    std::unique_ptr<MainWindowUi> m_ui;

    ElementPalette   *m_palette          = nullptr;
    LanguageManager  *m_languageManager  = nullptr;
    RecentFiles      *m_recentFiles      = nullptr;
    ExportController *m_exportController  = nullptr;
    ICController     *m_icController      = nullptr;
    SceneUiBinder    *m_binder            = nullptr;

    /// Shared IC-hover preview, parented to this MainWindow.
    /// QPointer so accesses during teardown are safe regardless of child-destruction order.
    QPointer<ICPreviewPopup> m_icPreviewPopup;

    WorkSpace *m_currentTab = nullptr;
    int m_tabIndex = -1;

    [[nodiscard]] int findTabWithFile(const QString &fileName) const;
};
