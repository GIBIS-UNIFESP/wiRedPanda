// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief CanvasItem-side port of App/Scene/ICRegistry.h's embedded-IC blob storage.
 */

#pragma once

#include <functional>

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

class CanvasItem;
class GraphicElement;
class IC;

/**
 * \class CanvasICRegistry
 * \brief CanvasItem-side port of ICRegistry, scoped to the blob-storage core.
 *
 * \details Ports hasBlob()/blob()/setBlob()/registerBlob()/removeBlob()/renameBlob()/
 * findICsByBlobName()/initEmbeddedIC()/uniqueBlobName()/createEmbeddedIC()/captureSnapshot()/
 * rollbackElements() -- the half
 * MCP's embed_ic/instantiate_ic handlers actually exercise (confirmed by reading
 * MCP/Server/Handlers/ICHandler.cpp directly, not assumed). Also ports findICsByFile()/
 * embedICsByFile()/extractToFile() (Phase 4 sub-step 7 -- ICDropZone/TrashButton's real
 * drag-and-drop targets need them; QuickICController is their caller, mirroring
 * ICController::embedICByFile()/extractICByBlobName()). Deliberately NOT ported, named
 * here and in the plan's "Phase 3 in depth" section rather than silently dropped:
 *
 * - File watching (QFileSystemWatcher/onFileChanged()): this canvas has no file-backed-IC
 *   live-reload concept yet.
 * - registerBlob()'s recursive makeBlobSelfContained() dependency-inlining: a blob embedding
 *   another file-backed IC is stored as-is here rather than recursively inlining the
 *   dependency chain into one self-contained blob. A real, narrower gap than production, not
 *   a silent behavior change -- the blob still resolves correctly as long as the referenced
 *   file exists at load time (exactly like an ordinary file-backed IC does); it just isn't
 *   portable to a machine without that file, the way a fully self-contained blob would be.
 * - renameBlob()'s recursive rewrite of nested blob references inside other blobs' metadata
 *   (ICRegistry::renameBlobReference()) -- same class of gap as the makeBlobSelfContained()
 *   one above, unrelated to this class's real QObject-ness (added for blobRenamed() below).
 *
 * A real QObject (unlike most other Canvas* Phase 3 ports, which are plain classes) so it can
 * emit blobRenamed() -- QuickWorkspaceManager listens for this to keep an open inline-IC tab's
 * displayed title in sync with its blob's current name, mirroring ICRegistry's identical
 * signal and WorkspaceManager::onBlobRenamed(). Composed as a plain (non-heap, non-pointer)
 * member of CanvasItem, same as before -- a QObject-derived value member with its owner as
 * QObject parent is a normal, supported Qt pattern; nothing about adding Q_OBJECT requires
 * heap allocation.
 *
 * Owned by CanvasItem the same way ICRegistry is owned by Scene.
 */
class CanvasICRegistry : public QObject
{
    Q_OBJECT

public:
    explicit CanvasICRegistry(CanvasItem *canvas);

    /// Returns \c true if a blob named \a name is stored in the registry.
    [[nodiscard]] bool hasBlob(const QString &name) const;
    /// Returns the raw .panda bytes for the embedded IC named \a name.
    [[nodiscard]] QByteArray blob(const QString &name) const;
    /// Stores or replaces the blob \a data under \a name.
    void setBlob(const QString &name, const QByteArray &data);
    /// Stores blob \a data under \a name. Unlike ICRegistry::registerBlob(), does not
    /// recursively inline nested file-backed dependencies -- see this class's doc comment.
    void registerBlob(const QString &name, const QByteArray &data);
    /// Removes the blob named \a name from the registry.
    void removeBlob(const QString &name);
    /// Renames a blob from \a oldName to \a newName and updates every IC instance referencing
    /// it. Unlike ICRegistry::renameBlob(), does not rewrite nested blob references inside
    /// other blobs' metadata -- see this class's doc comment.
    void renameBlob(const QString &oldName, const QString &newName);
    /// Returns a mutable reference to the full blob map (name -> .panda bytes), for
    /// CanvasItem::deserializationContext()'s SerializationContext::blobRegistry pointer.
    [[nodiscard]] QMap<QString, QByteArray> &blobMapRef() { return m_blobs; }

    /// Finds all embedded IC elements with \a blobName.
    [[nodiscard]] QList<GraphicElement *> findICsByBlobName(const QString &blobName) const;
    /// Initializes an embedded IC by looking up its blob in the registry.
    bool initEmbeddedIC(IC *ic, const QString &blobName);
    /// Returns \a baseName if available, or appends a numeric suffix to avoid collision.
    [[nodiscard]] QString uniqueBlobName(const QString &baseName) const;

    /// Creates a new embedded IC from file bytes, registers the blob, and pushes an undo
    /// macro (CanvasRegisterBlobCommand + CanvasAddItemsCommand). Mirrors
    /// ICRegistry::createEmbeddedIC().
    IC *createEmbeddedIC(const QString &blobName, const QByteArray &fileBytes, const QString &contextDir);

    /// Serializes \a targets' current state for atomic-rollback purposes. Mirrors
    /// ICRegistry::captureSnapshot(); used by QuickWorkSpace::onChildICBlobSaved() so a
    /// mid-reload failure across multiple IC instances can restore the ones already updated.
    static QByteArray captureSnapshot(const QList<GraphicElement *> &targets);
    /// Restores \a elements from a previously captured \a snapshot. Mirrors
    /// ICRegistry::rollbackElements().
    static void rollbackElements(const QList<GraphicElement *> &elements, const QByteArray &snapshot, CanvasItem *canvas);

    /// Finds all file-based (non-embedded) IC elements referencing \a fileName. Mirrors
    /// ICRegistry::findICsByFile().
    [[nodiscard]] QList<GraphicElement *> findICsByFile(const QString &fileName) const;

    /// Registers \a fileBytes under \a blobName and converts every file-based IC instance
    /// referencing \a fileName to embedded, pushing one CanvasUpdateBlobCommand. Returns the
    /// number of instances converted -- 0 if none reference \a fileName, in which case the
    /// blob is NOT registered here (the caller registers it separately via
    /// CanvasRegisterBlobCommand, matching ICController::embedICByFile()'s exact contract).
    /// Mirrors ICRegistry::embedICsByFile().
    int embedICsByFile(const QString &fileName, const QByteArray &fileBytes, const QString &blobName);

    /// Writes the blob named \a blobName to \a filePath, then converts every embedded IC
    /// instance referencing it to file-based, pushing one CanvasUpdateBlobCommand. Throws on
    /// write failure. Mirrors ICRegistry::extractToFile().
    int extractToFile(const QString &blobName, const QString &filePath);

signals:
    /// Emitted at the end of a successful renameBlob() (redo and undo alike -- both funnel
    /// through renameBlob() itself). Mirrors ICRegistry::blobRenamed(); QuickWorkspaceManager
    /// listens for this to retitle any open inline-IC tab tracking the renamed blob.
    void blobRenamed(const QString &oldName, const QString &newName);

private:
    /// Mutates each of \a targets via \a mutate under a SimulationBlocker, rolling all of them
    /// back to \a oldData if any mutation throws partway through. Mirrors
    /// ICRegistry::reloadTargetsAtomically().
    void reloadTargetsAtomically(const QList<GraphicElement *> &targets, const QByteArray &oldData,
                                  const std::function<void(IC *)> &mutate);

    CanvasItem *m_canvas;
    QMap<QString, QByteArray> m_blobs;
};
