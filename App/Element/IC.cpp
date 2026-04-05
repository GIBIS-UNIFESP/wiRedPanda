// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/IC.h"

#include <QDir>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QSaveFile>
#include <QScopeGuard>
#include <QSet>
#include <QStyleOptionGraphicsItem>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/ICDefinition.h"
#include "App/Element/ICRegistry.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"

namespace {

bool comparePorts(QNEPort *port1, QNEPort *port2)
{
    auto *elem1 = port1->graphicElement();
    auto *elem2 = port2->graphicElement();
    if (!elem1 || !elem2) {
        return false;
    }

    // Primary sort: top-to-bottom by parent element Y, then left-to-right by X.
    // This gives an intuitive pin order that matches the visual layout in the sub-circuit.
    QPointF p1 = elem1->pos();
    QPointF p2 = elem2->pos();

    if (p1 != p2) {
        return (p1.y() < p2.y()) || (qFuzzyCompare(p1.y(), p2.y()) && (p1.x() < p2.x()));
    }

    // Secondary sort: when two ports share the same parent element position, sort by
    // the port's own local coordinates (left-to-right, top-to-bottom)
    p1 = port1->pos();
    p2 = port2->pos();

    return (p1.x() < p2.x()) || (qFuzzyCompare(p1.x(), p2.x()) && (p1.y() < p2.y()));
}

void sortPorts(QVector<QNEPort *> &ports)
{
    std::stable_sort(ports.begin(), ports.end(), comparePorts);
}

void buildPortLabels(const QVector<QNEPort *> &ports, QVector<QString> &labels)
{
    for (int i = 0; i < ports.size(); ++i) {
        auto *port = ports.at(i);
        auto *elm = port->graphicElement();
        QString lb = elm->label();

        if (!port->name().isEmpty()) {
            lb += " " + port->name();
        }

        // Append generic properties (e.g. clock frequency) in brackets so the IC pin tooltip
        // carries enough context for the user to identify the signal without opening the sub-circuit
        if (!elm->genericProperties().isEmpty()) {
            lb += " [" + elm->genericProperties() + "]";
        }

        labels[i] = lb;
    }
}

} // anonymous namespace

template<>
struct ElementInfo<IC> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::IC,
        .group = ElementGroup::IC,
        .hasLabel = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QString(); };
        meta.titleText = QT_TRANSLATE_NOOP("IC", "INTEGRATED CIRCUIT");
        meta.translatedName = QT_TRANSLATE_NOOP("IC", "IC");
        meta.trContext = "IC";
        meta.defaultAppearances = QStringList();
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new IC(); });
        return true;
    }();
};

IC::IC(QGraphicsItem *parent)
    : GraphicElement(ElementType::IC, parent)
{
    // ICs display their label vertically alongside the chip body, matching the physical
    // convention of reading IC labels along the side of a physical DIP package
    m_label->setRotation(90);
}

IC::~IC()
{
    qDeleteAll(m_internalElements);
}

QStringList IC::externalFiles() const
{
    QStringList result;
    if (!isEmbedded() && !m_file.isEmpty()) {
        result.append(m_file);
    }
    return result;
}

void IC::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;

    if (isEmbedded()) {
        map.insert("name", m_blobName);
    } else if (!m_file.isEmpty()) {
        map.insert("name", QFileInfo(m_file).fileName());
    }

    stream << map;
}

