// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/WorkspaceManager.h"

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QEvent>
#include <QMouseEvent>
#include <QTabBar>
#include <QTabWidget>
#include <QUndoStack>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/SentryHelpers.h"
#include "App/Core/Settings.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/ElementPalette.h"
#include "App/UI/FileDialogProvider.h"
#include "App/UI/MainWindowHost.h"

WorkspaceManager::WorkspaceManager(QTabWidget *tab, MainWindowHost &host, QObject *parent)
    : QObject(parent)
    , m_tab(tab)
    , m_host(host)
{
    // Watch the tab bar so a middle-click closes the tab under the cursor. Cache the tab bar
    // pointer: the filter must never call back through m_tab, because a teardown event can reach
    // it while m_tab is mid-destruction (its dynamic type already demoted below QTabWidget).
    m_tabBar = m_tab->tabBar();
    m_tabBar->installEventFilter(this);
}

bool WorkspaceManager::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_tabBar && event->type() == QEvent::MouseButtonRelease) {
        const auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::MiddleButton) {
            const int index = m_tabBar->tabAt(mouseEvent->pos());
            if (index >= 0) {
                closeTab(index);
                return true;
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

QFileInfo WorkspaceManager::currentFile() const
{
    return m_currentTab ? m_currentTab->fileInfo() : QFileInfo();
}

QString WorkspaceManager::displayName(const WorkSpace *ws, const QFileInfo &fileInfo) const
{
    if (!ws) {
        return {};
    }
    // Inline IC tabs use "[blobName]" — never the parent filename.
    if (ws->isInlineIC()) {
        return "[" + ws->inlineBlobName() + "]";
    }
    if (fileInfo.exists()) {
        return fileInfo.fileName();
    }
    // Unsaved tab: the numbered placeholder assigned at creation (fall back for any
    // workspace not created through createNewTab), plus a marker if it was recovered from
    // an autosave so the user can tell it apart from a fresh, empty tab.
    QString name = ws->untitledTitle().isEmpty() ? tr("New Project") : ws->untitledTitle();
    if (ws->isRecovered()) {
        name += tr(" (recovered)");
    }
    return name;
}

QString WorkspaceManager::currentTabName() const
{
    return displayName(m_currentTab, currentFile());
}

QDir WorkspaceManager::currentDir() const
{
    return m_currentTab ? m_currentTab->fileInfo().absoluteDir() : QDir();
}

QFileInfo WorkspaceManager::icListFile() const
{
    // Walk up the parent workspace chain to find the root file on disk.
    // Inline IC workspaces have no file of their own.
    auto *ws = m_currentTab;
    while (ws && ws->isInlineIC() && ws->parentWorkspace()) {
        ws = ws->parentWorkspace();
    }
    if (ws) {
        return ws->fileInfo();
    }
    return currentFile();
}

QString WorkspaceManager::dolphinFileName() const
{
    if (!m_currentTab) {
        return {};
    }
    return m_currentTab->dolphinFileName();
}

void WorkspaceManager::setDolphinFileName(const QString &fileName)
{
    if (!m_currentTab) {
        return;
    }
    m_currentTab->setDolphinFileName(fileName);
}

QString WorkspaceManager::nextUntitledTitle() const
{
    // Collect the titles other tabs currently show, minus any trailing unsaved "*".
    QStringList taken;
    taken.reserve(m_tab->count());
    for (int i = 0; i < m_tab->count(); ++i) {
        QString title = m_tab->tabText(i);
        if (title.endsWith(QLatin1Char('*'))) {
            title.chop(1);
        }
        taken.append(title);
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

void WorkspaceManager::createNewTab()
{
    qCDebug(zero) << "Creating new workspace.";
    auto *workspace = new WorkSpace(m_host.widget());

    connect(workspace, &WorkSpace::fileChanged, this, &WorkspaceManager::setCurrentFile);
    connect(workspace->scene()->icRegistry(), &ICRegistry::blobRenamed, this, &WorkspaceManager::onBlobRenamed);

    workspace->scene()->updateTheme();

    // Number the placeholder title so several unsaved tabs stay distinguishable.
    const QString untitledTitle = nextUntitledTitle();
    workspace->setUntitledTitle(untitledTitle);

    qCDebug(zero) << "Adding tab. #tabs: " << m_tab->count() << ", current tab: " << m_tabIndex;
    m_tab->addTab(workspace, untitledTitle);
    sentryBreadcrumb("ui", QStringLiteral("Tab opened"));

    qCDebug(zero) << "Selecting the newly created tab.";
    // The view's fast-mode state is applied when SceneUiBinder binds this tab below.
    m_tab->setCurrentIndex(m_tab->count() - 1);
}

QString WorkspaceManager::promptSavePath(const QString &fileName)
{
    QString resolved = fileName.isEmpty() ? currentFile().absoluteFilePath() : fileName;

    // If saving to an autosave path or no path at all, prompt the user for a real filename.
    const QStringList autosaves = Settings::autosaveFiles();
    if ((resolved.isEmpty() || autosaves.contains(resolved)) && currentFile().fileName().isEmpty()) {
        const QString path = resolved.isEmpty() ? currentFile().absolutePath() : QFileInfo(resolved).absolutePath();
        resolved = FileDialogs::provider()->getSaveFileName(m_host.widget(), tr("Save File"), path, tr("Panda files") + " (*.panda)").fileName;
    }

    if (resolved.isEmpty()) {
        return {};
    }

    if (!resolved.endsWith(".panda")) {
        resolved.append(".panda");
    }

    return resolved;
}

void WorkspaceManager::save(const QString &fileName)
{
    if (!m_currentTab) {
        return;
    }

    if (m_currentTab->isInlineIC()) {
        // No path needed: WorkSpace::save() serializes to a blob and emits a signal instead.
        m_currentTab->save(fileName);
        m_host.palette()->updateICList(icListFile());
        m_host.showStatusMessage(tr("File saved successfully."), 4000);
        return;
    }

    // Mirror the pre-refactor WorkSpace::save() bookkeeping: if the path we're about to
    // resolve is itself a currently-tracked autosave-recovery record, forget that record
    // once the save to the real, resolved path succeeds.
    const QString originalFileName = fileName.isEmpty() ? currentFile().absoluteFilePath() : fileName;
    const bool wasAutosaveRecord = !originalFileName.isEmpty() && Settings::autosaveFiles().contains(originalFileName);

    QString resolvedFileName = promptSavePath(fileName);
    if (resolvedFileName.isEmpty()) {
        return; // Brand-new project with nothing to resolve to, or the user cancelled the dialog.
    }

    for (;;) {
        const auto outcome = m_currentTab->save(resolvedFileName);
        if (outcome == WorkSpace::SaveOutcome::Saved) {
            break;
        }

        // ReadOnlyTarget: WorkSpace::save() only returns this in interactive mode (it
        // throws instead otherwise, since there's no one to dismiss a dialog in a
        // headless/CLI/MCP context), so prompting here is always safe. OneDrive lock,
        // ZIP-extracted folder, network drive, write-protected attribute -- re-prompt for
        // a writable location instead of leaving the user stuck on a modal error.
        const QString newPath = FileDialogs::provider()->getSaveFileName(
            m_host.widget(), tr("Save File (original location is read-only)"),
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

    m_host.palette()->updateICList(icListFile());
    m_host.showStatusMessage(tr("File saved successfully."), 4000);
}

int WorkspaceManager::closeTabAnyway()
{
    QMessageBox msgBox;
    msgBox.setParent(m_host.widget());
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setText(tr("File not saved. Close tab anyway?"));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::No);
    return msgBox.exec();
}

int WorkspaceManager::confirmSave(const bool multiple)
{
    QMessageBox msgBox;
    msgBox.setParent(m_host.widget());

    // When closing all tabs at once, offer "Yes to All" / "No to All" to avoid
    // repeated per-file prompts.
    if (multiple) {
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel);
    } else {
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    }

    const QString fileName = currentFile().fileName().isEmpty() ? tr("New Project") : currentFile().fileName();

    msgBox.setText(fileName + tr(" has been modified.\nDo you want to save your changes?"));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::Yes);
    return msgBox.exec();
}

void WorkspaceManager::newTab()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("ui", QStringLiteral("New project"));
        createNewTab();
    });
}

