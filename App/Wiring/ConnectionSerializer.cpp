// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Wiring/ConnectionSerializer.h"

#include <QIODevice>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

void ConnectionSerializer::save(const Connection &connection, QDataStream &stream)
{
    // Calculate and save port serial IDs deterministically. See Port::makeSerialId().
    auto calculateSerialId = [](Port *port) -> quint64 {
        if (!port) return 0;

        GraphicElement *elem = port->graphicElement();
        if (!elem) return 0;

        return Port::makeSerialId(static_cast<quint64>(elem->id()), port->globalIndex());
    };

    QMap<QString, QVariant> map;
    map.insert("startPortId", calculateSerialId(connection.startPort()));
    map.insert("endPortId", calculateSerialId(connection.endPort()));
    map.insert("wireMode", static_cast<quint8>(connection.wireMode()));

    if (!connection.waypoints().isEmpty()) {
        QByteArray wpBytes;
        QDataStream wpStream(&wpBytes, QIODevice::WriteOnly);
        wpStream.setVersion(QDataStream::Qt_5_12);
        wpStream << connection.waypoints();
        map.insert("waypoints", wpBytes);
    }

    stream << map;
}

void ConnectionSerializer::load(Connection &connection, QDataStream &stream, SerializationContext &context)
{
    quint64 id1, id2;

    if (VersionInfo::hasConnectionQMap(context.version)) {
        QMap<QString, QVariant> map = Serialization::readBoundedMetadata(stream);

        if (stream.status() != QDataStream::Ok) {
            throw PANDACEPTION("Stream error reading connection map at offset %1",
                              stream.device()->pos());
        }

        id1 = map.value("startPortId").toULongLong();
        id2 = map.value("endPortId").toULongLong();

        if (map.contains("wireMode")) {
            connection.setWireMode(static_cast<WireMode>(map.value("wireMode").toUInt()));
        }

        if (map.contains("waypoints")) {
            const QByteArray wpBytes = map.value("waypoints").toByteArray();
            QDataStream wpStream(wpBytes);
            wpStream.setVersion(QDataStream::Qt_5_12);
            connection.setWaypoints(Serialization::readBoundedPointVector(wpStream));
        }
    } else {
        stream >> id1;
        stream >> id2;

        if (stream.status() != QDataStream::Ok) {
            throw PANDACEPTION("Stream error reading connection port IDs at offset %1",
                              stream.device()->pos());
        }

        // For backwards compatibility with old files: if ID not found and oldPtrMap is provided, try the mapping
        if (!context.portMap.contains(id1) && !context.oldPtrToSerialId.isEmpty()) {
            quint64 newId = context.oldPtrToSerialId.value(id1, 0);
            if (newId != 0) {
                id1 = newId;
            }
        }

        if (!context.portMap.contains(id2) && !context.oldPtrToSerialId.isEmpty()) {
            quint64 newId = context.oldPtrToSerialId.value(id2, 0);
            if (newId != 0) {
                id2 = newId;
            }
        }
    }

    if (!context.portMap.contains(id1) || !context.portMap.contains(id2)) {
        return;
    }

    auto *port1 = context.portMap.value(id1);
    auto *port2 = context.portMap.value(id2);

    if (port1 && port2) {
        // Exactly one of the pair must be the output (wire start) and the other the
        // input (wire end); which one was saved as startPortId/endPortId doesn't matter.
        Port *outputCandidate = port1->isOutput() ? port1 : port2->isOutput() ? port2 : nullptr;
        Port *inputCandidate = port1->isInput() ? port1 : port2->isInput() ? port2 : nullptr;

        if (outputCandidate && inputCandidate && outputCandidate != inputCandidate) {
            auto *outputPort = dynamic_cast<OutputPort *>(outputCandidate);
            auto *inputPort = dynamic_cast<InputPort *>(inputCandidate);
            if (outputPort && inputPort) {
                connection.setStartPort(outputPort);
                connection.setEndPort(inputPort);
            }
        }
    }

    connection.updatePosFromPorts();
}
