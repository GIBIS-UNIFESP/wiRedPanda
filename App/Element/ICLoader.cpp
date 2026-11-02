// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICLoader.h"

#include <algorithm>

#include <QCryptographicHash>
#include <QDataStream>
#include <QFile>
#include <QHash>
#include <QPixmap>
#include <QSaveFile>
#include <QScopeGuard>
#include <QSet>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ICDefinition.h"
#include "App/Element/ICFileCache.h"
#include "App/Element/ICRenderer.h"
#include "App/IO/ExternalFilePath.h"
#include "App/IO/FileUtils.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace {

bool comparePorts(Port *port1, Port *port2)
{
    auto *elem1 = port1->element();
    auto *elem2 = port2->element();
    if (!elem1 || !elem2) {
        return false; // LCOV_EXCL_LINE — ElementPorts::addPort() unconditionally calls setElement() right after constructing a port, so every port reachable here already has a non-null owner.
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

// A boundary element's label, or its type's name where it has none: the name its pins go by, and its proxy node's label
QString boundaryLabel(const Element *elm)
{
    return elm->label().isEmpty() ? ElementFactory::typeToText(elm->elementType()) : elm->label();
}

// The source name a boundary element's pin is saved under: the label, then the port's source name when the element stands
// for more than one pin, as loadBoundaryElement() names the proxy's port
QString boundarySourceName(Port *port)
{
    const Element *elm = port->element();
    const int pins = port->isOutput() ? elm->outputSize() : elm->inputSize();
    const QString sourcePortName = port->sourceName();
    return pins > 1 && !sourcePortName.isEmpty() ? boundaryLabel(elm) + " " + sourcePortName : boundaryLabel(elm);
}

// A pin's name is its boundary element's label, then the port's name when the element stands for more than one pin. The
// name is a function, evaluated when asked, so a port named by its translated text (a display's segment) shows the
// current language; the source name is the one generated code names its signals from.
PinName pinNameOf(Port *port)
{
    auto *elm = port->element();
    if (!elm) {
        return {}; // LCOV_EXCL_LINE — same invariant as comparePorts() above: every port has a non-null owner from construction onward.
    }
    const QString label = boundaryLabel(elm);
    const auto portName = port->nameFunction();
    const QString sourcePortName = port->sourceName();
    return PinName{
        [label, portName] {
            const QString own = portName();
            return own.isEmpty() ? label : label + " " + own;
        },
        sourcePortName.isEmpty() ? label : label + " " + sourcePortName,
    };
}

QVector<PinName> pinNamesOf(const QVector<Port *> &ports)
{
    QVector<PinName> names;
    names.reserve(ports.size());
    for (auto *port : ports) {
        names.append(pinNameOf(port));
    }
    return names;
}

// Maximum nesting depth for IC-within-IC blob loading.
// Each level deserializes a full panda stream; deep nesting exhausts the call stack.
constexpr int kMaxICNestingDepth = 16;
thread_local int s_icLoadDepth = 0;

// The IC files the loads under way record as they read them, and those a snapshot's restore carries for files gone
thread_local ICLoader::Files *s_icFilesRead = nullptr;
thread_local const ICLoader::Files *s_icFilesCarried = nullptr;
// Whether the loads under way are a snapshot's restore, whose circuits' pictures and sounds fall back unreported
thread_local bool s_icRestoring = false;
// Whether the loads under way restore a captured reload: the snapshot's files then stand in for the files on disk
thread_local bool s_icRestoreCaptured = false;

void recordICFileRead(const QString &path, const QByteArray &bytes)
{
    if (s_icFilesRead) {
        s_icFilesRead->insert(path, bytes);
    }
}

// The bytes a snapshot carries for an IC file no longer on disk: looked up where the stored name resolved, then
// where it pointed when stored, which is absolute in a snapshot
const QByteArray *carriedICFile(const QFileInfo &resolved, const QString &fileName, QString &path)
{
    if (!s_icFilesCarried) {
        return nullptr;
    }
    for (const QString &candidate : {resolved.absoluteFilePath(), QFileInfo(fileName).absoluteFilePath()}) {
        if (const auto it = s_icFilesCarried->constFind(candidate); it != s_icFilesCarried->constEnd()) {
            path = candidate;
            return &it.value();
        }
    }
    return nullptr;
}

} // anonymous namespace

void ICLoader::loadFile(IC &ic, const QString &fileName, const QString &contextDir)
{
    qCDebug(zero) << "Reading IC.";

    QFileInfo fileInfo(ExternalFilePath::resolve(fileName, contextDir));

    // A snapshot's restore loads what the file held when the snapshot was taken: when the file is gone, and also when
    // the restore is of a captured reload, whose definition is the one to give back
    const bool onDisk = fileInfo.exists() && fileInfo.isFile();
    if (!onDisk || s_icRestoreCaptured) {
        QString path;
        if (const QByteArray *carried = carriedICFile(fileInfo, fileName, path)) {
            const QFileInfo carriedInfo(path);
            deserializeAndLoad(ic, *carried, carriedInfo.absolutePath(), path);
            recordICFileRead(path, *carried);
            ic.m_blobName.clear();
            ic.m_file = path;
            if (ic.label().isEmpty()) {
                ic.setLabel(carriedInfo.baseName().toUpper());
            }
            qCDebug(zero) << "Finished reading IC (from the snapshot).";
            return;
        }
    }
    if (!onDisk) {
        throw PANDACEPTION("%1 not found.", fileInfo.absoluteFilePath());
    }

    // Clear blob name only after validation so the IC remains consistent if
    // the file is not found and an exception is thrown above.
    ic.m_blobName.clear();

    // Use the cached file bytes of the file cache the IC reads through, when there is one (avoids re-reading from disk)
    if (ICFileCache *cache = ic.fileCache()) {
        const QByteArray &cached = cache->cachedFileBytes(fileInfo.absoluteFilePath());
        if (!cached.isEmpty()) {
            deserializeAndLoad(ic, cached, fileInfo.absolutePath(), fileInfo.absoluteFilePath());
            recordICFileRead(fileInfo.absoluteFilePath(), cached);
            ic.m_file = fileInfo.absoluteFilePath();
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

    qCDebug(zero) << "Finished reading IC.";
}

std::optional<std::pair<QString, QByteArray>> ICLoader::fileBytes(const IC &ic, const QString &fileName,
                                                                  const QString &contextDir)
{
    // The bytes loadFile() would load: a captured reload's restore takes them from its snapshot, as loadFile() does
    const QFileInfo fileInfo(ExternalFilePath::resolve(fileName, contextDir));
    const bool onDisk = fileInfo.exists() && fileInfo.isFile();
    if (!onDisk || s_icRestoreCaptured) {
        QString path;
        if (const QByteArray *carried = carriedICFile(fileInfo, fileName, path)) {
            return std::pair(path, *carried);
        }
    }
    if (!onDisk) {
        return std::nullopt;
    }

    const QString path = fileInfo.absoluteFilePath();
    if (ICFileCache *cache = ic.fileCache()) {
        if (const QByteArray &cached = cache->cachedFileBytes(path); !cached.isEmpty()) {
            return std::pair(path, cached);
        }
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return std::nullopt;
    }
    return std::pair(path, file.readAll());
}

void ICLoader::loadFileDirectly(IC &ic, const QFileInfo &fileInfo)
{
    // Depth cap: shares s_icLoadDepth/kMaxICNestingDepth with deserializeAndLoad() rather than
    // using a separate counter, so a chain that mixes file-backed and embedded-blob hops can't
    // bypass the limit by crossing between the two loading paths — depth bounds the overall
    // recursion, not "how many hops happened to go through this particular function." Cycle
    // detection (below) alone isn't enough: a long, non-cyclic chain of distinct legitimate
    // files (A embeds B embeds C embeds ... , no repeats) would otherwise recurse unbounded here
    // and exhaust the call stack.
    if (s_icLoadDepth >= kMaxICNestingDepth) {
        throw PANDACEPTION("IC nesting depth limit (%1) exceeded while loading %2",
                           QString::number(kMaxICNestingDepth), fileInfo.absoluteFilePath());
    }
    ++s_icLoadDepth;
    const auto depthGuard = qScopeGuard([] { --s_icLoadDepth; });

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
    QByteArray bytes = file.readAll();
    file.close(); // must be closed before QSaveFile can write on Windows (mandatory file locking)
    recordICFileRead(fileInfo.absoluteFilePath(), bytes);

    const auto definition = ICDefinition::acquire(fileInfo.absoluteFilePath(), bytes);
    const QVersionNumber version = definition->version();
    QMap<QString, QByteArray> fileRegistry = definition->blobRegistry();

    QHash<quint64, Port *> portMap;
    SerializationContext subCtx = {.portMap = portMap, .version = version, .purpose = SerializationPurpose::PortableFile, .contextDir = fileInfo.absolutePath(), .fallBackOnFailedFiles = s_icRestoring};
    subCtx.blobRegistry = fileRegistry.isEmpty() ? nullptr : &fileRegistry;
    QByteArray records = definition->records();
    QDataStream elementsStream(&records, QIODevice::ReadOnly);
    elementsStream.setVersion(QDataStream::Qt_5_12);
    QList<QGraphicsItem *> items = Serialization::deserialize(elementsStream, subCtx);

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

    if ((version < FormatRev::current) && Application::migrationEnabled) {
        migrateFile(fileInfo, items, version, fileRegistry);
    }

    // Parsing (and migration, if triggered) succeeded — only now is it safe to
    // clear the old internal state and apply the freshly-parsed items. If any
    // step above had thrown, resetInternalState() would never have run and the
    // IC's previous internal graph would remain intact.
    ic.resetInternalState();
    ic.m_definition = definition;
    processLoadedItems(ic, items);
    ic.m_previewContextDir = fileInfo.absolutePath();

    if (ic.label().isEmpty()) {
        ic.setLabel(fileInfo.baseName().toUpper());
    }
}

void ICLoader::migrateFile(const QFileInfo &fileInfo, const QList<QGraphicsItem *> &items,
                            const QVersionNumber &version, const QMap<QString, QByteArray> &fileRegistry)
{
    Serialization::createVersionedBackup(fileInfo.absoluteFilePath(), version);

    // Build port metadata for the migrated file header
    QVector<Element *> elements;
    for (auto *item : items) {
        if (item->type() == Element::Type) {
            if (auto *elm = qgraphicsitem_cast<Element *>(item)) {
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

    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);
    payloadStream << migrationMeta;
    Serialization::serialize(items, payloadStream, {.purpose = SerializationPurpose::PortableFile});

    QSaveFile saveFile(fileInfo.absoluteFilePath());
    if (!saveFile.open(QIODevice::WriteOnly)) {
        throw PANDACEPTION("IC migration: cannot open file for writing: %1", fileInfo.absoluteFilePath());
    }
    QDataStream outStream(&saveFile);
    Serialization::writePandaHeader(outStream);
    Serialization::writePayload(outStream, payload);
    if (!FileUtils::commitWhenWritten(saveFile)) {
        // Covered: TestICUnit::testMigrateFileCommitFailureThrows() forces this via
        // RLIMIT_FSIZE (ScopedTinyFsizeLimit) -- Qt defers write() errors, so a failed
        // write during writePayload() above only surfaces here, at the flush before the commit.
        throw PANDACEPTION("IC migration: failed to commit re-saved file: %1", fileInfo.absoluteFilePath());
    }
}

void ICLoader::processLoadedItems(IC &ic, QList<QGraphicsItem *> &items)
{
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

        auto *elm = qgraphicsitem_cast<Element *>(item);
        if (!elm) {
            continue; // LCOV_EXCL_LINE — the Connection case above already handled the only other type Serialization::deserialize() ever produces; every remaining item is a Element.
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

    loadBoundaryPorts(ic, true, pinNamesOf(ic.m_internalInputs));
    loadBoundaryPorts(ic, false, pinNamesOf(ic.m_internalOutputs));

    // --- Update visual representation ---
    // The body is the footprint the ports give, and turns about its centre; the label goes below it
    // (ICPresenter::label())
    ic.pictureReplaced();

    // The hover preview popup (ICPreviewPopup) names the IC from here on, in place of its help text (IC::helpText())
    ic.m_loaded = true;
}

void ICLoader::deserializeAndLoad(IC &ic, const QByteArray &bytes, const QString &contextDir, const QString &path)
{
    if (s_icLoadDepth >= kMaxICNestingDepth) {
        throw PANDACEPTION("IC nesting depth limit (%1) exceeded — blob may be maliciously crafted",
                           QString::number(kMaxICNestingDepth));
    }

    ++s_icLoadDepth;
    const auto depthGuard = qScopeGuard([] { --s_icLoadDepth; });

    // Parse the bytes (once, shared with the other instances that load them) before clearing state, so a corrupt input
    // leaves the IC unchanged.
    const auto definition = ICDefinition::acquire(path, bytes);

    QHash<quint64, Port *> portMap;
    QMap<QString, QByteArray> blobRegistry = definition->blobRegistry();
    SerializationContext subCtx = {.portMap = portMap, .version = definition->version(), .purpose = SerializationPurpose::PortableFile, .contextDir = contextDir, .fallBackOnFailedFiles = s_icRestoring};
    subCtx.blobRegistry = blobRegistry.isEmpty() ? nullptr : &blobRegistry;
    QByteArray records = definition->records();
    QDataStream elementsStream(&records, QIODevice::ReadOnly);
    elementsStream.setVersion(QDataStream::Qt_5_12);
    QList<QGraphicsItem *> items = Serialization::deserialize(elementsStream, subCtx);

    // See loadFileDirectly()'s itemsGuard for why this nulls connections
    // before qDeleteAll() and why it's a no-op on the success path. Unlike
    // loadFileDirectly() (whose migrateFile() call can throw between items being
    // populated and processLoadedItems() draining them), nothing on this path can
    // currently throw in that window: resetInternalState() only asserts already-
    // guaranteed invariants, and processLoadedItems()'s steps before its drain
    // loop have no throwing path of their own. The
    // Connection-first deletion order below is therefore unreached today — kept
    // for structural symmetry with loadFileDirectly()'s identical-purpose guard,
    // which genuinely needs it.
    auto itemsGuard = qScopeGuard([&items] {
        for (qsizetype i = 0; i < items.size(); ++i) {
            if (items[i] && items[i]->type() == Connection::Type) { // LCOV_EXCL_LINE
                delete items[i]; // LCOV_EXCL_LINE
                items[i] = nullptr; // LCOV_EXCL_LINE
            }
        }
        qDeleteAll(items);
    });

    // Parsing succeeded — now clear old state and apply
    ic.resetInternalState();
    ic.m_definition = definition;
    processLoadedItems(ic, items);
    ic.m_previewContextDir = contextDir;
}

ICLoader::Files ICLoader::recordFilesRead(const std::function<void()> &load)
{
    Files files;
    Files *outer = s_icFilesRead;
    s_icFilesRead = &files;
    {
        const auto restore = qScopeGuard([outer] { s_icFilesRead = outer; });
        load();
    }
    if (outer) {
        outer->insert(files);
    }
    return files;
}

void ICLoader::withCarriedFiles(const Files &files, const bool restoring, const std::function<void()> &load)
{
    Files carried = s_icFilesCarried ? *s_icFilesCarried : Files();
    carried.insert(files);
    const Files *outer = s_icFilesCarried;
    const bool outerRestoring = s_icRestoring;
    s_icFilesCarried = &carried;
    s_icRestoring = outerRestoring || restoring;
    const auto restore = qScopeGuard([outer, outerRestoring] {
        s_icFilesCarried = outer;
        s_icRestoring = outerRestoring;
    });
    load();
}

void ICLoader::restoreCapturedFiles(const std::function<void()> &load)
{
    const bool outer = s_icRestoreCaptured;
    s_icRestoreCaptured = true;
    const auto restore = qScopeGuard([outer] { s_icRestoreCaptured = outer; });
    load();
}

QPixmap ICLoader::renderPreview(const ICDefinition &definition, const QString &contextDir)
{
    // Read as a load reads it, bounded by the same nesting limit: the original Input/Output elements, which the
    // load replaced with proxy Nodes, are drawn as placed
    if (s_icLoadDepth >= kMaxICNestingDepth) {
        return {};
    }
    ++s_icLoadDepth;
    const auto depthGuard = qScopeGuard([] { --s_icLoadDepth; });

    QList<QGraphicsItem *> items;
    // Connections first: deleting an element first would delete its wires through its ports
    const auto itemsGuard = qScopeGuard([&items] {
        for (auto *&item : items) {
            if (item->type() == Connection::Type) {
                delete item;
                item = nullptr;
            }
        }
        qDeleteAll(items);
    });
    try {
        QHash<quint64, Port *> portMap;
        QMap<QString, QByteArray> blobRegistry = definition.blobRegistry();
        SerializationContext context = {.portMap = portMap, .version = definition.version(), .purpose = SerializationPurpose::PortableFile, .contextDir = contextDir, .fallBackOnFailedFiles = true};
        context.blobRegistry = blobRegistry.isEmpty() ? nullptr : &blobRegistry;
        QByteArray records = definition.records();
        QDataStream elementsStream(&records, QIODevice::ReadOnly);
        elementsStream.setVersion(QDataStream::Qt_5_12);
        items = Serialization::deserialize(elementsStream, context);
    } catch (const Pandaception &) {
        // The circuit no longer loads (a file it names is gone or broken): no preview
        return {};
    }

    return ICRenderer::generatePreviewPixmap(items);
}

void ICLoader::loadFromBlob(IC &ic, const QByteArray &blob, const QString &contextDir)
{
    qCDebug(zero) << "Loading IC from blob.";

    deserializeAndLoad(ic, blob, contextDir);
    ic.m_file.clear(); // switching to blob-backed, no file association

    qCDebug(zero) << "Finished loading IC from blob.";
}

void ICLoader::loadFileCopy(IC &ic, const QString &fileName, const QByteArray &bytes, const QString &contextDir)
{
    const QFileInfo fileInfo(ExternalFilePath::resolve(fileName, contextDir));
    deserializeAndLoad(ic, bytes, fileInfo.absolutePath(), fileInfo.absoluteFilePath());
    recordICFileRead(fileInfo.absoluteFilePath(), bytes);
    ic.m_blobName.clear();
    ic.m_file = fileInfo.absoluteFilePath();
    if (ic.label().isEmpty()) {
        ic.setLabel(fileInfo.baseName().toUpper());
    }
}

QByteArray ICLoader::packFileCopy(const QByteArray &hash, const QByteArray &bytes)
{
    QMap<QString, QByteArray> parts;
    parts.insert(QStringLiteral("hash"), hash);
    parts.insert(QStringLiteral("bytes"), bytes);
    QByteArray packed;
    QDataStream stream(&packed, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    stream << parts;
    return packed;
}

bool ICLoader::unpackFileCopy(const QByteArray &packed, QByteArray &hash, QByteArray &bytes)
{
    QByteArray data(packed);
    QDataStream stream(&data, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    const QMap<QString, QByteArray> parts = Serialization::readBoundedBlobMap(stream);
    if (!parts.contains(QStringLiteral("hash")) || !parts.contains(QStringLiteral("bytes"))) {
        return false;
    }
    hash = parts.value(QStringLiteral("hash"));
    bytes = parts.value(QStringLiteral("bytes"));
    return true;
}

QByteArray ICLoader::fileHash(const QByteArray &bytes)
{
    return QCryptographicHash::hash(bytes, QCryptographicHash::Sha256);
}

void ICLoader::loadBoundaryElement(IC &ic, Element *elm, const bool isInput)
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
        nodeElm->setLabel(boundaryLabel(elm));

        if (isInput) {
            auto *srcPort = elm->outputPort(p);
            auto *nodeInput = nodeElm->inputPort();
            if (portCount > 1 && !srcPort->sourceName().isEmpty()) {
                nodeInput->setNameSource(srcPort->nameFunction(), srcPort->sourceName());
            }
            nodeElm->setParam(ParamId::Wired, elm->elementType() == ElementType::Clock);
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
            if (portCount > 1 && !srcPort->sourceName().isEmpty()) {
                nodeOutput->setNameSource(srcPort->nameFunction(), srcPort->sourceName());
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

void ICLoader::loadBoundaryPorts(IC &ic, const bool isInput, const QVector<PinName> &names)
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
            port->setNameSource(names.at(i).name, names.at(i).sourceName);
            // Mirror required/default-status from the sub-circuit's input elements so that
            // unconnected optional inputs (e.g. enable lines) don't flag the IC as invalid
            port->setRequired(internalPorts.at(i)->isRequired());
            port->setDefaultStatus(internalPorts.at(i)->status());
            port->setStatus(internalPorts.at(i)->status());
        } else {
            ic.outputPort(i)->setNameSource(names.at(i).name, names.at(i).sourceName);
        }
    }

    qCDebug(three) << "IC" << ic.m_file << "->" << (isInput ? "Inputs" : "Outputs")
                    << "min:" << (isInput ? ic.minInputSize() : ic.minOutputSize())
                    << "max:" << (isInput ? ic.maxInputSize() : ic.maxOutputSize())
                    << "current:" << (isInput ? ic.inputSize() : ic.outputSize());
}

IC::PortMetadata ICLoader::buildPortMetadata(const QVector<Element *> &elements)
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

    // The header names the pins in the source language, so a file holds no name in the interface language
    for (auto *port : inputPorts) {
        meta.inputLabels.append(boundarySourceName(port));
    }
    for (auto *port : outputPorts) {
        meta.outputLabels.append(boundarySourceName(port));
    }

    return meta;
}