void WorkspaceManager::loadPandaFile(const QString &fileName)
{
    const QFileInfo newFileInfo(fileName);

    for (int i = 0; i < m_tab->count(); ++i) {
        if (auto *workspace = qobject_cast<WorkSpace *>(m_tab->widget(i))) {
            if (workspace->fileInfo() == newFileInfo) {
                m_tab->setCurrentIndex(i);
                return;
            }
        }
    }

    createNewTab();
    qCDebug(zero) << "Loading in editor.";
    try {
        m_currentTab->load(fileName);
    } catch (...) {
        // Drop the half-populated tab so we don't leak an empty workspace
        // into the tab bar. Clear the undo stack first so closeTab doesn't
        // prompt the user to save the partial load.
        m_currentTab->scene()->undoStack()->clear();
        closeTab(m_tabIndex);
        throw;
    }
    // Tighten the scene rect to the loaded items immediately so subsequent
    // interactions (selection, drag release) don't cause a viewport jump.
    m_currentTab->scene()->resizeScene();
    m_host.palette()->updateICList(icListFile());
    m_host.palette()->updateEmbeddedICList(m_currentTab->scene());
    m_host.showStatusMessage(tr("File loaded successfully."), 4000);
}

void WorkspaceManager::openICInTab(const QString &blobName, int icElementId, const QByteArray &blob)
{
    if (!m_currentTab) {
        return;
    }

    // Check if this blob is already being edited in a tab
    for (int i = 0; i < m_tab->count(); ++i) {
        auto *ws = qobject_cast<WorkSpace *>(m_tab->widget(i));
        if (ws && ws->isInlineIC() && ws->inlineBlobName() == blobName
            && ws->parentWorkspace() == m_currentTab) {
            m_tab->setCurrentIndex(i);
            return;
        }
    }

    auto *parentWorkspace = m_currentTab;

    createNewTab();

    m_currentTab->loadFromBlob(blob, parentWorkspace, icElementId, parentWorkspace->scene()->contextDir());
    // Tighten the scene rect to the loaded items immediately so subsequent
    // interactions (selection, drag release) don't cause a viewport jump.
    m_currentTab->scene()->resizeScene();

    // Hide management buttons for inline tabs (they use currentFile/currentDir which are empty)
    m_host.setICButtonsVisible(false);

    // Set tab title
    int tabIndex = m_tab->indexOf(m_currentTab);
    m_tab->setTabText(tabIndex, "[" + blobName + "]");

    m_host.palette()->updateICList(icListFile());
    m_host.palette()->updateEmbeddedICList(m_currentTab->scene());

    // Connect child tab's save signal to parent
    connect(m_currentTab, &WorkSpace::icBlobSaved, parentWorkspace, &WorkSpace::onChildICBlobSaved);
}

