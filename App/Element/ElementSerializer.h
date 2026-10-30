// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ElementSerializer: reads/writes a Element to a versioned QDataStream.
 */

#pragma once

#include <QCoreApplication>
#include <QHash>
#include <QList>
#include <QMap>
#include <QString>
#include <QtGlobal>
#include <QVariant>

class Element;
class Port;
class QDataStream;
class QPointF;
class QVersionNumber;
struct SerializationContext;
struct SerializationOptions;

/**
 * \class ElementSerializer
 * \brief Serializes a Element to/from a versioned QDataStream.
 *
 * \details Extracted from Element so persistence is a collaborator rather than a
 * cluster of methods on the element itself. A friend of Element, it reaches the
 * element's ports/orientation/appearance/context-directory state through that friendship.
 * The element keeps thin virtual save()/load() entry points that delegate here, so
 * subclasses (e.g. IC) can still extend the load via their override. All methods are
 * static and take the element explicitly; no serializer instance state is needed.
 */
class ElementSerializer
{
    // tr context for the load-error PANDACEPTION throws. lupdate extracts the plain
    // PANDACEPTION(...) calls below under the "GraphicElementSerializer" context (the
    // tr+=PANDACEPTION alias resolves via this class scope); PANDACEPTION_WITH_CONTEXT is NOT
    // extracted.
    Q_DECLARE_TR_FUNCTIONS(GraphicElementSerializer)

    // Test-only seam: several load-error throws (truncated-stream / implausible-count guards)
    // are private helpers one level below the public load() entry point. Precisely truncating a
    // full element stream at the exact byte offset each one needs is far more fragile than
    // calling the helper directly with a minimal hand-built stream.
    friend class TestElementSerializer;

public:
    /// Writes \a element to \a stream (current keyed-QMap format).
    static void save(const Element &element, QDataStream &stream, SerializationOptions options);

    /// Reads \a element from \a stream, dispatching on the format version in \a context.
    static void load(Element &element, QDataStream &stream, SerializationContext &context);

private:
    static void loadOldFormat(Element &element, QDataStream &stream, SerializationContext &context);
    static void loadNewFormat(Element &element, QDataStream &stream, SerializationContext &context);

    /// Rejects a non-finite (NaN / ±inf) element position read from a crafted file
    /// (fuzz-hardening) — see the .cpp for why this must abort load rather than proceed.
    static void validateFinitePos(const QPointF &pos);

    /// Rejects a non-finite (NaN / ±inf) element rotation read from a crafted file
    /// (fuzz-hardening) — see the .cpp for why this must abort load rather than proceed.
    static void validateFiniteAngle(qreal angle);

    /// Rejects a port count above the element type's maximum, which no save writes
    /// (fuzz-hardening) — see the .cpp for why this must abort load rather than proceed.
    static void validatePortCounts(const Element &element);

    static void loadPos(Element &element, QDataStream &stream);
    static void loadRotation(Element &element, QDataStream &stream, const QVersionNumber &version);
    static void loadLabel(Element &element, QDataStream &stream, const QVersionNumber &version);
    static void loadPortsSize(QDataStream &stream, const QVersionNumber &version);
    static void loadTrigger(Element &element, QDataStream &stream, const QVersionNumber &version);

    static void loadInputPorts(Element &element, QDataStream &stream, SerializationContext &context);
    static void loadInputPort(Element &element, QDataStream &stream, SerializationContext &context, int port);
    static void loadOutputPorts(Element &element, QDataStream &stream, SerializationContext &context);
    static void loadOutputPort(Element &element, QDataStream &stream, SerializationContext &context, int port);

    static void removeSurplusInputs(Element &element, quint64 inputSize_, SerializationContext &context);
    static void removeSurplusOutputs(Element &element, quint64 outputSize_, SerializationContext &context);

    static void loadPixmapAppearanceNames(Element &element, QDataStream &stream, SerializationContext &context);
    static void loadPixmapAppearanceName(Element &element, QDataStream &stream, int index, const SerializationContext &context);

    /// Reads a `QList<QMap<QString, QVariant>>` from \a stream, rejecting implausible counts up
    /// front (fuzz-hardening). A member (not a file-local helper) so its load-error throws extract
    /// under this class's tr context.
    static QList<QMap<QString, QVariant>> readPortList(QDataStream &stream, const char *label);

    /// Erases \a deletedPort's serial-ID entry from \a portMap during deserialization.
    static void removePortFromMap(Port *deletedPort, QHash<quint64, Port *> &portMap);
};
