// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/IC.h"

#include <QDir>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QSaveFile>
#include <QScopeGuard>
#include <QSet>
#include <QStyleOptionGraphicsItem>
#include <QSvgRenderer>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/ICPreviewPopup.h"
#include "App/Element/ICRegistry.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/UI/MainWindow.h"

namespace {

// Returns the shared IC preview popup, or nullptr if the chain isn't fully
// alive (e.g. during early init or late teardown).  The popup is created
// eagerly by MainWindow's constructor; null returns are exclusively a
// teardown-safety concern.
ICPreviewPopup *icPreviewPopup()
{
    auto *app = Application::instance();
    auto *mw  = app ? app->mainWindow() : nullptr;
    return mw ? mw->icPreviewPopup() : nullptr;
}

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

    // Enable hover events so the preview popup can be shown/hidden
    setAcceptHoverEvents(true);
}

IC::~IC()
{
    // If this IC is destroyed while the shared popup is pending or showing it,
    // cancel immediately — m_pendingIC is a QPointer so it auto-nulls, but the
    // popup could still be visible with stale content.
    if (auto *popup = icPreviewPopup(); popup && popup->pendingIC() == this) {
        popup->cancelHide();
        popup->hide();
    }
    qDeleteAll(m_internalConnections);
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
        m_file = Serialization::readBoundedString(stream);

        // Old files may have stored absolute paths from the original machine; keep only the
        // filename so we can resolve it relative to the current context directory.
        // Normalize backslashes first — QFileInfo::fileName() doesn't treat '\' as a
        // separator on Unix, so a Windows path like "C:\...\sub.panda" would be kept as-is.
        m_file = QFileInfo(QString(m_file).replace('\\', '/')).fileName();

        loadFile(m_file, context.contextDir);
    }

    // New format (V_4_1+): IC data stored in a QMap for extensibility
    if (VersionInfo::hasQMapFormat(context.version)) {
        QMap<QString, QVariant> map = Serialization::readBoundedMetadata(stream);

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
    //
    // When the embedded sub-circuit declares fewer ports than the outer
    // element header did (malformed/fuzzed input), the leftover keys still
    // resolved to ports that loadFromBlob() destroyed in setPortSize().
    // Evict those stale entries so a later QNEConnection::load() cannot
    // dereference the freed pointer.  Surfaced by libFuzzer.
    for (int i = 0; i < savedInputKeys.size(); ++i) {
        if (i < m_inputPorts.size()) {
            context.portMap[savedInputKeys[i]] = m_inputPorts[i];
        } else {
            context.portMap.remove(savedInputKeys[i]);
        }
    }
    for (int i = 0; i < savedOutputKeys.size(); ++i) {
        if (i < m_outputPorts.size()) {
            context.portMap[savedOutputKeys[i]] = m_outputPorts[i];
        } else {
            context.portMap.remove(savedOutputKeys[i]);
        }
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
    // No setInputSize(0)/setOutputSize(0) here: after the first load,
    // loadBoundaryPorts locks min == max, making those calls silent no-ops —
    // the boundary port counts are re-established by the next load anyway.
    // Clear derived simulation state BEFORE freeing the elements those
    // vectors reference. If we freed first, a simulation tick between the
    // free and the clear (via Application::notify + QMessageBox spinning
    // a nested event loop) would dereference dangling pointers.
    m_sortedInternalElements.clear();
    m_boundaryInputElements.clear();
    m_internalHasFeedback = false;
    qDeleteAll(m_internalConnections);
    m_internalConnections.clear();
    qDeleteAll(m_internalElements);
    m_internalElements.clear();
    // Bug 1 / Bug 3 postcondition: every owned vector must be cleared atomically
    // with its parent. If a future change adds a new internal vector and forgets
    // to clear it here, this assert trips immediately.
    Q_ASSERT(m_internalElements.isEmpty());
    Q_ASSERT(m_sortedInternalElements.isEmpty());
    Q_ASSERT(m_internalConnections.isEmpty());
    Q_ASSERT(m_internalInputs.isEmpty());
    Q_ASSERT(m_internalOutputs.isEmpty());
    Q_ASSERT(m_boundaryInputElements.isEmpty());
}

void IC::loadFile(const QString &fileName, const QString &contextDir)
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
    m_blobName.clear();

    // Use cached file bytes from ICRegistry when available (avoids re-reading from disk)
    if (auto *scene_ = qobject_cast<Scene *>(scene())) {
        auto *reg = scene_->icRegistry();
        const QByteArray &cached = reg->cachedFileBytes(fileInfo.absoluteFilePath());
        if (!cached.isEmpty()) {
            deserializeAndLoad(cached, fileInfo.absolutePath());
            m_file = fileInfo.absoluteFilePath();
            setToolTip(fileInfo.fileName());
            if (label().isEmpty()) {
                setLabel(fileInfo.baseName().toUpper());
            }
            qCDebug(zero) << "Finished reading IC (via cache).";
            return;
        }
    }

    // Fallback: direct file load (IC not yet in a scene, e.g. during deserialization).
    // loadFileDirectly() mirrors deserializeAndLoad()'s parse-first, reset-after shape:
    // a failed parse (corrupt file, missing dependency, circular reference) propagates
    // without ever leaving m_sortedInternalElements pointing at freed elements.
    loadFileDirectly(fileInfo);
    m_file = fileInfo.absoluteFilePath();
    setToolTip(fileInfo.fileName());

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
    auto fileRegistry = Serialization::deserializeBlobRegistry(preamble.metadata, preamble.version);

    QMap<quint64, QNEPort *> portMap;
    SerializationContext subCtx{portMap, preamble.version, fileInfo.absolutePath()};
    subCtx.blobRegistry = fileRegistry.isEmpty() ? nullptr : &fileRegistry;
    const auto items = Serialization::deserialize(stream, subCtx);
    file.close(); // must be closed before QSaveFile can write on Windows (mandatory file locking)

    if ((preamble.version < FormatRev::current) && Application::migrationEnabled) {
        migrateFile(fileInfo, items, preamble.version, fileRegistry);
    }

    // Parsing (and migration, if triggered) succeeded — only now is it safe to
    // clear the old internal state and apply the freshly-parsed items. If any
    // step above had thrown, resetInternalState() would never have run and the
    // IC's previous internal graph would remain intact.
    resetInternalState();
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
    // Snapshot the preview now, while the original Input/Output elements (buttons,
    // switches, LEDs, …) are still alive in `items`.  loadBoundaryElement() below
    // replaces each of them with a proxy Node, so a later render would only see
    // the simulation graph.
    generatePreviewPixmap(items);

    for (auto *item : items) {
        if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
            m_internalConnections.append(conn);
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

// Maximum nesting depth for IC-within-IC blob loading.
// Each level deserializes a full panda stream; deep nesting exhausts the call stack.
static constexpr int kMaxICNestingDepth = 16;
static thread_local int s_icLoadDepth = 0;

void IC::deserializeAndLoad(const QByteArray &bytes, const QString &contextDir)
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

    QMap<quint64, QNEPort *> portMap;
    SerializationContext subCtx{portMap, preamble.version, contextDir};
    subCtx.blobRegistry = blobRegistry.isEmpty() ? nullptr : &blobRegistry;
    const auto items = Serialization::deserialize(stream, subCtx);

    // Parsing succeeded — now clear old state and apply
    resetInternalState();
    processLoadedItems(items);
}

void IC::loadFromBlob(const QByteArray &blob, const QString &contextDir)
{
    qCDebug(zero) << "Loading IC from blob.";

    deserializeAndLoad(blob, contextDir);
    m_file.clear(); // switching to blob-backed, no file association

    if (!m_blobName.isEmpty()) {
        setToolTip(m_blobName);
    }

    qCDebug(zero) << "Finished loading IC from blob.";
}

/// Shared, lazily-constructed vector renderer for the IC mascot logo — one per process, drawn
/// directly in drawBody() so the logo stays crisp at any zoom. GUI-thread only, like pixmapCache().
static QSvgRenderer &icLogoRenderer()
{
    static QSvgRenderer renderer(QStringLiteral(":/Components/Logic/ic-panda2.svg"));
    return renderer;
}

void IC::generatePixmap()
{
    // The body is now drawn as vectors in drawBody()/paint(); m_pixmap is kept only so that the
    // base pixmapCenter()/boundingRect() have the right size (its image content is never displayed).
    // It must encompass both the 64×64 body and any ports that extend beyond it.
    const QSize size = portsBoundingRect().united(QRectF(0, 0, 64, 64)).size().toSize();
    QPixmap sizingPixmap(size);
    sizingPixmap.fill(Qt::transparent);
    m_pixmap = sizingPixmap;
    update();
}

void IC::drawBody(QPainter *painter)
{
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing, true);
    // boundingRect()'s top-left may be negative when ports extend past the 64×64 body; align the
    // local origin with it so the body lands exactly where the old rasterised pixmap was blitted.
    painter->translate(boundingRect().topLeft());
    // The body footprint is the (correctly-sized) m_pixmap rect — exactly the area the old raster
    // occupied — so the geometry is reproduced 1:1 at any zoom.
    const QRectF bounds(pixmap().rect());

    const QColor bodyColor = isEmbedded() ? QColor(90, 126, 160) : QColor(126, 126, 126);
    const QColor outlineColor = isEmbedded() ? QColor(58, 82, 110) : QColor(78, 78, 78);

    // IC body: styled like a physical DIP package. 7px inset on each side (14px total) so the port
    // connector dots visually overlap the border, matching the TruthTable and physical DIP look.
    painter->setBrush(bodyColor);
    painter->setPen(QPen(QBrush(outlineColor), 0.5, Qt::SolidLine));
    const QRectF finalRect(QPointF(7, 0), QSizeF(bounds.width() - 14, bounds.height()));
    painter->drawRoundedRect(finalRect, 3, 3);

    // Centre the wiRedPanda mascot logo on the body, rendered as vectors at its native size.
    QSvgRenderer &logo = icLogoRenderer();
    const QSizeF logoSize = logo.defaultSize();
    const QRectF logoRect(finalRect.center() - QPointF(logoSize.width() / 2, logoSize.height() / 2), logoSize);
    logo.render(painter, logoRect);

    // Thin dark strip at the bottom edge to simulate the package shadow/bevel.
    painter->setBrush(outlineColor);
    painter->setPen(QPen(QBrush(outlineColor), 0.5, Qt::SolidLine));
    QRectF shadowRect(finalRect.bottomLeft(), finalRect.bottomRight());
    shadowRect.adjust(0, -3, 0, 0);
    painter->drawRoundedRect(shadowRect, 3, 3);

    // Orientation notch (semicircle) at the top centre, matching the physical DIP pin-1 convention.
    // drawChord angle parameters are in 1/16th-degree units; -180*16 = lower half-circle.
    const QRectF topCenter(finalRect.topLeft() + QPointF(18, -12), QSizeF(24, 24));
    painter->drawChord(topCenter, 0, -180 * 16);

    painter->restore();
}

