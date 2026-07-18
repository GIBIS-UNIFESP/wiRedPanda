// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickICController: CanvasItem-side port of App/UI/ICController.h.
 */

#pragma once

#include <QObject>
#include <QString>

class CanvasItem;
class QuickMainWindowHost;

/**
 * \class QuickICController
 * \brief Owns the drag-and-drop-triggered half of the IC management workflow for
 * wiredpanda_quick: embed/extract-by-drag and remove-by-drag-to-trash. CanvasItem-side port
 * of ICController.
 *
 * \details Copy-and-adapt port (same precedent as CanvasCommands/CanvasICRegistry/
 * QuickWorkSpace/QuickExportController), not a modification of the production ICController:
 * MainWindowHost::currentTab() returns a concrete WorkSpace* (a Widgets type), and
 * ICController's confirm/prompt dialogs are QMessageBox/QInputDialog, both of which this
 * class has no use for.
 *
 * Scoped to exactly the four ICController methods ElementPalette.qml's IC-tab drag targets
 * (the ICDropZone/TrashButton port) actually trigger: embedICByFile(), extractICByBlobName(),
 * removeICFile(), removeEmbeddedIC() -- plus the two private helpers they depend on,
 * resolveUniqueBlobName() and ensureProjectSaved(). addICFromFile()/embedSelectedIC()/
 * extractSelectedIC()/makeSelfContained()/addEmbeddedICFromFile()/showRemoveICHint() are real,
 * deliberate deferrals, not oversights -- each needs a toolbar-button UI trigger that doesn't
 * exist in the Quick chrome yet (mirrors sub-step 4's identical "Add/remove-embedded-IC
 * buttons... not built" deferral for the same underlying reason).
 *
 * One deliberate reorganization from production: TrashButton::dropEvent()'s "remove this IC?"
 * Yes/No confirmation lives in this class's removeICFile()/removeEmbeddedIC() instead of in
 * the QML drop target -- Dialogs::provider()'s synchronous, blocking confirm API is a C++-only
 * interface (that's the whole point of the abstraction, see DialogProvider.h), so QML can't
 * call it directly the way a QWidget could show a QMessageBox inline in dropEvent().
 */
class QuickICController : public QObject
{
    Q_OBJECT

public:
    explicit QuickICController(QuickMainWindowHost &host, QObject *parent = nullptr);

public slots:
    /// Embeds the file-based IC named \a fileName (drag-and-drop target: dropping a
    /// file-based IC label onto the embedded section). Mirrors ICController::embedICByFile().
    void embedICByFile(const QString &fileName);

    /// Extracts the embedded IC blob \a blobName to a user-chosen .panda file (drag-and-drop
    /// target: dropping an embedded IC label onto the file-based section). Mirrors
    /// ICController::extractICByBlobName().
    void extractICByBlobName(const QString &blobName);

    /// Removes \a icFileName from the project directory (moved to the system trash) and every
    /// instance of it from the scene, after a Yes/No confirmation. Drag-and-drop target:
    /// dropping a file-based IC label onto the trash target. Mirrors
    /// ICController::removeICFile(), plus TrashButton::dropEvent()'s confirmation (see this
    /// class's own doc comment on why that moved here).
    void removeICFile(const QString &icFileName);

    /// Removes every embedded IC instance named \a blobName from the scene and the blob
    /// itself, after a Yes/No confirmation. Drag-and-drop target: dropping an embedded IC
    /// label onto the trash target. Mirrors ICController::removeEmbeddedIC(), plus
    /// TrashButton::dropEvent()'s confirmation.
    void removeEmbeddedIC(const QString &blobName);

private:
    /// Resolves a non-colliding blob name for \a initialName, prompting on collision via
    /// Dialogs::provider()->textPrompt(). Mirrors ICController::resolveUniqueBlobName().
    QString resolveUniqueBlobName(const QString &initialName, CanvasItem *canvas);

    /// Ensures \a canvas's project is saved to a file (IC paths are directory-relative). If it
    /// isn't, offers an inline "Save now?" prompt and performs the save instead of
    /// dead-ending on an error. Returns true once the project has a real directory, false if
    /// the user declined or cancelled. Mirrors ICController::ensureProjectSaved().
    bool ensureProjectSaved(CanvasItem *canvas);

    QuickMainWindowHost &m_host;
};
