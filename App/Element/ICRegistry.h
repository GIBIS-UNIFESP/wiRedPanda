// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief IC definition registry with file watching, embedded blob storage, and cycle detection.
 */

#pragma once

#include <QFileSystemWatcher>
#include <QMap>
#include <QObject>
#include <QSet>

#include "App/Element/ICDefinition.h"

class GraphicElement;
class Scene;

/**
 * \class ICRegistry
 * \brief Manages IC definitions, file watching, and cycle detection.
 *
 * \details Owned by Scene. Caches ICDefinition objects keyed by canonical file
 * path. Watches IC files for changes and reloads all IC instances referencing
 * a changed file. Provides cycle detection for circular IC references.
 */
class ICRegistry : public QObject
{
    Q_OBJECT

public:
    explicit ICRegistry(Scene *scene);

    /// Returns true if a definition for \a filePath is cached.
    bool has(const QString &filePath) const;

    /// Returns the cached definition, or loads it from disk. Returns nullptr on failure.
    const ICDefinition *definition(const QString &filePath, const QString &contextDir);

    /// Invalidates a cached definition (e.g., after file change).
    void invalidate(const QString &filePath);

    /// Registers a file for watching. Called when an IC element is added to the scene.
    void watchFile(const QString &filePath);

    /// Finds all IC elements in the scene that reference \a fileName.
    QList<GraphicElement *> findICsByFile(const QString &fileName) const;

    // --- Cycle detection ---

    /// Returns true if \a canonicalPath is currently being loaded (circular reference).
    bool isLoading(const QString &canonicalPath) const;

    /// Marks a file as currently loading. Returns false if already loading (cycle detected).
    bool beginLoading(const QString &canonicalPath);

    /// Removes a file from the loading set.
    void endLoading(const QString &canonicalPath);

signals:
    /// Emitted when an IC definition file changes on disk and its cached definition is invalidated.
    void definitionChanged(const QString &filePath);

private slots:
    /// Handles QFileSystemWatcher notifications; invalidates the definition and reloads IC instances.
    void onFileChanged(const QString &filePath);

private:
    Scene *m_scene;                          ///< Owning scene.
    QMap<QString, ICDefinition> m_definitions; ///< Cached IC definitions keyed by canonical file path.
    QFileSystemWatcher m_fileWatcher;        ///< Watches IC source files for external modifications.
    QSet<QString> m_loadingFiles;            ///< Files currently being loaded (for cycle detection).
};

