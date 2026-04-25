// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief WorkSpace widget: the complete circuit editing environment for one tab.
 */

#pragma once

#include <QFileInfo>
#include <QPointer>
#include <QUndoStack>

#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"

class GraphicsView;
class Simulation;

/**
 * \class WorkSpace
 * \brief A widget containing a complete circuit editing environment.
 *
 * WorkSpace serves as a container for all components needed for circuit editing,
 * including the GraphicsView for visualization and interaction, Scene for element
 * management, and Simulation for circuit simulation. It also handles file operations
 * like loading, saving, and autosaving.
 */
class WorkSpace : public QWidget
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the workspace with optional \a parent widget.
    explicit WorkSpace(QWidget *parent = nullptr);

    // --- Component Access ---

    /// Returns the GraphicsView embedded in this workspace.
    GraphicsView *view();
    /// Returns the Scene embedded in this workspace.
    Scene *scene();

    /// Returns the embedded Simulation.
    Simulation *simulation();

    // --- File Operations ---

    /// Returns the file info for the currently open circuit file.
    QFileInfo fileInfo();
    /// Loads a circuit from the file at \a fileName.
    void load(const QString &fileName);
    /// Loads a circuit from \a stream using the given format \a version and optional \a contextDir.
    void load(QDataStream &stream, const QVersionNumber &version, const QString &contextDir);
    /// Saves the current circuit to \a fileName.
    void save(const QString &fileName);
    /// Saves the current circuit to \a stream.
    void save(QDataStream &stream);
    /// Creates or replaces the autosave temporary file.
    void setAutosaveFile();

    /// Synchronously flushes any pending autosave (used in tests).
    void flushPendingAutosave() { autosave(); }

    // --- Inline IC Tab ---

    /// Loads a blob for editing in an inline tab.
    void loadFromBlob(const QByteArray &blob, WorkSpace *parent, int icElementId, const QString &parentContextDir);

    /// Returns true if this workspace is editing an embedded IC blob (not a file).
    bool isInlineIC() const { return m_isInlineIC; }

    /// Returns the parent workspace (for inline IC tabs).
    WorkSpace *parentWorkspace() const { return m_parentWorkspace; }

    /// Returns the element ID of the IC in the parent scene being edited.
    int parentICElementId() const { return m_parentICElementId; }

    /// Returns the blob name being edited (for tab title).
    const QString &inlineBlobName() const { return m_inlineBlobName; }

    /// Removes all IC instances with the given blob name.
    void removeEmbeddedIC(const QString &blobName);

    // --- Waveform Integration ---

    /// Returns the path of the associated BeWavedDolphin waveform file.
    QString dolphinFileName();
    /// Sets the associated BeWavedDolphin waveform file path to \a fileName.
    void setDolphinFileName(const QString &fileName);

    // --- ID Management ---

    /// Returns true if the loaded file was saved by a newer version of wiRedPanda.
    bool isFromNewerVersion() const;

    /// Returns the highest element ID assigned in this workspace.
    int lastId() const;

    /**
     * \brief Forces the element-ID counter to \a newLastId.
     * \param newLastId New minimum ID for the next allocated item.
     */
    void setLastId(int newLastId);

signals:
    /**
     * \brief Emitted whenever the file info of this workspace changes (load/save).
     * \param fileInfo New file info.
     */
    void fileChanged(const QFileInfo &fileInfo);

    /// Emitted when an inline IC tab saves its blob (propagated to parent).
    void icBlobSaved(int icElementId, const QByteArray &blob);

public slots:
    /// Receives a saved blob from a child inline tab.
    void onChildICBlobSaved(int icElementId, const QByteArray &blob);

private:
    // --- Internal methods ---

    void autosave();
    void createVersionedBackup(const QString &fileName, const QVersionNumber &version);
    void setAutosaveFileName();

    /// Atomically sets m_fileInfo and the scene's contextDir from \a filePath.
    void setCurrentFile(const QString &filePath);

    // --- Members ---

    GraphicsView m_view;
    Scene m_scene;
    QFileInfo m_fileInfo;
    QString m_dolphinFileName;
    QString m_autosaveFileName;
    QVersionNumber m_loadedVersion;
    int m_lastId = 0;

    // Inline IC tab state
    bool m_isInlineIC = false;
    QPointer<WorkSpace> m_parentWorkspace;
    int m_parentICElementId = -1;
    QString m_inlineBlobName;
};

