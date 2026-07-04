// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/// \file
/// GraphicElementSerializer: save/load implementation for GraphicElement.
/// Split out from GraphicElement so persistence is a collaborator (a friend class)
/// rather than a cluster of methods on the element, and to avoid a transitive
/// SerializationContext.h include for the 48+ files that depend on GraphicElement.h.

#include "App/Element/GraphicElementSerializer.h"

#include <cmath>

#include <QDir>
#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Wiring/Port.h"

namespace {

/// Per-element port count ceiling enforced when deserializing.  QDataStream's
/// container operator>> reads a 32-bit count and immediately reserve()s that
/// many entries — a malformed/fuzzed stream can specify ~4 billion and OOM
/// the process before any element-level validation runs.  No real circuit
/// has thousands of ports on a single element; 1024 is comfortably above
/// the largest existing IC (8x8 register file, ~80 ports) and well below
/// any threat of memory exhaustion.  Surfaced by libFuzzer.
constexpr quint32 kMaxPortsPerElement = 1024;

} // namespace

// readPortList() and removePortFromMap() are static members (not file-local helpers) so their
// load-error PANDACEPTION throws extract under the GraphicElementSerializer tr context.

QList<QMap<QString, QVariant>> GraphicElementSerializer::readPortList(QDataStream &stream, const char *label)
{
    const QString labelStr = QString::fromUtf8(label);
    quint32 count;
    stream >> count;
    if (stream.status() != QDataStream::Ok) {
        throw PANDACEPTION("Stream error reading %1 count at position %2",
                           labelStr, QString::number(stream.device()->pos()));
    }
    if (count > kMaxPortsPerElement) {
        throw PANDACEPTION("Refusing to read %1 with implausible count %2 (max %3)",
                           labelStr,
                           QString::number(count),
                           QString::number(kMaxPortsPerElement));
    }
    QList<QMap<QString, QVariant>> result;
    result.reserve(static_cast<int>(count));
    for (quint32 i = 0; i < count; ++i) {
        QMap<QString, QVariant> entry = Serialization::readBoundedMetadata(stream);
        if (stream.status() != QDataStream::Ok) {
            throw PANDACEPTION("Stream error reading %1 entry %2 at position %3",
                               labelStr,
                               QString::number(i),
                               QString::number(stream.device()->pos()));
        }
        result.append(std::move(entry));
    }
    return result;
}

void GraphicElementSerializer::removePortFromMap(Port *deletedPort, QHash<quint64, Port *> &portMap)
{
    for (auto it = portMap.begin(); it != portMap.end();) {
        if (it.value() == deletedPort) {
            it = portMap.erase(it);
        } else {
            ++it;
        }
    }
}

// ========== GraphicElement entry points (delegate to the serializer) ==========

void GraphicElement::save(QDataStream &stream) const
{
    GraphicElementSerializer::save(*this, stream);
}

void GraphicElement::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElementSerializer::load(*this, stream, context);
}

// ========== save / load ==========

void GraphicElementSerializer::save(const GraphicElement &element, QDataStream &stream)
{
    qCDebug(four) << "Saving element. Type: " << element.objectName();

    QMap<QString, QVariant> map;
    map.insert("pos", element.pos());
    map.insert("rotation", element.rotation());
    map.insert("label", element.label());
    // min/max port sizes are class metadata, not per-instance state.
    // No longer written; old files that contain these keys are harmlessly
    // ignored on load (the QMap is read as a whole, unused keys are skipped).
    map.insert("trigger", element.trigger());
    if (element.isFlippedX()) { map.insert("flippedX", true); }
    if (element.isFlippedY()) { map.insert("flippedY", true); }
    stream << map;

    // -------------------------------------------

    QList<QMap<QString, QVariant>> inputMap;

    for (int i = 0; i < element.m_ports.inputs().size(); ++i) {
        auto *port = element.m_ports.inputs().at(i);
        QMap<QString, QVariant> tempMap;
        // Note: We calculate but don't modify port state (no side effects in save())
        quint64 serialId = Port::makeSerialId(static_cast<quint64>(element.id()), port->globalIndex());
        tempMap.insert("serialId", serialId);
        tempMap.insert("name", port->name());

        inputMap << tempMap;
    }

    stream << inputMap;

    // -------------------------------------------

    QList<QMap<QString, QVariant>> outputMap;

    for (int i = 0; i < element.m_ports.outputs().size(); ++i) {
        auto *port = element.m_ports.outputs().at(i);
        QMap<QString, QVariant> tempMap;
        // Note: We calculate but don't modify port state (no side effects in save())
        quint64 serialId = Port::makeSerialId(static_cast<quint64>(element.id()), port->globalIndex());
        tempMap.insert("serialId", serialId);
        tempMap.insert("name", port->name());

        outputMap << tempMap;
    }

    stream << outputMap;

    // -------------------------------------------

    QList<QMap<QString, QVariant>> appearancesMap;

    for (const auto &appearance : element.m_appearance.alternativeAppearances()) {
        QMap<QString, QVariant> tempMap;
        tempMap.insert("skinName", appearance.startsWith(":/") ? appearance : QFileInfo(appearance).fileName());
        appearancesMap << tempMap;
    }

    stream << appearancesMap;

    // -------------------------------------------

    qCDebug(four) << "Finished saving element.";
}

