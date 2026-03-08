// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief WorkSpace widget: the complete circuit editing environment for one tab.
 */

#pragma once

#include <QFileInfo>
#include <QTemporaryFile>
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
    /// Loads a circuit from \a stream using the given format \a number.
    void load(QDataStream &stream, QVersionNumber number);
    /// Saves the current circuit to \a fileName.
    void save(const QString &fileName);
    /// Saves the current circuit to \a stream.
    void save(QDataStream &stream);
    /// Creates or replaces the autosave temporary file.
    void setAutosaveFile();

    // --- Waveform Integration ---

    /// Returns the path of the associated BeWavedDolphin waveform file.
    QString dolphinFileName();
    /// Sets the associated BeWavedDolphin waveform file path to \a fileName.
    void setDolphinFileName(const QString &fileName);

    // --- ID Management ---

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

private:
    // --- Internal methods ---

    void autosave();
    void setAutosaveFileName();

    // --- Members ---

    GraphicsView m_view;
    Scene m_scene;
    QFileInfo m_fileInfo;
    QString m_dolphinFileName;
    QTemporaryFile m_autosaveFile;
    int m_lastId = 0;
};

