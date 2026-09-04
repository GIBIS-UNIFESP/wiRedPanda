// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickAppController.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLocale>
#include <QMimeData>
#include <QPointF>
#include <QUndoStack>
#include <QUrl>
#include <QVariantMap>

#include "App/Core/Application.h"
#include "App/Core/ExerciseTourResources.h"
#include "App/Core/InstallRelativePaths.h"
#include "App/Core/MimeTypes.h"
#include "App/Core/Settings.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/DialogProvider.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/Simulation/Simulation.h"

QuickAppController::QuickAppController(QObject *parent)
    : QObject(parent)
    , m_workspaceManager(*this)
    , m_exportController(*this)
    , m_icController(*this)
{
    // m_palette/m_elementEditor/m_icPreview are plain member subobjects (not heap-allocated),
    // exposed to QML via elementPalette()/elementEditor()/icPreview()'s CONSTANT Q_PROPERTYs.
    // Without this, Qt/QML's default "no QObject parent at first JS exposure ->
    // JavaScriptOwnership" rule would apply to them exactly as it does to QuickWorkSpace's
    // m_canvas -- except here the GC calling delete on a pointer to a non-heap-allocated member
    // would be undefined behavior, not just a dangling pointer.
    QQmlEngine::setObjectOwnership(&m_palette, QQmlEngine::CppOwnership);
    QQmlEngine::setObjectOwnership(&m_elementEditor, QQmlEngine::CppOwnership);
    QQmlEngine::setObjectOwnership(&m_icPreview, QQmlEngine::CppOwnership);
    QQmlEngine::setObjectOwnership(&m_minimap, QQmlEngine::CppOwnership);

    connect(&m_workspaceManager, &QuickWorkspaceManager::currentTabChanged, this, [this] {
        bindCurrentTab();
        emit currentTabChanged();
        emit windowTitleChanged();
        emit undoRedoStateChanged();
    });
    connect(&m_workspaceManager, &QuickWorkspaceManager::tabsChanged, this, &QuickAppController::tabsChanged);
    connect(&m_workspaceManager, &QuickWorkspaceManager::titleChanged, this, &QuickAppController::windowTitleChanged);
    connect(&m_workspaceManager, &QuickWorkspaceManager::recentFileAdded, &m_recentFiles, &RecentFiles::addRecentFile);
    connect(&m_recentFiles, &RecentFiles::recentFilesUpdated, this, &QuickAppController::recentFilesChanged);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &QuickAppController::themeChanged);
    connect(&m_languageManager, &LanguageManager::translationChanged, this, &QuickAppController::currentLanguageChanged);

    m_statusMessageTimer.setSingleShot(true);
    connect(&m_statusMessageTimer, &QTimer::timeout, this, [this] {
        m_statusMessage.clear();
        emit statusMessageChanged();
    });

    setupLanguage();

    // Called in the constructor body, not the initializer list: m_dolphinController is a plain
    // member (already fully constructed by this point), and setHost() just stores the pointer
    // for later use -- never dereferenced synchronously during this call -- so `this` being
    // mid-construction here is safe, same reasoning as QuickExerciseEngineBinding's setCanvas().
    m_dolphinController.setHost(this);
}

QuickWorkSpace *QuickAppController::currentTab() const
{
    return m_workspaceManager.currentTab();
}

QFileInfo QuickAppController::currentFile() const
{
    return m_workspaceManager.currentFile();
}

QDir QuickAppController::currentDir() const
{
    return m_workspaceManager.currentDir();
}

QString QuickAppController::tabFilePath(QuickWorkSpace *tab) const
{
    if (!tab || tab->isInlineIC()) {
        return {};
    }
    // Mirrors WorkspaceManager::setCurrentFile()'s setTabToolTip(): the tooltip reflects
    // whichever path the tab was last associated with, set once at save/load time and never
    // re-validated against the filesystem afterwards -- an unsaved tab (never given a real file)
    // is the only case with nothing to show.
    const QFileInfo fileInfo = tab->fileInfo();
    return fileInfo.fileName().isEmpty() ? QString() : fileInfo.absoluteFilePath();
}

void QuickAppController::showStatusMessage(const QString &message, int timeout)
{
    m_statusMessage = message;
    emit statusMessageChanged();
    m_statusMessageTimer.start(timeout);
}

