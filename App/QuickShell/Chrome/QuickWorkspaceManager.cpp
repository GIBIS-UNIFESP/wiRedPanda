// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickWorkspaceManager.h"

#include <QFile>
#ifdef Q_OS_WASM
#include <QFileDialog>
#endif
#include <QUndoStack>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/SentryHelpers.h"
#include "App/Core/Settings.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/UI/FileDialogProvider.h"

QuickWorkspaceManager::QuickWorkspaceManager(QuickMainWindowHost &host, QObject *parent)
    : QObject(parent)
    , m_host(host)
{
}

QuickWorkSpace *QuickWorkspaceManager::tabAt(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_tabs.size())) {
        return nullptr;
    }
    return m_tabs.at(static_cast<size_t>(index)).get();
}

int QuickWorkspaceManager::indexOf(QuickWorkSpace *tab) const
{
    for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i) {
        if (m_tabs.at(static_cast<size_t>(i)).get() == tab) {
            return i;
        }
    }
    return -1;
}

void QuickWorkspaceManager::setCurrentIndex(int index)
{
    if (index == m_currentIndex) {
        return;
    }
    m_currentIndex = index;
    m_currentTab = (index < 0 || index >= static_cast<int>(m_tabs.size())) ? nullptr : m_tabs.at(static_cast<size_t>(index)).get();
    sentryBreadcrumb("ui", QStringLiteral("Tab changed to index %1").arg(index));
    emit currentTabChanged(m_currentTab);
}

void QuickWorkspaceManager::moveTab(int from, int to)
{
    const int count = static_cast<int>(m_tabs.size());
    if (from < 0 || from >= count || to < 0 || to >= count || from == to) {
        return;
    }

    auto moved = std::move(m_tabs.at(static_cast<size_t>(from)));
    m_tabs.erase(m_tabs.begin() + from);
    m_tabs.insert(m_tabs.begin() + to, std::move(moved));

    // m_currentTab itself is untouched by the reorder -- only re-derive where it now sits.
    const int newCurrentIndex = indexOf(m_currentTab);
    if (newCurrentIndex != m_currentIndex) {
        m_currentIndex = newCurrentIndex;
        emit currentTabChanged(m_currentTab);
    }
    sentryBreadcrumb("ui", QStringLiteral("Tab moved from index %1 to %2").arg(from).arg(to));
    emit tabsChanged();
}

QFileInfo QuickWorkspaceManager::currentFile() const
{
    return m_currentTab ? m_currentTab->fileInfo() : QFileInfo();
}

QString QuickWorkspaceManager::tabTitle(QuickWorkSpace *tab) const
{
    if (!tab) {
        return {};
    }
    if (tab->isInlineIC()) {
        return "[" + tab->inlineBlobName() + "]";
    }
    const QFileInfo fileInfo = tab->fileInfo();
    if (fileInfo.exists()) {
        return fileInfo.fileName();
    }
    QString name = tab->untitledTitle().isEmpty() ? tr("New Project") : tab->untitledTitle();
    if (tab->isRecovered()) {
        name += tr(" (recovered)");
    }
    return name;
}

QString QuickWorkspaceManager::currentTabName() const
{
    return tabTitle(m_currentTab);
}

QDir QuickWorkspaceManager::currentDir() const
{
    return m_currentTab ? m_currentTab->fileInfo().absoluteDir() : QDir();
}

QFileInfo QuickWorkspaceManager::icListFile() const
{
    // Walk up the parent workspace chain to find the root file on disk. Inline IC workspaces
    // have no file of their own.
    auto *ws = m_currentTab;
    while (ws && ws->isInlineIC() && ws->parentWorkspace()) {
        ws = ws->parentWorkspace();
    }
    if (ws) {
        return ws->fileInfo();
    }
    return currentFile();
}

QString QuickWorkspaceManager::dolphinFileName() const
{
    return m_currentTab ? m_currentTab->dolphinFileName() : QString();
}

void QuickWorkspaceManager::setDolphinFileName(const QString &fileName)
{
    if (m_currentTab) {
        m_currentTab->setDolphinFileName(fileName);
    }
}