void GraphicElementSerializer::load(GraphicElement &element, QDataStream &stream, SerializationContext &context)
{
    qCDebug(four) << "Loading element. Type: " << element.objectName();

    element.m_contextDir = context.contextDir;

    // Files before 4.1 used a flat sequential binary format; 4.1+ use a keyed QMap
    // format that tolerates fields being added or reordered in future versions.
    (!VersionInfo::hasQMapFormat(context.version)) ? loadOldFormat(element, stream, context) : loadNewFormat(element, stream, context);

    qCDebug(four) << "Updating port positions.";
    element.updatePortsProperties();
    // Apply the deserialized angle after ports are positioned so any non-rotatable element
    // can apply its own rotatePorts() path correctly
    element.m_orientation.applyLoadedOrientation();

    qCDebug(four) << "Finished loading element.";
}

// ========== Old format (pre-4.1) ==========

void GraphicElementSerializer::loadOldFormat(GraphicElement &element, QDataStream &stream, SerializationContext &context)
{
    loadPos(element, stream);
    loadRotation(element, stream, context.version);
    /* <Version1.2> */
    loadLabel(element, stream, context.version);
    /* <\Version1.2> */
    /* <Version1.3> */
    loadPortsSize(stream, context.version);
    /* <\Version1.3> */
    /* <Version1.9> */
    loadTrigger(element, stream, context.version);
    /* <\Version4.01> */
    if (VersionInfo::hasUnusedPriority(context.version)) {
        quint64 unusedPriority; stream >> unusedPriority;
    }
    /* <\Version1.9> */
    loadInputPorts(element, stream, context);
    loadOutputPorts(element, stream, context);
    /* <\Version2.7> */
    loadPixmapAppearanceNames(element, stream, context);
}

// ========== New format (4.1+) ==========

