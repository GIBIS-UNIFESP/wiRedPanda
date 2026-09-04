// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickICController.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUndoStack>

#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Core/ItemWithId.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/IO/FileUtils.h"
#include "App/IO/Serialization.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/DialogProvider.h"
#include "App/QuickShell/Chrome/QuickElementPalette.h"
#include "App/QuickShell/Chrome/QuickMainWindowHost.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/Simulation/Simulation.h"
#include "App/UI/FileDialogProvider.h"

QuickICController::QuickICController(QuickMainWindowHost &host, QObject *parent)
    : QObject(parent)
    , m_host(host)
{
}

QString QuickICController::resolveUniqueBlobName(const QString &initialName, CanvasItem *canvas)
{
    auto *reg = canvas->icRegistry();
    QString blobName = reg->uniqueBlobName(initialName.trimmed());

    // If the auto-generated name differs from the initial, let the user confirm or override.
    if (blobName != initialName.trimmed()) {
        const auto typed = Dialogs::provider()->textPrompt(
            tr("Name Collision"),
            tr("An embedded IC named \"%1\" already exists.\nSuggested name:").arg(initialName.trimmed()),
            blobName);
        if (!typed) {
            return {};
        }
        blobName = typed->trimmed();
        if (blobName.isEmpty()) {
            return {};
        }
        // Re-check in case the user typed a name that also collides.
        if (reg->hasBlob(blobName)) {
            blobName = reg->uniqueBlobName(blobName);
        }
    }
    return blobName;
}

bool QuickICController::ensureProjectSaved(CanvasItem *canvas)
{
    if (canvas && !canvas->contextDir().isEmpty()) {
        return true; // already backed by a real directory
    }

    // DialogButton has no dedicated "Save" value (see DialogProvider.h's own doc comment on
    // its deliberately small, as-needed button set) -- Yes stands in for it here, the same
    // kind of documented simplification the menu/toolbar shell's own "Switch to Tab" deferral
    // used.
    const auto choice = Dialogs::provider()->choice(tr("Save required"),
        tr("This action needs the project saved to a file first, so IC paths can be resolved.\n\nSave it now?"),
        {DialogButton::Yes, DialogButton::Cancel}, DialogButton::Yes);
    if (choice != DialogButton::Yes) {
        return false;
    }

    m_host.requestSave(); // blocking; may prompt for a path and returns once done or cancelled
    return canvas && !canvas->contextDir().isEmpty();
}

void QuickICController::embedICByFile(const QString &fileName)
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }

    auto *canvas = tab->canvas();
    if (!ensureProjectSaved(canvas)) {
        return;
    }
    const QString contextDir = canvas->contextDir();

    const QString absolutePath = QDir(contextDir).absoluteFilePath(fileName);
    QFile file(absolutePath);
    if (!file.open(QIODevice::ReadOnly)) {
        Dialogs::provider()->choice(tr("Error"), tr("Could not read IC file: %1").arg(file.errorString()),
                                     {DialogButton::Ok}, DialogButton::Ok);
        return;
    }
    const QByteArray fileBytes = file.readAll();
    file.close();

    const QString blobName = resolveUniqueBlobName(QFileInfo(absolutePath).baseName(), canvas);
    if (blobName.isEmpty()) {
        return;
    }

    auto *reg = canvas->icRegistry();
    if (reg->embedICsByFile(absolutePath, fileBytes, blobName) == 0) {
        // No existing instances -- register the blob only; don't add to scene.
        canvas->receiveCommand(new CanvasRegisterBlobCommand(blobName, fileBytes, canvas));
    }

    m_host.palette()->updateEmbeddedICList(reg);
    m_host.showStatusMessage(tr("IC embedded successfully."), 4000);
}

