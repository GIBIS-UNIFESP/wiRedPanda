// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ICController: integrated-circuit embed / extract / import / removal operations.
 */

#pragma once

#include <QObject>
#include <QString>

class IC;
class MainWindowHost;
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

    /// Embeds the currently selected file-backed IC as a blob in the circuit.
    void embedSelectedIC();

    /// Extracts the currently selected embedded IC back out to a .panda file.
    void extractSelectedIC();

    /// Embeds the file-based IC named \a fileName (drag-and-drop target).
    void embedICByFile(const QString &fileName);

    /// Extracts the embedded IC blob \a blobName to a user-chosen .panda file.
    void extractICByBlobName(const QString &blobName);

    /// Embeds every file-based IC in the circuit so it becomes self-contained.
    void makeSelfContained();

    /// Prompts for a .panda file and registers it as an embedded IC blob.
    void addEmbeddedICFromFile();

    /// Removes the embedded IC blob \a blobName from the current circuit.
    void removeEmbeddedIC(const QString &blobName);

private:
    /// Returns the first selected element if it is an IC, else nullptr.
    IC *selectedIC() const;

    /// Resolves a non-colliding blob name for \a initialName, prompting on collision.
    QString resolveUniqueBlobName(const QString &initialName, Scene *scene);

    /// Ensures \a scene's project is saved to a file (IC paths are directory-relative). If it
    /// isn't, offers an inline "Save now?" prompt and performs the save instead of dead-ending
    /// on an error. Returns true once the project has a real directory, false if the user
    /// declined or cancelled the save.
    bool ensureProjectSaved(Scene *scene);

    MainWindowHost &m_host;
};
