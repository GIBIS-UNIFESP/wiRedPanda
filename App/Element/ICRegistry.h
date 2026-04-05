// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief IC definition registry with file watching and embedded blob storage.
 */

#pragma once

#include <QFileSystemWatcher>
#include <QMap>
#include <QObject>
#include <QSet>

#include "App/Element/ICDefinition.h"

class GraphicElement;
class IC;
class Scene;

/**
 * \class ICRegistry
 * \brief Manages IC definitions, file watching, and embedded blob storage.
 *
 * \details Owned by Scene. Caches ICDefinition objects keyed by canonical file
 * path. Watches IC files for changes and reloads all IC instances referencing
 * a changed file.
 */
class ICRegistry : public QObject
{
    Q_OBJECT

public:
    explicit ICRegistry(Scene *scene);

    /// Returns true if a definition for \a filePath is cached.
    bool has(const QString &filePath) const;

    /// Returns the cached definition, or loads it from disk. Returns nullptr on failure.
    const ICDefinition *definition(const QString &filePath);

    /// Invalidates a cached definition (e.g., after file change).
    void invalidate(const QString &filePath);

    /// Registers a file for watching. Called when an IC element is added to the scene.
    void watchFile(const QString &filePath);

    /// Finds all IC elements in the scene that reference \a fileName.
    QList<GraphicElement *> findICsByFile(const QString &fileName) const;

    // --- Embedded IC blob storage ---

    /// Returns \c true if a blob named \a name is stored in the registry.
    bool hasBlob(const QString &name) const;
    /// Returns the raw .panda bytes for the embedded IC named \a name.
    QByteArray blob(const QString &name) const;
    /// Stores or replaces the blob \a data under \a name and invalidates the cached definition.
    void setBlob(const QString &name, const QByteArray &data);
    /// Stores blob \a data under \a name without invalidating the definition cache.
    void registerBlob(const QString &name, const QByteArray &data);
    /// Removes the blob named \a name from the registry.
    void removeBlob(const QString &name);
    /// Renames a blob from \a oldName to \a newName, updating the cache key.
    void renameBlob(const QString &oldName, const QString &newName);
    /// Returns a const reference to the full blob map (name → .panda bytes).
    const QMap<QString, QByteArray> &blobMap() const { return m_blobs; }
    /// Returns a mutable reference to the blob map (used by serialization).
    QMap<QString, QByteArray> &blobMapRef() { return m_blobs; }
    /// Removes all stored blobs.
    void clearBlobs();

    /// Finds all embedded IC elements with \a blobName.
    QList<GraphicElement *> findICsByBlobName(const QString &blobName) const;

    /// Initializes an embedded IC by looking up its blob in the registry.
    bool initEmbeddedIC(IC *ic, const QString &blobName);

    /// Returns \a baseName if available, or appends a numeric suffix to avoid collision.
    QString uniqueBlobName(const QString &baseName) const;

    // --- Batch operations (undo-aware) ---

    /// Creates a new embedded IC from file bytes, registers the blob, and pushes an undo command.
    IC *createEmbeddedIC(const QString &blobName, const QByteArray &fileBytes, const QString &contextDir);

    /// Converts all file-backed IC elements referencing \a fileName to embedded ICs using \a blobName.
    int embedICsByFile(const QString &fileName, const QByteArray &fileBytes,
                       const QString &blobName);

    /// Writes the blob to disk and converts all embedded ICs with \a blobName to file-backed.
    int extractToFile(const QString &blobName, const QString &filePath);

    // --- Serialization helpers ---

    /// Serializes \a targets into a self-contained .panda byte array (used for embedding).
    static QByteArray captureSnapshot(const QList<GraphicElement *> &targets);

signals:
    /// Emitted when an IC definition file changes on disk and its cached definition is invalidated.
    void definitionChanged(const QString &filePath);

private slots:
    /// Handles QFileSystemWatcher notifications; invalidates the definition and reloads IC instances.
    void onFileChanged(const QString &filePath);

private:
    /// Restores \a elements from a previously captured \a snapshot (used for atomic rollback).
    void rollbackElements(const QList<GraphicElement *> &elements, const QByteArray &snapshot);

    /// Recursively inlines all IC dependencies of blob \a name so it has no external file references.
    /// Uses \a blobs as working storage instead of m_blobs to avoid corrupting state on failure.
    void makeBlobSelfContained(const QString &name, QSet<QString> &visited,
                               QMap<QString, QByteArray> &blobs);

    Scene *m_scene;                          ///< Owning scene.
    QMap<QString, ICDefinition> m_definitions; ///< Cached IC definitions keyed by canonical file path.
    QMap<QString, QByteArray> m_blobs;       ///< Embedded IC blob storage (name → .panda bytes).
    QFileSystemWatcher m_fileWatcher;        ///< Watches IC source files for external modifications.
};