void QuickICController::extractICByBlobName(const QString &blobName)
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }

    auto *canvas = tab->canvas();
    if (!ensureProjectSaved(canvas)) {
        return;
    }
    const QString contextDir = canvas->contextDir();

    auto *reg = canvas->icRegistry();
    if (!reg->hasBlob(blobName)) {
        return;
    }

    const QString suggestion = QDir(contextDir).absoluteFilePath(blobName + ".panda");
    QString fileName = FileDialogs::provider()
                            ->getSaveFileName(nullptr, tr("Extract IC to file..."), suggestion, tr("Panda files") + " (*.panda)")
                            .fileName;

    if (fileName.isEmpty()) {
        return;
    }
    if (!fileName.endsWith(".panda")) {
        fileName.append(".panda");
    }

    reg->extractToFile(blobName, fileName);
    // Mirrors QuickAppController::bindCurrentTab()'s own currentFile()-instead-of-icListFile()
    // precedent (see QuickMainWindowHost.h's doc comment) -- inline IC tabs have no UI trigger
    // in the Quick chrome yet.
    m_host.palette()->updateICList(m_host.currentFile());
    m_host.palette()->updateEmbeddedICList(reg);
    m_host.showStatusMessage(tr("IC extracted to %1").arg(fileName), 4000);
}

void QuickICController::removeICFile(const QString &icFileName)
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }
    auto *canvas = tab->canvas();

    // See this class's doc comment: this confirmation lives here rather than in the QML drop
    // target, since Dialogs::provider() is a C++-only blocking interface.
    const auto choice = Dialogs::provider()->choice(
        tr("Remove IC"),
        tr("Remove this IC? Its file will be moved to the system trash and its instances deleted from the circuit."),
        {DialogButton::Yes, DialogButton::No}, DialogButton::No);
    if (choice != DialogButton::Yes) {
        return;
    }

    QList<ItemWithId *> toDelete;
    for (auto *element : canvas->elements()) {
        if (element->elementType() != ElementType::IC) {
            continue;
        }
        auto *ic = qobject_cast<IC *>(element);
        if (ic && !ic->isEmbedded() && QFileInfo(ic->file()).fileName().toLower() == icFileName.toLower()) {
            toDelete.append(element);
        }
    }

    // Move the source file to the system trash first: a mistaken removal stays recoverable
    // (unlike a hard delete), and bailing out here on failure means the scene instances are
    // never deleted while the file is still present.
    QFile file(m_host.currentDir().absolutePath() + "/" + icFileName);
    if (file.exists() && !file.moveToTrash()) {
        throw PANDACEPTION("Error moving file to trash: %1", file.errorString());
    }

    if (!toDelete.isEmpty()) {
        canvas->receiveCommand(new CanvasDeleteItemsCommand(toDelete, canvas));
    }

    m_host.palette()->updateICList(m_host.currentFile());
    // Auto-save after removal so the scene no longer references the removed file.
    m_host.requestSave();
}

void QuickICController::removeEmbeddedIC(const QString &blobName)
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }
    auto *canvas = tab->canvas();

    const auto choice = Dialogs::provider()->choice(tr("Remove IC"),
        tr("Remove all \"%1\" instances from the circuit?").arg(blobName),
        {DialogButton::Yes, DialogButton::No}, DialogButton::No);
    if (choice != DialogButton::Yes) {
        return;
    }

    QList<ItemWithId *> toDelete;
    for (auto *elm : canvas->elements()) {
        if (elm->isEmbedded() && elm->blobName() == blobName) {
            toDelete.append(elm);
        }
    }

    auto *reg = canvas->icRegistry();
    const bool hasBlob = reg->hasBlob(blobName);
    if (toDelete.isEmpty() && !hasBlob) {
        return;
    }

    // Pair the IC deletion with blob removal in a single macro so undo restores both --
    // eagerly removing the blob outside the command would leave restored ICs pointing at a
    // registry entry that no longer exists. Mirrors WorkSpace::removeEmbeddedIC().
    canvas->undoStack()->beginMacro(tr("Remove embedded IC \"%1\"").arg(blobName));
    if (!toDelete.isEmpty()) {
        canvas->receiveCommand(new CanvasDeleteItemsCommand(toDelete, canvas));
    }
    if (hasBlob) {
        canvas->receiveCommand(new CanvasRemoveBlobCommand(blobName, canvas));
    }
    canvas->undoStack()->endMacro();

    m_host.palette()->updateEmbeddedICList(reg);
}