void IC::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElement::load(stream, context);

    // Save the portMap keys for each port that GraphicElement::load() just
    // registered.  Loading the sub-circuit below destroys and recreates all
    // ports, so we must re-register the new ports under the original keys
    // to keep the outer portMap valid for connection deserialization.
    // Old format uses ptr values as keys; new format uses serialIds — we
    // build a reverse map to find each port's key in O(1) instead of scanning.
    QHash<QNEPort *, quint64> reversePortMap;
    reversePortMap.reserve(context.portMap.size());
    for (auto it = context.portMap.constBegin(); it != context.portMap.constEnd(); ++it) {
        reversePortMap[it.value()] = it.key();
    }

    QVector<quint64> savedInputKeys, savedOutputKeys;
    for (auto *port : std::as_const(m_inputPorts)) {
        if (auto it = reversePortMap.constFind(port); it != reversePortMap.constEnd()) {
            savedInputKeys.append(it.value());
        }
    }
    for (auto *port : std::as_const(m_outputPorts)) {
        if (auto it = reversePortMap.constFind(port); it != reversePortMap.constEnd()) {
            savedOutputKeys.append(it.value());
        }
    }

    // Old format (V_1_2 to V_4_1): IC file path was written as a plain QString
    if ((VersionInfo::hasLabels(context.version)) && (!VersionInfo::hasQMapFormat(context.version))) {
        stream >> m_file;

        // Old files may have stored absolute paths from the original machine; keep only the
        // filename so we can resolve it relative to the current context directory.
        // Normalize backslashes first — QFileInfo::fileName() doesn't treat '\' as a
        // separator on Unix, so a Windows path like "C:\...\sub.panda" would be kept as-is.
        m_file = QFileInfo(QString(m_file).replace('\\', '/')).fileName();

        loadFile(m_file, context.contextDir);
    }

    // New format (V_4_1+): IC data stored in a QMap for extensibility
    if (VersionInfo::hasQMapFormat(context.version)) {
        QMap<QString, QVariant> map; stream >> map;

        const QString name = map.contains("name")     ? map.value("name").toString()
                           : map.contains("fileName") ? map.value("fileName").toString()
                           : map.contains("blobName") ? map.value("blobName").toString()
                                                      : QString();

        if (name.isEmpty()) {
            throw PANDACEPTION("IC load: no IC name present in serialized data");
        }

        // Resolve: try exact name first (embedded blob names may contain dots),
        // then baseName (file-backed "chain_c.panda" with registry key "chain_c")
        const QString baseName = QFileInfo(name).baseName();
        const bool exactMatch = context.blobRegistry && context.blobRegistry->contains(name);
        const bool baseMatch = !exactMatch && context.blobRegistry && context.blobRegistry->contains(baseName);

        if (exactMatch) {
            m_blobName = name;
            loadFromBlob(context.blobRegistry->value(name), context.contextDir);
        } else if (baseMatch) {
            m_blobName = baseName;
            loadFromBlob(context.blobRegistry->value(baseName), context.contextDir);
        } else {
            m_file = name;
            loadFile(m_file, context.contextDir);
        }
    }

    // Re-register the new ports in the outer portMap under the original keys
    // so that subsequent connection deserialization can find them.
    for (int i = 0; i < qMin(savedInputKeys.size(), m_inputPorts.size()); ++i) {
        context.portMap[savedInputKeys[i]] = m_inputPorts[i];
    }
    for (int i = 0; i < qMin(savedOutputKeys.size(), m_outputPorts.size()); ++i) {
        context.portMap[savedOutputKeys[i]] = m_outputPorts[i];
    }
}

void IC::loadBoundaryPorts(const bool isInput, const QVector<QString> &labels)
{
    const auto &internalPorts = isInput ? m_internalInputs : m_internalOutputs;
    const int count = static_cast<int>(internalPorts.size());

    // Lock port count to exactly the number found in the sub-circuit file;
    // min == max == actual count prevents the user from adding/removing IC ports
    if (isInput) {
        setMaxInputSize(count);
        setMinInputSize(count);
        setInputSize(count);
    } else {
        setMaxOutputSize(count);
        setMinOutputSize(count);
        setOutputSize(count);
    }

    for (int i = 0; i < count; ++i) {
        if (isInput) {
            auto *port = inputPort(i);
            port->setName(labels.at(i));
            // Mirror required/default-status from the sub-circuit's input elements so that
            // unconnected optional inputs (e.g. enable lines) don't flag the IC as invalid
            port->setRequired(internalPorts.at(i)->isRequired());
            port->setDefaultStatus(internalPorts.at(i)->status());
            port->setStatus(internalPorts.at(i)->status());
        } else {
            outputPort(i)->setName(labels.at(i));
        }
    }

    qCDebug(three) << "IC" << m_file << "->" << (isInput ? "Inputs" : "Outputs")
                    << "min:" << (isInput ? minInputSize() : minOutputSize())
                    << "max:" << (isInput ? maxInputSize() : maxOutputSize())
                    << "current:" << (isInput ? inputSize() : outputSize());
}

void IC::resetInternalState()
{
    m_internalInputs.clear();
    m_internalOutputs.clear();
    setInputSize(0);
    setOutputSize(0);
    qDeleteAll(m_internalElements);
    m_internalElements.clear();
}

