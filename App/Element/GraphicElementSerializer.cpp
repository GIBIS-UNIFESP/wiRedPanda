// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/// \file
/// Serialization method implementations for GraphicElement (save/load).
/// Split out from GraphicElement.cpp to decouple serialization from the
/// rest of the element logic and avoid a transitive SerializationContext.h
/// include for the 48+ files that depend on GraphicElement.h.

#include <QDir>
#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Scene.h"

// ========== save / load ==========

void GraphicElement::save(QDataStream &stream) const
{

    qCDebug(four) << "Saving element. Type: " << objectName();

    QMap<QString, QVariant> map;
    map.insert("pos", pos());
    map.insert("rotation", rotation());
    map.insert("label", label());
    map.insert("minInputSize", m_minInputSize);
    map.insert("maxInputSize", m_maxInputSize);
    map.insert("minOutputSize", m_minOutputSize);
    map.insert("maxOutputSize", m_maxOutputSize);
    map.insert("trigger", m_trigger);
    stream << map;

    // -------------------------------------------

    QList<QMap<QString, QVariant>> inputMap;

    for (int i = 0; i < m_inputPorts.size(); ++i) {
        auto *port = m_inputPorts.at(i);
        QMap<QString, QVariant> tempMap;
        // Generate deterministic serial ID: (elementId << 16) | portIndex
        // Note: We calculate but don't modify port state (no side effects in save())
        quint64 serialId = (static_cast<quint64>(id()) << 16) | (static_cast<quint64>(i) & 0xFFFF);
        tempMap.insert("serialId", serialId);
        tempMap.insert("name", port->name());

        inputMap << tempMap;
    }

    stream << inputMap;

    // -------------------------------------------

    QList<QMap<QString, QVariant>> outputMap;

    for (int i = 0; i < m_outputPorts.size(); ++i) {
        auto *port = m_outputPorts.at(i);
        QMap<QString, QVariant> tempMap;
        // Generate deterministic serial ID: (elementId << 16) | portIndex
        // Note: We calculate but don't modify port state (no side effects in save())
        quint64 serialId = (static_cast<quint64>(id()) << 16) | (static_cast<quint64>(m_inputPorts.size() + i) & 0xFFFF);
        tempMap.insert("serialId", serialId);
        tempMap.insert("name", port->name());

        outputMap << tempMap;
    }

    stream << outputMap;

    // -------------------------------------------

    QList<QMap<QString, QVariant>> skinsMap;

    for (const auto &skinName : m_alternativeSkins) {
        QFileInfo fileInfo(skinName);
        QString skinName2 = skinName;

        // When a custom skin lives outside the project directory, copy it alongside
        // the .panda file so the project remains self-contained when moved or shared.
        const QString contextDir = Scene::resolveContextDir(this);
        if (!skinName.startsWith(":/") && !contextDir.isEmpty() && (fileInfo.absoluteDir() != QDir(contextDir))) {
            const QString newFile = contextDir + "/" + fileInfo.fileName();
            QFile::copy(skinName, newFile);
            // Store only the bare filename; the project dir is prepended on load.
            skinName2 = fileInfo.fileName();
        }

        // -------------------------------------------

        QMap<QString, QVariant> tempMap;
        tempMap.insert("skinName", skinName2);

        skinsMap << tempMap;
    }

    stream << skinsMap;

    // -------------------------------------------

    qCDebug(four) << "Finished saving element.";
}

void GraphicElement::load(QDataStream &stream, SerializationContext &context)
{
    qCDebug(four) << "Loading element. Type: " << objectName();

    m_contextDir = context.contextDir;

    // Files before 4.1 used a flat sequential binary format; 4.1+ use a keyed QMap
    // format that tolerates fields being added or reordered in future versions.
    (!VersionInfo::hasQMapFormat(context.version)) ? loadOldFormat(stream, context) : loadNewFormat(stream, context);

    qCDebug(four) << "Updating port positions.";
    updatePortsProperties();
    // Apply the deserialized angle after ports are positioned so any non-rotatable element
    // can apply its own rotatePorts() path correctly
    setRotation(m_angle);

    qCDebug(four) << "Finished loading element.";
}

// ========== Old format (pre-4.1) ==========

void GraphicElement::loadOldFormat(QDataStream &stream, SerializationContext &context)
{
    loadPos(stream);
    loadRotation(stream, context.version);
    /* <Version1.2> */
    loadLabel(stream, context.version);
    /* <\Version1.2> */
    /* <Version1.3> */
    loadPortsSize(stream, context.version);
    /* <\Version1.3> */
    /* <Version1.9> */
    loadTrigger(stream, context.version);
    /* <\Version4.01> */
    if (VersionInfo::hasUnusedPriority(context.version)) {
        quint64 unusedPriority; stream >> unusedPriority;
    }
    /* <\Version1.9> */
    loadInputPorts(stream, context);
    loadOutputPorts(stream, context);
    /* <\Version2.7> */
    loadPixmapSkinNames(stream, context);
}

