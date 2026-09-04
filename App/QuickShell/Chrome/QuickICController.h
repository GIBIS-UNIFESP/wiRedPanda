// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickICController: CanvasItem-side port of App/UI/ICController.h.
 */

#pragma once

#include <QObject>
#include <QString>

class CanvasItem;
class IC;
class QuickMainWindowHost;

/**
 * \class QuickICController
 * \brief Owns the drag-and-drop-triggered half of the IC management workflow for
 * wiredpanda: embed/extract-by-drag and remove-by-drag-to-trash. CanvasItem-side port
 * of ICController.
 *
 * \details Copy-and-adapt port (same precedent as CanvasCommands/CanvasICRegistry/
 * QuickWorkSpace/QuickExportController), not a modification of the production ICController:
 * MainWindowHost::currentTab() returns a concrete WorkSpace* (a Widgets type), and
 * ICController's confirm/prompt dialogs are QMessageBox/QInputDialog, both of which this
 * class has no use for.
 *
 * Originally scoped to just the four ICController methods ElementPalette.qml's IC-tab drag
 * targets (the ICDropZone/TrashButton port) trigger: embedICByFile(), extractICByBlobName(),
 * removeICFile(), removeEmbeddedIC() -- plus the two private helpers they depend on,
 * resolveUniqueBlobName() and ensureProjectSaved(). The remaining ICController methods
 * (addICFromFile()/embedSelectedIC()/extractSelectedIC()/makeSelfContained()/
 * addEmbeddedICFromFile()) were deliberate deferrals -- each needed a toolbar-button UI trigger
 * that didn't exist in the Quick chrome yet -- closed post-hoc (2026-09-01, plan's feature-gap
 * decision 6) once ElementPalette.qml grew IC-tab toolbar buttons and QuickElementEditor grew
 * its Embed/Extract actions for a selected IC. ICController::showRemoveICHint() (a modal
 * "Drag here to remove" popup, triggered by clicking a dedicated trash button) is NOT ported:
 * ElementPalette.qml's trash drop target already shows that hint as an always-visible inline
 * label (no button/click needed), so a redundant popup would only duplicate it.
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

    /// Copies a chosen .panda file (and its dependencies) into the project's IC directory,
    /// after a name-conflict check. Toolbar-button target (ElementPalette.qml's "Add IC..."
    /// button, file-based section). Mirrors ICController::addICFromFile().
    void addICFromFile();

    /// Embeds the currently selected file-backed IC as a blob in the circuit. Toolbar-button
    /// target (QuickElementEditor's "Embed" action for a selected file-based IC). Mirrors
    /// ICController::embedSelectedIC().
    void embedSelectedIC();

    /// Extracts the currently selected embedded IC back out to a user-chosen .panda file.
    /// Toolbar-button target (QuickElementEditor's "Extract to file" action for a selected
    /// embedded IC). Mirrors ICController::extractSelectedIC().
    void extractSelectedIC();

    /// Embeds every file-based IC in the circuit so it becomes self-contained. Toolbar-button
    /// target (ElementPalette.qml's "Make Self-Contained" button). Mirrors
    /// ICController::makeSelfContained().
    void makeSelfContained();

    /// Prompts for a .panda file and registers it as an embedded IC blob (no scene instance
    /// added). Toolbar-button target (ElementPalette.qml's "Add IC..." button, embedded
    /// section). Mirrors ICController::addEmbeddedICFromFile().
    void addEmbeddedICFromFile();

private:
    /// Returns the first selected element if it is an IC, else nullptr. Mirrors
    /// ICController::selectedIC().
    IC *selectedIC(CanvasItem *canvas) const;

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