void WorkspaceManager::onBlobRenamed(const QString &oldName, const QString &newName)
{
    for (int i = 0; i < m_tab->count(); ++i) {
        auto *ws = qobject_cast<WorkSpace *>(m_tab->widget(i));
        if (ws && ws->isInlineIC() && ws->inlineBlobName() == oldName) {
            ws->setInlineBlobName(newName);
            m_tab->setTabText(i, "[" + newName + "]");
        }
    }
}

void WorkspaceManager::openFile()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("file", QStringLiteral("Open file dialog"));
    #ifdef Q_OS_WASM
        auto fileContentReady = [this](const QString &fileName, const QByteArray &fileContent) {
            if (!fileName.isEmpty()) {
                // Write file content to a temporary file
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
            }
        };
        QFileDialog::getOpenFileContent("Panda files (*.panda)", fileContentReady);
    #else
        const QString fileName = FileDialogs::provider()->getOpenFileName(m_host.widget(), tr("Open File"), QString(), tr("Panda files") + " (*.panda)");

        if (fileName.isEmpty()) {
            return;
        }

        loadPandaFile(fileName);
    #endif
    });
}

void WorkspaceManager::saveFile()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("file", QStringLiteral("Save"));
        if (!m_currentTab) {
            return;
        }

        // Inline IC tabs serialize to a blob and emit to parent — no file dialog needed.
        if (m_currentTab->isInlineIC()) {
            save(QString());
            return;
        }

    #ifdef Q_OS_WASM
        saveFileAs();
    #else
        // TODO: if current file is autosave ask for filename

        // If the project has never been saved, fall through to a Save-As dialog.
        QString fileName = currentFile().absoluteFilePath();

        if (fileName.isEmpty()) {
            fileName = FileDialogs::provider()->getSaveFileName(m_host.widget(), tr("Save File as ..."), QString(), tr("Panda files") + " (*.panda)").fileName;

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

void WorkspaceManager::saveFileAs()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("file", QStringLiteral("Save as"));
        if (!m_currentTab) {
            return;
        }

    #ifdef Q_OS_WASM
        // Save to a temporary file in the virtual FS, then offer it as a browser download.
        const QString tmpPath = "/tmp/wiredpanda_save.panda";
        save(tmpPath);

        QFile file(tmpPath);
        if (file.open(QIODevice::ReadOnly)) {
            const QByteArray content = file.readAll();
            file.close();

            QString suggestedName = currentFile().fileName();
            if (suggestedName.isEmpty()) {
                suggestedName = "circuit.panda";
            }
            QFileDialog::saveFileContent(content, suggestedName);
        }
    #else
        QString fileName = FileDialogs::provider()->getSaveFileName(m_host.widget(), tr("Save File as ..."), currentFile().absoluteFilePath(), tr("Panda files") + " (*.panda)").fileName;

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

void WorkspaceManager::reloadFile()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("file", QStringLiteral("Reload file"));
        if (!currentFile().exists() || !m_currentTab) {
            return;
        }

        const QString file = currentFile().absoluteFilePath();

        closeTab(m_tabIndex);
        loadPandaFile(file);
    });
}

