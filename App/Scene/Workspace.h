// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief WorkSpace widget: the complete circuit editing environment for one tab.
 */

#pragma once

#include <QFileInfo>
#include <QPointer>
#include <QTimer>
#include <QUndoStack>

#include "App/Core/Settings.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"

class ExerciseOverlay;
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

    friend class TestWorkspaceUnit;

public:
    /// Outcome of a save attempt: whether it wrote to disk, or the target turned out to
    /// be read-only/unwritable (the caller decides whether to re-prompt for a different
    /// path or throw, depending on whether it's running interactively).
    enum class SaveOutcome {
        Saved,
        ReadOnlyTarget,
    };

    // --- Lifecycle ---

    /// Constructs the workspace with optional \a parent widget.
    explicit WorkSpace(QWidget *parent = nullptr);

    /// Flushes any pending debounced autosave on destruction.
    ~WorkSpace() override;

    // --- Component Access ---

    /// Returns the GraphicsView embedded in this workspace.
    GraphicsView *view();
    /// Returns the Scene embedded in this workspace.
    Scene *scene();
    /// \overload
    const Scene *scene() const;

    /// Returns the embedded Simulation.
    Simulation *simulation();

    // --- File Operations ---

    /// Returns the file info for the currently open circuit file.
    QFileInfo fileInfo() const;
    /// Loads a circuit from the file at \a fileName.
    void load(const QString &fileName);
    /// Loads a circuit from \a stream using the given format \a version and optional \a contextDir.
    void load(QDataStream &stream, const QVersionNumber &version, const QString &contextDir);

    /**
     * \brief Saves the current circuit to \a fileName.
     * \details Pure: \a fileName must already be a resolved, non-empty, ".panda"-suffixed
     * path (inline-IC tabs are the one exception -- they ignore it and serialize to a blob
     * instead). Shows no dialogs and never recurses; a read-only/unwritable target is
     * reported via the return value rather than retried in place, regardless of
     * Application::interactiveMode -- callers decide how to react to that themselves.
     * Throws PANDACEPTION only on a genuine, non-read-only I/O error.
     */
    SaveOutcome save(const QString &fileName);
    /// Saves the current circuit to \a stream.
    void save(QDataStream &stream);
    /// Creates or replaces the autosave temporary file.
    void setAutosaveFile();

    /// Forces any pending debounced autosave to run synchronously.
    void flushPendingAutosave();

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

    /// Updates the tracked blob name after the underlying blob is renamed elsewhere in the
    /// registry, so a later lookup/second rename still matches this tab. Does not itself touch
    /// the tab title -- the caller (WorkspaceManager) also owns retitling the QTabWidget entry.
    void setInlineBlobName(const QString &blobName) { m_inlineBlobName = blobName; }

    /// Stable placeholder title for an unsaved tab, e.g. "New Project" or "New Project 2".
    /// Assigned once at creation so the tab keeps the same number across edits; unused once
    /// the workspace is saved to a real file.
    const QString &untitledTitle() const { return m_untitledTitle; }
    void setUntitledTitle(const QString &title) { m_untitledTitle = title; }

    /// True if this tab was restored from an autosave on launch. Surfaced as a "(recovered)"
    /// title marker until the user saves it to a real file, so recovered work is obvious.
    bool isRecovered() const { return m_isRecovered; }
    void setRecovered(bool recovered) { m_isRecovered = recovered; }

    /// Removes all IC instances with the given blob name.
    void removeEmbeddedIC(const QString &blobName);

    // Minimap control
    void setMinimapVisible(bool visible);

    // --- Waveform Integration ---

    /// Returns the path of the associated BeWavedDolphin waveform file.
    QString dolphinFileName() const;
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

    /// Registers an exercise overlay so WorkSpace can reposition it on resize.
    /// Pass nullptr to detach. Does not take ownership.
    void setExerciseOverlay(ExerciseOverlay *overlay);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

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

    /// Restores/reclamps m_minimap's geometry against the current view bounds. On the
    /// first call, applies the persisted Settings::minimapGeometry() (size-then-position
    /// clamped) or a hardcoded default corner if none is set; later calls just re-clamp
    /// the minimap's current geometry into the (possibly shrunk) new bounds.
    void applyMinimapGeometry();

    /// Persists \a geometry as the user's minimap placement/size.
    void onMinimapGeometryChangeFinished(const QRect &geometry);

    // --- Members ---

    GraphicsView m_view;
    Scene m_scene;
    QFileInfo m_fileInfo;
    QString m_dolphinFileName;
    QString m_autosaveFileName;
    QTimer m_autosaveDebounceTimer;
    QVersionNumber m_loadedVersion;
    int m_lastId = 0;

    // Minimap overview widget (small, shows full scene and viewport)
    class MinimapWidget *m_minimap = nullptr;
    /// True once applyMinimapGeometry() has applied its first (restored-or-default)
    /// geometry; gates whether later resizeEvent()s re-read Settings (first time) or
    /// just re-clamp the minimap's own current geometry (every time after).
    bool m_minimapPositioned = false;

    // Inline IC tab state
    bool m_isInlineIC = false;
    QPointer<WorkSpace> m_parentWorkspace;
    int m_parentICElementId = -1;
    QString m_inlineBlobName;
    QString m_untitledTitle;
    bool m_isRecovered = false;

    // Exercise overlay — non-owning pointer
    ExerciseOverlay *m_exerciseOverlay = nullptr;
};