void GraphicElementSerializer::loadNewFormat(GraphicElement &element, QDataStream &stream, SerializationContext &context)
{
    QMap<QString, QVariant> map = Serialization::readBoundedMetadata(stream);

    // Check stream integrity after reading element properties map
    if (stream.status() != QDataStream::Ok) {
        throw PANDACEPTION("Stream error reading element properties at position %1",
                           QString::number(stream.device()->pos()));
    }

    if (map.contains("pos")) {
        const QPointF pos = map.value("pos").toPointF();
        validateFinitePos(pos);
        element.setPos(pos);
    }

    if (map.contains("rotation")) {
        const qreal angle = map.value("rotation").toReal();
        validateFiniteAngle(angle);
        element.m_orientation.setAngleRaw(angle);
    }

    if (map.contains("label")) {
        element.setLabel(map.value("label").toString());
    }

    // min/max port sizes are class metadata — ignore any stale saved values.
    // The actual port count is restored from the serialized port lists below.

    // -------------------------------------------

    if (map.contains("trigger")) {
        element.setTrigger(map.value("trigger").toString());
    }

    element.m_orientation.setFlippedXRaw(map.value("flippedX", false).toBool());
    element.m_orientation.setFlippedYRaw(map.value("flippedY", false).toBool());

    // -------------------------------------------

    QList<QMap<QString, QVariant>> inputMap = readPortList(stream, "input ports");

    int port = 0;

    for (const auto &input : std::as_const(inputMap)) {
        const QString name = input.value("name").toString();

        // Support both new format (serialId) and old format (ptr)
        quint64 serialId;
        if (input.contains("serialId")) {
            // New format: direct serial ID
            serialId = input.value("serialId").toULongLong();
        } else {
            // Old format fallback: use context.nextLocalId as element basis so that
            // serialIds remain unique even when element IDs are -1 (unassigned).
            serialId = Port::makeSerialId(static_cast<quint64>(context.nextLocalId), port);
        }

        if (port < element.m_ports.inputs().size()) {
            quint64 oldPtr = input.value("ptr").toULongLong();
            // Map old pointer IDs to new serial IDs for backwards compatibility
            if (oldPtr != 0) {
                context.oldPtrToSerialId[oldPtr] = serialId;
            }

            if (element.elementType() == ElementType::IC) {
                element.m_ports.inputs().value(port)->setName(name);
            }
        } else {
            quint64 oldPtr = input.value("ptr").toULongLong();
            element.m_ports.addPort(name, false);
            // Map old pointer IDs to new serial IDs for backwards compatibility
            if (oldPtr != 0) {
                context.oldPtrToSerialId[oldPtr] = serialId;
            }
        }

        context.portMap[serialId] = element.m_ports.inputs().value(port);

        ++port;
    }

    removeSurplusInputs(element, static_cast<quint64>(inputMap.size()), context);

    // -------------------------------------------

    QList<QMap<QString, QVariant>> outputMap = readPortList(stream, "output ports");

    port = 0;

    for (const auto &output : std::as_const(outputMap)) {
        const QString name = output.value("name").toString();

        // Support both new format (serialId) and old format (ptr)
        quint64 serialId;
        if (output.contains("serialId")) {
            // New format: direct serial ID
            serialId = output.value("serialId").toULongLong();
        } else {
            // Old format fallback: use context.nextLocalId as element basis so that
            // serialIds remain unique even when element IDs are -1 (unassigned).
            serialId = Port::makeSerialId(static_cast<quint64>(context.nextLocalId), static_cast<int>(element.m_ports.inputs().size()) + port);
        }

        if (port < element.m_ports.outputs().size()) {
            quint64 oldPtr = output.value("ptr").toULongLong();
            // Map old pointer IDs to new serial IDs for backwards compatibility
            if (oldPtr != 0) {
                context.oldPtrToSerialId[oldPtr] = serialId;
            }

            if (element.elementType() == ElementType::IC) {
                element.m_ports.outputs().value(port)->setName(name);
            }
        } else {
            quint64 oldPtr = output.value("ptr").toULongLong();
            element.m_ports.addPort(name, true);
            // Map old pointer IDs to new serial IDs for backwards compatibility
            if (oldPtr != 0) {
                context.oldPtrToSerialId[oldPtr] = serialId;
            }
        }

        context.portMap[serialId] = element.m_ports.outputs().value(port);

        ++port;
    }

    removeSurplusOutputs(element, static_cast<quint64>(outputMap.size()), context);

    // -------------------------------------------

    QList<QMap<QString, QVariant>> appearancesMap = readPortList(stream, "appearances");

    if (stream.status() != QDataStream::Ok) {
        throw PANDACEPTION("Stream error reading appearances at position %1",
                           QString::number(stream.device()->pos()));
    }

    int index = 0;

    for (const auto &entry : std::as_const(appearancesMap)) {
        if (index >= element.m_appearance.alternativeAppearances().size()) {
            throw PANDACEPTION("Appearance index %1 out of range (size=%2) — stream may be corrupt",
                               QString::number(index),
                               QString::number(element.m_appearance.alternativeAppearances().size()));
        }

        const QString name = entry.value("skinName").toString();

        if (!name.startsWith(":/")) {
            element.m_appearance.setAlternativeAppearanceAt(index, name);
        }

        ++index;
    }

    // If all alternative appearance slots still match the defaults, the user never
    // applied a custom appearance — record that so the "Reset to default" action works.
    element.m_appearance.recomputeUsingDefault();

    element.refresh();

    // Advance the per-element counter so that the next element's fallback serialIds
    // use a different base (only relevant for V4.1.9–V4.3 files lacking serialId keys).
    ++context.nextLocalId;
}

// ========== Property loading helpers (old format) ==========

// validateFinitePos() is a static member (not a file-local helper) so its load-error
// PANDACEPTION throw extracts under the GraphicElementSerializer tr context.

/// Rejects a non-finite (NaN / ±inf) element position read from a crafted file.
/// A non-finite position makes the item's scene transform — and any bounding rect
/// computed from it — non-finite, tripping the qSaturateRound assertion in
/// QSizeF::toSize() and aborting the process.  Surfaced by libFuzzer (fuzz_ic_file).
void GraphicElementSerializer::validateFinitePos(const QPointF &pos)
{
    if (!std::isfinite(pos.x()) || !std::isfinite(pos.y())) {
        throw PANDACEPTION("Non-finite element position — stream may be corrupt");
    }
}

