// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickAppController: QML-facing root object tying the Quick chrome together.
 */

#pragma once

#include <QFileInfo>
#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QRect>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QVariantList>

#include "App/BeWavedDolphin/DolphinHost.h"
#include "App/Core/Enums.h"
#include "App/Core/ThemeManager.h"
#include "App/Exercise/ExerciseEngine.h"
#include "App/IO/RecentFiles.h"
#include "App/QuickShell/Chrome/QuickElementEditor.h"
#include "App/QuickShell/Chrome/QuickElementPalette.h"
#include "App/QuickShell/Chrome/QuickExerciseController.h"
#include "App/QuickShell/Chrome/QuickExportController.h"
#include "App/QuickShell/Chrome/QuickICController.h"
#include "App/QuickShell/Chrome/QuickICPreview.h"
#include "App/QuickShell/Chrome/QuickMainWindowHost.h"
#include "App/QuickShell/Chrome/QuickMinimap.h"
#include "App/QuickShell/Chrome/QuickTourController.h"
#include "App/QuickShell/Chrome/QuickUpdateController.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/QuickShell/Chrome/QuickWorkspaceManager.h"
#include "App/QuickShell/Dolphin/QuickDolphinController.h"
#include "App/UI/LanguageManager.h"

class CanvasItem;

/// One entry in QuickAppController::languages(). Mirrors MainWindow::populateLanguageMenu()'s
/// per-action fields (displayName/flagIcon), built from LanguageManager::availableLanguages().
/// A real QML value type (not QVariantMap) so QML can name it explicitly in Main.qml's
/// `required property languageEntry modelData` and qmllint can check field access against it.
class LanguageEntry
{
    Q_GADGET
    QML_VALUE_TYPE(languageEntry)

    Q_PROPERTY(QString code READ code FINAL)
    Q_PROPERTY(QString displayName READ displayName FINAL)
    Q_PROPERTY(QString flagIcon READ flagIcon FINAL)

public:
    LanguageEntry() = default;
    LanguageEntry(QString code, QString displayName, QString flagIcon)
        : m_code(std::move(code))
        , m_displayName(std::move(displayName))
        , m_flagIcon(std::move(flagIcon))
    {
    }

    [[nodiscard]] QString code() const { return m_code; }
    [[nodiscard]] QString displayName() const { return m_displayName; }
    [[nodiscard]] QString flagIcon() const { return m_flagIcon; }

private:
    QString m_code;
    QString m_displayName;
    QString m_flagIcon;
};

/// One entry in QuickAppController::examplesList(). Mirrors MainWindow::setupExamplesMenu()'s
/// per-action fields (prettified title, file path). A real QML value type (not QVariantMap),
/// same reasoning as LanguageEntry above.
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

/// One entry in QuickAppController::exercisesList()/toursList(). Mirrors
/// MainWindow::populateContentMenu()'s per-action fields (translated title/description,
/// completed state), built from ExerciseTourResources::discover() + Settings::completedExercises()/
/// completedTours(). A real QML value type for the same qmlcachegen reason as
/// ExampleEntry/LanguageEntry.
class LearnEntry
{
    Q_GADGET
    QML_VALUE_TYPE(learnEntry)

    Q_PROPERTY(QString title READ title FINAL)
    Q_PROPERTY(QString description READ description FINAL)
    Q_PROPERTY(QString path READ path FINAL)
    Q_PROPERTY(bool completed READ completed FINAL)

public:
    LearnEntry() = default;
    LearnEntry(QString title, QString description, QString path, bool completed)
        : m_title(std::move(title))
        , m_description(std::move(description))
        , m_path(std::move(path))
        , m_completed(completed)
    {
    }

    [[nodiscard]] QString title() const { return m_title; }
    [[nodiscard]] QString description() const { return m_description; }
    [[nodiscard]] QString path() const { return m_path; }
    [[nodiscard]] bool completed() const { return m_completed; }

private:
    QString m_title;
    QString m_description;
    QString m_path;
    bool m_completed = false;
};

/**
 * \class QuickAppController
 * \brief Owns the Quick chrome's document model and export workflow, exposes them to QML,
 * and re-binds reactive UI state (undo/redo, window title) to whichever tab is current --
 * the CanvasItem-side counterpart of MainWindow + SceneUiBinder's tab-rebinding half.
 *
 * \details Implements QuickMainWindowHost directly on itself (same multi-inheritance pattern
 * QuickDialogProvider uses for DialogProvider/FileDialogProvider) rather than via a
 * separate object, since there's exactly one instance of this class per running app.
 */
class QuickAppController : public QObject, public QuickMainWindowHost, public DolphinHost
{
    Q_OBJECT

