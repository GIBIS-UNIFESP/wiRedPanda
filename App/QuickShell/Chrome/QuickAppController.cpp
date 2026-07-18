// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickAppController.h"

#include <QClipboard>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLocale>
#include <QPointF>
#include <QQmlEngine>
#include <QUndoStack>

#include "App/Core/InstallRelativePaths.h"
#include "App/Core/Settings.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/DialogProvider.h"
#include "App/QuickShell/Chrome/QuickICPreview.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/Scene/ClipboardManager.h"
#include "App/Simulation/Simulation.h"

QuickAppController::QuickAppController(QObject *parent)
    : QObject(parent)
    , m_exportController(*this)
    , m_icController(*this)
{
    // m_palette/m_elementEditor/m_icPreview are plain member subobjects (not heap-allocated),
    // exposed to QML via elementPalette()/elementEditor()/icPreview()'s CONSTANT Q_PROPERTYs.
    // Without this, Qt/QML's default "no QObject parent at first JS exposure ->
    // JavaScriptOwnership" rule would apply to them exactly as it did to QuickWorkSpace's
    // m_canvas (see that class's constructor for the full story, including the real SIGSEGV
    // that found it) -- except here the GC calling delete on a pointer to a non-heap-allocated
    // member would be undefined behavior, not just a dangling pointer. Defensive fix applied
    // preemptively (not yet reproduced as a crash for these specifically, but the risk is the
    // same mechanism and strictly more severe).
    QQmlEngine::setObjectOwnership(&m_palette, QQmlEngine::CppOwnership);
    QQmlEngine::setObjectOwnership(&m_elementEditor, QQmlEngine::CppOwnership);
    QQmlEngine::setObjectOwnership(&m_icPreview, QQmlEngine::CppOwnership);

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

    setupLanguage();
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

void QuickAppController::showStatusMessage(const QString & /*message*/, int /*timeout*/)
{
    // No-op until a status bar exists (sub-step 3's remaining chrome / sub-step 7).
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

    // Mirrors SceneUiBinder::unbind()'s "stop so it doesn't keep running in the background".
    // m_boundCanvas is a QPointer since QuickWorkspaceManager::closeTab() erases the closed
    // QuickWorkSpace -- and therefore its CanvasItem -- before emitting currentTabChanged for
    // whatever tab becomes current next, so the previously-bound canvas may already be gone.
    if (m_boundCanvas) {
        if (auto *sim = m_boundCanvas->simulation()) {
            sim->stop();
        }
    }
    m_boundCanvas = nullptr;

    auto *tab = currentTab();
    auto *canvas = activeCanvas();
    if (!canvas) {
        // Mirrors MainWindow's "no tab" branch (MainWindow.cpp's setupExamplesMenu()-adjacent
        // startup code): an empty palette IC section, no embedded-IC registry to read from.
        m_palette.updateICList(QFileInfo());
        m_palette.updateEmbeddedICList(nullptr);
        m_elementEditor.setCanvas(nullptr);
        m_icPreview.setCanvas(nullptr);
        return;
    }

    // Mirrors SceneUiBinder::bind()'s "if (m_ui->actionPlay->isChecked())" resume-on-switch --
    // symmetric here (explicit stop too), unlike bind()'s start-only version, because a freshly
    // constructed CanvasItem always auto-starts its own Simulation (unlike Scene, which starts
    // paused until something calls start()); binding into a paused session must actively stop
    // a brand-new tab's already-running simulation, not just skip starting it.
    applySimulationRunningState(m_simulationRunning);
    m_boundCanvas = canvas;

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

    // Mirrors SceneUiBinder::bind()'s updateEmbeddedICList(scene) call plus WorkspaceManager's
    // updateICList(icListFile()) calls on load/save -- currentFile() is used directly rather
    // than icListFile()'s parent-workspace-chain walk, since inline IC tabs have no UI trigger
    // in the Quick chrome yet (see QuickElementPalette::updateICList()'s doc comment).
    m_palette.updateICList(currentFile());
    m_palette.updateEmbeddedICList(canvas->icRegistry());
    m_tabConnections.append(connect(tab, &QuickWorkSpace::fileChanged, this, [this](const QFileInfo &fileInfo) {
        m_palette.updateICList(fileInfo);
    }));

    // Mirrors ElementEditor::setScene(): rebinds the property panel to the new tab's
    // selection. Closes the "element-editor rebinding" deferral named back in sub-step 3.
    m_elementEditor.setCanvas(canvas);

    // Rebinds the IC hover-preview presenter to the new tab's CanvasItem signals -- setCanvas()
    // itself hides any popup left pending/visible from the previously-bound tab.
    m_icPreview.setCanvas(canvas);
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

void QuickAppController::mute(bool muted)
{
    if (auto *c = activeCanvas()) c->mute(muted);
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
    return ClipboardManager::canPaste(QGuiApplication::clipboard()->mimeData());
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

bool QuickAppController::confirmClose()
{
    if (!m_workspaceManager.hasModifiedFiles()) {
        const DialogButton reply = Dialogs::provider()->choice(
            tr("Exit wiRedPanda"), tr("Are you sure?"), {DialogButton::Yes, DialogButton::Cancel}, DialogButton::Yes);
        return reply == DialogButton::Yes;
    }
    return m_workspaceManager.closeFiles();
}

void QuickAppController::applySimulationRunningState(bool running)
{
    if (auto *c = activeCanvas()) {
        if (auto *sim = c->simulation()) {
            running ? sim->start() : sim->stop();
        }
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
    if (active) {
        if (m_simulationRunning) {
            applySimulationRunningState(true);
        }
    } else if (!m_backgroundSimulationEnabled) {
        applySimulationRunningState(false);
    }
}