void QuickAppController::setICButtonsVisible(bool visible)
{
    if (m_icButtonsVisible == visible) {
        return;
    }
    m_icButtonsVisible = visible;
    emit icButtonsVisibleChanged();
}

void QuickAppController::refreshICButtonsEnabled()
{
    // Mirrors MainWindow::refreshICButtonsEnabled(): importing a file-based IC needs a real,
    // readable project file to copy into.
    auto *tab = currentTab();
    const bool hasFile = tab && tab->fileInfo().isReadable();
    if (m_icButtonsEnabled == hasFile) {
        return;
    }
    m_icButtonsEnabled = hasFile;
    emit icButtonsEnabledChanged();
}

QString QuickAppController::statusInfo() const
{
    auto *canvas = m_boundCanvas.data();
    if (!canvas) {
        return {};
    }

    const int zoomPct = qRound(canvas->zoomScale() * 100.0);
    const int selected = static_cast<int>(canvas->selectedElements().size());
    const int total = static_cast<int>(canvas->elements().size());
    return tr("Zoom: %1%    Selected: %2 / %3").arg(zoomPct).arg(selected).arg(total);
}

QString QuickAppController::dolphinFileName()
{
    auto *tab = currentTab();
    return tab ? tab->dolphinFileName() : QString();
}

void QuickAppController::setDolphinFileName(const QString &fileName)
{
    if (auto *tab = currentTab()) {
        tab->setDolphinFileName(fileName);
    }
}

void QuickAppController::openWaveform()
{
    Application::guardedSlot(this, [this] {
        if (!m_waveformWindowOpen) {
            auto *canvas = activeCanvas();
            if (!canvas) {
                return;
            }
            auto *tab = currentTab();
            m_dolphinController.createWaveform(canvas, tab ? tab->dolphinFileName() : QString());
            m_waveformWindowOpen = true;
        }
        emit waveformOpenRequested();
    });
}

CanvasItem *QuickAppController::activeCanvas() const
{
    auto *tab = currentTab();
    return tab ? tab->canvas() : nullptr;
}

