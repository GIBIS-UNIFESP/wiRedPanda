// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

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

    // --- Embedded IC blob storage ---

    bool hasBlob(const QString &name) const;
    QByteArray blob(const QString &name) const;
    void setBlob(const QString &name, const QByteArray &data);
    void registerBlob(const QString &name, const QByteArray &data);
    void removeBlob(const QString &name);
    void renameBlob(const QString &oldName, const QString &newName);
    const QMap<QString, QByteArray> &blobMap() const { return m_blobs; }
    QMap<QString, QByteArray> &blobMapRef() { return m_blobs; }
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

    int embedICsByFile(const QString &fileName, const QByteArray &fileBytes,
                       const QString &blobName);

    /// Writes the blob to disk and converts all embedded ICs with \a blobName to file-backed.
    int extractToFile(const QString &blobName, const QString &filePath);

    // --- Serialization helpers ---

    static QByteArray captureSnapshot(const QList<GraphicElement *> &targets);

    // --- Cycle detection ---

    /// Returns true if \a canonicalPath is currently being loaded (circular reference).
    bool isLoading(const QString &canonicalPath) const;

    /// Marks a file as currently loading. Returns false if already loading (cycle detected).
    bool beginLoading(const QString &canonicalPath);

    /// Removes a file from the loading set.
    void endLoading(const QString &canonicalPath);

signals:
    void definitionChanged(const QString &filePath);

private slots:
    void onFileChanged(const QString &filePath);

private:
    void makeBlobSelfContained(const QString &name, QSet<QString> &visited);

    Scene *m_scene;
    QMap<QString, ICDefinition> m_definitions;
    QMap<QString, QByteArray> m_blobs;  // embedded IC blob storage
    QFileSystemWatcher m_fileWatcher;
    QSet<QString> m_loadingFiles;
};

