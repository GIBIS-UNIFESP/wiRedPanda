// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Circuit and waveform file serialization/deserialization utilities.
 */

#pragma once

#include <QCoreApplication>
#include <QMap>
#include <QString>
#include <QVersionNumber>

#include "App/IO/SerializationContext.h"

class QGraphicsItem;
class QNEPort;

/**
 * \class Serialization
 * \brief Static utility class for reading and writing circuit and waveform files.
 *
 * \details All I/O passes through a QDataStream; the format is versioned using a
 * magic header followed by the version number.  The context object carries per-load
 * state (port map, version, IC directory, optional copy-operation) so that element
 * implementations receive everything they need without global variables.
 */
class Serialization
{
    Q_DECLARE_TR_FUNCTIONS(Serialization)

public:
    /**
     * \brief Serializes \a items to \a stream in the current .panda binary format.
     * \param items Items to serialize (graphic elements and connections).
     * \param stream Destination data stream.
     */
    static void serialize(const QList<QGraphicsItem *> &items, QDataStream &stream);

    /**
     * \brief Deserializes items from \a stream until the stream is exhausted.
     * \param stream Source data stream.
     * \param context Per-load context (portMap, version, contextDir, copyOperation).
     * \return List of deserialized QGraphicsItems ready to be added to a scene.
     */
    static QList<QGraphicsItem *> deserialize(QDataStream &stream, SerializationContext &context);

    // --- View Serialization ---

    /// Returns the canvas viewport rectangle from the last saved session.
    static QRectF loadRect(QDataStream &stream, const QVersionNumber &version);

    // --- Waveform Serialization ---

    /// Returns the BeWavedDolphin waveform file name stored in \a stream at \a version.
    static QString loadDolphinFileName(QDataStream &stream, const QVersionNumber &version);

    // --- Utility ---

    /// Returns the human-readable element type name for the given \a type integer.
    static QString typeName(const int type);

    // --- File Headers ---

    /// Reads and validates the .panda circuit file header; returns the stored version number.
    static QVersionNumber readPandaHeader(QDataStream &stream);

    /**
     * \brief Reads and validates the BeWavedDolphin waveform file header.
     * \param stream Source data stream positioned at the start of the file.
     */
    static void readDolphinHeader(QDataStream &stream);

    /**
     * \brief Writes the BeWavedDolphin waveform file header to \a stream.
     * \param stream Destination data stream.
     */
    static void writeDolphinHeader(QDataStream &stream);

    /**
     * \brief Writes the .panda circuit file header to \a stream.
     * \param stream Destination data stream.
     */
    static void writePandaHeader(QDataStream &stream);

    /**
     * \brief Copies \a fileName to a versioned sidecar before overwriting it during migration.
     * \param fileName Absolute path of the file to back up.
     * \param version  The old file-format version (used to build the sidecar name).
     *
     * \details The backup name has the form \c basename.vX.Y.Z.ext (e.g. \c ic.v4.1.panda).
     * If a backup with that name already exists it is left untouched.
     * Silently logs a warning if the copy fails; does not throw.
     */
    static void createVersionedBackup(const QString &fileName, const QVersionNumber &version);

    // --- Panda Preamble ---

    /// Result of reading a .panda file preamble (header + dolphin + rect + metadata).
    struct Preamble {
        QVersionNumber version;
        QMap<QString, QVariant> metadata;
    };

    /// Reads the full .panda preamble: header, dolphin filename, rect, and metadata (V_4_5+).
    static Preamble readPreamble(QDataStream &stream);

    // --- Embedded ICs ---

    /// Extracts the embedded IC registry from a metadata map. Returns empty map if not present.
    static QMap<QString, QByteArray> deserializeBlobRegistry(const QMap<QString, QVariant> &metadata);

    /// Serializes embedded ICs into a metadata map (sets the "embeddedICs" key).
    static void serializeBlobRegistry(const QMap<QString, QByteArray> &blobs, QMap<QString, QVariant> &metadata);

    // --- Magic Headers ---

    constexpr static quint32 MAGIC_HEADER_CIRCUIT = 0x57504346; // "WPCF" (wiRedPanda Circuit File)
    constexpr static quint32 MAGIC_HEADER_WAVEFORM = 0x57505746; // "WPWF" (wiRedPanda Waveform File)
};