    Q_PROPERTY(QuickWorkSpace *currentTab READ currentTab NOTIFY currentTabChanged FINAL)
    Q_PROPERTY(int tabCount READ tabCount NOTIFY tabsChanged FINAL)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentTabChanged FINAL)
    Q_PROPERTY(QString windowTitle READ windowTitle NOTIFY windowTitleChanged FINAL)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY undoRedoStateChanged FINAL)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY undoRedoStateChanged FINAL)
    Q_PROPERTY(QString undoText READ undoText NOTIFY undoRedoStateChanged FINAL)
    Q_PROPERTY(QString redoText READ redoText NOTIFY undoRedoStateChanged FINAL)
    // Mirrors Scene::contextMenu()'s selectAllAction->setEnabled(!elements().isEmpty()) for the
    // empty-canvas context menu's "Select all" item. Reuses undoRedoStateChanged as its NOTIFY
    // signal rather than adding a new one: every element add/delete is itself an undoable
    // command, so the undo stack's index already changes exactly when this could flip.
    Q_PROPERTY(bool hasElements READ hasElements NOTIFY undoRedoStateChanged FINAL)
    Q_PROPERTY(QStringList recentFiles READ recentFiles NOTIFY recentFilesChanged FINAL)
    Q_PROPERTY(bool simulationRunning READ isSimulationRunning WRITE setSimulationRunning NOTIFY simulationRunningChanged FINAL)
    Q_PROPERTY(bool backgroundSimulationEnabled READ isBackgroundSimulationEnabled WRITE setBackgroundSimulationEnabled NOTIFY backgroundSimulationEnabledChanged FINAL)
    Q_PROPERTY(QuickElementPalette *elementPalette READ elementPalette CONSTANT FINAL)
    Q_PROPERTY(QuickElementEditor *elementEditor READ elementEditor CONSTANT FINAL)
    Q_PROPERTY(QuickICPreview *icPreview READ icPreview CONSTANT FINAL)
    Q_PROPERTY(QuickMinimap *minimap READ minimap CONSTANT FINAL)
    Q_PROPERTY(QuickExerciseController *exercise READ exercise CONSTANT FINAL)
    Q_PROPERTY(QuickTourController *tour READ tour CONSTANT FINAL)
    Q_PROPERTY(QuickDolphinController *dolphin READ dolphin CONSTANT FINAL)
    Q_PROPERTY(QuickUpdateController *updateController READ updateController CONSTANT FINAL)
    // theme is a plain int (Enums::ElementType's own precedent for exposing a C++ enum class to
    // QML without registering it) -- Theme is declared at namespace scope in ThemeManager.h, not
    // inside a Q_GADGET/Q_NAMESPACE, so it has no Q_ENUM registration to expose directly.
    Q_PROPERTY(int theme READ themeInt WRITE setThemeInt NOTIFY themeChanged FINAL)
    // TourOverlay.qml's dim/spotlight colors are a bespoke accent identity (amber on dark,
    // blue on light) shared between the callout border and the spotlight ring -- not
    // decomposable into existing QPalette roles the way ExerciseOverlay.qml's plain root.palette
    // reuse was, so it needs the resolved dark/light bool directly. Shares themeChanged() as its
    // NOTIFY: ThemeManager::effectiveTheme() only ever changes together with a real
    // ThemeManager::themeChanged() emission (explicit switch, or System-mode OS change via
    // onSystemColorSchemeChanged()).
    Q_PROPERTY(bool darkTheme READ isDarkTheme NOTIFY themeChanged FINAL)
    Q_PROPERTY(QList<LanguageEntry> languages READ languages CONSTANT FINAL)
    Q_PROPERTY(QString currentLanguage READ currentLanguage NOTIFY currentLanguageChanged FINAL)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged FINAL)
    // The chrome-toggle half of Show Gates/Wires -- backed directly by CanvasItem's own
    // setGatesVisible()/setWiresVisible(). Per-tab, like muted, not cached like
    // simulationRunning -- see bindCurrentTab()'s own emit visibilityChanged() re-sync.
    Q_PROPERTY(bool gatesVisible READ isGatesVisible WRITE setGatesVisible NOTIFY visibilityChanged FINAL)
    Q_PROPERTY(bool wiresVisible READ isWiresVisible WRITE setWiresVisible NOTIFY visibilityChanged FINAL)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged FINAL)
    // Permanent right-aligned status-bar indicator (zoom % + selection count), mirroring
    // SceneUiBinder::updateStatusInfo()'s exact text.
    Q_PROPERTY(QString statusInfo READ statusInfo NOTIFY statusInfoChanged FINAL)
    // View > Labels Under Icons: a plain Settings-backed global toggle, not per-canvas (mirrors
    // MainWindow::on_actionLabelsUnderIcons_triggered()'s identical Settings::setLabelsUnderIcons()
    // + immediate re-style, minus the QToolBar::setToolButtonStyle() call -- Main.qml's ToolButtons
    // read this property directly instead).
    Q_PROPERTY(bool labelsUnderIcons READ labelsUnderIcons WRITE setLabelsUnderIcons NOTIFY labelsUnderIconsChanged FINAL)
    // View > Show IC Preview: same Settings-backed shape as labelsUnderIcons above, but
    // inverted (Settings stores the "disabled" bit, actionICPreview's checked state and this
    // property are both the "enabled" sense) -- mirrors MainWindow::
    // on_actionICPreview_triggered()'s identical Settings::setIcPreviewDisabled(!checked).
    // QuickICPreview::showForIC() already reads Settings::icPreviewDisabled() directly (ported
    // together with the rest of ICPreviewPopup's state machine); this is just the missing menu
    // toggle for it.
    Q_PROPERTY(bool icPreviewEnabled READ icPreviewEnabled WRITE setIcPreviewEnabled NOTIFY icPreviewEnabledChanged FINAL)
    // Backs ElementPalette.qml's IC Add/Remove/Make-Self-Contained toolbar controls -- mirrors
    // MainWindowHost::setICButtonsVisible()/refreshICButtonsEnabled() (see QuickMainWindowHost.h).
    Q_PROPERTY(bool icButtonsVisible READ icButtonsVisible NOTIFY icButtonsVisibleChanged FINAL)
    Q_PROPERTY(bool icButtonsEnabled READ icButtonsEnabled NOTIFY icButtonsEnabledChanged FINAL)

