// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/IO/Serialization.h"

#include <QApplication>
#include <QIODevice>

#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/GlobalProperties.h"
#include "App/Nodes/QNEConnection.h"

void Serialization::writePandaHeader(QDataStream &stream)
{
    stream.setVersion(QDataStream::Qt_5_12);
    stream << MAGIC_HEADER_CIRCUIT;
    stream << GlobalProperties::version;
}

QVersionNumber Serialization::readPandaHeader(QDataStream &stream)
{
    stream.setVersion(QDataStream::Qt_5_12);

    // Peek the first four bytes; if they match the magic number the file was
    // written by a modern release.  Otherwise we rewind and attempt to parse
    // the legacy text-based header formats.
    qint64 originalPos = stream.device()->pos();
    quint32 magicHeader;
    stream >> magicHeader;

    QVersionNumber version;

    if (magicHeader == MAGIC_HEADER_CIRCUIT) {
        stream >> version;
    } else {
        // Rewind — the four bytes we consumed are actually the start of a
        // legacy header (either an app-name string or clipboard data).
        stream.device()->seek(originalPos);

        QString appName;
        stream >> appName; // "WiredPanda 1.1"

        if (appName.isEmpty()) {
            // Clipboard paste streams have no file header at all; the stream
            // starts directly with the center-point QPointF.  A null center
            // point would indicate genuinely corrupt data.
            stream.device()->seek(originalPos);

            QPointF center;
            stream >> center;

            if (center.isNull()) {
                throw PANDACEPTION("Invalid file format.");
            }

            stream.device()->seek(originalPos);
            // Version 4.1 is the last release that used this headerless format.
            version = QVersionNumber(4, 1); // no version in stream, assume 4.1
        } else if (appName.startsWith("wiRedPanda", Qt::CaseInsensitive)) {
            // Older files encoded the version as "wiRedPanda X.Y" inside a QString.
            QStringList split = appName.split(" ");
            version = QVersionNumber::fromString(split.at(1));
        } else {
            throw PANDACEPTION("Invalid file format.");
        }
    }

    return version.normalized();
 }

void Serialization::writeDolphinHeader(QDataStream &stream)
{
    stream.setVersion(QDataStream::Qt_5_12);
    stream << MAGIC_HEADER_WAVEFORM;
    stream << GlobalProperties::version;
}

void Serialization::readDolphinHeader(QDataStream &stream)
{
    stream.setVersion(QDataStream::Qt_5_12);

    // Same two-phase detection as readPandaHeader: magic number for modern
    // waveform files, legacy app-name string for older beWavedDolphin saves.
    qint64 originalPos = stream.device()->pos();
    quint32 magicHeader;
    stream >> magicHeader;

    if (magicHeader == MAGIC_HEADER_WAVEFORM) {
        QVersionNumber version;
        stream >> version;
    } else {
        stream.device()->seek(originalPos);

        QString appName;
        stream >> appName;

        if (!appName.startsWith("beWavedDolphin")) {
            throw PANDACEPTION("Invalid file format.");
        }
    }
}

void Serialization::serialize(const QList<QGraphicsItem *> &items, QDataStream &stream)
{
    // Elements must be written before connections because deserialization reads
    // elements first to build the port map, then resolves connection endpoints
    // using that map.  Reversing the order would cause every connection load to fail.
    for (auto *item : items) {
        if (auto *element = qgraphicsitem_cast<GraphicElement *>(item)) {
            stream << element;
        }
    }

    for (auto *item : items) {
        if (auto *connection = qgraphicsitem_cast<QNEConnection *>(item)) {
            stream << connection;
        }
    }
}

QList<QGraphicsItem *> Serialization::deserialize(QDataStream &stream, QMap<quint64, QNEPort *> portMap, const QVersionNumber version)
{
    // portMap maps the raw pointer value (quint64) that was stored at save time to
    // the newly allocated QNEPort object at load time.  Raw pointer values are used
    // as keys because they were the only unique, stable port identity available when
    // the binary format was designed; they are meaningless as pointers after reload
    // but work perfectly as opaque 64-bit tokens for this cross-reference purpose.
    // When portMap is empty (top-level file load), QNEConnection::load() falls back
    // to directly casting the stored integer back to a pointer — only safe during
    // the same process session (e.g., clipboard paste with in-process copy/paste).
    QList<QGraphicsItem *> itemList;

    while (!stream.atEnd()) {
        int type; stream >> type;
        qCDebug(three) << "Type: " << typeName(type);

        switch (type) {
        case GraphicElement::Type: {
            ElementType elmType; stream >> elmType;

            auto *elm = ElementFactory::buildElement(elmType);
            itemList.append(elm);
            elm->load(stream, portMap, version);
            break;
        }

        case QNEConnection::Type: {
            qCDebug(three) << "Building connection.";
            auto *conn = new QNEConnection();

            qCDebug(three) << "Loading connection.";
            conn->load(stream, portMap);

            qCDebug(three) << "Appending connection.";
            itemList.append(conn);
            break;
        }

        default:
            throw PANDACEPTION("Invalid type. Data is possibly corrupted.");
        }
    }

    qCDebug(zero) << "Finished deserializing.";
    return itemList;
}

QString Serialization::loadDolphinFileName(QDataStream &stream, const QVersionNumber version)
{
    QString filename;

    if (version >= VERSION("3.0")) {
        stream >> filename;

        // Versions 3.0–3.2 used the sentinel string "none" instead of an empty
        // QString to indicate that no waveform file was associated; normalize it here
        // so callers can simply check isEmpty()
        if ((version < VERSION("3.3")) && (filename == "none")) {
            filename.clear();
        }
    }

    // Versions before 3.0 didn't store a dolphin filename at all; return empty string
    return filename;
}

QRectF Serialization::loadRect(QDataStream &stream, const QVersionNumber version)
{
    QRectF rect;

    // The stored rect is always discarded by the caller (WorkSpace recomputes it from
    // items after load), but it must still be read to advance the stream past this field
    if (version >= VERSION("1.4")) {
        stream >> rect;
    }

    return rect;
}

QString Serialization::typeName(const int type) {
    // These offsets must stay in sync with the ::Type enum constants defined in
    // QNEPort, QNEConnection, and GraphicElement (all use QGraphicsItem::UserType + N)
    static const QHash<int, QString> typeMap = {
        { QGraphicsItem::UserType + 1, "QNEPort" },
        { QGraphicsItem::UserType + 2, "QNEConnection" },
        { QGraphicsItem::UserType + 3, "GraphicElement" },
    };

    return typeMap.value(type, "UnknownType");
}