int WorkspaceManager::findTabWithFile(const QString &fileName) const
{
    const QFileInfo newFileInfo(fileName);
    for (int i = 0; i < m_tab->count(); ++i) {
        if (auto *workspace = qobject_cast<WorkSpace *>(m_tab->widget(i))) {
            if (workspace != m_currentTab && workspace->fileInfo() == newFileInfo) {
                return i;
            }
        }
    }
    return -1;
}

bool WorkspaceManager::warnIfOpenInAnotherTab(const QString &fileName)
{
    const int conflictTab = findTabWithFile(fileName);
    if (conflictTab == -1) {
        return false;
    }

    QMessageBox msgBox(QMessageBox::Warning,
                       tr("File Conflict"),
                       tr("The file \"%1\" is already open in another tab.").arg(QFileInfo(fileName).fileName()),
                       QMessageBox::Ok,
                       m_host.widget());
    QPushButton *switchBtn = msgBox.addButton(tr("Switch to Tab"), QMessageBox::ActionRole);
    msgBox.exec();
    if (msgBox.clickedButton() == switchBtn) {
        m_tab->setCurrentIndex(conflictTab);
    }
    return true;
}

bool WorkspaceManager::closeFiles()
{
    // Close from last to first so that tab indices stay stable during iteration.
    while (m_tab->count() != 0) {
        if (!closeTab(m_tab->count() - 1)) {
            return false;
        }
    }
    return true;
}

bool WorkspaceManager::hasModifiedFiles()
{
    const QStringList autosaves = Settings::autosaveFiles();

    const auto workspaces = m_tab->findChildren<WorkSpace *>();

    for (auto *workspace : workspaces) {
        auto *scene = workspace->scene();
        if (!scene) { // LCOV_EXCL_LINE — unreachable: WorkSpace::scene() returns &m_scene (a value member's address), never null.
            continue; // LCOV_EXCL_LINE — see above.
        }

        auto *undoStack = scene->undoStack();
        if (!undoStack) { // LCOV_EXCL_LINE — unreachable: Scene::undoStack() returns &m_undoStack (a value member's address), never null.
            continue; // LCOV_EXCL_LINE — see above.
        }

        // An un-clean undo stack means uncommitted edits since the last save.
        if (!undoStack->isClean()) {
            return true;
        }

        // An autosave file that is still in the list has not been explicitly
        // saved by the user yet and should be treated as modified. The list
        // stores absolute paths (see WorkSpace::autosave), so compare the
        // workspace's absolute path — not its basename.
        const QString filePath = workspace->fileInfo().absoluteFilePath();

        if (!filePath.isEmpty() && autosaves.contains(filePath)) {
            return true;
        }
    }

    return false;
}