// ========== New format (4.1+) ==========

void GraphicElement::loadNewFormat(QDataStream &stream, SerializationContext &context)
{
    QMap<QString, QVariant> map; stream >> map;

    // Check stream integrity after reading element properties map
    if (stream.status() != QDataStream::Ok) {
        throw PANDACEPTION("Stream error reading element properties at position %1",
                          stream.device()->pos());
    }

    if (map.contains("pos")) {
        setPos(map.value("pos").toPointF());
    }

    if (map.contains("rotation")) {
        m_angle = map.value("rotation").toReal();
    }

    if (map.contains("label")) {
        setLabel(map.value("label").toString());
    }

    // -------------------------------------------

    const quint64 minInputSize = map.value("minInputSize").toULongLong();
    const quint64 maxInputSize = map.value("maxInputSize").toULongLong();
    const quint64 minOutputSize = map.value("minOutputSize").toULongLong();
    const quint64 maxOutputSize = map.value("maxOutputSize").toULongLong();

    // Only update constraints when the element allows variable port counts OR when
    // the saved value falls within the current allowed range.  Prevents a fixed-port
    // element (min == max) from being silently corrupted by a stale saved value.
    if ((m_minInputSize != m_maxInputSize) || (m_minInputSize <= maxInputSize)) {
        m_minInputSize = minInputSize;
        m_maxInputSize = maxInputSize;
    }

    if ((m_minOutputSize != m_maxOutputSize) || (m_minOutputSize <= maxOutputSize)) {
        m_minOutputSize = minOutputSize;
        m_maxOutputSize = maxOutputSize;
    }

    // -------------------------------------------

    if (map.contains("trigger")) {
        setTrigger(map.value("trigger").toString());
    }

    // -------------------------------------------

    QList<QMap<QString, QVariant>> inputMap; stream >> inputMap;

    // Check stream integrity after reading input ports map
    if (stream.status() != QDataStream::Ok) {
        throw PANDACEPTION("Stream error reading input ports at position %1",
                          stream.device()->pos());
    }

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
            serialId = (static_cast<quint64>(context.nextLocalId) << 16) | (port & 0xFFFF);
        }

        if (port < m_inputPorts.size()) {
            m_inputPorts.value(port)->setSerialId(serialId);
            quint64 oldPtr = input.value("ptr").toULongLong();
            // Map old pointer IDs to new serial IDs for backwards compatibility
            if (oldPtr != 0) {
                context.oldPtrToSerialId[oldPtr] = serialId;
            }

            if (elementType() == ElementType::IC) {
                m_inputPorts.value(port)->setName(name);
            }
        } else {
            quint64 oldPtr = input.value("ptr").toULongLong();
            addPort(name, false);
            m_inputPorts.value(port)->setSerialId(serialId);
            // Map old pointer IDs to new serial IDs for backwards compatibility
            if (oldPtr != 0) {
                context.oldPtrToSerialId[oldPtr] = serialId;
            }
        }

        context.portMap[serialId] = m_inputPorts.value(port);

        ++port;
    }

    removeSurplusInputs(static_cast<quint64>(inputMap.size()), context);

    // -------------------------------------------

    QList<QMap<QString, QVariant>> outputMap; stream >> outputMap;

    // Check stream integrity after reading output ports map
    if (stream.status() != QDataStream::Ok) {
        throw PANDACEPTION("Stream error reading output ports at position %1",
                          stream.device()->pos());
    }

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
            serialId = (static_cast<quint64>(context.nextLocalId) << 16) | ((m_inputPorts.size() + port) & 0xFFFF);
        }

        if (port < m_outputPorts.size()) {
            m_outputPorts.value(port)->setSerialId(serialId);
            quint64 oldPtr = output.value("ptr").toULongLong();
            // Map old pointer IDs to new serial IDs for backwards compatibility
            if (oldPtr != 0) {
                context.oldPtrToSerialId[oldPtr] = serialId;
            }

            if (elementType() == ElementType::IC) {
                m_outputPorts.value(port)->setName(name);
            }
        } else {
            quint64 oldPtr = output.value("ptr").toULongLong();
            addPort(name, true);
            m_outputPorts.value(port)->setSerialId(serialId);
            // Map old pointer IDs to new serial IDs for backwards compatibility
            if (oldPtr != 0) {
                context.oldPtrToSerialId[oldPtr] = serialId;
            }
        }

        context.portMap[serialId] = m_outputPorts.value(port);

        ++port;
    }

    removeSurplusOutputs(static_cast<quint64>(outputMap.size()), context);

    // -------------------------------------------

    QList<QMap<QString, QVariant>> skinsMap; stream >> skinsMap;

    // Check stream integrity after reading skins map
    if (stream.status() != QDataStream::Ok) {
        throw PANDACEPTION("Stream error reading skins at position %1",
                          stream.device()->pos());
    }

    int skin = 0;

    for (const auto &skinName : std::as_const(skinsMap)) {
        const QString name = skinName.value("skinName").toString();

        if (!name.startsWith(":/")) {
                m_alternativeSkins[skin] = name;
        }

        ++skin;
    }

    // If all alternative skin slots still match the defaults, the user never
    // applied a custom skin — record that so the "Reset to default" action works.
    m_usingDefaultSkin = std::equal(
        m_defaultSkins.begin(), m_defaultSkins.end(),
        m_alternativeSkins.begin(), m_alternativeSkins.end()
        );

    refresh();

    // Advance the per-element counter so that the next element's fallback serialIds
    // use a different base (only relevant for V4.1.9–V4.3 files lacking serialId keys).
    ++context.nextLocalId;
}

