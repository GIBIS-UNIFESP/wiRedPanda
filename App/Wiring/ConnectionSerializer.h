// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ConnectionSerializer: reads/writes a Connection's endpoints to a versioned QDataStream.
 */

#pragma once

#include <QCoreApplication>

class Connection;
class QDataStream;
struct SerializationContext;

/**
 * \class ConnectionSerializer
 * \brief Serializes a Connection's endpoints to/from a versioned QDataStream.
 *
 * \details Extracted from Connection so persistence is a collaborator rather than methods
 * on the wire itself, mirroring GraphicElementSerializer. It needs no friendship — the
 * connection's public port API is sufficient. All methods are static and take the
 * connection explicitly; no serializer instance state is needed. The connection keeps thin
 * save()/load() entry points that delegate here.
 */
class ConnectionSerializer
{
    // tr context for the load-error strings. lupdate extracts the plain PANDACEPTION()
    // throws below under the "ConnectionSerializer" context (the tr+=PANDACEPTION alias
    // resolves via the enclosing class scope); PANDACEPTION_WITH_CONTEXT is NOT extracted.
    Q_DECLARE_TR_FUNCTIONS(ConnectionSerializer)

public:
    /// Writes \a connection's start/end port serial IDs to \a stream (current QMap format).
    static void save(const Connection &connection, QDataStream &stream);

    /// Reads \a connection's endpoints from \a stream, resolving ports via \a context.
    static void load(Connection &connection, QDataStream &stream, SerializationContext &context);
};
