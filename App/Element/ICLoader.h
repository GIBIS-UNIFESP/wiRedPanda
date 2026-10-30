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

class GraphicElement;
class Scene;
class QGraphicsItem;

/**
 * \class ICLoader
 * \brief Loads an IC's sub-circuit (from file or embedded blob), migrates old-format
 * files, and builds the resulting boundary port lists.
 *
 * \details Extracted from IC so file/blob loading is a collaborator rather than a
 * cluster of methods on the element itself, mirroring the GraphicElement /
 * GraphicElementSerializer split. A friend of IC, it reaches the element's
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

    /// Scans \a elements for Input/Output groups, sorts by Y/X position, and builds labels.
    static IC::PortMetadata buildPortMetadata(const QVector<GraphicElement *> &elements);

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

    /// Renders \a ic's hover preview from the definition it loaded last, read again: its bytes, or its file. A
    /// definition that no longer loads leaves the preview null.
    static void renderPreview(IC &ic);

private:
    /// The scene whose file cache \a ic reads through: its own, or for a copy outside one the live IC's.
    static Scene *filesScene(const IC &ic);
    static void loadFileDirectly(IC &ic, const QFileInfo &fileInfo);
    /// Re-saves \a fileInfo in the current format. Pure: touches only its parameters, no IC state.
    static void migrateFile(const QFileInfo &fileInfo, const QList<QGraphicsItem *> &items,
                             const QVersionNumber &version, const QMap<QString, QByteArray> &fileRegistry);
    static void deserializeAndLoad(IC &ic, const QByteArray &bytes, const QString &contextDir);

    /// Transfers ownership of \a items into ic's internal-element/connection vectors,
    /// deleting boundary Input/Output elements after their ports are proxied. Removes
    /// each pointer from \a items as ownership transfers so the caller's qScopeGuard
    /// only deletes still-owned items on throw (no double-free).
    static void processLoadedItems(IC &ic, QList<QGraphicsItem *> &items);
    static void loadBoundaryElement(IC &ic, GraphicElement *elm, bool isInput);
    /// Sizes ic's pins on one side to its boundary ports and names each from \a labels, in the source language from
    /// \a sourceLabels.
    static void loadBoundaryPorts(IC &ic, bool isInput, const QVector<QString> &labels, const QVector<QString> &sourceLabels);
};
