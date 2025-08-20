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

    QVersionNumber version;

    if (magicHeader == MAGIC_HEADER_CIRCUIT) {
        stream >> version;
    } else {
        stream.device()->seek(originalPos);

        QString appName;
        stream >> appName; // "WiredPanda 1.1"

        if (appName.isEmpty()) { // copy/paste stream, no header
            stream.device()->seek(originalPos);

            QPointF center;
            stream >> center;

            if (center.isNull()) {
                throw Pandaception(tr("1Invalid file format."));
            }

            stream.device()->seek(originalPos);
            version = QVersionNumber(4, 1); // no version in stream, assume 4.1
        } else if (appName.startsWith("wiRedPanda", Qt::CaseInsensitive)) {
            QStringList split = appName.split(" ");
            version = QVersionNumber::fromString(split.at(1));
        } else {
            throw Pandaception(tr("2Invalid file format."));
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

    if (magicHeader == MAGIC_HEADER_WAVEFORM) {
        QVersionNumber version;
        stream >> version;
    } else {
        stream.device()->seek(originalPos);

        QString appName;
        stream >> appName;

        if (!appName.startsWith("beWavedDolphin")) {
            throw Pandaception(tr("3Invalid file format."));
        }
    }
}

void Serialization::saveNodeMappings(const QMap<int, QSet<Destination>> map, QDataStream &stream)
{
    stream << map;
}

QMap<int, QSet<Destination>> Serialization::loadNodeMappings(QDataStream &stream)
{
    QMap<int, QSet<Destination>> map;
    stream >> map;
    return map;
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
            conn->load(stream, portMap, version);

            qCDebug(three) << "Appending connection.";
            itemList.append(conn);
            break;
        }

        default:
            throw Pandaception(tr("Invalid type. Data is possibly corrupted."));
        }
    }

    qCDebug(zero) << "Finished deserializing.";
    return itemList;
}

void Serialization::serializeWithWireless(const QList<QGraphicsItem *> &items, QDataStream &stream, const QMap<int, QSet<Destination>> &nodeMapping)
{
    if (items.isEmpty()) {
        return;
    }
    
    // First serialize the items normally
    serialize(items, stream);

    // Extract node IDs from the items efficiently
    QList<int> nodeIds;
    nodeIds.reserve(items.size());
    
    for (const auto *item : items) {
        if (item && item->type() == GraphicElement::Type) {
            if (const auto *element = qgraphicsitem_cast<const GraphicElement *>(item)) {
                nodeIds.append(element->id());
            }
        }
    }

    // Filter and save only the relevant wireless mappings
    if (!nodeIds.isEmpty() && !nodeMapping.isEmpty()) {
        auto filteredMappings = filterNodeMappings(nodeMapping, nodeIds);
        saveNodeMappings(filteredMappings, stream);
        qCDebug(zero) << "Serialized" << filteredMappings.size() << "wireless mappings with items";
    } else {
        // Save empty mappings to maintain stream format
        QMap<int, QSet<Destination>> emptyMappings;
        saveNodeMappings(emptyMappings, stream);
    }
}

QList<QGraphicsItem *> Serialization::deserializeWithWireless(QDataStream &stream, QMap<quint64, QNEPort *> portMap, const QVersionNumber version, QMap<int, QSet<Destination>> &outNodeMapping)
{
    // First deserialize the items normally
    auto items = deserialize(stream, portMap, version);

    // Then load the wireless mappings
    if (!stream.atEnd()) {
        outNodeMapping = loadNodeMappings(stream);
    }

    return items;
}

QMap<int, QSet<Destination>> Serialization::translateNodeMappings(const QMap<int, QSet<Destination>> &mappings, const QMap<int, int> &idTranslation)
{
    if (mappings.isEmpty() || idTranslation.isEmpty()) {
        return mappings;
    }
    
    QMap<int, QSet<Destination>> translatedMappings;
    // Note: QMap doesn't have reserve() in Qt5, capacity grows automatically

    for (auto it = mappings.cbegin(); it != mappings.cend(); ++it) {
        const int oldSourceId = it.key();
        const int newSourceId = idTranslation.value(oldSourceId, oldSourceId);

        QSet<Destination> translatedDestinations;
        const auto &sourceDestinations = it.value();
        translatedDestinations.reserve(sourceDestinations.size());
        
        for (const auto &dest : sourceDestinations) {
            Destination newDest;
            newDest.connectionId = dest.connectionId;
            newDest.nodeId = idTranslation.value(dest.nodeId, dest.nodeId);
            translatedDestinations.insert(newDest);
        }

        if (!translatedDestinations.isEmpty()) {
            translatedMappings.insert(newSourceId, std::move(translatedDestinations));
        }
    }

    qCDebug(zero) << "Translated" << mappings.size() << "mappings to" << translatedMappings.size() << "mappings";
    return translatedMappings;
}

QMap<int, QSet<Destination>> Serialization::filterNodeMappings(const QMap<int, QSet<Destination>> &mappings, const QList<int> &nodeIds)
{
    if (mappings.isEmpty() || nodeIds.isEmpty()) {
        return QMap<int, QSet<Destination>>();
    }
    
    // Convert to QSet for O(1) lookup performance
    QSet<int> nodeIdSet;
    nodeIdSet.reserve(nodeIds.size());
    for (int nodeId : nodeIds) {
        nodeIdSet.insert(nodeId);
    }
    
    QMap<int, QSet<Destination>> filteredMappings;
    // Note: QMap doesn't have reserve() in Qt5, capacity grows automatically

    for (auto it = mappings.cbegin(); it != mappings.cend(); ++it) {
        const int sourceId = it.key();

        // Include mapping if source node is in the filter set
        if (nodeIdSet.contains(sourceId)) {
            QSet<Destination> filteredDestinations;
            const auto &sourceDestinations = it.value();
            filteredDestinations.reserve(sourceDestinations.size());

            // Only include destinations that are also in the filter set
            for (const auto &dest : sourceDestinations) {
                if (nodeIdSet.contains(dest.nodeId)) {
                    filteredDestinations.insert(dest);
                }
            }

            if (!filteredDestinations.isEmpty()) {
                filteredMappings.insert(sourceId, std::move(filteredDestinations));
            }
        }
    }

    qCDebug(zero) << "Filtered" << mappings.size() << "mappings to" << filteredMappings.size() << "mappings";
    return filteredMappings;
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