QString QuickWorkspaceManager::nextUntitledTitle() const
{
    QStringList taken;
    taken.reserve(static_cast<qsizetype>(m_tabs.size()));
    for (const auto &tab : m_tabs) {
        taken.append(tabTitle(tab.get()));
    }

    const QString base = tr("New Project");
    if (!taken.contains(base)) {
        return base;
    }
    for (int n = 2;; ++n) {
        const QString candidate = tr("New Project %1").arg(n);
        if (!taken.contains(candidate)) {
            return candidate;
        }
    }
}

void QuickWorkspaceManager::createNewTab()
{
    auto workspace = std::make_unique<QuickWorkSpace>();

    // workspace has no QObject parent (owned via m_tabs' unique_ptr instead) -- once it
    // crosses into QML/JS (AppController.currentTab/tabAt() both return QuickWorkSpace*,
    // exposed as QML_ANONYMOUS), QML's ownership heuristic would otherwise default it to
    // JavaScriptOwnership and its GC could delete it out from under this unique_ptr, causing
    // a double-free when m_tabs is destroyed.
    QQmlEngine::setObjectOwnership(workspace.get(), QQmlEngine::CppOwnership);

    connect(workspace.get(), &QuickWorkSpace::fileChanged, this, &QuickWorkspaceManager::onTabFileChanged);
    // Mirrors WorkspaceManager's identical per-workspace connection: retitles an open inline-IC
    // tab when its tracked blob is renamed elsewhere. Connected here (not just for inline tabs)
    // since createNewTab() is the single shared construction path for both root and inline tabs
    // (openICInTab() below calls this first) -- every canvas's own registry can rename a blob
    // some other tab's inline view is tracking.
    connect(workspace->canvas()->icRegistry(), &CanvasICRegistry::blobRenamed, this, &QuickWorkspaceManager::onBlobRenamed);
    // Mirrors SceneUiBinder.cpp's identical Scene::icOpenRequested connection: an embedded IC's
    // blob is read from *this* tab's own canvas registry and opened inline; a file-backed IC's
    // path is opened as a new top-level tab. Connected per-tab (not just the current one) for
    // the same reason as blobRenamed above -- createNewTab() is the shared construction path.
    connect(workspace->canvas(), &CanvasItem::icOpenRequested, this,
            [this, canvas = workspace->canvas()](int elementId, const QString &blobName, const QString &filePath) {
                if (!blobName.isEmpty()) {
                    openICInTab(blobName, elementId, canvas->icRegistry()->blob(blobName));
                } else if (!filePath.isEmpty()) {
                    loadPandaFile(filePath);
                }
            });

    const QString untitledTitle = nextUntitledTitle();
    workspace->setUntitledTitle(untitledTitle);

    m_tabs.push_back(std::move(workspace));
    sentryBreadcrumb("ui", QStringLiteral("Tab opened"));
    emit tabsChanged();

    setCurrentIndex(static_cast<int>(m_tabs.size()) - 1);
}

QString QuickWorkspaceManager::promptSavePath(const QString &fileName)
{
    QString resolved = fileName.isEmpty() ? currentFile().absoluteFilePath() : fileName;

    const QStringList autosaves = Settings::autosaveFiles();
    if ((resolved.isEmpty() || autosaves.contains(resolved)) && currentFile().fileName().isEmpty()) {
        const QString path = resolved.isEmpty() ? currentFile().absolutePath() : QFileInfo(resolved).absolutePath();
        resolved = FileDialogs::provider()->getSaveFileName(nullptr, tr("Save File"), path, tr("Panda files") + " (*.panda)").fileName;
    }

    if (resolved.isEmpty()) {
        return {};
    }

    if (!resolved.endsWith(".panda")) {
        resolved.append(".panda");
    }

    return resolved;
}