void IC::loadFile(const QString &fileName, const QString &contextDir)
{
    qCDebug(zero) << "Reading IC.";

    m_blobName.clear();

    // Try the full path combined with contextDir first (handles relative paths
    // and same-OS absolute paths). If not found, fall back to just the filename
    // resolved against contextDir — this handles cross-platform absolute paths
    // from old .panda files (e.g. a Windows "C:\...\sub.panda" opened on Linux).
    QFileInfo fileInfo(QDir(contextDir), fileName);

    if (!fileInfo.exists() || !fileInfo.isFile()) {
        fileInfo.setFile(QDir(contextDir), QFileInfo(QString(fileName).replace('\\', '/')).fileName());
    }

    if (!fileInfo.exists() || !fileInfo.isFile()) {
        throw PANDACEPTION("%1 not found.", fileInfo.absoluteFilePath());
    }

    // Delegate file I/O, cycle detection, migration to ICRegistry if available
    if (auto *scene_ = qobject_cast<Scene *>(scene())) {
        auto *reg = scene_->icRegistry();
        const ICDefinition *def = reg->definition(fileInfo.absoluteFilePath(), fileInfo.absolutePath());
        if (def && def->isValid()) {
            loadFromDefinition(def, fileInfo.absolutePath());
            m_file = fileInfo.absoluteFilePath();  // restore after loadFromBlob clears it
            setToolTip(fileInfo.fileName());
            if (label().isEmpty()) {
                setLabel(fileInfo.baseName().toUpper());
            }
            qCDebug(zero) << "Finished reading IC (via ICRegistry).";
            return;
        }
    }

    // Fallback: direct file load (IC not yet in a scene, e.g. during deserialization).
    // Reset here rather than at the top — the ICRegistry path above handles its own
    // reset inside loadFromBlob(), so resetting before it would be wasted work.
    resetInternalState();
    m_file = fileInfo.absoluteFilePath();
    setToolTip(fileInfo.fileName());
    loadFileDirectly(fileInfo);

    qCDebug(zero) << "Finished reading IC.";
}

void IC::loadFileDirectly(const QFileInfo &fileInfo)
{
    // Cycle detection: if this file is already being loaded up the call stack,
    // a circular IC reference exists (A→B→A→…). Throw instead of stack-overflowing.
    static QSet<QString> s_loadingFiles;
    const QString canonicalPath = fileInfo.canonicalFilePath();
    if (s_loadingFiles.contains(canonicalPath)) {
        throw PANDACEPTION("Circular IC reference detected: %1", canonicalPath);
    }
    s_loadingFiles.insert(canonicalPath);
    auto removeGuard = qScopeGuard([&] { s_loadingFiles.remove(canonicalPath); });

    QFile file(fileInfo.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        throw PANDACEPTION("Error opening file: %1", file.errorString());
    }

    QDataStream stream(&file);
    const auto preamble = Serialization::readPreamble(stream);
    auto fileRegistry = Serialization::deserializeBlobRegistry(preamble.metadata);

    QMap<quint64, QNEPort *> portMap;
    SerializationContext subCtx{portMap, preamble.version, fileInfo.absolutePath()};
    subCtx.blobRegistry = fileRegistry.isEmpty() ? nullptr : &fileRegistry;
    const auto items = Serialization::deserialize(stream, subCtx);
    file.close(); // must be closed before QSaveFile can write on Windows (mandatory file locking)

    if ((preamble.version < AppVersion::current) && Application::migrationEnabled) {
        migrateFile(fileInfo, items, preamble.version, fileRegistry);
    }

    processLoadedItems(items);

    if (label().isEmpty()) {
        setLabel(fileInfo.baseName().toUpper());
    }
}

void IC::migrateFile(const QFileInfo &fileInfo, const QList<QGraphicsItem *> &items,
                     const QVersionNumber &version, const QMap<QString, QByteArray> &fileRegistry)
{
    Serialization::createVersionedBackup(fileInfo.absoluteFilePath(), version);

    // Build port metadata for the migrated file header
    QVector<GraphicElement *> elements;
    for (auto *item : items) {
        if (item->type() == GraphicElement::Type) {
            if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
                elements.append(elm);
            }
        }
    }
    const auto portMeta = buildPortMetadata(elements);

    QMap<QString, QVariant> migrationMeta;
    migrationMeta["inputCount"] = portMeta.inputCount;
    migrationMeta["outputCount"] = portMeta.outputCount;
    migrationMeta["inputLabels"] = portMeta.inputLabels;
    migrationMeta["outputLabels"] = portMeta.outputLabels;
    Serialization::serializeBlobRegistry(fileRegistry, migrationMeta);

    QSaveFile saveFile(fileInfo.absoluteFilePath());
    if (!saveFile.open(QIODevice::WriteOnly)) {
        throw PANDACEPTION("IC migration: cannot open file for writing: %1", fileInfo.absoluteFilePath());
    }
    QDataStream outStream(&saveFile);
    Serialization::writePandaHeader(outStream);
    outStream << migrationMeta;
    Serialization::serialize(items, outStream);
    if (!saveFile.commit()) {
        throw PANDACEPTION("IC migration: failed to commit re-saved file: %1", fileInfo.absoluteFilePath());
    }
}

