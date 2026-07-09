// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ICController.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Core/SentryHelpers.h"
#include "App/Element/IC.h"
#include "App/IO/FileUtils.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Commands.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/UI/ElementPalette.h"
#include "App/UI/FileDialogProvider.h"
#include "App/UI/MainWindowHost.h"

ICController::ICController(MainWindowHost &host, QObject *parent)
    : QObject(parent)
    , m_host(host)
{
}

IC *ICController::selectedIC() const
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return nullptr;
    }

    const auto selected = tab->scene()->selectedElements();
    if (selected.isEmpty()) {
        return nullptr;
    }

    if (selected.first()->elementType() != ElementType::IC) {
        return nullptr;
    }

    return static_cast<IC *>(selected.first());
}

void ICController::addICFromFile()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("ic", QStringLiteral("Add IC"));
        auto *tab = m_host.currentTab();
        if (!tab) {
            return;
        }

        // The IC list is directory-relative.  If the project hasn't been saved yet
        // we don't have a directory to copy into, so require a save first.
        if (!tab->fileInfo().isReadable()) {
            throw PANDACEPTION("Save file first.");
        }

        const QString selectedFile = FileDialogs::provider()->getOpenFileName(m_host.widget(), tr("Open File"), QString(), tr("Panda") + " (*.panda)");

        if (selectedFile.isEmpty()) {
            return;
        }

        const QStringList files = {selectedFile};

        QMessageBox::information(m_host.widget(), tr("Info"), tr("Selected files (and their dependencies) will be copied to the current project folder."));

        // Copy the chosen .panda file (and any ICs it depends on transitively)
        // into the project's directory so that relative paths work when reopened.
        for (const auto &file : files) {
            const QFileInfo srcInfo(file);
            QFileInfo destPath(m_host.currentDir().absolutePath() + "/" + srcInfo.fileName());

            // A *different* file with the same name already in the project folder would make
            // copyPandaFile() skip the copy, silently binding the IC to that pre-existing
            // file's content. Warn and let the user replace it or keep the existing one.
            if (destPath.exists() && !FileUtils::filesHaveSameContent(srcInfo, destPath)) {
                QMessageBox box(QMessageBox::Warning, tr("File name conflict"),
                    tr("A different file named \"%1\" already exists in the project folder.").arg(srcInfo.fileName()),
                    QMessageBox::NoButton, m_host.widget());
                auto *replaceButton = box.addButton(tr("Replace"), QMessageBox::AcceptRole);
                auto *keepButton = box.addButton(tr("Keep Existing"), QMessageBox::RejectRole);
                box.addButton(QMessageBox::Cancel);
                box.exec();

                if (box.clickedButton() == replaceButton) {
                    QFile::remove(destPath.absoluteFilePath()); // let copyPandaFile write the new file
                } else if (box.clickedButton() == keepButton) {
                    continue; // bind to the existing file; don't overwrite or pull in the source's deps
                } else {
                    return; // Cancel
                }
            }

            Serialization::copyPandaFile(srcInfo, destPath);
        }

        m_host.palette()->updateICList(m_host.icListFile());
    });
}

void ICController::showRemoveICHint()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("ic", QStringLiteral("Remove IC"));
        QMessageBox::information(m_host.widget(), tr("Info"), tr("Drag here to remove."));
    });
}

void ICController::removeICFile(const QString &icFileName)
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }

    QList<QGraphicsItem *> toDelete;
    for (auto *element : tab->scene()->elements()) {
        if (element->elementType() != ElementType::IC) {
            continue;
        }
        const auto *ic = qobject_cast<IC *>(element);
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
        tab->scene()->receiveCommand(new DeleteItemsCommand(toDelete, tab->scene()));
        tab->simulation()->restart();
    }

    m_host.palette()->updateICList(m_host.icListFile());
    // Auto-save after removal so the scene no longer references the removed file.
    m_host.requestSave();
}

