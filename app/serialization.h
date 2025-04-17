// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QCoreApplication>
#include <QMap>
#include <QVersionNumber>

class QGraphicsItem;
class QNEPort;

class Serialization
{
    Q_DECLARE_TR_FUNCTIONS(Serialization)

public:
    //! Serializes the list of QGraphicItems through a binary data stream.
    static void serialize(const QList<QGraphicsItem *> &items, QDataStream &stream);

    /**
     * @brief deserialize: Deserializes a list of QGraphicItems coming through a binary data stream. It stops at the end of the stream.
     * @return the list of deserialized items.
     * @param portMap is used to return a map of all input and output ports. This mapping may be used to check and to create connections between element ports.
     */
    static QList<QGraphicsItem *> deserialize(QDataStream &stream, QMap<quint64, QNEPort *> portMap, const QVersionNumber version);

    //! returns the canvas pose from the last saved session.
    static QRectF loadRect(QDataStream &stream, const QVersionNumber version);

    static QMap<int, QSet<QPair<int, int>>> loadNodeMappings(QDataStream &stream);
    static QString loadDolphinFileName(QDataStream &stream, const QVersionNumber version);
    static QString typeName(const int type);
    static QVersionNumber readPandaHeader(QDataStream &stream);
    static void readDolphinHeader(QDataStream &stream);
    static void saveNodeMappings(const QMap<int, QSet<QPair<int, int>>> map, QDataStream &stream);
    static void writeDolphinHeader(QDataStream &stream);
    static void writePandaHeader(QDataStream &stream);

    constexpr static quint32 MAGIC_HEADER_CIRCUIT = 0x57504346; // "WPCF" (wiRedPanda Circuit File)
    constexpr static quint32 MAGIC_HEADER_WAVEFORM = 0x57505746; // "WPWF" (wiRedPanda Waveform File)
};
