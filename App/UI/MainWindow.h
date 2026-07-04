// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Main application window providing menus, toolbars, and tab management.
 */

#pragma once

#include <functional>
#include <memory>

#include <QDir>
#include <QPointer>
#include <QSpacerItem>

#include "App/BeWavedDolphin/DolphinHost.h"
#include "App/UI/MainWindowHost.h"
#include "App/UI/MainWindowUI.h"

#ifdef USE_KDE_FRAMEWORKS
#include <KXmlGuiWindow>
using MainWindowBase = KXmlGuiWindow;
#else
#include <QMainWindow>
using MainWindowBase = QMainWindow;
#endif

class ElementLabel;
class ElementPalette;
class ExerciseEngine;
class ExerciseOverlay;
class ExportController;
class ICController;
class ICPreviewPopup;
class LanguageManager;
class BewavedDolphin;
class SceneUiBinder;
class TourEngine;
class TourOverlay;
class WorkSpace;
class WorkspaceManager;
#ifdef USE_KDE_FRAMEWORKS
class KRecentFilesAction;
#else
class RecentFiles;
#endif

/**
 * \class MainWindow
 * \brief The top-level application window hosting the tab bar, menus, element palette, and editor.
 *
 * \details MainWindow orchestrates the full editing workflow: it owns the tab widget (each tab is a
 * WorkSpace), connects undo/redo stacks, handles file open/save/export, manages translations,
 * and integrates the element palette, element editor, and IC list panel.
 */
class MainWindow : public MainWindowBase, public DolphinHost, public MainWindowHost
{
    Q_OBJECT
    friend class TestMainWindowGui;

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
    WorkSpace *currentTab() const override;

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

    /// Returns the QFileInfo of the currently active .panda file.
    QFileInfo currentFile() const override;

    /// Returns true if any open workspace has unsaved changes or is a recovered autosave.
    bool hasModifiedFiles();

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

    /// \reimp MainWindowHost — shows/hides the IC management tool buttons.
    void setICButtonsVisible(bool visible) override;

    /// \reimp MainWindowHost — re-evaluates the Add-IC button enabled state.
    void refreshICButtonsEnabled() override;

    // --- Help ---

    /// Rich-text body of the "Shortcuts and Tips" dialog, generated from this window's
    /// actions so the list can never drift from the real key bindings and picks up any
    /// action added later. Public so GUI tests can assert its completeness without
    /// opening a modal dialog.
    QString shortcutsHelpHtml() const;

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

protected:
    // --- Protected Interface ---

    /// \reimp
    void closeEvent(QCloseEvent *event) override;

private:
    Q_DISABLE_COPY(MainWindow)

    // --- Tab Lifecycle ---

    /// Reacts to WorkspaceManager::currentTabChanged: rebinds the chrome to the new scene.
    void onCurrentTabChanged(WorkSpace *newTab);

    /// Refreshes the window title to "<current tab>[*] — wiRedPanda <version>", using
    /// setWindowModified() for the "*". Falls back to the bare app name when no tab is open.
    void updateWindowTitle();

    /// \reimp MainWindowHost — file info used to populate the file-based IC palette.
    QFileInfo icListFile() const override;

    /// Connects \a action to \a method on the current tab's scene (guarded by a current-tab check).
    void connectSceneAction(QAction *action, void (Scene::*method)());

    // --- File Helpers ---

#ifndef USE_KDE_FRAMEWORKS
    void createRecentFileActions();
    void updateRecentFileActions();
    void openRecentFile();
#endif

    // --- Settings & Theme ---

    void updateSettings();
    void updateTheme();
    void aboutThisVersion();

    // --- Action Handlers ---

    void startExercise(const QString &resourcePath);
    void startTour(const QString &resourcePath);
    QRect resolveTourTarget(const QString &id) const;

    void clickTarget(const QString &id);