QString ICController::resolveUniqueBlobName(const QString &initialName, Scene *scene)
{
    auto *reg = scene->icRegistry();
    QString blobName = reg->uniqueBlobName(initialName.trimmed());

    // If the auto-generated name differs from the initial, let the user confirm or override
    if (blobName != initialName.trimmed()) {
        bool ok = false;
        blobName = QInputDialog::getText(m_host.widget(),
            tr("Name Collision"),
            tr("An embedded IC named \"%1\" already exists.\nSuggested name:").arg(initialName.trimmed()),
            QLineEdit::Normal, blobName, &ok);
        blobName = blobName.trimmed();
        if (!ok || blobName.isEmpty()) {
            return {};
        }
        // Re-check in case the user typed a name that also collides
        if (reg->hasBlob(blobName)) {
            blobName = reg->uniqueBlobName(blobName);
        }
    }
    return blobName;
}

void ICController::embedSelectedIC()
{
    sentryBreadcrumb("ic", QStringLiteral("Embed IC"));
    auto *firstIC = selectedIC();
    if (!firstIC || firstIC->file().isEmpty()) {
        return;
    }

    auto *scene = m_host.currentTab()->scene();

    const QString contextDir = scene->contextDir();
    if (contextDir.isEmpty()) {
        QMessageBox::warning(m_host.widget(), tr("Error"), tr("Please save the project first so ICs can be resolved."));
        return;
    }

    QString blobName = resolveUniqueBlobName(QFileInfo(firstIC->file()).baseName(), scene);
    if (blobName.isEmpty()) {
        return;
    }

    QFile file(QDir(contextDir).absoluteFilePath(firstIC->file()));
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(m_host.widget(), tr("Error"), tr("Could not read IC file: %1").arg(file.errorString()));
        return;
    }
    QByteArray fileBytes = file.readAll();
    file.close();

    scene->icRegistry()->embedICsByFile(firstIC->file(), fileBytes, blobName);
    m_host.palette()->updateEmbeddedICList(scene);
    m_host.showStatusMessage(tr("IC embedded successfully."), 4000);
}

void ICController::extractSelectedIC()
{
    sentryBreadcrumb("ic", QStringLiteral("Extract IC"));
    auto *firstIC = selectedIC();
    if (!firstIC || !firstIC->isEmbedded()) {
        return;
    }

    auto *scene = m_host.currentTab()->scene();
    const QString blobName = firstIC->blobName();
    const QString contextDir = scene->contextDir();
    if (contextDir.isEmpty()) {
        QMessageBox::warning(m_host.widget(), tr("Error"), tr("Please save the project first."));
        return;
    }

    const QString suggestion = QDir(contextDir).absoluteFilePath(blobName + ".panda");
    QString fileName = FileDialogs::provider()->getSaveFileName(m_host.widget(), tr("Extract IC to file..."), suggestion, tr("Panda files") + " (*.panda)").fileName;

    if (fileName.isEmpty()) {
        return;
    }

    if (!fileName.endsWith(".panda")) {
        fileName.append(".panda");
    }

    scene->icRegistry()->extractToFile(blobName, fileName);
    m_host.palette()->updateICList(m_host.icListFile());
    m_host.palette()->updateEmbeddedICList(scene);
    m_host.showStatusMessage(tr("IC extracted to %1").arg(fileName), 4000);
}

void ICController::embedICByFile(const QString &fileName)
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }

    auto *scene = tab->scene();
    const QString contextDir = scene->contextDir();
    if (contextDir.isEmpty()) {
        QMessageBox::warning(m_host.widget(), tr("Error"), tr("Please save the project first so ICs can be resolved."));
        return;
    }

    const QString absolutePath = QDir(contextDir).absoluteFilePath(fileName);
    QFile file(absolutePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(m_host.widget(), tr("Error"), tr("Could not read IC file: %1").arg(file.errorString()));
        return;
    }
    QByteArray fileBytes = file.readAll();
    file.close();

    QString blobName = resolveUniqueBlobName(QFileInfo(absolutePath).baseName(), scene);
    if (blobName.isEmpty()) {
        return;
    }

    auto *reg = scene->icRegistry();
    if (reg->embedICsByFile(absolutePath, fileBytes, blobName) == 0) {
        // No existing instances — register the blob only; don't add to scene.
        scene->receiveCommand(new RegisterBlobCommand(blobName, fileBytes, scene));
    }

    m_host.palette()->updateEmbeddedICList(scene);
    m_host.showStatusMessage(tr("IC embedded successfully."), 4000);
}