void QuickAppController::bindCurrentTab()
{
    for (const auto &connection : std::as_const(m_tabConnections)) {
        disconnect(connection);
    }
    m_tabConnections.clear();

    // m_boundCanvas may already be null here: closeTab() erases (and destroys) the previous
    // tab's QuickWorkSpace -- and its CanvasItem -- before currentTabChanged fires for the new
    // current tab, and QPointer is what makes that erase-before-signal ordering safe to observe.
    if (m_boundCanvas) {
        m_boundCanvas->simulation()->stop();
    }

    auto *tab = currentTab();
    auto *canvas = activeCanvas();
    m_boundCanvas = canvas;

    if (!canvas) {
        // Mirrors MainWindow's "no tab" branch: an empty palette IC section, no embedded-IC
        // registry to read from, and the IC toolbar buttons left exactly as the last real tab
        // set them (MainWindow.cpp's own no-tab branch returns before touching them either).
        m_palette.updateICList(QFileInfo());
        m_palette.updateEmbeddedICList(nullptr);
        m_elementEditor.setCanvas(nullptr);
        m_icPreview.setCanvas(nullptr);
        m_minimap.setCanvas(nullptr);
        m_exerciseController.setCanvas(nullptr);
        emit mutedChanged();
        emit visibilityChanged();
        emit statusInfoChanged();
        return;
    }

    // Mirrors MainWindow.cpp's identical tab-switch call site: an inline IC tab has no project
    // file/directory of its own for Add/Remove/Make-Self-Contained to operate on.
    setICButtonsVisible(!tab->isInlineIC());
    refreshICButtonsEnabled();

    applySimulationRunningState(m_simulationRunning);

    auto *undoStack = canvas->undoStack();
    const auto reemit = [this] {
        emit undoRedoStateChanged();
        emit windowTitleChanged();
    };
    m_tabConnections.append(connect(undoStack, &QUndoStack::canUndoChanged, this, reemit));
    m_tabConnections.append(connect(undoStack, &QUndoStack::canRedoChanged, this, reemit));
    m_tabConnections.append(connect(undoStack, &QUndoStack::undoTextChanged, this, reemit));
    m_tabConnections.append(connect(undoStack, &QUndoStack::redoTextChanged, this, reemit));
    m_tabConnections.append(connect(undoStack, &QUndoStack::cleanChanged, this, reemit));

    // statusInfo (zoom % + selection count / total) mirrors SceneUiBinder::bind()'s own
    // zoomChanged/selectionChanged/circuitHasChanged connections -- indexChanged is this
    // codebase's already-established circuit-changed proxy (every element add/delete goes
    // through the undo stack), covering the total-element-count half with no dedicated signal
    // needed on CanvasItem.
    m_tabConnections.append(connect(canvas, &CanvasItem::zoomChanged, this, &QuickAppController::statusInfoChanged));
    m_tabConnections.append(connect(canvas, &CanvasItem::selectionChanged, this, &QuickAppController::statusInfoChanged));
    m_tabConnections.append(connect(undoStack, &QUndoStack::indexChanged, this, &QuickAppController::statusInfoChanged));

    // Mirrors SceneUiBinder::bind()'s updateEmbeddedICList(scene) call plus WorkspaceManager's
    // updateICList(icListFile()) calls on load/save -- currentFile() is used directly rather
    // than icListFile()'s parent-workspace-chain walk, since inline IC tabs have no UI trigger
    // in the Quick chrome yet (see QuickElementPalette::updateICList()'s doc comment).
    m_palette.updateICList(currentFile());
    m_palette.updateEmbeddedICList(canvas->icRegistry());
    m_tabConnections.append(connect(tab, &QuickWorkSpace::fileChanged, this, [this](const QFileInfo &fileInfo) {
        m_palette.updateICList(fileInfo);
    }));

    // Mirrors ElementEditor::setScene(): rebinds the property panel to the new tab's selection.
    m_elementEditor.setCanvas(canvas);

    // Rebinds the IC hover-preview presenter to the new tab's CanvasItem signals -- setCanvas()
    // itself hides any popup left pending/visible from the previously-bound tab.
    m_icPreview.setCanvas(canvas);

    // Rebinds the minimap presenter to the new tab's CanvasItem, triggering an immediate
    // (throttled) thumbnail regen so it shows the newly-current circuit rather than whatever
    // the previously-bound tab last rendered.
    m_minimap.setCanvas(canvas);

    // Rebinds the exercise controller's "current canvas" record; only actually rebinds the
    // underlying ExerciseEngine if an exercise is currently active (mirrors MainWindow.cpp's
    // own tab-switch handler, itself guarded on m_exerciseEngine->isActive()).
    m_exerciseController.setCanvas(canvas);

    // Resyncs the Mute menu item to this tab's own Simulation::isUserMuted() state -- mute is
    // per-tab (unlike simulationRunning's global intent), so a tab switch must re-emit even
    // though setMuted() itself was never called for this canvas.
    emit mutedChanged();
    // Same reasoning as mutedChanged() above: Show Gates/Wires is per-tab CanvasItem state.
    emit visibilityChanged();
    emit statusInfoChanged();
}

void QuickAppController::applySimulationRunningState(bool running)
{
    if (!m_boundCanvas) {
        return;
    }
    if (running) {
        m_boundCanvas->simulation()->start();
    } else {
        m_boundCanvas->simulation()->stop();
    }
}

void QuickAppController::setSimulationRunning(bool running)
{
    if (m_simulationRunning == running) {
        return;
    }
    m_simulationRunning = running;
    applySimulationRunningState(running);
    emit simulationRunningChanged();
}

void QuickAppController::setBackgroundSimulationEnabled(bool enabled)
{
    if (m_backgroundSimulationEnabled == enabled) {
        return;
    }
    m_backgroundSimulationEnabled = enabled;
    emit backgroundSimulationEnabledChanged();
}

void QuickAppController::handleWindowActiveChanged(bool active)
{
    if (m_backgroundSimulationEnabled || !m_simulationRunning || !m_boundCanvas) {
        return;
    }
    if (active) {
        m_boundCanvas->simulation()->start();
    } else {
        m_boundCanvas->simulation()->stop();
    }
}

QString QuickAppController::windowTitle() const
{
    auto *tab = currentTab();
    if (!tab) {
        return QStringLiteral("wiRedPanda " APP_VERSION);
    }
    QString title = m_workspaceManager.tabTitle(tab);
    if (!tab->canvas()->undoStack()->isClean()) {
        title += QStringLiteral("*");
    }
    return title + QStringLiteral(" — wiRedPanda " APP_VERSION);
}

bool QuickAppController::canUndo() const
{
    auto *c = activeCanvas();
    return c && c->undoStack()->canUndo();
}

