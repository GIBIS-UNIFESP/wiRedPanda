// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Circuit and waveform file serialization/deserialization utilities.
 */

#pragma once

#include <QCoreApplication>
#include <QMap>
#include <QVersionNumber>

class QGraphicsItem;
class QNEPort;

/**
 * \class Serialization
 * \brief Static utility class for reading and writing circuit and waveform files.
 *
 * \details All I/O passes through a QDataStream; the format is versioned using a
 * magic header followed by the version number.
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
     * \param portMap Map of port IDs to port pointers for reconnection.
     * \param version File format version for backward compatibility.
     * \return List of deserialized QGraphicsItems ready to be added to a scene.
     */
    static QList<QGraphicsItem *> deserialize(QDataStream &stream, QMap<quint64, QNEPort *> portMap, const QVersionNumber version);

    // --- View Serialization ---

    /// Returns the canvas viewport rectangle from the last saved session.
    static QRectF loadRect(QDataStream &stream, const QVersionNumber version);

    // --- Waveform Serialization ---

    /// Returns the BeWavedDolphin waveform file name stored in \a stream at \a version.
    static QString loadDolphinFileName(QDataStream &stream, const QVersionNumber version);

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

    // --- Magic Headers ---

    constexpr static quint32 MAGIC_HEADER_CIRCUIT = 0x57504346; // "WPCF" (wiRedPanda Circuit File)
    constexpr static quint32 MAGIC_HEADER_WAVEFORM = 0x57505746; // "WPWF" (wiRedPanda Waveform File)
};

