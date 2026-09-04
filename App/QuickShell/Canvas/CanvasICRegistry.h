// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief CanvasItem-side port of App/Scene/ICRegistry.h's embedded-IC blob storage.
 */

#pragma once

#include <functional>

#include <QByteArray>
#include <QFileSystemWatcher>
#include <QList>
#include <QMap>
#include <QObject>
#include <QSet>
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
 * rollbackElements() -- the half MCP's embed_ic/instantiate_ic handlers actually exercise.
 * Also ports findICsByFile()/
 * embedICsByFile()/extractToFile() -- ICDropZone/TrashButton's real drag-and-drop targets
 * need them; QuickICController is their caller, mirroring ICController::embedICByFile()/
 * extractICByBlobName(). Also ports watchFile()/onFileChanged() -- CanvasItem::addItem()
 * calls watchFile() for every file-backed IC it registers, mirroring
 * Scene::registerItem()'s identical hook.
 *
 * A real QObject (unlike most other Canvas* ports, which are plain classes) so it can
 * emit blobRenamed()/definitionChanged() -- QuickWorkspaceManager listens for the former to
 * keep an open inline-IC tab's displayed title in sync with its blob's current name, mirroring
 * ICRegistry's identical signal and WorkspaceManager::onBlobRenamed(); the latter has no
 * listener yet (ICRegistry's own doesn't either -- it exists so a future palette/status-bar
 * refresh has something to connect to, same as production). Composed as a plain (non-heap,
 * non-pointer) member of CanvasItem, same as before -- a QObject-derived value member with its
 * owner as QObject parent is a normal, supported Qt pattern; nothing about adding Q_OBJECT
 * requires heap allocation.
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
    /// Stores blob \a data under \a name, first recursively inlining any file-backed IC
    /// dependencies it references so the stored blob is fully self-contained. Mirrors
    /// ICRegistry::registerBlob() exactly.
    void registerBlob(const QString &name, const QByteArray &data);
    /// Removes the blob named \a name from the registry.
    void removeBlob(const QString &name);
    /// Renames a blob from \a oldName to \a newName, updates every IC instance referencing it,
    /// and rewrites the reference inside any other blob's own metadata that embeds it as a
    /// nested dependency. Mirrors ICRegistry::renameBlob() exactly.
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

    /// Finds every file-backed IC element on the canvas whose file() matches \a fileName.
    /// Mirrors ICRegistry::findICsByFile().
    [[nodiscard]] QList<GraphicElement *> findICsByFile(const QString &fileName) const;

    /// Converts every file-backed IC referencing \a fileName into an embedded IC under
    /// \a blobName, storing \a fileBytes as the blob and pushing one undoable
    /// CanvasUpdateBlobCommand covering all of them. Returns the number of ICs converted (0 if
    /// none referenced that file). Mirrors ICRegistry::embedICsByFile().
    int embedICsByFile(const QString &fileName, const QByteArray &fileBytes, const QString &blobName);

    /// Writes the \a blobName blob out to \a filePath and converts every embedded IC using it
    /// into a file-backed IC referencing that path, pushing one undoable
    /// CanvasUpdateBlobCommand. Returns the number of ICs converted (0 if none used that
    /// blob). Mirrors ICRegistry::extractToFile().
    int extractToFile(const QString &blobName, const QString &filePath);

    /// Registers \a filePath for watching. Called from CanvasItem::addItem() for every
    /// file-backed IC element it registers. Mirrors ICRegistry::watchFile().
    void watchFile(const QString &filePath);

signals:
    /// Emitted at the end of a successful renameBlob() (redo and undo alike -- both funnel
    /// through renameBlob() itself). Mirrors ICRegistry::blobRenamed(); QuickWorkspaceManager
    /// listens for this to retitle any open inline-IC tab tracking the renamed blob.
    void blobRenamed(const QString &oldName, const QString &newName);

    /// Emitted at the end of onFileChanged(), whether or not any IC instance actually
    /// referenced the changed file. Mirrors ICRegistry::definitionChanged().
    void definitionChanged(const QString &filePath);

private slots:
    /// Handles QFileSystemWatcher notifications: reloads every IC instance referencing
    /// \a filePath and pushes one undoable CanvasUpdateBlobCommand covering all of them.
    /// Mirrors ICRegistry::onFileChanged() exactly, including the guardedSlot() wrapping --
    /// see that method's own doc comment for why a queued slot needs its own catch frame
    /// rather than relying on an upstream one.
    void onFileChanged(const QString &filePath);

private:
    /// Maximum recursion depth for makeBlobSelfContained()'s dependency walk. Cycle detection
    /// via its own \c visited set alone doesn't bound a long, non-cyclic chain of distinct
    /// legitimate files (A embeds B embeds C embeds ..., no repeats) -- mirrors
    /// ICRegistry::kMaxBlobNestingDepth exactly.
    static constexpr int kMaxBlobNestingDepth = 16;

    /// Applies \a mutate to each of \a targets in turn, rolling every already-mutated target
    /// in \a targets back to \a oldData if \a mutate throws partway through -- an all-or-
    /// nothing reload, mirroring embedICsByFile()/extractToFile()'s shared atomicity
    /// requirement (a mid-batch IC::loadFile()/loadFromBlob() failure must not leave some
    /// targets converted and others not).
    void reloadTargetsAtomically(const QList<GraphicElement *> &targets, const QByteArray &oldData,
                                  const std::function<void(IC *)> &mutate);

    /// Renames a nested blob reference from \a oldName to \a newName inside \a blobData's
    /// metadata (a no-op if \a blobData doesn't reference \a oldName at all). Mirrors
    /// ICRegistry::renameBlobReference() exactly -- a pure QByteArray transform, no CanvasItem
    /// dependency.
    static void renameBlobReference(QByteArray &blobData, const QString &oldName, const QString &newName);

    /// Recursively inlines all IC dependencies of blob \a name so it has no external file
    /// references. Uses \a blobs as working storage instead of m_blobs to avoid corrupting
    /// state on failure. \a depth counts recursion levels from the initial call (0) and throws
    /// once it would exceed kMaxBlobNestingDepth, so a long chain of distinct dependency files
    /// can't exhaust the stack. Mirrors ICRegistry::makeBlobSelfContained() exactly, reading
    /// file-backed dependencies relative to this canvas's own contextDir() instead of the
    /// scene's.
    void makeBlobSelfContained(const QString &name, QSet<QString> &visited,
                                QMap<QString, QByteArray> &blobs, int depth = 0);

    CanvasItem *m_canvas;
    QMap<QString, QByteArray> m_blobs;
    QFileSystemWatcher m_fileWatcher; ///< Watches file-backed IC source files for external modifications.
};