bool QuickAppController::canRedo() const
{
    auto *c = activeCanvas();
    return c && c->undoStack()->canRedo();
}

bool QuickAppController::hasElements() const
{
    auto *c = activeCanvas();
    return c && !c->elements().isEmpty();
}

QString QuickAppController::undoText() const
{
    auto *c = activeCanvas();
    return c ? c->undoStack()->undoText() : QString();
}

QString QuickAppController::redoText() const
{
    auto *c = activeCanvas();
    return c ? c->undoStack()->redoText() : QString();
}

QString QuickAppController::shortcutsHelpHtml() const
{
    // QString::arg()'s multi-argument overload maxes out at 9 placeholders per call -- 13
    // needed here, so chained across three calls.
    return QStringLiteral(
        "<table>"
        "<tr><td>Ctrl+N</td><td>%1</td></tr>"
        "<tr><td>Ctrl+O</td><td>%2</td></tr>"
        "<tr><td>Ctrl+S</td><td>%3</td></tr>"
        "<tr><td>Ctrl+Z</td><td>%4</td></tr>"
        "<tr><td>Ctrl+Shift+Z</td><td>%5</td></tr>"
        "<tr><td>Ctrl+X / C / V / D</td><td>%6</td></tr>"
        "<tr><td>Del</td><td>%7</td></tr>"
        "<tr><td>Ctrl+A</td><td>%8</td></tr>"
        "<tr><td>Ctrl+R / Ctrl+Shift+R</td><td>%9</td></tr>"
        "<tr><td>Ctrl+H</td><td>%10</td></tr>"
        "<tr><td>[ ] { } &lt; &gt;</td><td>%11</td></tr>"
        "<tr><td>Ctrl+= / Ctrl+-</td><td>%12</td></tr>"
        "<tr><td>Ctrl+0 / Ctrl+Shift+F</td><td>%13</td></tr>"
        "</table>")
        .arg(tr("New project"), tr("Open file"), tr("Save file"), tr("Undo"), tr("Redo"),
             tr("Cut / Copy / Paste / Duplicate"), tr("Delete selection"), tr("Select all"),
             tr("Rotate right / left"))
        .arg(tr("Flip horizontally"), tr("Cycle element type / properties (selected element)"))
        .arg(tr("Zoom in / out"), tr("Reset zoom / zoom to fit"));
}

QString QuickAppController::aboutHtml() const
{
    return tr("<p>wiRedPanda is software developed by students of the Federal University of São Paulo"
               " to help students learn about logic circuits.</p>"
               "<p>Software version: %1</p>"
               "<p><strong>Creators:</strong></p>"
               "<ul>"
               "<li> Davi Morales </li>"
               "<li> Lucas Lellis </li>"
               "<li> Rodrigo Torres </li>"
               "<li> Prof. Fábio Cappabianco, Ph.D. </li>"
               "</ul>"
               "<p> wiRedPanda is currently maintained by Prof. Fábio Cappabianco, Ph.D., João Pedro M. Oliveira, Matheus R. Esteves and Maycon A. Santana.</p>"
               "<p> Please file a report at our GitHub page if you find a bug or want to request a new feature.</p>"
               "<p><a href=\"https://gibis-unifesp.github.io/wiRedPanda/\">Visit our website!</a></p>")
        .arg(appVersion());
}

QString QuickAppController::aboutThisVersionText() const
{
    return tr("wiRedPanda %1\n\n"
               "This version includes automatic migration of older project files.\n"
               "When you open a project file older than the current version, it will be automatically "
               "upgraded to the current format and a versioned backup will be created.\n\n"
               "To open projects containing ICs (or boxes), appearances, and/or beWavedDolphin simulations, "
               "their files must be in the same directory as the main project file.\n"
               "wiRedPanda %1 will automatically list all other .panda files located "
               "in the same directory as the current project as ICs in the editor tab.\n"
               "You have to save new projects before accessing ICs and appearances, or running "
               "beWavedDolphin simulations.")
        .arg(appVersion());
}

QString QuickAppController::aboutQtHtml() const
{
    return tr("<p>This program uses Qt version %1.</p>"
               "<p>Qt is a C++ toolkit for cross-platform application development.</p>"
               "<p>Qt is available under multiple licensing options, including open-source editions "
               "such as the GNU (L)GPL, as well as a commercial license.</p>"
               "<p><a href=\"https://www.qt.io/\">https://www.qt.io/</a></p>")
        .arg(QLatin1String(qVersion()));
}

