// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ICController: integrated-circuit embed / extract / import / removal operations.
 */

#pragma once

#include <functional>

#include <QList>
#include <QObject>
#include <QString>

class IC;
class MainWindowHost;
class QUndoCommand;
class Scene;

/**
 * \class ICController
 * \brief Owns the IC management workflow: importing file-based ICs into the project
 * directory, embedding/extracting blobs, and making a circuit self-contained.
 *
 * \details Extracted from MainWindow. Reaches circuit state (current tab, scene,
 * IC registry, palette, status bar) through a MainWindowHost, so it never depends on
 * a concrete MainWindow. Widget-state helpers for the IC tool buttons remain on
 * MainWindow because they belong to its tab lifecycle.
 */
class ICController : public QObject
{
    Q_OBJECT

public:
    explicit ICController(MainWindowHost &host, QObject *parent = nullptr);

public slots:
    /// Copies a chosen .panda file (and its dependencies) into the project's IC directory.
    void addICFromFile();

    /// Shows the "drag here to remove" hint for the file-based IC trash button.
    void showRemoveICHint();

    /// Deletes \a icFileName from the project directory and any instances of it in the scene.
    void removeICFile(const QString &icFileName);

    /// Embeds every distinct file among the selected file-backed ICs as a blob in the circuit, as one undo step.
    void embedSelectedIC();

    /// Extracts every distinct blob among the selected embedded ICs to a .panda file, one save dialog each, as one
    /// undo step.
    void extractSelectedIC();

    /// Embeds the file-based IC named \a fileName (drag-and-drop target).
    void embedICByFile(const QString &fileName);

    /// Extracts the embedded IC blob \a blobName to a user-chosen .panda file.
    void extractICByBlobName(const QString &blobName);

    /// Embeds every file-based IC in the circuit so it becomes self-contained, as one undo step.
    void makeSelfContained();

    /// Prompts for a .panda file and registers it as an embedded IC blob.
    void addEmbeddedICFromFile();

    /// Removes the embedded IC blob \a blobName from the current circuit.
    void removeEmbeddedIC(const QString &blobName);

private:
    /// Returns the selected ICs of the current tab, in id order.
    QList<IC *> selectedICs() const;

    /// Runs \a edits, which make their undo commands children of the command they are given, and pushes that command
    /// onto \a scene's undo stack as one step named \a text if they made any, also when an edit throws.
    static void runAsOneStep(Scene *scene, const QString &text, const std::function<void(QUndoCommand *)> &edits);

    /// Resolves a non-colliding blob name for \a initialName, prompting on collision.
    QString resolveUniqueBlobName(const QString &initialName, Scene *scene);

    /// Ensures \a scene's project is saved to a file (IC paths are directory-relative). If it
    /// isn't, offers an inline "Save now?" prompt and performs the save instead of dead-ending
    /// on an error. Returns true once the project has a real directory, false if the user
    /// declined or cancelled the save.
    bool ensureProjectSaved(Scene *scene);

    MainWindowHost &m_host;
};