void QuickWorkspaceManager::save(const QString &fileName)
{
    if (!m_currentTab) {
        return;
    }

    if (m_currentTab->isInlineIC()) {
        m_currentTab->save(fileName);
        m_host.showStatusMessage(tr("File saved successfully."), 4000);
        // TODO: refresh the palette's IC list here.
        return;
    }

    const QString originalFileName = fileName.isEmpty() ? currentFile().absoluteFilePath() : fileName;
    const bool wasAutosaveRecord = !originalFileName.isEmpty() && Settings::autosaveFiles().contains(originalFileName);

    QString resolvedFileName = promptSavePath(fileName);
    if (resolvedFileName.isEmpty()) {
        return;
    }

    for (;;) {
        const auto outcome = m_currentTab->save(resolvedFileName);
        if (outcome == QuickWorkSpace::SaveOutcome::Saved) {
            break;
        }

        const QString newPath = FileDialogs::provider()->getSaveFileName(
            nullptr, tr("Save File (original location is read-only)"),
            QFileInfo(resolvedFileName).fileName(),
            tr("Panda files") + " (*.panda)").fileName;
        if (newPath.isEmpty()) {
            return;
        }
        resolvedFileName = newPath.endsWith(".panda") ? newPath : newPath + ".panda";
    }

    if (wasAutosaveRecord) {
        QStringList autosaves = Settings::autosaveFiles();
        autosaves.removeAll(originalFileName);
        Settings::setAutosaveFiles(autosaves);
    }

    m_host.showStatusMessage(tr("File saved successfully."), 4000);
    // TODO: refresh the palette's IC list here.
}

int QuickWorkspaceManager::closeTabAnyway()
{
    const DialogButton result = Dialogs::provider()->choice(
        QString(), tr("File not saved. Close tab anyway?"), {DialogButton::Yes, DialogButton::No}, DialogButton::No);
    return static_cast<int>(result);
}

DialogButton QuickWorkspaceManager::confirmSave(const bool multiple)
{
    const QString fileName = currentFile().fileName().isEmpty() ? tr("New Project") : currentFile().fileName();
    const QString text = fileName + tr(" has been modified.\nDo you want to save your changes?");

    const QList<DialogButton> buttons = multiple
        ? QList<DialogButton>{DialogButton::Yes, DialogButton::YesToAll, DialogButton::No, DialogButton::NoToAll, DialogButton::Cancel}
        : QList<DialogButton>{DialogButton::Yes, DialogButton::No, DialogButton::Cancel};

    return Dialogs::provider()->choice(QString(), text, buttons, DialogButton::Yes);
}

void QuickWorkspaceManager::newTab()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("ui", QStringLiteral("New project"));
        createNewTab();
    });
}

void QuickWorkspaceManager::loadPandaFile(const QString &fileName)
{
    const QFileInfo newFileInfo(fileName);

    for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i) {
        if (m_tabs.at(static_cast<size_t>(i))->fileInfo() == newFileInfo) {
            setCurrentIndex(i);
            return;
        }
    }

    createNewTab();
    try {
        m_currentTab->load(fileName);
    } catch (...) {
        m_currentTab->canvas()->undoStack()->clear();
        closeTab(m_currentIndex);
        throw;
    }
    m_host.showStatusMessage(tr("File loaded successfully."), 4000);
    // TODO: refresh the palette's IC list here.
}

void QuickWorkspaceManager::openICInTab(const QString &blobName, int icElementId, const QByteArray &blob)
{
    if (!m_currentTab) {
        return;
    }

    for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i) {
        auto *ws = m_tabs.at(static_cast<size_t>(i)).get();
        if (ws->isInlineIC() && ws->inlineBlobName() == blobName && ws->parentWorkspace() == m_currentTab) {
            setCurrentIndex(i);
            return;
        }
    }

    auto *parentWorkspace = m_currentTab;

    createNewTab();

    m_currentTab->loadFromBlob(blob, parentWorkspace, icElementId, parentWorkspace->canvas()->contextDir());

    // createNewTab()'s own setCurrentIndex() call already fired currentTabChanged() (and thus
    // QuickAppController::bindCurrentTab()'s generic setICButtonsVisible(!isInlineIC()) gating)
    // BEFORE loadFromBlob() above set isInlineIC() true -- so that generic gating saw the new
    // tab while it still looked like a normal one. Mirrors WorkspaceManager::openICInTab()'s own
    // explicit `m_host.setICButtonsVisible(false)` call here for exactly this reason: these
    // buttons use currentFile/currentDir, which are empty for an inline tab.
    m_host.setICButtonsVisible(false);

    // TODO: refresh the palette's IC list here.

    connect(m_currentTab, &QuickWorkSpace::icBlobSaved, parentWorkspace, &QuickWorkSpace::onChildICBlobSaved);
}