    static void on_actionDarkTheme_triggered();
    static void on_actionLightTheme_triggered();
    static void on_actionSystemTheme_triggered();
    void on_actionAboutQt_triggered();
    void on_actionAbout_triggered();
    void on_actionExit_triggered();
    void on_actionFastMode_triggered(const bool checked);
    void on_actionFlipHorizontally_triggered();
    void on_actionFlipVertically_triggered();
    void on_actionAlignLeft_triggered();
    void on_actionAlignRight_triggered();
    void on_actionAlignTop_triggered();
    void on_actionAlignBottom_triggered();
    void on_actionAlignHorizontalCenter_triggered();
    void on_actionAlignVerticalCenter_triggered();
    void on_actionDistributeHorizontally_triggered();
    void on_actionDistributeVertically_triggered();
    void on_actionFullscreen_triggered();
    void on_actionGates_triggered(const bool checked);
    void on_actionLabelsUnderIcons_triggered(const bool checked);
    void on_actionICPreview_triggered(const bool checked);
    void on_actionCheckForUpdates_triggered(const bool checked);
    void on_actionMute_triggered(const bool checked);
    void on_actionPlay_toggled(const bool checked);
    void on_actionReportTranslationError_triggered();
    void on_actionResetZoom_triggered() const;
    void on_actionZoomToFit_triggered() const;
    void on_actionRestart_triggered();
    void on_actionRotateLeft_triggered();
    void on_actionRotateRight_triggered();
    void on_actionSelectAll_triggered();
    void on_actionShortcuts_and_Tips_triggered();
    void on_actionShowMinimap_triggered(const bool checked);
    void on_actionWaveform_triggered();
    void on_actionWires_triggered(const bool checked);
    void on_actionZoomIn_triggered() const;
    void on_actionZoomOut_triggered() const;

#ifdef Q_OS_WASM
    /// Emscripten beforeunload callback — saves window geometry before the browser tab closes.
    static const char *onBeforeUnload(int eventType, const void *reserved, void *userData);
#endif

    // --- Constructor Setup ---

#ifdef USE_KDE_FRAMEWORKS
    /// Creates KDE actions via KActionCollection, calls setupGUI(), and wires up dynamic menus.
    void setupKdeActions();
    /// Opens the KNewStuff dialog and loads any newly downloaded .panda circuits.
    void downloadCircuits();
#endif

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
    /// Wires the Exercises menu to populate itself on every open.
    void setupExercisesMenu();
    /// Wires the Tours menu to populate itself on every open.
    void setupToursMenu();
    /// Rebuilds \a menu with one action per discovered Exercise/Tour item plus a trailing
    /// "Open Folder" action; shared by setupExercisesMenu()/setupToursMenu().
    void populateContentMenu(QMenu *menu, const QString &categoryKey,
                              const QString &openFolderText, const QString &openFolderFailureText,
                              const QStringList &completed, const std::function<void(const QString &)> &onSelect);
    /// Registers the global search shortcut (scene-property shortcuts live in SceneUiBinder).
    void setupShortcuts();
    /// Connects all QAction and widget signals to their slots.
    void setupConnections();

    // --- Members ---

    std::unique_ptr<MainWindowUi> m_ui;

    ElementPalette   *m_palette          = nullptr;
    LanguageManager  *m_languageManager  = nullptr;
#ifdef USE_KDE_FRAMEWORKS
    KRecentFilesAction *m_recentFilesAction = nullptr;
#else
    RecentFiles      *m_recentFiles      = nullptr;
#endif
    ExportController *m_exportController  = nullptr;
    ICController     *m_icController      = nullptr;
    SceneUiBinder    *m_binder            = nullptr;
    WorkspaceManager *m_workspaceManager  = nullptr;

    ExerciseEngine  *m_exerciseEngine  = nullptr;
    ExerciseOverlay *m_exerciseOverlay = nullptr;

    TourEngine  *m_tourEngine  = nullptr;
    /// QPointer: can be destroyed as a side effect of a WA_DeleteOnClose window
    /// (e.g. BeWavedDolphin) it was reparented onto, outside our own control.
    QPointer<TourOverlay> m_tourOverlay;

    QPointer<BewavedDolphin> m_bwd;

    /// The tab being left, captured for onCurrentTabChanged(): by the time that slot runs,
    /// currentTab() already reflects the arriving tab (WorkspaceManager updates its current-tab
    /// field before emitting the signal), so this is tracked separately. QPointer since the
    /// previous tab may already be mid-close (deleteLater) by the time it's read.
    QPointer<WorkSpace> m_previousTab;

    /// Shared IC-hover preview, parented to this MainWindow.
    /// QPointer so accesses during teardown are safe regardless of child-destruction order.
    QPointer<ICPreviewPopup> m_icPreviewPopup;
};
