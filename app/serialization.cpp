// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "serialization.h"

#include "common.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "qneconnection.h"

#include <QApplication>
#include <QIODevice>

// Safe version reading to prevent allocation overflow attacks
static QVersionNumber readVersionSafely(QDataStream &stream) {
    // QVersionNumber serialization format (Qt 5.12+):
    // - qint32: number of segments
    // - qint32[]: segment values
    
    qint32 segmentCount;
    stream >> segmentCount;
    
    if (stream.status() != QDataStream::Ok) {
        throw Pandaception("Stream error while reading version segment count.", "Stream error while reading version segment count.");
    }
    
    // Validate segment count is reasonable (0-10 segments max)
    if (segmentCount < 0 || segmentCount > 10) {
        throw Pandaception("Invalid version segment count.", "Invalid version segment count.");
    }
    
    // Validate we have enough data for all segments
    qint64 bytesNeeded = static_cast<qint64>(segmentCount) * 4; // 4 bytes per int32
    if (stream.device()->bytesAvailable() < bytesNeeded) {
        throw Pandaception("Insufficient data for version segments.", "Insufficient data for version segments.");
    }
    
    QVector<int> segments;
    segments.reserve(segmentCount);
    
    for (qint32 i = 0; i < segmentCount; ++i) {
        qint32 segment;
        stream >> segment;
        
        if (stream.status() != QDataStream::Ok) {
            throw Pandaception("Stream error while reading version segment.", "Stream error while reading version segment.");
        }
        
        // Validate segment value is reasonable
        if (segment < 0 || segment > 1000) {
            throw Pandaception("Invalid version segment value.", "Invalid version segment value.");
        }
        
        segments.append(segment);
    }
    
    return QVersionNumber(segments);
}

// Safe string reading to prevent allocation overflow attacks
static QString readStringSafely(QDataStream &stream, int maxLength = 1024) {
    qint32 length;
    stream >> length;
    
    if (stream.status() != QDataStream::Ok) {
        throw Pandaception("Stream error while reading string length.", "Stream error while reading string length.");
    }
    
    if (length < 0 || length > maxLength) {
        throw Pandaception("String length out of bounds.", "String length out of bounds.");
    }
    
    // For QString, each character is 2 bytes in the stream (UTF-16)
    qint64 bytesNeeded = static_cast<qint64>(length) * 2;
    if (stream.device()->bytesAvailable() < bytesNeeded) {
        throw Pandaception("Insufficient data for string content.", "Insufficient data for string content.");
    }
    
    QString result;
    result.reserve(length);
    
    for (qint32 i = 0; i < length; ++i) {
        quint16 ch;
        stream >> ch;
        if (stream.status() != QDataStream::Ok) {
            throw Pandaception("Stream error while reading string character.", "Stream error while reading string character.");
        }
        result.append(QChar(ch));
    }
    
    return result;
}


void Serialization::writePandaHeader(QDataStream &stream)
{
    stream.setVersion(QDataStream::Qt_5_12);
    stream << MAGIC_HEADER_CIRCUIT;
    stream << GlobalProperties::version;
}

QVersionNumber Serialization::readPandaHeader(QDataStream &stream)
{
    stream.setVersion(QDataStream::Qt_5_12);

    qint64 originalPos = stream.device()->pos();
    quint32 magicHeader;
    stream >> magicHeader;

    // Check stream status after reading magic header
    if (stream.status() != QDataStream::Ok) {
        throw PANDACEPTION("Stream error while reading file header.");
    }

    QVersionNumber version;

    if (magicHeader == MAGIC_HEADER_CIRCUIT) {
        // Validate we have enough bytes left for version
        qint64 bytesAvailable = stream.device()->size() - stream.device()->pos();
        if (bytesAvailable < 12) { // QVersionNumber typically needs at least 12 bytes
            throw PANDACEPTION("Insufficient data for version information.");
        }
        
        // Use safe version reading to prevent allocation overflow
        try {
            version = readVersionSafely(stream);
        } catch (const std::exception&) {
            throw PANDACEPTION("Invalid version information in file header.");
        }
    } else {
        stream.device()->seek(originalPos);

        // Use safe string reading to prevent allocation overflow
        QString appName;
        try {
            appName = readStringSafely(stream, 256); // Max 256 chars for app name
        } catch (const std::exception&) {
            throw PANDACEPTION("Invalid application name in file header.");
        }

        // Validate stream status after reading app name
        if (stream.status() != QDataStream::Ok) {
            throw PANDACEPTION("Stream error while reading application name.");
        }
        
        // Additional validation: app name should be reasonable length if not empty
        if (!appName.isEmpty() && appName.length() > 256) {
            throw PANDACEPTION("Application name too long in file header.");
        }
        
        if (appName.isEmpty()) { // copy/paste stream, no header
            stream.device()->seek(originalPos);

            QPointF center;
            stream >> center;
            
            if (stream.status() != QDataStream::Ok) {
                throw PANDACEPTION("Stream error while reading center point.");
            }

            if (center.isNull()) {
                throw PANDACEPTION("Invalid file format.");
            }

            stream.device()->seek(originalPos);
            version = QVersionNumber(4, 1); // no version in stream, assume 4.1
        } else if (appName.startsWith("wiRedPanda", Qt::CaseInsensitive)) {
            QStringList split = appName.split(" ");
            if (split.size() < 2) {
                throw PANDACEPTION("Invalid application version format.");
            }
            version = QVersionNumber::fromString(split.at(1));
            
            if (version.isNull()) {
                throw PANDACEPTION("Invalid version number format.");
            }
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

    qint64 originalPos = stream.device()->pos();
    quint32 magicHeader;
    stream >> magicHeader;
    
    if (stream.status() != QDataStream::Ok) {
        throw PANDACEPTION("Stream error while reading waveform file header.");
    }

    if (magicHeader == MAGIC_HEADER_WAVEFORM) {
        QVersionNumber version;
        try {
            version = readVersionSafely(stream);
        } catch (const std::exception&) {
            throw PANDACEPTION("Invalid waveform version information.");
        }
    } else {
        stream.device()->seek(originalPos);

        // Use safe string reading to prevent allocation overflow
        QString appName;
        try {
            appName = readStringSafely(stream, 256); // Max 256 chars for app name
        } catch (const std::exception&) {
            throw PANDACEPTION("Invalid waveform application name in file header.");
        }

        if (!appName.startsWith("beWavedDolphin")) {
            throw PANDACEPTION("Invalid file format.");
        }
    }
}

void Serialization::serialize(const QList<QGraphicsItem *> &items, QDataStream &stream)
{
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

        if ((version < VERSION("3.3")) && (filename == "none")) {
            filename.clear();
        }
    }

    return filename;
}

QRectF Serialization::loadRect(QDataStream &stream, const QVersionNumber version)
{
    QRectF rect;

    if (version >= VERSION("1.4")) {
        stream >> rect;
    }

    return rect;
}

QString Serialization::typeName(const int type) {
    static const QHash<int, QString> typeMap = {
        { QGraphicsItem::UserType + 1, "QNEPort" },
        { QGraphicsItem::UserType + 2, "QNEConnection" },
        { QGraphicsItem::UserType + 3, "GraphicElement" },
    };

    return typeMap.value(type, "UnknownType");
}