public:
    explicit QuickAppController(QObject *parent = nullptr);

    // --- QuickMainWindowHost ---
    QuickWorkSpace *currentTab() const override;
    QFileInfo currentFile() const override;
    QDir currentDir() const override;
    /// Sets statusMessage and restarts a single-shot timer that clears it after \a timeout ms --
    /// mirrors QStatusBar::showMessage()'s own auto-clear-after-timeout contract. Called from
    /// every site that needs it (QuickICController/QuickExportController/QuickWorkspaceManager).
    /// The permanent zoom/selection indicator is a separate property, statusInfo() below.
    void showStatusMessage(const QString &message, int timeout) override;
    QuickElementPalette *palette() override { return &m_palette; }
    void requestSave() override { saveFile(); }
    void setICButtonsVisible(bool visible) override;
    void refreshICButtonsEnabled() override;

    [[nodiscard]] QString statusMessage() const { return m_statusMessage; }
    [[nodiscard]] bool icButtonsVisible() const { return m_icButtonsVisible; }
    [[nodiscard]] bool icButtonsEnabled() const { return m_icButtonsEnabled; }

    /// Zoom percentage + selection count over the current tab's total element count, formatted
    /// exactly like SceneUiBinder::updateStatusInfo(). Empty with no tab open (mirrors that
    /// method's own m_statusInfo->clear() branch).
    [[nodiscard]] QString statusInfo() const;

    // --- DolphinHost ---
    // currentFile()/currentDir() above (QuickMainWindowHost) already satisfy this interface's
    // identical pure virtuals too: a single override in a most-derived class satisfies
    // matching pure virtuals from multiple unrelated base classes in standard C++.
    [[nodiscard]] QString dolphinFileName() override;
    void setDolphinFileName(const QString &fileName) override;
    void save(const QString &fileName) override { m_workspaceManager.save(fileName); }

    /// Opens (or raises, if already open) the BeWavedDolphin waveform window for the current
    /// tab's circuit -- mirrors MainWindow::on_actionWaveform_triggered()'s `if (m_bwd) {
    /// raise(); activateWindow(); return; }` early branch: m_waveformWindowOpen tracks that same
    /// condition (there's no live BewavedDolphin* to null-check here -- DolphinWindow.qml is a
    /// single static, reused QML instance, matching TruthTableDialog's own precedent), so a
    /// second Waveform click while one's already open just re-shows/raises it rather than
    /// rebuilding (and thereby discarding any unsaved edits/undo history) a working waveform.
    Q_INVOKABLE void openWaveform();
    /// Called from DolphinWindow.qml's onClosing (after checkSave() allows the close through) so
    /// the next openWaveform() rebuilds instead of just re-raising a window that's no longer
    /// showing anything current.
    Q_INVOKABLE void notifyWaveformClosed() { m_waveformWindowOpen = false; }

    // --- QML-facing accessors ---
    Q_INVOKABLE QuickWorkSpace *tabAt(int index) const { return m_workspaceManager.tabAt(index); }
    Q_INVOKABLE QString tabTitle(QuickWorkSpace *tab) const { return m_workspaceManager.tabTitle(tab); }
    /// The tab's full absolute file path (empty for an untitled or inline-IC tab) -- backs the
    /// tab bar's hover tooltip, mirrors WorkspaceManager::setCurrentFile()'s
    /// m_tab->setTabToolTip(tabIndex, fileInfo.absoluteFilePath()) call.
    Q_INVOKABLE QString tabFilePath(QuickWorkSpace *tab) const;
    [[nodiscard]] int tabCount() const { return m_workspaceManager.count(); }
    /// The index of \a tab in the tab list, or -1 if it isn't (any longer) open. Mirrors
    /// QuickWorkspaceManager::indexOf() -- exposed here so callers holding a QuickWorkSpace*
    /// (e.g. to close it, QuickAppController::closeTab() takes an index) don't need direct
    /// QuickWorkspaceManager access.
    [[nodiscard]] int indexOf(QuickWorkSpace *tab) const { return m_workspaceManager.indexOf(tab); }
    [[nodiscard]] int currentIndex() const { return m_workspaceManager.currentIndex(); }
    void setCurrentIndex(int index) { m_workspaceManager.setCurrentIndex(index); }
    [[nodiscard]] QString windowTitle() const;
    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;
    [[nodiscard]] QString undoText() const;
    [[nodiscard]] QString redoText() const;
    [[nodiscard]] bool hasElements() const;

    /// Rich-text body of the "Shortcuts and Tips" dialog. Mirrors MainWindow::shortcutsHelpHtml(),
    /// scoped to the shortcuts this sub-step actually wires (the rest of MainWindow's version is
    /// generated from its own QAction set, which doesn't exist here).
    Q_INVOKABLE QString shortcutsHelpHtml() const;

    /// The app's own version string (the same APP_VERSION windowTitle() already embeds), for the
    /// About/About this version dialogs' %1 substitution.
    Q_INVOKABLE QString appVersion() const { return QStringLiteral(APP_VERSION); }
    /// Rich-text body of the Help > About dialog. Mirrors MainWindow::on_actionAbout_triggered()'s
    /// QMessageBox::about() text exactly.
    Q_INVOKABLE QString aboutHtml() const;
    /// Body of the Help > About this version dialog. Mirrors MainWindow::aboutThisVersion()'s
    /// QMessageBox text exactly.
    Q_INVOKABLE QString aboutThisVersionText() const;
    /// Rich-text body of the Help > About Qt dialog. Widgets shows Qt's own built-in
    /// QMessageBox::aboutQt() (its full license/credits text, generated by Qt itself) -- no QML
    /// equivalent exists, so this is a deliberately smaller substitute (Qt's own runtime version
    /// plus a licensing note and a link), not a line-for-line port.
    Q_INVOKABLE QString aboutQtHtml() const;
    /// Opens the project's Weblate page. Mirrors
    /// MainWindow::on_actionReportTranslationError_triggered() exactly.
    Q_INVOKABLE void reportTranslationError() const;

    // Not const: RecentFiles::recentFiles() isn't const-qualified upstream either.
    [[nodiscard]] QStringList recentFiles() { return m_recentFiles.recentFiles(); }

    /// Just the file name for one recentFiles() entry -- QFileInfo(path).fileName(), matching
    /// MainWindow::updateRecentFileActions()'s own QFileInfo(files.at(i)).fileName() exactly, so
    /// the Recent Files menu shows "&1 circuit.panda" instead of the full path.
    Q_INVOKABLE static QString recentFileBaseName(const QString &path) { return QFileInfo(path).fileName(); }

    /// Bundled example .panda files, prettified for display. Mirrors
    /// MainWindow::setupExamplesMenu()'s title-prettification logic exactly (word-split on
    /// '-'/'_', capitalize each word). QML iterates this directly as a Repeater model. Not
    /// reactive (no NOTIFY) since the bundled example set never changes at runtime, unlike
    /// recentFiles.
    Q_INVOKABLE QList<ExampleEntry> examplesList() const;

    /// Discovered exercise content (built-in + user-provided, see ExerciseTourResources::discover()),
    /// each entry's title/description translated and completed flag looked up from
    /// Settings::completedExercises(). Mirrors MainWindow::setupExercisesMenu()'s
    /// aboutToShow-populated menu -- QML re-reads this each time the Learn menu opens instead
    /// (Instantiator's own onObjectAdded-per-open equivalent), so no NOTIFY is needed: content
    /// added/removed on disk between menu opens is exactly what a fresh discover() call picks up.
    Q_INVOKABLE QList<LearnEntry> exercisesList() const;

    /// Same shape as exercisesList(), scanning "Tours" (Settings::completedTours()) instead of
    /// "Exercises" -- backs the Learn menu's Tours submenu (Main.qml's toursMenu).
    Q_INVOKABLE QList<LearnEntry> toursList() const;

    /// Opens \a category's ("Exercises" or "Tours") preferred, real filesystem folder in the
    /// OS file manager -- mirrors MainWindow::populateContentMenu()'s "Open My
    /// Exercises/Tours Folder" action exactly (ExerciseTourResources::preferredContentDir() +
    /// QDesktopServices::openUrl()), including its warning dialog if the folder can't be
    /// created/accessed (preferredContentDir() returns "" in that case).
    Q_INVOKABLE void openMyContentFolder(const QString &category) const;

    /// Restores the persisted window geometry (Settings::quickWindowGeometry()), or an
    /// invalid/empty QRect if none was ever saved -- QML checks width/height before applying.
    Q_INVOKABLE QRect restoreWindowGeometry() const;
    Q_INVOKABLE void saveWindowGeometry(int x, int y, int width, int height);

    /// Same shape as restoreWindowGeometry()/saveWindowGeometry() above, for DolphinWindow.qml --
    /// mirrors BewavedDolphin's own restoreGeometry(Settings::dolphinGeometry())/
    /// setDolphinGeometry(saveGeometry()) constructor/destructor pair, backed by
    /// Settings::quickDolphinGeometry() (a separate key/format -- see its own doc comment).
    Q_INVOKABLE QRect restoreDolphinWindowGeometry() const;
    Q_INVOKABLE void saveDolphinWindowGeometry(int x, int y, int width, int height);

    /// Mirrors MainWindow::closeEvent()'s logic: confirms exit (a plain yes/no) if nothing is
    /// modified, or runs the per-tab close-and-save-prompt flow (closeFiles()) otherwise.
    /// Returns true if it's OK to actually close the window.
    Q_INVOKABLE bool confirmClose();

    [[nodiscard]] bool isSimulationRunning() const { return m_simulationRunning; }
    void setSimulationRunning(bool running);
    [[nodiscard]] bool isBackgroundSimulationEnabled() const { return m_backgroundSimulationEnabled; }
    void setBackgroundSimulationEnabled(bool enabled);

    /// Mute is genuinely per-tab state (Simulation::isUserMuted(), "persists across
    /// stop/start cycles" per its own doc comment -- each tab owns its own Simulation),
    /// unlike simulationRunning's global user-intent property -- so this is computed live from
    /// the current tab, mirroring canUndo()'s pattern, not simulationRunning's cached-member one.
    [[nodiscard]] bool isMuted() const;
    /// Mirrors MainWindow::on_actionMute_triggered(): calls Simulation::setUserMuted(), NOT
    /// CanvasItem::mute() directly -- setUserMuted() both records the intent isMuted() reads
    /// back and propagates to every AudioOutputElement via the SimulationHost::setMuted()
    /// callback (CanvasItem::mute()'s real caller); calling mute() directly would never
    /// persist the mute state.
    void setMuted(bool muted);

    /// Mirrors MainWindow::on_actionLabelsUnderIcons_triggered(): a plain Settings passthrough,
    /// no canvas/per-tab state involved.
    [[nodiscard]] bool labelsUnderIcons() const;
    void setLabelsUnderIcons(bool enabled);

    /// Mirrors MainWindow::on_actionICPreview_triggered(): a plain Settings passthrough
    /// (inverted sense -- see the Q_PROPERTY's own doc comment above).
    [[nodiscard]] bool icPreviewEnabled() const;
    void setIcPreviewEnabled(bool enabled);

    /// Mirrors MainWindowUi's actionShowGates/actionShowWires: `true` (the default) when there
    /// is no current tab, matching CanvasItem's own default-visible state. Defined in the .cpp
    /// (unlike isMuted()'s sibling properties above) since CanvasItem is only forward-declared
    /// here.
    [[nodiscard]] bool isGatesVisible() const;
    void setGatesVisible(bool visible);
    [[nodiscard]] bool isWiresVisible() const;
    void setWiresVisible(bool visible);

    /// Called from Main.qml's ApplicationWindow.onActiveChanged. Pauses the active canvas's
    /// simulation while the window is inactive (unless backgroundSimulationEnabled is set),
    /// and resumes it on reactivation -- without touching simulationRunning itself, since a
    /// user-paused simulation must stay paused across a window focus round-trip.
    Q_INVOKABLE void handleWindowActiveChanged(bool active);

    [[nodiscard]] QuickElementPalette *elementPalette() { return &m_palette; }
    [[nodiscard]] QuickElementEditor *elementEditor() { return &m_elementEditor; }
    [[nodiscard]] QuickICPreview *icPreview() { return &m_icPreview; }
    [[nodiscard]] QuickMinimap *minimap() { return &m_minimap; }
    [[nodiscard]] QuickExerciseController *exercise() { return &m_exerciseController; }
    [[nodiscard]] QuickTourController *tour() { return &m_tourController; }
    /// The BeWavedDolphin waveform-editor presenter.
    [[nodiscard]] QuickDolphinController *dolphin() { return &m_dolphinController; }
    [[nodiscard]] QuickUpdateController *updateController() { return &m_updateController; }

    /// Mirrors MainWindowUi's actionLightTheme/actionDarkTheme/actionSystemTheme radio group,
    /// as the raw Theme ordinal (Light=0, Dark=1, System=2).
    [[nodiscard]] int themeInt() const { return static_cast<int>(ThemeManager::theme()); }
    void setThemeInt(int value) { ThemeManager::setTheme(static_cast<Theme>(value)); }
    [[nodiscard]] bool isDarkTheme() const { return ThemeManager::effectiveTheme() == Theme::Dark; }

    /// Every available UI language, prettified for display. Mirrors
    /// MainWindow::populateLanguageMenu()'s per-action displayName()/flagIcon() lookups. Not
    /// reactive (no NOTIFY) since the bundled translation set never changes at runtime, unlike
    /// currentLanguage -- same reasoning as examplesList()'s own CONSTANT choice, just exposed
    /// as a property here since (unlike the Examples submenu) nothing needs to force a re-scan.
    [[nodiscard]] QList<LanguageEntry> languages() const;

    /// The currently active language code (e.g. "en", "pt_BR"), driving the Language submenu's
    /// checked radio item. Mirrors Settings::language(), defaulting to "en" the same way
    /// MainWindow::populateLanguageMenu()'s own checked-state check does.
    [[nodiscard]] QString currentLanguage() const;

    /// Adds one element from a palette entry to the current tab's canvas. \a type/\a
    /// icFileName/\a isEmbedded mirror QuickElementPalette's entry fields exactly (QML passes
    /// them straight through from whichever entry was dragged or double-clicked); \a x/\a y are
    /// canvas-local screen coordinates (== canvasHost-local) -- CanvasItem::addElementFromPalette()
    /// converts them to world coordinates internally via screenToWorld(). No-op if there is no
    /// current tab. Mirrors MainWindow's connection to ElementPalette::addElementRequested().
    Q_INVOKABLE void addElementToCurrentTab(int type, const QString &icFileName, bool isEmbedded, qreal x, qreal y);

    /// Returns true if the system clipboard holds a pasteable wiRedPanda payload. Mirrors
    /// Scene::contextMenu()'s ClipboardManager::canPaste(QApplication::clipboard()->mimeData())
    /// check, backing the empty-canvas context menu's Paste item. Duplicates that check's tiny
    /// MIME-format test rather than calling it: App/Scene/ClipboardManager.cpp is WIDGETS_ONLY
    /// (tightly coupled to Scene/QGraphicsItem), and the actual logic is two hasFormat() calls.
    Q_INVOKABLE static bool canPaste();

    /// Saves the current tab directly to \a fileName, no dialog -- mirrors MainWindow::save(),
    /// the direct-filename counterpart to saveFile()/saveFileAs()'s dialog-driven pair. Used by
    /// QuickFileHandler's save_circuit MCP command, which (unlike the GUI) always supplies an
    /// explicit path.
    void saveCurrentTabAs(const QString &fileName) { m_workspaceManager.save(fileName); }
    /// Direct-filename Arduino/SystemVerilog export, no dialog -- mirrors MainWindow::
    /// exportToArduino()/exportToSystemVerilog(), the same direct-filename/dialog split
    /// saveCurrentTabAs() has relative to saveFile(). Used by QuickFileHandler's
    /// export_arduino/export_systemverilog MCP commands.
    void exportArduinoTo(const QString &fileName) { m_exportController.exportToArduino(fileName); }
    void exportSystemVerilogTo(const QString &fileName) { m_exportController.exportToSystemVerilog(fileName); }

    /// Runs the real, canvas-mutating half of the Tour/Exercise "click" vocabulary's two
    /// "setup*Demo" ids (App/Resources/Exercises/README.md's "Closed widget/action vocabulary")
    /// -- the ids Main.qml's dispatchTourClick() can't handle by touching QML state alone, since
    /// they build real GraphicElement/Connection instances. Mirrors
    /// MainWindow::clickTarget()'s "setupElementEditorDemo"/"setupWaveformDemo" cases, adapted to
    /// CanvasItem's command API -- see this method's own .cpp doc comment for why only
    /// "setupElementEditorDemo" is actually implemented. Unrecognized ids (everything else in
    /// the vocabulary is handled entirely in QML) are a silent no-op.
    Q_INVOKABLE void runTourDemoAction(const QString &id);