IC *QuickICController::selectedIC(CanvasItem *canvas) const
{
    if (!canvas) {
        return nullptr;
    }
    const auto selected = canvas->selectedElements();
    if (selected.isEmpty() || selected.first()->elementType() != ElementType::IC) {
        return nullptr;
    }
    return static_cast<IC *>(selected.first());
}

void QuickICController::addICFromFile()
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }
    auto *canvas = tab->canvas();

    // The IC list is directory-relative. If the project hasn't been saved yet there's no
    // directory to copy into, so offer to save now instead of dead-ending on an error.
    if (!ensureProjectSaved(canvas)) {
        return;
    }

    const QString selectedFile = FileDialogs::provider()->getOpenFileName(nullptr, tr("Open File"), QString(), tr("Panda") + " (*.panda)");
    if (selectedFile.isEmpty()) {
        return;
    }

    Dialogs::provider()->choice(tr("Info"),
        tr("Selected files (and their dependencies) will be copied to the current project folder."),
        {DialogButton::Ok}, DialogButton::Ok);

    // Copy the chosen .panda file (and any ICs it depends on transitively) into the project's
    // directory so that relative paths work when reopened.
    const QFileInfo srcInfo(selectedFile);
    QFileInfo destPath(m_host.currentDir().absolutePath() + "/" + srcInfo.fileName());

    // A *different* file with the same name already in the project folder would make
    // copyPandaFile() skip the copy, silently binding the IC to that pre-existing file's
    // content. Warn and let the user replace it or keep the existing one. DialogButton has no
    // dedicated Replace/Keep-Existing pair (see DialogProvider.h's own doc comment on its
    // deliberately small, as-needed button set) -- Yes/No stand in for them here, same
    // simplification ensureProjectSaved() already uses for Save, spelled out in the dialog text
    // itself so the generic labels stay unambiguous.
    if (destPath.exists() && !FileUtils::filesHaveSameContent(srcInfo, destPath)) {
        const auto choice = Dialogs::provider()->choice(tr("File name conflict"),
            tr("A different file named \"%1\" already exists in the project folder.\n\nYes: replace it. No: keep the existing file. Cancel: abort.").arg(srcInfo.fileName()),
            {DialogButton::Yes, DialogButton::No, DialogButton::Cancel}, DialogButton::Cancel);

        if (choice == DialogButton::Yes) {
            QFile::remove(destPath.absoluteFilePath()); // let copyPandaFile write the new file
        } else if (choice == DialogButton::No) {
            m_host.palette()->updateICList(m_host.currentFile()); // bind to the existing file as-is
            return;
        } else {
            return; // Cancel
        }
    }

    Serialization::copyPandaFile(srcInfo, destPath);
    m_host.palette()->updateICList(m_host.currentFile());
}

void QuickICController::embedSelectedIC()
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }
    auto *canvas = tab->canvas();

    auto *firstIC = selectedIC(canvas);
    if (!firstIC || firstIC->file().isEmpty()) {
        return;
    }

    if (!ensureProjectSaved(canvas)) {
        return;
    }
    const QString contextDir = canvas->contextDir();

    const QString blobName = resolveUniqueBlobName(QFileInfo(firstIC->file()).baseName(), canvas);
    if (blobName.isEmpty()) {
        return;
    }

    QFile file(QDir(contextDir).absoluteFilePath(firstIC->file()));
    if (!file.open(QIODevice::ReadOnly)) {
        Dialogs::provider()->choice(tr("Error"), tr("Could not read IC file: %1").arg(file.errorString()),
                                     {DialogButton::Ok}, DialogButton::Ok);
        return;
    }
    const QByteArray fileBytes = file.readAll();
    file.close();

    canvas->icRegistry()->embedICsByFile(firstIC->file(), fileBytes, blobName);
    m_host.palette()->updateEmbeddedICList(canvas->icRegistry());
    m_host.showStatusMessage(tr("IC embedded successfully."), 4000);
}

