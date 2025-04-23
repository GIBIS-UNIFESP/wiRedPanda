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