public slots:
    // --- File menu ---
    void newTab() { m_workspaceManager.newTab(); }
    void openFile() { m_workspaceManager.openFile(); }
    void saveFile() { m_workspaceManager.saveFile(); }
    void saveFileAs() { m_workspaceManager.saveFileAs(); }
    void reloadFile() { m_workspaceManager.reloadFile(); }
    void openRecentFile(const QString &path) { m_workspaceManager.loadPandaFile(path); }
    /// Opens the first local `.panda` file found in \a urls, exactly like File > Open --
    /// mirrors Scene::droppedPandaFile()'s validation (local file, case-insensitive `.panda`
    /// suffix). Called from Main.qml's canvas DropArea when the drop isn't a palette-element
    /// drag (item drops carry no QUrl at all, so this is naturally a no-op for those). Returns
    /// true if a file was opened, for the caller's own logging/feedback if it ever wants it.
    bool openDroppedPandaFile(const QList<QUrl> &urls);
    /// Closes the tab at \a index (prompting to save if needed). Mirrors the MainWindow tab
    /// close button / Ctrl+W path -- called from Main.qml's tab bar close (X) button.
    bool closeTab(int index) { return m_workspaceManager.closeTab(index); }
    /// Closes the tab at \a index immediately, without a save prompt. See
    /// QuickWorkspaceManager::removeTabWithoutPrompt()'s doc comment; used only by
    /// QuickFileHandler's close_circuit MCP command.
    void removeTabWithoutPrompt(int index) { m_workspaceManager.removeTabWithoutPrompt(index); }
    /// Moves the tab at \a from to sit at \a to. See QuickWorkspaceManager::moveTab()'s own
    /// doc comment -- called from Main.qml's tab bar drag gesture (tab->setMovable(true)).
    void moveTab(int from, int to) { m_workspaceManager.moveTab(from, to); }

    // --- Edit menu (the six scene-property shortcuts -- [ ] { } < > -- are NOT here: they're
    // already implemented directly in CanvasItem::keyPressEvent()) ---
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

    // --- View menu (zoom) --- mirrors GraphicsView's own actions exactly (no reactive
    // canZoomIn/canZoomOut property exposed -- like Transform's rotate/flip, these are always
    // enabled and safely no-op at the zoom limits, the same design zoomIn()/zoomOut() already
    // use internally, rather than needing per-tab signal relay wiring for a menu-enabled nicety).
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void zoomToFit();

    // --- Simulation menu (Play/pause is the simulationRunning WRITE property above, not a
    // slot -- it needs a paired READ for the menu's checkable MenuItem to bind against; mute
    // is the same shape, see the muted WRITE property above) ---
    void restartSimulation();

    // --- Export menu ---
    void exportArduino() { m_exportController.exportArduinoDialog(); }
    void exportSystemVerilog() { m_exportController.exportSystemVerilogDialog(); }
    void exportPdf() { m_exportController.exportPdfDialog(); }
    void exportImage() { m_exportController.exportImageDialog(); }

    // --- IC drag-and-drop targets (ElementPalette.qml's IC tab) ---
    void embedICByFile(const QString &fileName) { m_icController.embedICByFile(fileName); }
    void extractICByBlobName(const QString &blobName) { m_icController.extractICByBlobName(blobName); }
    void removeICFile(const QString &icFileName) { m_icController.removeICFile(icFileName); }
    void removeEmbeddedIC(const QString &blobName) { m_icController.removeEmbeddedIC(blobName); }

    // --- IC toolbar-button targets (ElementPalette.qml's "Add IC.../Make Self-Contained"
    // buttons, QuickElementEditor's Embed/Extract actions for a selected IC) ---
    void addICFromFile() { m_icController.addICFromFile(); }
    void embedSelectedIC() { m_icController.embedSelectedIC(); }
    void extractSelectedIC() { m_icController.extractSelectedIC(); }
    void makeSelfContained() { m_icController.makeSelfContained(); }
    void addEmbeddedICFromFile() { m_icController.addEmbeddedICFromFile(); }

    /// "Edit sub-circuit" -- ElementContextMenu.cpp's onEditSubcircuit branch: for an embedded
    /// IC, opens its blob in a new tab (mirrors MainWindow's editSubcircuitRequested lambda,
    /// QuickWorkspaceManager::openICInTab() -- already implemented and used from another call
    /// site); for a file-backed IC, opens the referenced file directly (mirrors
    /// openSubcircuitFileRequested -> MainWindow::loadPandaFile(), the same
    /// m_workspaceManager.loadPandaFile() openRecentFile() itself calls).
    void editSelectedSubcircuit();
    /// Direct pass-through to QuickWorkspaceManager::openICInTab() for callers that already
    /// have the raw blob name/element id/blob bytes in hand (editSelectedSubcircuit() above
    /// covers the selection-driven path; this is the lower-level entry point it and any future
    /// direct caller -- e.g. a QML drag target -- both go through).
    void openICInTab(const QString &blobName, int icElementId, const QByteArray &blob) { m_workspaceManager.openICInTab(blobName, icElementId, blob); }

    // --- Language menu ---
    /// Mirrors the Language submenu action lambdas: m_languageManager.loadTranslation(code).
    /// QML's qsTr()-bound text retranslates automatically once the new QTranslator is
    /// installed (Qt Quick's engine listens for QEvent::LanguageChange itself) -- unlike
    /// MainWindow::retranslateUi(), no explicit re-translation call is needed here.
    void switchLanguage(const QString &code) { m_languageManager.loadTranslation(code); }

