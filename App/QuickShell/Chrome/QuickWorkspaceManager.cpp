// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickWorkspaceManager.h"

#include <QFile>
#include <QQmlEngine>
#include <QUndoStack>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/SentryHelpers.h"
#include "App/Core/Settings.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/UI/FileDialogProvider.h"

QuickWorkspaceManager::QuickWorkspaceManager(QObject *parent)
    : QObject(parent)
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

QFileInfo QuickWorkspaceManager::currentFile() const
{
    return m_currentTab ? m_currentTab->fileInfo() : QFileInfo();
}

QString QuickWorkspaceManager::tabTitle(QuickWorkSpace *tab) const
{
    return tab ? tab->title() : QString();
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
    // Owned here via m_tabs (a vector of unique_ptr) -- constructed with no QObject parent
    // (QuickWorkspaceManager isn't a QQuickItem/can't parent it in a QML-meaningful way), so
    // without this, QML's default "no parent at first JS exposure" rule would mark it
    // JavaScriptOwnership the first time AppController.currentTab/tabAt() hands it to QML,
    // and the GC could delete it out from under this vector. Same mechanism, same real
    // reproduced crash, as QuickWorkSpace's own m_canvas fix -- see that constructor's comment
    // for the full story.
    QQmlEngine::setObjectOwnership(workspace.get(), QQmlEngine::CppOwnership);

    connect(workspace.get(), &QuickWorkSpace::fileChanged, this, &QuickWorkspaceManager::onTabFileChanged);
    // WorkspaceManager also connects ICRegistry::blobRenamed here to retitle an open inline-IC
    // tab when its tracked blob is renamed elsewhere -- CanvasICRegistry (Phase 3, scoped to
    // the blob-storage core) has no such signal yet, and isn't even a QObject. Deliberately not
    // ported: an inline-IC tab's title would go stale after an out-of-tab rename until this
    // lands. Named here rather than silently dropped.

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
        // Palette IC-list refresh + status message: no palette exists yet (sub-step 4).
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

    // Palette IC-list refresh + status message: no palette exists yet (sub-step 4).
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
    // Palette IC-list refresh + status message: no palette exists yet (sub-step 4).
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

    // IC management button visibility + palette refresh: no chrome exists yet (sub-steps 3/4).

    connect(m_currentTab, &QuickWorkSpace::icBlobSaved, parentWorkspace, &QuickWorkSpace::onChildICBlobSaved);
}

void QuickWorkspaceManager::openFile()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("file", QStringLiteral("Open file dialog"));
        const QString fileName = FileDialogs::provider()->getOpenFileName(nullptr, tr("Open File"), QString(), tr("Panda files") + " (*.panda)");

        if (fileName.isEmpty()) {
            return;
        }

        loadPandaFile(fileName);
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
    });
}

void QuickWorkspaceManager::saveFileAs()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("file", QStringLiteral("Save as"));
        if (!m_currentTab) {
            return;
        }

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

    // Simplified from WorkspaceManager's version -- see this method's header doc comment.
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

    // IC-button enabled-state refresh: no chrome exists yet (sub-step 3).
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

    return true;
}
