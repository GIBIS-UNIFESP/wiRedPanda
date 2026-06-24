// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Wiring/ConnectionSerializer.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QIODevice>
#include <QMap>
#include <QVariant>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

void ConnectionSerializer::save(const Connection &connection, QDataStream &stream)
{
    // Calculate and save port serial IDs deterministically, using the same packing as
    // GraphicElementSerializer (globalIndex() offsets output ports past the inputs).
    auto calculateSerialId = [](Port *port) -> quint64 {
        GraphicElement *elem = port ? port->graphicElement() : nullptr;
        if (!elem) {
            return 0;
        }
        return Port::makeSerialId(static_cast<quint64>(elem->id()), port->globalIndex());
    };

    QMap<QString, QVariant> map;
    map.insert("startPortId", calculateSerialId(connection.startPort()));
    map.insert("endPortId", calculateSerialId(connection.endPort()));
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
    } else {
        stream >> id1;
        stream >> id2;

        if (stream.status() != QDataStream::Ok) {
            throw PANDACEPTION("Stream error reading connection port IDs at offset %1",
                               stream.device()->pos());
        }

        // For backwards compatibility with old files: if an ID is not found and an
        // oldPtrMap is provided, try the mapping.
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

    // Exactly one endpoint must be an output and the other an input; normalize the pair so the
    // wire always runs output(start) -> input(end), regardless of the order they were serialized.
    if (port1 && port2 && (port1->isOutput() != port2->isOutput())) {
        auto *outputPort = dynamic_cast<OutputPort *>(port1->isOutput() ? port1 : port2);
        auto *inputPort = dynamic_cast<InputPort *>(port1->isOutput() ? port2 : port1);
        if (outputPort && inputPort) {
            connection.setStartPort(outputPort);
            connection.setEndPort(inputPort);
        }
    }

    connection.updatePosFromPorts();
}