void IC::processLoadedItems(const QList<QGraphicsItem *> &items)
{
    for (auto *item : items) {
        if (item->type() != GraphicElement::Type) {
            continue;
        }

        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (!elm) {
            continue;
        }

        // Input/Output elements become the IC's external ports; everything else is internal logic
        switch (elm->elementGroup()) {
        case ElementGroup::Input:  loadBoundaryElement(elm, true);  break;
        case ElementGroup::Output: loadBoundaryElement(elm, false); break;
        default:                   m_internalElements.append(elm);  break;
        }
    }

    // --- Build sorted, labelled port lists ---
    // Sort top-to-bottom by Y position so port order on the IC body matches visual layout
    sortPorts(m_internalInputs);
    sortPorts(m_internalOutputs);

    QVector<QString> inputLabels(m_internalInputs.size());
    QVector<QString> outputLabels(m_internalOutputs.size());
    buildPortLabels(m_internalInputs, inputLabels);
    buildPortLabels(m_internalOutputs, outputLabels);
    loadBoundaryPorts(true, inputLabels);
    loadBoundaryPorts(false, outputLabels);

    // --- Update visual representation ---
    // Position label just below the IC body, which grows with port count
    const qreal bottom = portsBoundingRect().united(QRectF(0, 0, 64, 64)).bottom();
    m_label->setPos(30, bottom + 5);

    generatePixmap();
}

void IC::loadFromBlob(const QByteArray &blob, const QString &contextDir)
{
    qCDebug(zero) << "Loading IC from blob.";

    // Parse the blob before clearing state so a corrupt blob leaves the IC unchanged.
    QByteArray data(blob);
    QDataStream stream(&data, QIODevice::ReadOnly);

    // Blob is a full .panda file
    const auto preamble = Serialization::readPreamble(stream);
    auto blobRegistry = Serialization::deserializeBlobRegistry(preamble.metadata);

    QMap<quint64, QNEPort *> portMap;
    SerializationContext subCtx{portMap, preamble.version, contextDir};
    subCtx.blobRegistry = blobRegistry.isEmpty() ? nullptr : &blobRegistry;
    const auto items = Serialization::deserialize(stream, subCtx);

    // Parsing succeeded — now clear old state and apply
    resetInternalState();
    m_file.clear(); // switching to blob-backed, no file association

    processLoadedItems(items);

    if (!m_blobName.isEmpty()) {
        setToolTip(m_blobName);
    }

    qCDebug(zero) << "Finished loading IC from blob.";
}

void IC::loadFromDefinition(const ICDefinition *def, const QString &contextDir)
{
    m_definition = def;
    loadFromBlob(def->blobBytes(), contextDir);

    qCDebug(zero) << "Finished loading IC from definition.";
}