void GraphicElementSerializer::loadPos(GraphicElement &element, QDataStream &stream)
{
    QPointF pos; stream >> pos;
    validateFinitePos(pos);
    element.setPos(pos);
}

/// Rejects a non-finite (NaN / ±inf) element rotation read from a crafted file.
/// setAngleRaw(NaN) makes the item's scene transform NaN, so its bounding rect
/// is NaN and trips the same qSaturateRound assertion as a non-finite position
/// does.  Surfaced by libFuzzer (fuzz_ic_file).
void GraphicElementSerializer::validateFiniteAngle(const qreal angle)
{
    if (!std::isfinite(angle)) {
        throw PANDACEPTION("Non-finite element rotation — stream may be corrupt");
    }
}

void GraphicElementSerializer::loadRotation(GraphicElement &element, QDataStream &stream, const QVersionNumber &version)
{
    qreal angle; stream >> angle;
    validateFiniteAngle(angle);
    element.m_orientation.setAngleRaw(angle);

    // In versions before 4.1 the coordinate system for inputs and outputs was
    // rotated 90° relative to the current convention.  Apply a compensating offset
    // so old files render correctly without migrating every saved angle.
    if (!VersionInfo::hasQMapFormat(version)) {
        if ((element.elementGroup() == ElementGroup::Input) || (element.elementGroup() == ElementGroup::StaticInput)) {
            element.m_orientation.setAngleRaw(element.m_orientation.angle() + 90);
        }

        if ((element.elementGroup() == ElementGroup::Output) || (element.elementGroup() == ElementGroup::IC) || (element.elementGroup() == ElementGroup::Gate)) {
            // Displays and Node never had the old convention applied, so skip them.
            if ((element.elementType() == ElementType::Display7) || (element.elementType() == ElementType::Display14) || (element.elementType() == ElementType::Display16) || (element.elementType() == ElementType::Node)) {
                return;
            }

            element.m_orientation.setAngleRaw(element.m_orientation.angle() - 90);
        }
    }
}

void GraphicElementSerializer::loadLabel(GraphicElement &element, QDataStream &stream, const QVersionNumber &version)
{
    if (VersionInfo::hasLabels(version)) {
        element.setLabel(Serialization::readBoundedString(stream));
    }
}

void GraphicElementSerializer::loadPortsSize(QDataStream &stream, const QVersionNumber &version)
{
    if (VersionInfo::hasPortSizes(version)) {
        // Drain the 4 saved values to advance the stream position.
        // min/max port sizes are class metadata — the compiled-in values are authoritative.
        quint64 unused; stream >> unused; stream >> unused; stream >> unused; stream >> unused;
    }
}

void GraphicElementSerializer::loadTrigger(GraphicElement &element, QDataStream &stream, const QVersionNumber &version)
{
    if (VersionInfo::hasTrigger(version)) {
        QKeySequence trigger; stream >> trigger;
        element.setTrigger(trigger);
    }
}

// ========== Port loading (old format) ==========

void GraphicElementSerializer::loadInputPorts(GraphicElement &element, QDataStream &stream, SerializationContext &context)
{
    qCDebug(four) << "Loading input ports.";
    quint64 inputSize; stream >> inputSize;
    if (inputSize > kMaxPortsPerElement) {
        throw PANDACEPTION("Refusing old-format input port list with implausible count %1 (max %2)",
                           QString::number(inputSize),
                           QString::number(kMaxPortsPerElement));
    }

    for (size_t port = 0; port < inputSize; ++port) {
        loadInputPort(element, stream, context, static_cast<int>(port));
    }

    removeSurplusInputs(element, inputSize, context);
}

void GraphicElementSerializer::loadInputPort(GraphicElement &element, QDataStream &stream, SerializationContext &context, const int port)
{
    quint64 ptr;  stream >> ptr;
    const QString name = Serialization::readBoundedString(stream);
    int flags;    stream >> flags;

    if (port < element.m_ports.inputs().size()) {
        if (element.elementType() == ElementType::IC) {
            element.m_ports.inputs().value(port)->setName(name);
        }
    } else {
        element.m_ports.addPort(name, false);
    }

    context.portMap[ptr] = element.m_ports.inputs().value(port);
}