void QuickWorkspaceManager::openFile()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("file", QStringLiteral("Open file dialog"));
#ifdef Q_OS_WASM
        // A browser sandbox has no real filesystem path to hand FileDialogs::provider() --
        // route through the browser's own upload API instead, mirroring WorkspaceManager::
        // openFile()'s identical WASM branch exactly.
        auto fileContentReady = [this](const QString &fileName, const QByteArray &fileContent) {
            if (fileName.isEmpty()) {
                return;
            }
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly)) {
                return;
            }
            if (file.write(fileContent) != fileContent.size()) {
                file.close();
                return;
            }
            file.close();
            loadPandaFile(fileName);
        };
        QFileDialog::getOpenFileContent("Panda files (*.panda)", fileContentReady);
#else
        const QString fileName = FileDialogs::provider()->getOpenFileName(nullptr, tr("Open File"), QString(), tr("Panda files") + " (*.panda)");

        if (fileName.isEmpty()) {
            return;
        }

        loadPandaFile(fileName);
#endif
    });
}

void QuickWorkspaceManager::saveFile()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("file", QStringLiteral("Save"));
        if (!m_currentTab) {
            return;
        }

        if (m_currentTab->isInlineIC()) {
            save(QString());
            return;
        }

#ifdef Q_OS_WASM
        // No real filesystem to save into directly -- always route through the Save-As browser
        // download flow, mirroring WorkspaceManager::saveFile()'s identical WASM branch exactly.
        saveFileAs();
#else
        QString fileName = currentFile().absoluteFilePath();

        if (fileName.isEmpty()) {
            fileName = FileDialogs::provider()->getSaveFileName(nullptr, tr("Save File as ..."), QString(), tr("Panda files") + " (*.panda)").fileName;

            if (fileName.isEmpty()) {
                return;
            }

            if (!fileName.endsWith(".panda")) {
                fileName.append(".panda");
            }
        }

        if (warnIfOpenInAnotherTab(fileName)) {
            return;
        }

        save(fileName);
#endif
    });
}

void QuickWorkspaceManager::saveFileAs()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("file", QStringLiteral("Save as"));
        if (!m_currentTab) {
            return;
        }

#ifdef Q_OS_WASM
        // Save to a temporary file in the virtual FS, then offer it as a browser download --
        // mirrors WorkspaceManager::saveFileAs()'s identical WASM branch exactly.
        const QString tmpPath = QStringLiteral("/tmp/wiredpanda_save.panda");
        save(tmpPath);

        QFile file(tmpPath);
        if (file.open(QIODevice::ReadOnly)) {
            const QByteArray content = file.readAll();
            file.close();

            QString suggestedName = currentFile().fileName();
            if (suggestedName.isEmpty()) {
                suggestedName = QStringLiteral("circuit.panda");
            }
            QFileDialog::saveFileContent(content, suggestedName);
        }
#else
        QString fileName = FileDialogs::provider()->getSaveFileName(nullptr, tr("Save File as ..."), currentFile().absoluteFilePath(), tr("Panda files") + " (*.panda)").fileName;

        if (fileName.isEmpty()) {
            return;
        }

        if (!fileName.endsWith(".panda")) {
            fileName.append(".panda");
        }

        if (warnIfOpenInAnotherTab(fileName)) {
            return;
        }

        save(fileName);
#endif
    });
}

void QuickWorkspaceManager::reloadFile()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("file", QStringLiteral("Reload file"));
        if (!currentFile().exists() || !m_currentTab) {
            return;
        }

        const QString file = currentFile().absoluteFilePath();

        closeTab(m_currentIndex);
        loadPandaFile(file);
    });
}

int QuickWorkspaceManager::findTabWithFile(const QString &fileName) const
{
    const QFileInfo newFileInfo(fileName);
    for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i) {
        if (m_tabs.at(static_cast<size_t>(i)).get() != m_currentTab && m_tabs.at(static_cast<size_t>(i))->fileInfo() == newFileInfo) {
            return i;
        }
    }
    return -1;
}

bool QuickWorkspaceManager::warnIfOpenInAnotherTab(const QString &fileName)
{
    const int conflictTab = findTabWithFile(fileName);
    if (conflictTab == -1) {
        return false;
    }

    Dialogs::provider()->choice(
        tr("File Conflict"),
        tr("The file \"%1\" is already open in another tab.").arg(QFileInfo(fileName).fileName()),
        {DialogButton::Ok}, DialogButton::Ok);
    return true;
}

bool QuickWorkspaceManager::closeFiles()
{
    while (!m_tabs.empty()) {
        if (!closeTab(static_cast<int>(m_tabs.size()) - 1)) {
            return false;
        }
    }
    return true;
}