// ========== Property loading helpers (old format) ==========

void GraphicElement::loadPos(QDataStream &stream)
{
    QPointF pos; stream >> pos;
    setPos(pos);
}

void GraphicElement::loadRotation(QDataStream &stream, const QVersionNumber &version)
{
    qreal angle; stream >> angle;
    m_angle = angle;

    // In versions before 4.1 the coordinate system for inputs and outputs was
    // rotated 90° relative to the current convention.  Apply a compensating offset
    // so old files render correctly without migrating every saved angle.
    if (!VersionInfo::hasQMapFormat(version)) {
        if ((m_elementGroup == ElementGroup::Input) || (m_elementGroup == ElementGroup::StaticInput)) {
            m_angle += 90;
        }

        if ((m_elementGroup == ElementGroup::Output) || (m_elementGroup == ElementGroup::IC) || (m_elementGroup == ElementGroup::Gate)) {
            // Displays and Node never had the old convention applied, so skip them.
            if ((m_elementType == ElementType::Display7) || (m_elementType == ElementType::Display14) || (m_elementType == ElementType::Display16) || (m_elementType == ElementType::Node)) {
                return;
            }

            m_angle -= 90;
        }
    }
}

void GraphicElement::loadLabel(QDataStream &stream, const QVersionNumber &version)
{
    if (VersionInfo::hasLabels(version)) {
        QString labelText; stream >> labelText;
        setLabel(labelText);
    }
}

void GraphicElement::loadPortsSize(QDataStream &stream, const QVersionNumber &version)
{
    if (VersionInfo::hasPortSizes(version)) {
        quint64 minInputSize;  stream >> minInputSize;
        quint64 maxInputSize;  stream >> maxInputSize;
        quint64 minOutputSize; stream >> minOutputSize;
        quint64 maxOutputSize; stream >> maxOutputSize;

        // Only override the compiled-in constraints when the element has a variable port count
        // (min != max) or the saved count is within the allowed range.  This prevents a fixed-port
        // element from being corrupted by a stale value that was written before its constraints
        // were tightened.
        if ((m_minInputSize != m_maxInputSize) || (m_minInputSize <= maxInputSize)) {
            m_minInputSize = minInputSize;
            m_maxInputSize = maxInputSize;
        }

        if ((m_minOutputSize != m_maxOutputSize) || (m_minOutputSize <= maxOutputSize)) {
            m_minOutputSize = minOutputSize;
            m_maxOutputSize = maxOutputSize;
        }
    }
}

void GraphicElement::loadTrigger(QDataStream &stream, const QVersionNumber &version)
{
    if (VersionInfo::hasTrigger(version)) {
        QKeySequence trigger; stream >> trigger;
        setTrigger(trigger);
    }
}

// ========== Port loading (old format) ==========

void GraphicElement::loadInputPorts(QDataStream &stream, SerializationContext &context)
{
    qCDebug(four) << "Loading input ports.";
    quint64 inputSize; stream >> inputSize;

    for (size_t port = 0; port < inputSize; ++port) {
        loadInputPort(stream, context, static_cast<int>(port));
    }

    removeSurplusInputs(inputSize, context);
}

void GraphicElement::loadInputPort(QDataStream &stream, SerializationContext &context, const int port)
{
    quint64 ptr;  stream >> ptr;
    QString name; stream >> name;
    int flags;    stream >> flags;

    if (port < m_inputPorts.size()) {
        if (canSetPortNames()) {
            setInputPortName(port, name);
        }
    } else {
        addPort(name, false);
    }

    context.portMap[ptr] = m_inputPorts.value(port);
}

