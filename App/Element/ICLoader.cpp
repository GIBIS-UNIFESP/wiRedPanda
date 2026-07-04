// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICLoader.h"

#include <algorithm>

#include <QDir>
#include <QSaveFile>
#include <QScopeGuard>
#include <QSet>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ICRenderer.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace {

bool comparePorts(Port *port1, Port *port2)
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

void sortPorts(QVector<Port *> &ports)
{
    std::stable_sort(ports.begin(), ports.end(), comparePorts);
}

void buildPortLabels(const QVector<Port *> &ports, QVector<QString> &labels)
{
    for (int i = 0; i < ports.size(); ++i) {
        auto *port = ports.at(i);
        auto *elm = port->graphicElement();
        if (!elm) {
            continue;
        }
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

// Maximum nesting depth for IC-within-IC blob loading.
// Each level deserializes a full panda stream; deep nesting exhausts the call stack.
constexpr int kMaxICNestingDepth = 16;
thread_local int s_icLoadDepth = 0;

} // anonymous namespace

void ICLoader::loadFile(IC &ic, const QString &fileName, const QString &contextDir)
{
    qCDebug(zero) << "Reading IC.";

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

    // Clear blob name only after validation so the IC remains consistent if
    // the file is not found and an exception is thrown above.
    ic.m_blobName.clear();

    // Use cached file bytes from ICRegistry when available (avoids re-reading from disk)
    if (auto *scene_ = qobject_cast<Scene *>(ic.scene())) {
        auto *reg = scene_->icRegistry();
        const QByteArray &cached = reg->cachedFileBytes(fileInfo.absoluteFilePath());
        if (!cached.isEmpty()) {
            deserializeAndLoad(ic, cached, fileInfo.absolutePath());
            ic.m_file = fileInfo.absoluteFilePath();
            // No Qt tooltip: the filename is shown in the hover preview popup
            // (see ICPreviewPopup) so the two don't overlap. Clear the base
            // class's translated-name tooltip set at construction.
            ic.setToolTip(QString());
            if (ic.label().isEmpty()) {
                ic.setLabel(fileInfo.baseName().toUpper());
            }
            qCDebug(zero) << "Finished reading IC (via cache).";
            return;
        }
    }

    // Fallback: direct file load (IC not yet in a scene, e.g. during deserialization).
    // loadFileDirectly() mirrors deserializeAndLoad()'s parse-first, reset-after shape:
    // a failed parse (corrupt file, missing dependency, circular reference) propagates
    // without ever leaving m_sortedInternalElements pointing at freed elements.
    loadFileDirectly(ic, fileInfo);
    ic.m_file = fileInfo.absoluteFilePath();
    // Name is carried by the hover preview popup, not a Qt tooltip — see above.
    ic.setToolTip(QString());

    qCDebug(zero) << "Finished reading IC.";
}

void ICLoader::loadFileDirectly(IC &ic, const QFileInfo &fileInfo)
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
    auto fileRegistry = Serialization::deserializeBlobRegistry(preamble.metadata, preamble.version);

    QHash<quint64, Port *> portMap;
    SerializationContext subCtx = {portMap, preamble.version, fileInfo.absolutePath()};
    subCtx.blobRegistry = fileRegistry.isEmpty() ? nullptr : &fileRegistry;
    QList<QGraphicsItem *> items = Serialization::deserialize(stream, subCtx);
    file.close(); // must be closed before QSaveFile can write on Windows (mandatory file locking)

    // Cleans up whatever is still in `items` if an exception unwinds through
    // migrateFile()/processLoadedItems() below. processLoadedItems() drains
    // items via takeFirst() as ownership transfers, so on success this guard
    // finds an empty list and does nothing. Connections are nulled out before
    // qDeleteAll(): a Connection* still owned by an element's port at this
    // point would otherwise be deleted twice — once directly here, once via
    // that port's destructor (~InputPort/~OutputPort drain their
    // connections).
    auto itemsGuard = qScopeGuard([&items] {
        for (qsizetype i = 0; i < items.size(); ++i) {
            if (items[i] && items[i]->type() == Connection::Type) {
                delete items[i];
                items[i] = nullptr;
            }
        }
        qDeleteAll(items);
    });

    if ((preamble.version < FormatRev::current) && Application::migrationEnabled) {
        migrateFile(fileInfo, items, preamble.version, fileRegistry);
    }

    // Parsing (and migration, if triggered) succeeded — only now is it safe to
    // clear the old internal state and apply the freshly-parsed items. If any
    // step above had thrown, resetInternalState() would never have run and the
    // IC's previous internal graph would remain intact.
    ic.resetInternalState();
    processLoadedItems(ic, items);

    if (ic.label().isEmpty()) {
        ic.setLabel(fileInfo.baseName().toUpper());
    }
}