bool QuickWorkspaceManager::hasModifiedFiles()
{
    const QStringList autosaves = Settings::autosaveFiles();

    for (const auto &workspace : m_tabs) {
        auto *undoStack = workspace->canvas()->undoStack();

        if (!undoStack->isClean()) {
            return true;
        }

        const QString filePath = workspace->fileInfo().absoluteFilePath();
        if (!filePath.isEmpty() && autosaves.contains(filePath)) {
            return true;
        }
    }

    return false;
}

void QuickWorkspaceManager::loadAutosaveFiles()
{
    QStringList autosaves(Settings::autosaveFiles());

    for (auto it = autosaves.begin(); it != autosaves.end();) {
        QFile file(*it);

        if (!file.exists()) {
            it = autosaves.erase(it);
            continue;
        }

        try {
            loadPandaFile(*it);
        } catch (const std::exception &e) {
            if (Application::interactiveMode) {
                Dialogs::provider()->choice(tr("Error!"), QString::fromUtf8(e.what()), {DialogButton::Ok}, DialogButton::Ok);
            }
            it = autosaves.erase(it);
            continue;
        }

        m_currentTab->setAutosaveFile();
        m_currentTab->setRecovered(true);

        ++it;
    }

    Settings::setAutosaveFiles(autosaves);
}

void QuickWorkspaceManager::onTabFileChanged(const QFileInfo &fileInfo)
{
    auto *senderWs = qobject_cast<QuickWorkSpace *>(sender());
    if (!senderWs) {
        senderWs = m_currentTab;
    }
    if (!senderWs) {
        return;
    }

    if (indexOf(senderWs) < 0) {
        return;
    }

    if (senderWs == m_currentTab) {
        emit titleChanged();
    }

    if (!senderWs->isInlineIC()) {
        emit recentFileAdded(fileInfo.absoluteFilePath());
    }

    if (senderWs == m_currentTab) {
        m_host.refreshICButtonsEnabled();
    }
}

void QuickWorkspaceManager::onBlobRenamed(const QString &oldName, const QString &newName)
{
    for (auto &ws : m_tabs) {
        if (ws->isInlineIC() && ws->inlineBlobName() == oldName) {
            ws->setInlineBlobName(newName);
            if (ws.get() == m_currentTab) {
                emit titleChanged();
            }
        }
    }
}

bool QuickWorkspaceManager::closeTab(const int tabIndex)
{
    setCurrentIndex(tabIndex);

    bool needsSave = false;
    if (m_currentTab) {
        needsSave = !m_currentTab->canvas()->undoStack()->isClean();
    }

    if (needsSave) {
        const DialogButton selectedButton = confirmSave(false);

        if (selectedButton == DialogButton::Cancel) {
            return false;
        }

        if (selectedButton == DialogButton::Yes) {
            try {
                save();
            } catch (const std::exception &e) {
                Dialogs::provider()->choice(tr("Error"), QString::fromUtf8(e.what()), {DialogButton::Ok}, DialogButton::Ok);

                if (closeTabAnyway() == static_cast<int>(DialogButton::No)) {
                    return false;
                }
            }
        }
    }

    removeTabAt(tabIndex);
    return true;
}

void QuickWorkspaceManager::removeTabWithoutPrompt(int tabIndex)
{
    setCurrentIndex(tabIndex);
    removeTabAt(tabIndex);
}

void QuickWorkspaceManager::removeTabAt(int tabIndex)
{
    sentryBreadcrumb("ui", QStringLiteral("Tab closed"));
    m_tabs.erase(m_tabs.begin() + tabIndex);

    // Keep m_currentTab/m_currentIndex consistent: closing shifts every later index down by
    // one, and the "current" tab (if any remain) is whatever now occupies the closed slot (or
    // the new last tab, if the closed one was the last).
    if (m_tabs.empty()) {
        m_currentIndex = -1;
        m_currentTab = nullptr;
        emit currentTabChanged(nullptr);
    } else {
        const int newIndex = qMin(tabIndex, static_cast<int>(m_tabs.size()) - 1);
        m_currentIndex = -1; // force setCurrentIndex() to re-emit even if the numeric index is unchanged
        setCurrentIndex(newIndex);
    }
    emit tabsChanged();
}
