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
    // First serialize the items normally
    serialize(items, stream);
    
    // Extract node IDs from the items
    QList<int> nodeIds;
    for (auto *item : items) {
        if (auto *element = qgraphicsitem_cast<GraphicElement *>(item)) {
            nodeIds.append(element->id());
        }
    }
    
    // Filter and save only the relevant wireless mappings
    auto filteredMappings = filterNodeMappings(nodeMapping, nodeIds);
    saveNodeMappings(filteredMappings, stream);
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
    QMap<int, QSet<Destination>> translatedMappings;
    
    for (auto it = mappings.begin(); it != mappings.end(); ++it) {
        int oldSourceId = it.key();
        int newSourceId = idTranslation.value(oldSourceId, oldSourceId);
        
        QSet<Destination> translatedDestinations;
        for (const auto &dest : it.value()) {
            Destination newDest;
            newDest.connectionId = dest.connectionId;
            newDest.nodeId = idTranslation.value(dest.nodeId, dest.nodeId);
            translatedDestinations.insert(newDest);
        }
        
        if (!translatedDestinations.isEmpty()) {
            translatedMappings[newSourceId] = translatedDestinations;
        }
    }
    
    return translatedMappings;
}

QMap<int, QSet<Destination>> Serialization::filterNodeMappings(const QMap<int, QSet<Destination>> &mappings, const QList<int> &nodeIds)
{
    QMap<int, QSet<Destination>> filteredMappings;
    QSet<int> nodeIdSet(nodeIds.begin(), nodeIds.end());
    
    for (auto it = mappings.begin(); it != mappings.end(); ++it) {
        int sourceId = it.key();
        
        // Include mapping if source node is in the list
        if (nodeIdSet.contains(sourceId)) {
            QSet<Destination> filteredDestinations;
            
            // Only include destinations that are also in the list
            for (const auto &dest : it.value()) {
                if (nodeIdSet.contains(dest.nodeId)) {
                    filteredDestinations.insert(dest);
                }
            }
            
            if (!filteredDestinations.isEmpty()) {
                filteredMappings[sourceId] = filteredDestinations;
            }
        }
    }
    
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