void ICLoader::migrateFile(const QFileInfo &fileInfo, const QList<QGraphicsItem *> &items,
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

void ICLoader::processLoadedItems(IC &ic, QList<QGraphicsItem *> &items)
{
    // Snapshot the preview now, while the original Input/Output elements (buttons,
    // switches, LEDs, …) are still alive in `items`.  loadBoundaryElement() below
    // replaces each of them with a proxy Node, so a later render would only see
    // the simulation graph.
    ICRenderer::generatePreviewPixmap(ic, items);

    // Drain items one at a time: as ownership of each transfers (to
    // m_internalConnections/m_internalElements) or the item is deleted
    // (loadBoundaryElement()'s original Input/Output element), remove it from
    // `items` immediately. This keeps the invariant that `items` only ever
    // holds pointers this function has NOT yet taken ownership of — the
    // caller's qScopeGuard relies on that to avoid double-deleting/deleting a
    // dangling pointer if an exception unwinds through this loop.
    while (!items.isEmpty()) {
        auto *item = items.takeFirst();

        if (auto *conn = qgraphicsitem_cast<Connection *>(item)) {
            ic.m_internalConnections.append(conn);
            continue;
        }

        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (!elm) {
            continue;
        }

        // Input/Output elements become the IC's external ports; everything else is internal logic
        switch (elm->elementGroup()) {
        case ElementGroup::Input:  loadBoundaryElement(ic, elm, true);  break;
        case ElementGroup::Output: loadBoundaryElement(ic, elm, false); break;
        default:                   ic.m_internalElements.append(elm);  break;
        }
    }

    // --- Build sorted, labelled port lists ---
    // Sort top-to-bottom by Y position so port order on the IC body matches visual layout
    sortPorts(ic.m_internalInputs);
    sortPorts(ic.m_internalOutputs);

    QVector<QString> inputLabels(ic.m_internalInputs.size());
    QVector<QString> outputLabels(ic.m_internalOutputs.size());
    buildPortLabels(ic.m_internalInputs, inputLabels);
    buildPortLabels(ic.m_internalOutputs, outputLabels);
    loadBoundaryPorts(ic, true, inputLabels);
    loadBoundaryPorts(ic, false, outputLabels);

    // --- Update visual representation ---
    // Position label just below the IC body, which grows with port count
    const qreal bottom = ic.portsBoundingRect().united(QRectF(0, 0, 64, 64)).bottom();
    ic.m_label->setPos(30, bottom + 5);

    ICRenderer::generatePixmap(ic);
}

void ICLoader::deserializeAndLoad(IC &ic, const QByteArray &bytes, const QString &contextDir)
{
    if (s_icLoadDepth >= kMaxICNestingDepth) {
        throw PANDACEPTION("IC nesting depth limit (%1) exceeded — blob may be maliciously crafted",
                           QString::number(kMaxICNestingDepth));
    }

    ++s_icLoadDepth;
    const auto depthGuard = qScopeGuard([] { --s_icLoadDepth; });

    // Parse the bytes before clearing state so a corrupt input leaves the IC unchanged.
    QByteArray data(bytes);
    QDataStream stream(&data, QIODevice::ReadOnly);

    const auto preamble = Serialization::readPreamble(stream);
    auto blobRegistry = Serialization::deserializeBlobRegistry(preamble.metadata, preamble.version);

    QHash<quint64, Port *> portMap;
    SerializationContext subCtx = {portMap, preamble.version, contextDir};
    subCtx.blobRegistry = blobRegistry.isEmpty() ? nullptr : &blobRegistry;
    QList<QGraphicsItem *> items = Serialization::deserialize(stream, subCtx);

    // See loadFileDirectly()'s itemsGuard for why this nulls connections
    // before qDeleteAll() and why it's a no-op on the success path.
    auto itemsGuard = qScopeGuard([&items] {
        for (qsizetype i = 0; i < items.size(); ++i) {
            if (items[i] && items[i]->type() == Connection::Type) {
                delete items[i];
                items[i] = nullptr;
            }
        }
        qDeleteAll(items);
    });

    // Parsing succeeded — now clear old state and apply
    ic.resetInternalState();
    processLoadedItems(ic, items);
}

void ICLoader::loadFromBlob(IC &ic, const QByteArray &blob, const QString &contextDir)
{
    qCDebug(zero) << "Loading IC from blob.";

    deserializeAndLoad(ic, blob, contextDir);
    ic.m_file.clear(); // switching to blob-backed, no file association

    // Name is carried by the hover preview popup, not a Qt tooltip; clear the
    // base class's translated-name tooltip so no bubble fights the preview.
    ic.setToolTip(QString());

    qCDebug(zero) << "Finished loading IC from blob.";
}

void ICLoader::loadBoundaryElement(IC &ic, GraphicElement *elm, const bool isInput)
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
            ic.m_internalInputs.append(nodeInput);

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
            ic.m_internalOutputs.append(nodeOutput);

            // Re-route connections from original input to proxy Node's input
            for (auto *conn : srcPort->connections()) {
                conn->setEndPort(nodeElm->inputPort());
            }
        }

        ic.m_internalElements.append(nodeElm);
    }

    // Detach any connections still attached to elm's ports before deleting it.
    // In a valid circuit all connections were re-routed above, so these lists
    // are empty.  In a fuzz-corrupted blob, extra ports may have connections that
    // were not re-routed; drainConnections() in the port destructor would free
    // them while their pointers are still live in the items list passed to
    // processLoadedItems(), causing a heap-use-after-free.  Calling setEndPort /
    // setStartPort(nullptr) detaches cleanly without deleting the connection.
    for (int p = 0; p < elm->inputSize(); ++p) {
        const auto conns = elm->inputPort(p)->connections();
        for (auto *c : conns) { c->setEndPort(nullptr); }
    }
    for (int p = 0; p < elm->outputSize(); ++p) {
        const auto conns = elm->outputPort(p)->connections();
        for (auto *c : conns) { c->setStartPort(nullptr); }
    }

    delete elm;
}

