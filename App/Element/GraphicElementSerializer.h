// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief GraphicElementSerializer: reads/writes a GraphicElement to a versioned QDataStream.
 */

#pragma once

#include <QCoreApplication>
#include <QHash>
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QtGlobal>

class GraphicElement;
class Port;
class QDataStream;
class QVersionNumber;
struct SerializationContext;

/**
 * \class GraphicElementSerializer
 * \brief Serializes a GraphicElement to/from a versioned QDataStream.
 *
 * \details Extracted from GraphicElement so persistence is a collaborator rather than a
 * cluster of methods on the element itself. A friend of GraphicElement, it reaches the
 * element's ports/orientation/appearance/context-directory state through that friendship.
 * The element keeps thin virtual save()/load() entry points that delegate here, so
 * subclasses (e.g. IC) can still extend the load via their override. All methods are
 * static and take the element explicitly; no serializer instance state is needed.
 */
class GraphicElementSerializer
{
    // tr context for the load-error PANDACEPTION throws. lupdate extracts the plain
    // PANDACEPTION(...) calls below under the "GraphicElementSerializer" context (the
    // tr+=PANDACEPTION alias resolves via this class scope); PANDACEPTION_WITH_CONTEXT is NOT
    // extracted.
    Q_DECLARE_TR_FUNCTIONS(GraphicElementSerializer)

public:
    /// Writes \a element to \a stream (current keyed-QMap format).
    static void save(const GraphicElement &element, QDataStream &stream);

    /// Reads \a element from \a stream, dispatching on the format version in \a context.
    static void load(GraphicElement &element, QDataStream &stream, SerializationContext &context);

private:
    static void loadOldFormat(GraphicElement &element, QDataStream &stream, SerializationContext &context);
    static void loadNewFormat(GraphicElement &element, QDataStream &stream, SerializationContext &context);

    static void loadPos(GraphicElement &element, QDataStream &stream);
    static void loadRotation(GraphicElement &element, QDataStream &stream, const QVersionNumber &version);
    static void loadLabel(GraphicElement &element, QDataStream &stream, const QVersionNumber &version);
    static void loadPortsSize(QDataStream &stream, const QVersionNumber &version);
    static void loadTrigger(GraphicElement &element, QDataStream &stream, const QVersionNumber &version);

    static void loadInputPorts(GraphicElement &element, QDataStream &stream, SerializationContext &context);
    static void loadInputPort(GraphicElement &element, QDataStream &stream, SerializationContext &context, int port);
    static void loadOutputPorts(GraphicElement &element, QDataStream &stream, SerializationContext &context);
    static void loadOutputPort(GraphicElement &element, QDataStream &stream, SerializationContext &context, int port);

    static void removeSurplusInputs(GraphicElement &element, quint64 inputSize_, SerializationContext &context);
    static void removeSurplusOutputs(GraphicElement &element, quint64 outputSize_, SerializationContext &context);

    static void loadPixmapAppearanceNames(GraphicElement &element, QDataStream &stream, SerializationContext &context);
    static void loadPixmapAppearanceName(GraphicElement &element, QDataStream &stream, int index, const QString &contextDir);

    /// Reads a `QList<QMap<QString, QVariant>>` from \a stream, rejecting implausible counts up
    /// front (fuzz-hardening). A member (not a file-local helper) so its load-error throws extract
    /// under this class's tr context.
    static QList<QMap<QString, QVariant>> readPortList(QDataStream &stream, const char *label);

    /// Erases \a deletedPort's serial-ID entry from \a portMap during deserialization.
    static void removePortFromMap(Port *deletedPort, QHash<quint64, Port *> &portMap);
};