void IC::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    // Hide the preview popup immediately on double-click so it doesn't overlap the sub-circuit tab
    if (auto *popup = icPreviewPopup()) {
        popup->cancelHide();
        popup->hide();
    }
    event->accept();
    emit requestOpenSubCircuit(id(), m_blobName, m_file);
}

// --- Hover preview ---

void IC::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (auto *popup = icPreviewPopup()) {
        popup->showForIC(this, event->screenPos());
    }
}

void IC::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (auto *popup = icPreviewPopup()) {
        popup->updatePendingPos(event->screenPos());
    }
}

void IC::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    if (auto *popup = icPreviewPopup()) {
        popup->scheduleHide();
    }
}

void IC::generatePreviewPixmap(const QList<QGraphicsItem *> &items)
{
    // Split the freshly-deserialized items into elements and connections.  The
    // boundary Input/Output elements are still in their designed form here; the
    // substitution to proxy Nodes happens later in processLoadedItems().
    QVector<GraphicElement *> elements;
    QVector<QNEConnection *> connections;
    elements.reserve(items.size());
    connections.reserve(items.size());
    for (auto *item : items) {
        if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
            connections.append(conn);
        } else if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
            elements.append(elm);
        }
    }

    // Skip for empty or very large circuits.
    if (elements.isEmpty() || elements.size() > ICPreviewPopup::MaxElementCount) {
        m_previewPixmap = QPixmap();
        return;
    }

    // Temporarily borrow the items into a throwaway scene so QGraphicsScene::render()
    // can be used without disturbing the real scene.  The scope guard guarantees
    // cleanup even if render() throws.
    QGraphicsScene tempScene;
    tempScene.setBackgroundBrush(QColor(42, 42, 42));

    auto cleanup = qScopeGuard([&] {
        for (auto *elm  : std::as_const(elements))    { tempScene.removeItem(elm);  }
        for (auto *conn : std::as_const(connections)) { tempScene.removeItem(conn); }
    });

    for (auto *elm : std::as_const(elements)) {
        tempScene.addItem(elm);
    }
    for (auto *conn : std::as_const(connections)) {
        tempScene.addItem(conn);
    }

    // Compute the bounding rect with some padding
    const QRectF sourceRect = tempScene.itemsBoundingRect().adjusted(-16, -16, 16, 16);

    // Scale to fit within max preview dimensions while preserving aspect ratio
    QSize targetSize = sourceRect.size().toSize();
    targetSize.scale(ICPreviewPopup::MaxWidth, ICPreviewPopup::MaxHeight, Qt::KeepAspectRatio);

    if (targetSize.isEmpty()) {
        m_previewPixmap = QPixmap();
        return;
    }

    // QPixmap(QSize) is uninitialised; tempScene.render() paints the background
    // brush over the source→target affine, but subpixel rounding can leave a
    // 1-pixel sliver unpainted at the right/bottom edge, exposing whatever was
    // in memory (commonly white on Windows).  Fill explicitly to avoid that.
    QPixmap preview(targetSize);
    preview.fill(QColor(42, 42, 42));

    QPainter painter(&preview);
    painter.setRenderHint(QPainter::Antialiasing);
    tempScene.render(&painter, QRectF(), sourceRect);
    painter.end();

    m_previewPixmap = preview;
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

    // Draw the body as vectors (crisp at any zoom) rather than blitting a fixed-resolution pixmap.
    drawBody(painter);
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
            if (element) {
                element->updateLogic();
            }
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
