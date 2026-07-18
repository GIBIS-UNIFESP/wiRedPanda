// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickICController.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUndoStack>

#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
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
    // kind of documented simplification sub-step 2's own "Switch to Tab" deferral used.
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

    QList<QGraphicsItem *> toDelete;
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

    QList<QGraphicsItem *> toDelete;
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