void ICLoader::loadBoundaryPorts(IC &ic, const bool isInput, const QVector<QString> &labels)
{
    const auto &internalPorts = isInput ? ic.m_internalInputs : ic.m_internalOutputs;
    const int count = static_cast<int>(internalPorts.size());

    // Lock port count to exactly the number found in the sub-circuit file;
    // min == max == actual count prevents the user from adding/removing IC ports
    if (isInput) {
        ic.setMaxInputSize(count);
        ic.setMinInputSize(count);
        ic.setInputSize(count);
    } else {
        ic.setMaxOutputSize(count);
        ic.setMinOutputSize(count);
        ic.setOutputSize(count);
    }

    for (int i = 0; i < count; ++i) {
        if (isInput) {
            auto *port = ic.inputPort(i);
            port->setName(labels.at(i));
            // Mirror required/default-status from the sub-circuit's input elements so that
            // unconnected optional inputs (e.g. enable lines) don't flag the IC as invalid
            port->setRequired(internalPorts.at(i)->isRequired());
            port->setDefaultStatus(internalPorts.at(i)->status());
            port->setStatus(internalPorts.at(i)->status());
        } else {
            ic.outputPort(i)->setName(labels.at(i));
        }
    }

    qCDebug(three) << "IC" << ic.m_file << "->" << (isInput ? "Inputs" : "Outputs")
                    << "min:" << (isInput ? ic.minInputSize() : ic.minOutputSize())
                    << "max:" << (isInput ? ic.maxInputSize() : ic.maxOutputSize())
                    << "current:" << (isInput ? ic.inputSize() : ic.outputSize());
}

IC::PortMetadata ICLoader::buildPortMetadata(const QVector<GraphicElement *> &elements)
{
    IC::PortMetadata meta;
    QVector<Port *> inputPorts, outputPorts;

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
