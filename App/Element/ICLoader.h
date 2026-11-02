// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ICLoader: loads an IC's sub-circuit from file or embedded blob.
 */

#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <QByteArray>
#include <QCoreApplication>
#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QString>
#include <QVector>
#include <QVersionNumber>

#include "App/Element/IC.h"
#include "App/Element/PinOrder.h"

class Element;
class ICDefinition;
class QGraphicsItem;

/// A pin of an IC: its name, evaluated when asked, and the same name in the source language.
struct PinName
{
    std::function<QString()> name;
    QString sourceName;
};

/**
 * \class ICLoader
 * \brief Loads an IC's sub-circuit (from file or embedded blob), migrates old-format
 * files, and builds the resulting boundary port lists.
 *
 * \details Extracted from IC so file/blob loading is a collaborator rather than a
 * cluster of methods on the element itself, mirroring the Element /
 * ElementSerializer split. A friend of IC, it reaches the element's
 * internal-element/port vectors directly. All methods are static and take the IC
 * explicitly; IC keeps all state (it's already heavily friended by tests), so this
 * is a pure logic extraction, not a new owner-back-pointer collaborator.
 */
class ICLoader
{
    Q_DECLARE_TR_FUNCTIONS(ICLoader)

public:
    /// Loads \a ic's circuit from \a fileName and rebuilds the logic mapping.
    static void loadFile(IC &ic, const QString &fileName, const QString &contextDir = {});

    /// Loads \a ic from in-memory blob bytes (full .panda file format).
    static void loadFromBlob(IC &ic, const QByteArray &blob, const QString &contextDir);

    /// Loads \a ic from \a bytes, a document's copy of the definition of the file \a fileName, as loadFile() loads the
    /// file: \a ic is file-backed, and the file is recorded as read.
    static void loadFileCopy(IC &ic, const QString &fileName, const QByteArray &bytes, const QString &contextDir);

    /// Returns the stored form of a file copy: the hash of the file as it was when saved, and the definition's \a bytes.
    static QByteArray packFileCopy(const QByteArray &hash, const QByteArray &bytes);
    /// Reads a file copy's stored form into \a hash and \a bytes; returns \c false if it holds neither.
    static bool unpackFileCopy(const QByteArray &packed, QByteArray &hash, QByteArray &bytes);
    /// Returns the hash that identifies a file's \a bytes in a document.
    static QByteArray fileHash(const QByteArray &bytes);

    /// Scans \a elements for Input/Output groups, sorts by Y/X position, and builds labels.
    static IC::PortMetadata buildPortMetadata(const QVector<Element *> &elements);
    static IC::PortMetadata buildPortMetadata(const QVector<PinRef> &inputs, const QVector<PinRef> &outputs);

    /// IC files by absolute path, with their bytes.
    using Files = QMap<QString, QByteArray>;

    /// The absolute path and the bytes loadFile() would load \a ic from for \a fileName, without loading them; none
    /// when there is no file and no snapshot carries one.
    static std::optional<std::pair<QString, QByteArray>> fileBytes(const IC &ic, const QString &fileName,
                                                                   const QString &contextDir = {});

    /// Runs \a load and returns every IC file it read, nested ones included; a load running around it records them
    /// too.
    static Files recordFilesRead(const std::function<void()> &load);

    /// Runs \a load with \a files in place of any of them no longer on disk, as a snapshot's restore does: the IC a
    /// snapshot names loads what it had when the snapshot was taken. When \a restoring, a snapshot's restore, the
    /// circuits \a load reads show or play the default for a picture or a sound that does not load, unreported; a load
    /// inside one inherits both.
    static void withCarriedFiles(const Files &files, bool restoring, const std::function<void()> &load);

    /// Renders the hover preview of \a definition, read from \a contextDir as a load reads it. A definition that no
    /// longer loads renders a null preview.
    static QPixmap renderPreview(const ICDefinition &definition, const QString &contextDir);

    /// Runs \a load with the IC files its snapshot carries standing in for the files on disk, so the restore of an
    /// undone or redone reload gives back the definition the snapshot captured, not the file as it is now.
    static void restoreCapturedFiles(const std::function<void()> &load);

private:
    static void loadFileDirectly(IC &ic, const QFileInfo &fileInfo);
    /// Re-saves \a fileInfo in the current format. Pure: touches only its parameters, no IC state.
    static void migrateFile(const QFileInfo &fileInfo, const QList<QGraphicsItem *> &items,
                             const QVersionNumber &version, const QMap<QString, QByteArray> &fileRegistry);
    /// Builds \a ic's elements from the .panda \a bytes, through the shared definition of \a bytes read from \a path
    /// (empty for a blob).
    static void deserializeAndLoad(IC &ic, const QByteArray &bytes, const QString &contextDir, const QString &path = {});

    /// Transfers ownership of \a items into ic's internal-element/connection vectors,
    /// deleting boundary Input/Output elements after their ports are proxied. Removes
    /// each pointer from \a items as ownership transfers so the caller's qScopeGuard
    /// only deletes still-owned items on throw (no double-free).
    static void processLoadedItems(IC &ic, QList<QGraphicsItem *> &items, const QMap<QString, QVariant> &metadata,
                                   const QHash<quint64, Port *> &portMap);
    static void loadBoundaryElement(IC &ic, Element *elm, bool isInput, QHash<const Port *, Port *> &nodes);
    /// Sizes ic's pins on one side to its boundary ports and names each from \a names.
    static void loadBoundaryPorts(IC &ic, bool isInput, const QVector<PinName> &names);
};