void WorkspaceManager::loadAutosaveFiles()
{
    QStringList autosaves(Settings::autosaveFiles());

    qCDebug(zero) << "All autosave files: " << autosaves;

    for (auto it = autosaves.begin(); it != autosaves.end();) {
        QFile file(*it);

        if (!file.exists()) {
            qCDebug(zero) << "Removing from config the autosave file that does not exist.";
            it = autosaves.erase(it);
            continue;
        }

        try {
            loadPandaFile(*it);
        } catch (const std::exception &e) {
            if (Application::interactiveMode) {
                QMessageBox::critical(nullptr, tr("Error!"), e.what());
            }
            qCDebug(zero) << "Removing autosave file that is corrupted.";
            it = autosaves.erase(it);
            continue;
        }

        // Mark the newly loaded tab so it knows it came from an autosave,
        // causing it to prompt for a real save path on the next Ctrl+S.
        m_currentTab->setAutosaveFile();
        // Flag it as recovered so its title carries a "(recovered)" marker until saved.
        m_currentTab->setRecovered(true);

        ++it;
    }

    Settings::setAutosaveFiles(autosaves);
}

void WorkspaceManager::setCurrentFile(const QFileInfo &fileInfo)
{
    // Find the tab belonging to the workspace that emitted the signal.
    // The sender may differ from m_currentTab (e.g. a parent workspace emitting
    // fileChanged while an inline IC child tab is active).
    auto *senderWs = qobject_cast<WorkSpace *>(sender());
    if (!senderWs) {
        senderWs = m_currentTab;
    }
    if (!senderWs) {
        return;
    }

    const int tabIndex = m_tab->indexOf(senderWs);
    if (tabIndex < 0) {
        return;
    }

    QString text = displayName(senderWs, fileInfo);

    // Append an asterisk to the tab title to indicate unsaved changes,
    // following the common editor convention.
    auto *scene = senderWs->scene();
    if (scene) {
        auto *undoStack = scene->undoStack();
        if (undoStack && !undoStack->isClean()) {
            text += "*";
        }
    }

    m_tab->setTabText(tabIndex, text);

    // Keep the window title in step when it's the visible tab that changed.
    if (senderWs == m_currentTab) {
        emit titleChanged();
    }

    // Only update tooltip and recent files for file-backed tabs.
    if (!senderWs->isInlineIC()) {
        m_tab->setTabToolTip(tabIndex, fileInfo.absoluteFilePath());
        qCDebug(zero) << "Adding file to recent files.";
        emit recentFileAdded(fileInfo.absoluteFilePath());
    }

    if (senderWs == m_currentTab) {
        m_host.refreshICButtonsEnabled();
    }
}

bool WorkspaceManager::closeTab(const int tabIndex)
{
    qCDebug(zero) << "Closing tab " << tabIndex + 1 << ", #tabs: " << m_tab->count();
    // Activate the tab being closed so m_currentTab reflects the right workspace
    // before we inspect its undo stack.
    m_tab->setCurrentIndex(tabIndex);

    qCDebug(zero) << "Checking if needs to save file.";

    bool needsSave = false;
    if (m_currentTab) {
        auto *scene = m_currentTab->scene();
        if (scene) {
            auto *undoStack = scene->undoStack();
            needsSave = undoStack && !undoStack->isClean();
        }
    }

    if (needsSave) {
        const int selectedButton = confirmSave(false);

        if (selectedButton == QMessageBox::Cancel) {
            return false;
        }

        if (selectedButton == QMessageBox::Yes) {
            try {
                save();
            } catch (const std::exception &e) {
                QMessageBox::critical(m_host.widget(), tr("Error"), e.what());

                // If saving failed ask whether to discard and close anyway.
                if (closeTabAnyway() == QMessageBox::No) {
                    return false;
                }
            }
        }
    }

    qCDebug(zero) << "Deleting tab.";
    m_currentTab->deleteLater();
    sentryBreadcrumb("ui", QStringLiteral("Tab closed"));
    m_tab->removeTab(tabIndex);

    qCDebug(zero) << "Closed tab " << tabIndex << ", #tabs: " << m_tab->count() << ", current tab: " << m_tabIndex;

    return true;
}

void WorkspaceManager::onCurrentIndexChanged(int newIndex)
{
    sentryBreadcrumb("ui", QStringLiteral("Tab changed to index %1").arg(newIndex));
    m_tabIndex = newIndex;
    m_currentTab = (newIndex == -1) ? nullptr : qobject_cast<WorkSpace *>(m_tab->currentWidget());
    qCDebug(zero) << "Selecting tab: " << newIndex;
    emit currentTabChanged(m_currentTab);
}