void GraphicElementSerializer::loadOutputPorts(GraphicElement &element, QDataStream &stream, SerializationContext &context)
{
    qCDebug(four) << "Loading output ports.";
    quint64 outputSize; stream >> outputSize;
    if (outputSize > kMaxPortsPerElement) {
        throw PANDACEPTION("Refusing old-format output port list with implausible count %1 (max %2)",
                           QString::number(outputSize),
                           QString::number(kMaxPortsPerElement));
    }

    for (size_t port = 0; port < outputSize; ++port) {
        loadOutputPort(element, stream, context, static_cast<int>(port));
    }

    removeSurplusOutputs(element, outputSize, context);
}

void GraphicElementSerializer::loadOutputPort(GraphicElement &element, QDataStream &stream, SerializationContext &context, const int port)
{
    quint64 ptr;  stream >> ptr;
    const QString name = Serialization::readBoundedString(stream);
    int flags;    stream >> flags;

    if (port < element.m_ports.outputs().size()) {
        if (element.elementType() == ElementType::IC) {
            element.m_ports.outputs().value(port)->setName(name);
        }
    } else {
        element.m_ports.addPort(name, true);
    }

    context.portMap[ptr] = element.m_ports.outputs().value(port);
}

// ========== Surplus port removal ==========

void GraphicElementSerializer::removeSurplusInputs(GraphicElement &element, const quint64 inputSize_, SerializationContext &context)
{
    // The element may have been constructed with more ports than are stored in the file
    // (e.g. default constructor creates minInputSize ports).  Trim the excess from the end,
    // but never go below minInputSize to avoid leaving an unusable element.
    while ((element.inputSize() > static_cast<int>(inputSize_)) && (inputSize_ >= static_cast<quint64>(element.minInputSize()))) {
        // Remove from the vector before deleting: a reentrant access during deserialization
        // must never observe a dangling pointer still present in m_ports.
        auto *deletedPort = element.m_ports.takeLastInput();
        removePortFromMap(deletedPort, context.portMap);
        delete deletedPort;
    }
}

void GraphicElementSerializer::removeSurplusOutputs(GraphicElement &element, const quint64 outputSize_, SerializationContext &context)
{
    // Same trimming logic as removeSurplusInputs, applied to output ports
    while ((element.outputSize() > static_cast<int>(outputSize_)) && (outputSize_ >= static_cast<quint64>(element.minOutputSize()))) {
        // Remove from the vector before deleting: a reentrant access during deserialization
        // must never observe a dangling pointer still present in m_ports.
        auto *deletedPort = element.m_ports.takeLastOutput();
        removePortFromMap(deletedPort, context.portMap);
        delete deletedPort;
    }
}

// ========== Appearance loading ==========

void GraphicElementSerializer::loadPixmapAppearanceNames(GraphicElement &element, QDataStream &stream, SerializationContext &context)
{
    if (VersionInfo::hasAppearanceNames(context.version)) {
        qCDebug(four) << "Loading pixmap appearance names.";
        quint64 outputSize; stream >> outputSize;
        if (outputSize > kMaxPortsPerElement) {
            throw PANDACEPTION("Refusing old-format appearance list with implausible count %1 (max %2)",
                               QString::number(outputSize),
                               QString::number(kMaxPortsPerElement));
        }

        if (element.elementType() == ElementType::IC) {
            // IC paints itself procedurally and has no appearance slots.
            // Drain the saved appearance names from old file formats to keep the stream position valid.
            for (quint64 i = 0; i < outputSize; ++i) {
                Serialization::readBoundedString(stream); // drain appearance name
            }
            return;
        }

        for (size_t i = 0; i < outputSize; ++i) {
            loadPixmapAppearanceName(element, stream, static_cast<int>(i), context.contextDir);
        }

        element.m_appearance.recomputeUsingDefault();

        element.refresh();
    }
}

void GraphicElementSerializer::loadPixmapAppearanceName(GraphicElement &element, QDataStream &stream, const int index, const QString &contextDir)
{
    const QString name = Serialization::readBoundedString(stream);

    if (index >= element.m_appearance.alternativeAppearances().size()) {
        throw PANDACEPTION("Appearance index %1 out of range (size=%2) for appearance name \"%3\" — stream may be corrupt",
                           QString::number(index), QString::number(element.m_appearance.alternativeAppearances().size()), name);
    }

    // Only override the alternative appearance if it is a filesystem path; resource
    // paths (":/...") are always available and should not be replaced by the
    // potentially missing saved path from an older project file.
    if (!name.startsWith(":/")) {
        if (!contextDir.isEmpty() && QDir::isRelativePath(name)) {
            element.m_appearance.setAlternativeAppearanceAt(index, contextDir + "/" + name);
        } else {
            element.m_appearance.setAlternativeAppearanceAt(index, name);
        }
    }
}
