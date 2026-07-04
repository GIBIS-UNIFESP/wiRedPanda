// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ConnectionSerializer: reads/writes a Connection to a versioned QDataStream.
 */

#pragma once

#include <QCoreApplication>

class Connection;
class QDataStream;
struct SerializationContext;

/**
 * \class ConnectionSerializer
 * \brief Serializes a Connection to/from a versioned QDataStream.
 *
 * \details Extracted from Connection so persistence is a collaborator rather than a
 * pair of methods on the wire itself, mirroring the GraphicElement /
 * GraphicElementSerializer split. Unlike that split, no friend relationship is
 * needed: both directions only ever touch the endpoint ports through Connection's
 * existing public accessors (startPort()/endPort()/setStartPort()/setEndPort()/
 * updatePosFromPorts()). Connection keeps thin save()/load() entry points that
 * delegate here.
 */
class ConnectionSerializer
{
    // tr context for the load-error PANDACEPTION throws. lupdate extracts the plain
    // PANDACEPTION(...) calls below under the "ConnectionSerializer" context (the
    // tr+=PANDACEPTION alias resolves via this class scope); PANDACEPTION_WITH_CONTEXT is NOT
    // extracted.
    Q_DECLARE_TR_FUNCTIONS(ConnectionSerializer)

public:
    /// Writes \a connection's endpoint port serial IDs to \a stream.
    static void save(const Connection &connection, QDataStream &stream);

    /// Reads \a connection's endpoints from \a stream, resolving ports via \a context.
    static void load(Connection &connection, QDataStream &stream, SerializationContext &context);
};