void IC::generatePixmap()
{
    // The pixmap must encompass both the 64×64 body and any ports that extend beyond it
    const QSize size = portsBoundingRect().united(QRectF(0, 0, 64, 64)).size().toSize();
    QPixmap tempPixmap(size);
    tempPixmap.fill(Qt::transparent);

    QPainter tmpPainter(&tempPixmap);

    const QColor bodyColor = isEmbedded() ? QColor(90, 126, 160) : QColor(126, 126, 126);
    const QColor outlineColor = isEmbedded() ? QColor(58, 82, 110) : QColor(78, 78, 78);

    // IC body: styled like a physical DIP package
    tmpPainter.setBrush(bodyColor);
    tmpPainter.setPen(QPen(QBrush(outlineColor), 0.5, Qt::SolidLine));

    // 7px inset on each side (14px total width reduction) so the port connector dots
    // visually overlap the border, matching the TruthTable and physical DIP appearance
    QPoint topLeft = tempPixmap.rect().topLeft();
    topLeft.setX(topLeft.x() + 7);
    QSize finalSize = tempPixmap.rect().size();
    finalSize.setWidth(finalSize.width() - 14);
    QRectF finalRect = QRectF(topLeft, finalSize);
    tmpPainter.drawRoundedRect(finalRect, 3, 3);

    // Centre the wiRedPanda mascot logo on the IC body
    QPixmap panda(":/Components/Logic/ic-panda2.svg");
    QPointF pandaOrigin = finalRect.center();
    pandaOrigin.setX(pandaOrigin.x() - panda.width() / 2);
    pandaOrigin.setY(pandaOrigin.y() - panda.height() / 2);
    tmpPainter.drawPixmap(pandaOrigin, panda);

    // Draw a thin dark strip at the bottom edge to simulate the package shadow/bevel
    tmpPainter.setBrush(outlineColor);
    tmpPainter.setPen(QPen(QBrush(outlineColor), 0.5, Qt::SolidLine));

    // Collapse the two-point rect to a 3px-tall strip flush with the bottom of the body
    QRectF shadowRect(finalRect.bottomLeft(), finalRect.bottomRight());
    shadowRect.adjust(0, -3, 0, 0);
    tmpPainter.drawRoundedRect(shadowRect, 3, 3);

    // Draw the orientation notch (semicircle) at the top centre, matching the physical
    // DIP IC convention for pin-1 identification.
    // drawChord angle parameters are in 1/16th-degree units; -180*16 = lower half-circle.
    QRectF topCenter = QRectF(finalRect.topLeft() + QPointF(18, -12), QSize(24, 24));
    tmpPainter.drawChord(topCenter, 0, -180 * 16);

    m_pixmap = tempPixmap;
    update();
}

void IC::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    emit requestOpenSubCircuit(id(), m_blobName, m_file);
}

QRectF IC::boundingRect() const
{
    return portsBoundingRect().united(QRectF(0, 0, 64, 64));
}

void IC::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    if (isSelected()) {
        painter->save();
        painter->setBrush(m_selectionBrush);
        painter->setPen(QPen(m_selectionPen, 0.5, Qt::SolidLine));
        painter->drawRoundedRect(boundingRect(), 5, 5);
        painter->restore();
    }

    painter->drawPixmap(boundingRect().topLeft(), pixmap());
}

void IC::loadBoundaryElement(GraphicElement *elm, const bool isInput)
{
    // Each port of a boundary element (input or output) becomes one external pin on the IC.
    // A proxy Node element is inserted as a bridge between the IC's external port and the
    // internal sub-circuit wiring.
    //
    // Input elements:  [IC external input] → Node.input → Node.output → [internal wires]
    // Output elements: [internal wires] → Node.input → Node.output → [IC external output]
    const int portCount = isInput ? elm->outputSize() : elm->inputSize();

    for (int p = 0; p < portCount; ++p) {
        auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
        nodeElm->setPos(elm->pos());
        nodeElm->setLabel(elm->label().isEmpty()
                              ? ElementFactory::typeToText(elm->elementType())
                              : elm->label());

        if (isInput) {
            auto *srcPort = elm->outputPort(p);
            auto *nodeInput = nodeElm->inputPort();
            if (portCount > 1) {
                nodeInput->setName(srcPort->name());
            }
            nodeInput->setRequired(elm->elementType() == ElementType::Clock);
            nodeInput->setDefaultStatus(srcPort->status());
            nodeInput->setStatus(srcPort->status());
            m_internalInputs.append(nodeInput);

            // Re-route connections from original output to proxy Node's output
            const auto conns = srcPort->connections();
            for (auto *conn : conns) {
                conn->setStartPort(nodeElm->outputPort());
            }
        } else {
            auto *srcPort = elm->inputPort(p);
            auto *nodeOutput = nodeElm->outputPort();
            if (portCount > 1) {
                nodeOutput->setName(srcPort->name());
            }
            m_internalOutputs.append(nodeOutput);

            // Re-route connections from original input to proxy Node's input
            for (auto *conn : srcPort->connections()) {
                conn->setEndPort(nodeElm->inputPort());
            }
        }

        m_internalElements.append(nodeElm);
    }

    delete elm;
}