signals:
    void currentTabChanged();
    void tabsChanged();
    void windowTitleChanged();
    void undoRedoStateChanged();
    void icButtonsVisibleChanged();
    void icButtonsEnabledChanged();
    void recentFilesChanged();
    void simulationRunningChanged();
    void backgroundSimulationEnabledChanged();
    void themeChanged();
    void currentLanguageChanged();
    void mutedChanged();
    void visibilityChanged();
    void labelsUnderIconsChanged();
    void icPreviewEnabledChanged();
    void statusMessageChanged();
    void statusInfoChanged();
    /// Tells Main.qml's static DolphinWindow instance to become visible/raised/activated --
    /// emitted whether openWaveform() just rebuilt the waveform or is just re-showing an
    /// already-open one, mirroring TruthTableDialog's identical onXRequested-opens-a-static-
    /// instance precedent.
    void waveformOpenRequested();

private:
    /// Returns the active tab's canvas, or nullptr. Shared by every Edit/Transform/Align
    /// action above -- mirrors MainWindow::connectSceneAction()'s "guarded by a current-tab
    /// check" pattern.
    CanvasItem *activeCanvas() const;

    /// Disconnects the previous tab's undo-stack signal wiring and reconnects to the new
    /// current tab's, re-emitting undoRedoStateChanged()/windowTitleChanged() as needed. Also
    /// stops the previously-bound canvas's simulation and starts/stops the new one to match
    /// simulationRunning, refreshes the palette's IC lists, and rebinds m_elementEditor/
    /// m_icPreview to the new canvas (QuickElementEditor::setCanvas() itself refreshes from
    /// the new canvas's current selection; QuickICPreview::setCanvas() hides any popup left
    /// pending/visible from the previous tab). Also re-emits mutedChanged() so the Mute menu
    /// item resyncs to the newly-current tab's own Simulation::isUserMuted() state -- the
    /// CanvasItem-side counterpart of SceneUiBinder::bind()/unbind()'s mute-sync.
    void bindCurrentTab();

    /// Starts or stops \a canvas's simulation to match \a running. CanvasItem's constructor
    /// unconditionally auto-starts its Simulation (unlike Scene, which starts paused), so
    /// this must be a symmetric start-or-stop on every call, not just a conditional stop.
    void applySimulationRunningState(bool running);

    /// Loads the initial UI language, mirroring MainWindow::setupLanguage(): if
    /// Settings::language() was never set (first run), auto-detects from the system locale
    /// (exact code, then base language, then "en"), otherwise loads the persisted choice.
    /// Called once from the constructor.
    void setupLanguage();

    QuickWorkspaceManager m_workspaceManager;
    RecentFiles m_recentFiles;
    QuickExportController m_exportController;
    QuickICController m_icController;
    QuickElementPalette m_palette;
    QuickElementEditor m_elementEditor;
    QuickICPreview m_icPreview;
    QuickMinimap m_minimap;
    /// m_exerciseEngine must be declared (and thus constructed) before m_exerciseController,
    /// which takes a pointer to it in its own constructor.
    ExerciseEngine m_exerciseEngine;
    QuickExerciseController m_exerciseController{&m_exerciseEngine};
    /// Unlike ExerciseEngine, TourEngine has zero Scene/CanvasItem coupling (see
    /// QuickTourController's own doc comment), so this owns its TourEngine directly -- no
    /// sibling engine member or setCanvas() binding needed.
    QuickTourController m_tourController;
    QuickDolphinController m_dolphinController;
    QuickUpdateController m_updateController;
    bool m_waveformWindowOpen = false;
    LanguageManager m_languageManager;
    QList<QMetaObject::Connection> m_tabConnections;
    bool m_simulationRunning = true;
    bool m_backgroundSimulationEnabled = false;
    QString m_statusMessage;
    /// Single-shot; restarted on every showStatusMessage() call, clears m_statusMessage on fire.
    QTimer m_statusMessageTimer;
    bool m_icButtonsVisible = true;
    bool m_icButtonsEnabled = true;
    QPointer<CanvasItem> m_boundCanvas;
};

/**
 * \brief Exposes the single, C++-constructed \c QuickAppController instance (see Main.cpp) to
 * QML as the \c AppController singleton, rather than a \c QQmlContext context property --
 * context properties are invisible to qmllint/the QML Language Server, which the Qt docs
 * identify as their central drawback. Uses the \c QML_FOREIGN wrapper pattern Qt documents
 * for exposing a pre-existing, externally-owned instance (rather than one the engine
 * constructs itself via create()): \c s_instance is set once, before the QML engine loads
 * Main.qml.
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
        // A QObject with no parent becomes eligible for JavaScriptOwnership -- and GC
        // deletion -- the moment QML first sees it. s_instance is a static-storage object
        // Main.cpp owns for the whole process lifetime, so it must stay C++-owned.
        QQmlEngine::setObjectOwnership(s_instance, QQmlEngine::CppOwnership);
        return s_instance;
    }
};