void GraphicElement::loadOutputPorts(QDataStream &stream, SerializationContext &context)
{
    qCDebug(four) << "Loading output ports.";
    quint64 outputSize; stream >> outputSize;

    for (size_t port = 0; port < outputSize; ++port) {
        loadOutputPort(stream, context, static_cast<int>(port));
    }

    removeSurplusOutputs(outputSize, context);
}

void GraphicElement::loadOutputPort(QDataStream &stream, SerializationContext &context, const int port)
{
    quint64 ptr;  stream >> ptr;
    QString name; stream >> name;
    int flags;    stream >> flags;

    if (port < m_outputPorts.size()) {
        if (canSetPortNames()) {
            setOutputPortName(port, name);
        }
    } else {
        addPort(name, true);
    }

    context.portMap[ptr] = m_outputPorts.value(port);
}

// ========== Surplus port removal ==========

void GraphicElement::removeSurplusInputs(const quint64 inputSize_, SerializationContext &context)
{
    // The element may have been constructed with more ports than are stored in the file
    // (e.g. default constructor creates minInputSize ports).  Trim the excess from the end,
    // but never go below minInputSize to avoid leaving an unusable element.
    while ((inputSize() > static_cast<int>(inputSize_)) && (inputSize_ >= m_minInputSize)) {
        auto *deletedPort = m_inputPorts.constLast();
        removePortFromMap(deletedPort, context.portMap);
        delete deletedPort;
        m_inputPorts.removeLast();
    }
}

void GraphicElement::removeSurplusOutputs(const quint64 outputSize_, SerializationContext &context)
{
    // Same trimming logic as removeSurplusInputs, applied to output ports
    while ((outputSize() > static_cast<int>(outputSize_)) && (outputSize_ >= m_minOutputSize)) {
        auto *deletedPort = m_outputPorts.constLast();
        removePortFromMap(deletedPort, context.portMap);
        delete deletedPort;
        m_outputPorts.removeLast();
    }
}

void GraphicElement::removePortFromMap(QNEPort *deletedPort, QMap<quint64, QNEPort *> &portMap)
{
    for (auto it = portMap.begin(); it != portMap.end();) {
        if (it.value() == deletedPort) {
            it = portMap.erase(it);
        } else {
            ++it;
        }
    }
}

// ========== Skin loading ==========

void GraphicElement::loadPixmapSkinNames(QDataStream &stream, SerializationContext &context)
{

    if (VersionInfo::hasSkinNames(context.version)) {
        qCDebug(four) << tr("Loading pixmap skin names.");
        quint64 outputSize; stream >> outputSize;

        if (m_elementType == ElementType::IC) {
            // IC paints itself procedurally and has no skin slots.
            // Drain the saved skin names from old file formats to keep the stream position valid.
            for (quint64 i = 0; i < outputSize; ++i) {
                QString discarded; stream >> discarded;
            }
            return;
        }

        for (size_t skin = 0; skin < outputSize; ++skin) {
            loadPixmapSkinName(stream, static_cast<int>(skin), context.contextDir);
        }

        m_usingDefaultSkin = std::equal(
            m_defaultSkins.begin(), m_defaultSkins.end(),
            m_alternativeSkins.begin(), m_alternativeSkins.end()
            );

        refresh();
    }
}

void GraphicElement::loadPixmapSkinName(QDataStream &stream, const int skin, const QString &contextDir)
{
    QString name; stream >> name;

    if (skin >= m_alternativeSkins.size()) {
        throw PANDACEPTION("Skin index %1 out of range (size=%2) for skin name \"%3\" — stream may be corrupt",
                           QString::number(skin), QString::number(m_alternativeSkins.size()), name);
    }

    // Only override the alternative skin if it is a filesystem path; resource
    // paths (":/...") are always available and should not be replaced by the
    // potentially missing saved path from an older project file.
    if (!name.startsWith(":/")) {
        if (!contextDir.isEmpty() && QDir::isRelativePath(name)) {
            m_alternativeSkins[skin] = contextDir + "/" + name;
        } else {
            m_alternativeSkins[skin] = name;
        }
    }
}

// ========== Stream operator ==========

QDataStream &operator<<(QDataStream &stream, const GraphicElement *item)
{
    // Type tags are now written by Serialization::serialize() for symmetry
    // This is now only called from serialize(), so type is already written
    qCDebug(four) << "Writing element.";
    const auto *elm = qgraphicsitem_cast<const GraphicElement *>(item);
    elm->save(stream);
    return stream;
}