void QuickAppController::reportTranslationError() const
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://hosted.weblate.org/projects/wiredpanda/wiredpanda")));
}

void QuickAppController::undo()
{
    if (auto *c = activeCanvas()) {
        c->undoStack()->undo();
    }
}

void QuickAppController::redo()
{
    if (auto *c = activeCanvas()) {
        c->undoStack()->redo();
    }
}

void QuickAppController::cut()
{
    if (auto *c = activeCanvas()) c->cutAction();
}

void QuickAppController::copy()
{
    if (auto *c = activeCanvas()) c->copyAction();
}

void QuickAppController::paste()
{
    if (auto *c = activeCanvas()) c->pasteAction();
}

void QuickAppController::duplicateSelection()
{
    if (auto *c = activeCanvas()) c->duplicateAction();
}

void QuickAppController::deleteSelection()
{
    if (auto *c = activeCanvas()) c->deleteSelected();
}

void QuickAppController::selectAll()
{
    if (auto *c = activeCanvas()) c->selectAll();
}

void QuickAppController::rotateRight()
{
    if (auto *c = activeCanvas()) c->rotateRight();
}

void QuickAppController::rotateLeft()
{
    if (auto *c = activeCanvas()) c->rotateLeft();
}

void QuickAppController::flipHorizontal()
{
    if (auto *c = activeCanvas()) c->flipHorizontally();
}

void QuickAppController::flipVertical()
{
    if (auto *c = activeCanvas()) c->flipVertically();
}

bool QuickAppController::isMuted() const
{
    auto *c = activeCanvas();
    return c && c->simulation()->isUserMuted();
}

void QuickAppController::setMuted(bool muted)
{
    auto *c = activeCanvas();
    if (!c || c->simulation()->isUserMuted() == muted) {
        return;
    }
    c->simulation()->setUserMuted(muted);
    emit mutedChanged();
}

bool QuickAppController::labelsUnderIcons() const
{
    return Settings::labelsUnderIcons();
}

void QuickAppController::setLabelsUnderIcons(bool enabled)
{
    if (Settings::labelsUnderIcons() == enabled) {
        return;
    }
    Settings::setLabelsUnderIcons(enabled);
    emit labelsUnderIconsChanged();
}

bool QuickAppController::icPreviewEnabled() const
{
    return !Settings::icPreviewDisabled();
}

void QuickAppController::setIcPreviewEnabled(bool enabled)
{
    if (icPreviewEnabled() == enabled) {
        return;
    }
    Settings::setIcPreviewDisabled(!enabled);
    emit icPreviewEnabledChanged();
}

bool QuickAppController::isGatesVisible() const
{
    auto *c = activeCanvas();
    return !c || c->gatesVisible();
}

void QuickAppController::setGatesVisible(bool visible)
{
    if (auto *c = activeCanvas()) {
        c->setGatesVisible(visible);
        emit visibilityChanged();
    }
}

bool QuickAppController::isWiresVisible() const
{
    auto *c = activeCanvas();
    return !c || c->wiresVisible();
}

void QuickAppController::setWiresVisible(bool visible)
{
    if (auto *c = activeCanvas()) {
        c->setWiresVisible(visible);
        emit visibilityChanged();
    }
}

bool QuickAppController::openDroppedPandaFile(const QList<QUrl> &urls)
{
    for (const QUrl &url : urls) {
        if (!url.isLocalFile()) {
            continue;
        }
        const QString path = url.toLocalFile();
        if (path.endsWith(QLatin1String(".panda"), Qt::CaseInsensitive)) {
            openRecentFile(path);
            return true;
        }
    }
    return false;
}

void QuickAppController::restartSimulation()
{
    if (auto *c = activeCanvas()) c->restartSimulation();
}

void QuickAppController::alignLeft()
{
    if (auto *c = activeCanvas()) c->alignLeft();
}

void QuickAppController::alignRight()
{
    if (auto *c = activeCanvas()) c->alignRight();
}

void QuickAppController::alignTop()
{
    if (auto *c = activeCanvas()) c->alignTop();
}

void QuickAppController::alignBottom()
{
    if (auto *c = activeCanvas()) c->alignBottom();
}