IC::PortMetadata IC::buildPortMetadata(const QVector<GraphicElement *> &elements)
{
    PortMetadata meta;
    QVector<QNEPort *> inputPorts, outputPorts;

    for (auto *elm : elements) {
        if (elm->elementGroup() == ElementGroup::Input) {
            for (auto *port : elm->outputs()) { inputPorts.append(port); }
        } else if (elm->elementGroup() == ElementGroup::Output) {
            for (auto *port : elm->inputs()) { outputPorts.append(port); }
        }
    }

    sortPorts(inputPorts);
    sortPorts(outputPorts);

    meta.inputCount = static_cast<int>(inputPorts.size());
    meta.outputCount = static_cast<int>(outputPorts.size());

    QVector<QString> inLabels(inputPorts.size());
    QVector<QString> outLabels(outputPorts.size());
    buildPortLabels(inputPorts, inLabels);
    buildPortLabels(outputPorts, outLabels);

    meta.inputLabels = QStringList(inLabels.begin(), inLabels.end());
    meta.outputLabels = QStringList(outLabels.begin(), outLabels.end());

    return meta;
}

void IC::initializeSimulation()
{
    m_sortedInternalElements.clear();
    m_boundaryInputElements.clear();
    m_internalHasFeedback = false;

    if (m_internalElements.isEmpty()) {
        return;
    }

    // Initialize simulation vectors on all internal elements
    for (auto *elm : std::as_const(m_internalElements)) {
        elm->initSimulationVectors(elm->inputSize(), elm->outputSize());
    }

    // Build connection graph and wire wireless Tx→Rx
    Simulation::buildConnectionGraph(m_internalElements);
    Simulation::connectWirelessElements(m_internalElements);

    // Initialize nested ICs recursively
    for (auto *elm : std::as_const(m_internalElements)) {
        if (elm->elementType() == ElementType::IC) {
            static_cast<IC *>(elm)->initializeSimulation();
        }
    }

    // Record boundary input elements (driven externally, should not run updateLogic)
    for (auto *port : std::as_const(m_internalInputs)) {
        if (auto *elm = port->graphicElement()) {
            m_boundaryInputElements.insert(elm);
        }
    }

    // Topological sort with feedback detection using shared helpers
    const auto txMap = Simulation::buildTxMap(m_internalElements);
    const auto successors = Simulation::buildSuccessorGraph(m_internalElements, txMap);
    const auto result = Simulation::topologicalSort(m_internalElements, successors);
    m_internalHasFeedback = !result.feedbackNodes.isEmpty();

    // Remove boundary input elements so the hot loop in updateLogic()
    // doesn't need a per-element QSet lookup on every simulation tick.
    m_sortedInternalElements = result.sorted;
    m_sortedInternalElements.erase(
        std::remove_if(m_sortedInternalElements.begin(), m_sortedInternalElements.end(),
            [this](auto *elm) { return m_boundaryInputElements.contains(elm); }),
        m_sortedInternalElements.end());
}

void IC::updateLogic()
{
    if (m_sortedInternalElements.isEmpty()) {
        return;
    }

    // Permissive mode so ICs can propagate Unknown through their internal elements.
    if (!simUpdateInputsAllowUnknown()) {
        return;
    }

    // Push external input values to boundary input nodes' GraphicElement outputs.
    // Boundary input nodes are Nodes whose input port is in m_internalInputs.
    // We set their output value directly and skip them in the update loop.
    for (int i = 0; i < inputSize() && i < m_internalInputs.size(); ++i) {
        auto *boundaryElement = m_internalInputs.at(i)->graphicElement();
        if (boundaryElement) {
            boundaryElement->setOutputValue(0, simInputs().at(i));
        }
    }

    // Run internal elements in topological order.
    // Boundary input nodes are already excluded from m_sortedInternalElements.
    if (m_internalHasFeedback) {
        Simulation::iterativeSettle(m_sortedInternalElements);
    } else {
        for (auto *element : std::as_const(m_sortedInternalElements)) {
            element->updateLogic();
        }
    }

    // Pull output values from boundary output nodes
    for (int i = 0; i < outputSize() && i < m_internalOutputs.size(); ++i) {
        auto *boundaryElement = m_internalOutputs.at(i)->graphicElement();
        if (boundaryElement) {
            setOutputValue(i, boundaryElement->outputValue(0));
        }
    }
}

void IC::refresh()
{
}

void IC::loadFromDrop(const QString &fileName, const QString &contextDir)
{
    loadFile(fileName, contextDir);
}