void QuickICController::extractSelectedIC()
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }
    auto *canvas = tab->canvas();

    auto *firstIC = selectedIC(canvas);
    if (!firstIC || !firstIC->isEmbedded()) {
        return;
    }

    const QString blobName = firstIC->blobName();
    if (!ensureProjectSaved(canvas)) {
        return;
    }
    const QString contextDir = canvas->contextDir();

    const QString suggestion = QDir(contextDir).absoluteFilePath(blobName + ".panda");
    QString fileName = FileDialogs::provider()
                            ->getSaveFileName(nullptr, tr("Extract IC to file..."), suggestion, tr("Panda files") + " (*.panda)")
                            .fileName;
    if (fileName.isEmpty()) {
        return;
    }
    if (!fileName.endsWith(".panda")) {
        fileName.append(".panda");
    }

    canvas->icRegistry()->extractToFile(blobName, fileName);
    m_host.palette()->updateICList(m_host.currentFile());
    m_host.palette()->updateEmbeddedICList(canvas->icRegistry());
    m_host.showStatusMessage(tr("IC extracted to %1").arg(fileName), 4000);
}

void QuickICController::makeSelfContained()
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }
    auto *canvas = tab->canvas();

    if (!ensureProjectSaved(canvas)) {
        return;
    }
    const QString contextDir = canvas->contextDir();

    // Collect unique file paths from all file-backed ICs.
    QStringList uniqueFiles;
    for (auto *elm : canvas->elements()) {
        if (elm->elementType() != ElementType::IC || elm->isEmbedded()) {
            continue;
        }
        const QString icFile = static_cast<IC *>(elm)->file();
        if (!icFile.isEmpty() && !uniqueFiles.contains(icFile)) {
            uniqueFiles.append(icFile);
        }
    }

    if (uniqueFiles.isEmpty()) {
        m_host.showStatusMessage(tr("No file-based ICs to embed."), 4000);
        return;
    }

    int totalEmbedded = 0;
    bool completed = true;
    auto *reg = canvas->icRegistry();

    for (const QString &icFile : std::as_const(uniqueFiles)) {
        const QString fullPath = QDir(contextDir).absoluteFilePath(icFile);
        QFile file(fullPath);
        if (!file.open(QIODevice::ReadOnly)) {
            Dialogs::provider()->choice(tr("Error"), tr("Could not read IC file: %1").arg(file.errorString()),
                                         {DialogButton::Ok}, DialogButton::Ok);
            completed = false;
            break;
        }
        const QByteArray fileBytes = file.readAll();
        file.close();

        const QString blobName = resolveUniqueBlobName(QFileInfo(icFile).baseName(), canvas);
        if (blobName.isEmpty()) {
            completed = false;
            break; // user cancelled
        }

        totalEmbedded += reg->embedICsByFile(fullPath, fileBytes, blobName);
    }

    m_host.palette()->updateEmbeddedICList(reg);
    // Only claim the circuit is self-contained when every file-based IC was embedded. On a
    // read error or a cancelled prompt the loop breaks early, so report the partial result
    // honestly (or stay quiet if nothing was embedded -- the error/cancel already spoke).
    if (completed) {
        m_host.showStatusMessage(tr("Embedded %1 IC(s). Circuit is now self-contained.").arg(totalEmbedded), 4000);
    } else if (totalEmbedded > 0) {
        m_host.showStatusMessage(tr("Embedded %1 IC(s); some file-based ICs remain.").arg(totalEmbedded), 4000);
    }
}

void QuickICController::addEmbeddedICFromFile()
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }
    auto *canvas = tab->canvas();

    const QString fileName = FileDialogs::provider()->getOpenFileName(nullptr, tr("Select IC file to embed"), m_host.currentDir().absolutePath(), tr("Panda files") + " (*.panda)");
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        Dialogs::provider()->choice(tr("Error"), tr("Could not read file: %1").arg(file.errorString()),
                                     {DialogButton::Ok}, DialogButton::Ok);
        return;
    }
    const QByteArray fileBytes = file.readAll();
    file.close();

    const QString blobName = resolveUniqueBlobName(QFileInfo(fileName).baseName(), canvas);
    if (blobName.isEmpty()) {
        return;
    }

    canvas->receiveCommand(new CanvasRegisterBlobCommand(blobName, fileBytes, canvas));
    m_host.palette()->updateEmbeddedICList(canvas->icRegistry());
}