void QuickAppController::alignHorizontalCenter()
{
    if (auto *c = activeCanvas()) c->alignHorizontalCenter();
}

void QuickAppController::alignVerticalCenter()
{
    if (auto *c = activeCanvas()) c->alignVerticalCenter();
}

void QuickAppController::distributeHorizontally()
{
    if (auto *c = activeCanvas()) c->distributeHorizontally();
}

void QuickAppController::distributeVertically()
{
    if (auto *c = activeCanvas()) c->distributeVertically();
}

void QuickAppController::zoomIn()
{
    if (auto *c = activeCanvas()) c->zoomIn();
}

void QuickAppController::zoomOut()
{
    if (auto *c = activeCanvas()) c->zoomOut();
}

void QuickAppController::resetZoom()
{
    if (auto *c = activeCanvas()) c->resetZoom();
}

void QuickAppController::zoomToFit()
{
    if (auto *c = activeCanvas()) c->zoomToFit();
}

void QuickAppController::addElementToCurrentTab(int type, const QString &icFileName, bool isEmbedded, qreal x, qreal y)
{
    if (auto *c = activeCanvas()) {
        c->addElementFromPalette(static_cast<ElementType>(type), icFileName, isEmbedded, QPointF(x, y));
    }
}

bool QuickAppController::canPaste()
{
    const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData();
    // Mirrors the formats ClipboardManager::paste() reads -- copy()/cut() write only the
    // current format, while older app versions wrote the legacy one.
    return mimeData && (mimeData->hasFormat(MimeType::Clipboard) || mimeData->hasFormat(MimeType::ClipboardLegacy));
}

void QuickAppController::runTourDemoAction(const QString &id)
{
    auto *c = activeCanvas();
    if (!c) {
        return;
    }
    if (id == QStringLiteral("setupElementEditorDemo")) {
        c->addTourDemoInputSwitch();
    } else if (id == QStringLiteral("setupWaveformDemo")) {
        c->addTourDemoWaveformCircuit();
    }
}

QList<ExampleEntry> QuickAppController::examplesList() const
{
    QList<ExampleEntry> result;

    const QString examplesPath = InstallRelativePaths::resolve(QStringLiteral("Examples"));
    if (examplesPath.isEmpty()) {
        return result;
    }

    const auto entryList = QDir(examplesPath).entryList({"*.panda"}, QDir::Files);
    for (const auto &entry : entryList) {
        QString title = QFileInfo(entry).completeBaseName();
        title.replace(QLatin1Char('-'), QLatin1Char(' '));
        title.replace(QLatin1Char('_'), QLatin1Char(' '));
        QStringList words = title.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        for (QString &word : words) {
            word[0] = word[0].toUpper();
        }

        result.append(ExampleEntry(words.join(QLatin1Char(' ')), examplesPath + "/" + entry));
    }

    return result;
}

QList<LearnEntry> QuickAppController::exercisesList() const
{
    const QStringList completed = Settings::completedExercises();
    QList<LearnEntry> result;
    for (const auto &entry : ExerciseTourResources::discover(QStringLiteral("Exercises"))) {
        QString title = ExerciseTourResources::translate(entry.id + QStringLiteral(".title"), entry.title);
        title.replace(QLatin1Char('&'), QStringLiteral("&&")); // literal '&' would otherwise be swallowed as a mnemonic marker -- matches MainWindow::populateContentMenu()
        const QString description = ExerciseTourResources::translate(entry.id + QStringLiteral(".description"), entry.description);
        result.append(LearnEntry(title, description, entry.path, completed.contains(entry.id)));
    }
    return result;
}

QList<LearnEntry> QuickAppController::toursList() const
{
    const QStringList completed = Settings::completedTours();
    QList<LearnEntry> result;
    for (const auto &entry : ExerciseTourResources::discover(QStringLiteral("Tours"))) {
        QString title = ExerciseTourResources::translate(entry.id + QStringLiteral(".title"), entry.title);
        title.replace(QLatin1Char('&'), QStringLiteral("&&")); // literal '&' would otherwise be swallowed as a mnemonic marker -- matches MainWindow::populateContentMenu()
        const QString description = ExerciseTourResources::translate(entry.id + QStringLiteral(".description"), entry.description);
        result.append(LearnEntry(title, description, entry.path, completed.contains(entry.id)));
    }
    return result;
}