void ICController::extractICByBlobName(const QString &blobName)
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }

    auto *scene = tab->scene();
    const QString contextDir = scene->contextDir();
    if (contextDir.isEmpty()) {
        QMessageBox::warning(m_host.widget(), tr("Error"), tr("Please save the project first."));
        return;
    }

    // Find any embedded IC with this blobName to get the blob data
    auto *reg = scene->icRegistry();
    if (!reg->hasBlob(blobName)) {
        return;
    }

    const QString suggestion = QDir(contextDir).absoluteFilePath(blobName + ".panda");
    QString fileName = FileDialogs::provider()->getSaveFileName(m_host.widget(), tr("Extract IC to file..."), suggestion, tr("Panda files") + " (*.panda)").fileName;

    if (fileName.isEmpty()) {
        return;
    }

    if (!fileName.endsWith(".panda")) {
        fileName.append(".panda");
    }

    reg->extractToFile(blobName, fileName);
    m_host.palette()->updateICList(m_host.icListFile());
    m_host.palette()->updateEmbeddedICList(scene);
    m_host.showStatusMessage(tr("IC extracted to %1").arg(fileName), 4000);
}

void ICController::makeSelfContained()
{
    sentryBreadcrumb("ic", QStringLiteral("Make self-contained"));
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }

    auto *scene = tab->scene();
    const QString contextDir = scene->contextDir();
    if (contextDir.isEmpty()) {
        QMessageBox::warning(m_host.widget(), tr("Error"), tr("Please save the project first."));
        return;
    }

    // Collect unique file paths from all file-backed ICs
    QStringList uniqueFiles;
    for (auto *elm : scene->elements()) {
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
    auto *reg = scene->icRegistry();

    for (const QString &icFile : std::as_const(uniqueFiles)) {
        const QString fullPath = QDir(contextDir).absoluteFilePath(icFile);
        QFile file(fullPath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(m_host.widget(), tr("Error"), tr("Could not read IC file: %1").arg(file.errorString()));
            completed = false;
            break;
        }
        QByteArray fileBytes = file.readAll();
        file.close();

        QString blobName = resolveUniqueBlobName(QFileInfo(icFile).baseName(), scene);
        if (blobName.isEmpty()) {
            completed = false;
            break; // User cancelled
        }

        totalEmbedded += reg->embedICsByFile(fullPath, fileBytes, blobName);
    }

    m_host.palette()->updateEmbeddedICList(scene);
    // Only claim the circuit is self-contained when every file-based IC was embedded. On a
    // read error or a cancelled prompt the loop breaks early, so report the partial result
    // honestly (or stay quiet if nothing was embedded — the error/cancel already spoke).
    if (completed) {
        m_host.showStatusMessage(tr("Embedded %1 IC(s). Circuit is now self-contained.").arg(totalEmbedded), 4000);
    } else if (totalEmbedded > 0) {
        m_host.showStatusMessage(tr("Embedded %1 IC(s); some file-based ICs remain.").arg(totalEmbedded), 4000);
    }
}

void ICController::addEmbeddedICFromFile()
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }
    QString fileName = FileDialogs::provider()->getOpenFileName(m_host.widget(), tr("Select IC file to embed"), m_host.currentDir().absolutePath(), tr("Panda files") + " (*.panda)");
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(m_host.widget(), tr("Error"), tr("Could not read file: %1").arg(file.errorString()));
        return;
    }
    QByteArray fileBytes = file.readAll();
    file.close();

    auto *scene = tab->scene();
    QString blobName = resolveUniqueBlobName(QFileInfo(fileName).baseName(), scene);
    if (blobName.isEmpty()) {
        return;
    }

    scene->receiveCommand(new RegisterBlobCommand(blobName, fileBytes, scene));
    m_host.palette()->updateEmbeddedICList(scene);
}

void ICController::removeEmbeddedIC(const QString &blobName)
{
    auto *tab = m_host.currentTab();
    if (tab) {
        tab->removeEmbeddedIC(blobName);
        m_host.palette()->updateEmbeddedICList(tab->scene());
    }
}