void QuickAppController::openMyContentFolder(const QString &category) const
{
    const QString dir = ExerciseTourResources::preferredContentDir(category);
    if (dir.isEmpty()) {
        const QString message = category == QStringLiteral("Exercises")
            ? tr("Could not create or access a folder for custom exercises.")
            : tr("Could not create or access a folder for custom tours.");
        Dialogs::provider()->choice(tr("Error"), message, {DialogButton::Ok}, DialogButton::Ok);
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void QuickAppController::editSelectedSubcircuit()
{
    auto *canvas = activeCanvas();
    if (!canvas) {
        return;
    }
    const auto selected = canvas->selectedElements();
    if (selected.isEmpty() || selected.constFirst()->elementType() != ElementType::IC) {
        return;
    }

    auto *elm = selected.constFirst();
    if (elm->isEmbedded()) {
        const QString blobName = elm->blobName();
        m_workspaceManager.openICInTab(blobName, elm->id(), canvas->icRegistry()->blob(blobName));
    } else {
        m_workspaceManager.loadPandaFile(static_cast<IC *>(elm)->file());
    }
}

namespace {
// Qt resource paths (":/Interface/...") need the "qrc" scheme prefix to be usable as a QML
// Image.source URL. Same helper as QuickElementPalette.cpp's own toQmlUrl() -- kept local
// rather than shared, matching that one-line helper's own precedent.
QString toQmlUrl(const QString &resourcePath)
{
    return resourcePath.startsWith(QLatin1Char(':')) ? QStringLiteral("qrc") + resourcePath : resourcePath;
}
} // namespace

QList<LanguageEntry> QuickAppController::languages() const
{
    QList<LanguageEntry> result;
    for (const QString &code : m_languageManager.availableLanguages()) {
        result.append(LanguageEntry(code, m_languageManager.displayName(code), toQmlUrl(m_languageManager.flagIcon(code))));
    }
    return result;
}

QString QuickAppController::currentLanguage() const
{
    const QString language = Settings::language();
    return language.isEmpty() ? QStringLiteral("en") : language;
}

void QuickAppController::setupLanguage()
{
    QString language = Settings::language();
    if (language.isEmpty()) {
        const QLocale systemLocale = QLocale::system();
        const QString systemLang = systemLocale.name();
        const QString baseLang = systemLang.split('_').first();

        const auto available = m_languageManager.availableLanguages();
        if (available.contains(systemLang)) {
            language = systemLang;
        } else if (available.contains(baseLang)) {
            language = baseLang;
        } else {
            language = QStringLiteral("en");
        }
    }

    m_languageManager.loadTranslation(language);
}

QRect QuickAppController::restoreWindowGeometry() const
{
    return Settings::quickWindowGeometry();
}

void QuickAppController::saveWindowGeometry(int x, int y, int width, int height)
{
    Settings::setQuickWindowGeometry(QRect(x, y, width, height));
}

QRect QuickAppController::restoreDolphinWindowGeometry() const
{
    return Settings::quickDolphinGeometry();
}

void QuickAppController::saveDolphinWindowGeometry(int x, int y, int width, int height)
{
    Settings::setQuickDolphinGeometry(QRect(x, y, width, height));
}

bool QuickAppController::confirmClose()
{
    // Main.qml's ApplicationWindow hardcodes visible: true (unlike MainWindow, which --mcp
    // mode never show()s -- see Main.cpp's own doc comment on that difference), so it's always
    // a real, closeable top-level window here, including under --mcp. QCoreApplication::quit()
    // (called on stdin EOF, see QuickMCPProcessor::onStdinReadable()) closes every top-level
    // window as part of shutting down, which fires this via Main.qml's onClosing -- with no
    // user present to answer a confirmation dialog in MCP mode, that would otherwise hang the
    // process forever in a nested QEventLoop parked in QuickDialogProvider::choice(). Application::
    // interactiveMode is already false for the whole --mcp/--mcp-gui session (see Main.cpp), so
    // it's the correct gate.
    if (!Application::interactiveMode) {
        return true;
    }

    if (!m_workspaceManager.hasModifiedFiles()) {
        const DialogButton reply = Dialogs::provider()->choice(
            tr("Exit wiRedPanda"), tr("Are you sure?"), {DialogButton::Yes, DialogButton::Cancel}, DialogButton::Yes);
        return reply == DialogButton::Yes;
    }
    return m_workspaceManager.closeFiles();
}
