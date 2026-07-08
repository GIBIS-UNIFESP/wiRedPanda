// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/IC.h"

#include <algorithm>

#include <QFileInfo>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/ICLoader.h"
#include "App/Element/ICRenderer.h"
#include "App/Element/ICSimulation.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

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
    // No popup teardown needed here: ICPreviewPopup tracks the pending IC through a
    // QPointer that auto-nulls on our destruction (and executeShow() guards on it), so a
    // pending/showing preview can never dereference a deleted IC. Emitting a signal from a
    // destructor is unsafe — Qt asserts "class destructor may have already run".
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
    QHash<Port *, quint64> reversePortMap;
    reversePortMap.reserve(context.portMap.size());
    for (auto it = context.portMap.constBegin(); it != context.portMap.constEnd(); ++it) {
        reversePortMap[it.value()] = it.key();
    }

    QVector<quint64> savedInputKeys, savedOutputKeys;
    for (auto *port : inputs()) {
        if (auto it = reversePortMap.constFind(port); it != reversePortMap.constEnd()) {
            savedInputKeys.append(it.value());
        }
    }
    for (auto *port : outputs()) {
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
            // Don't assign m_file until loadFile() has confirmed name actually resolves to a
            // real file — ICLoader::loadFile() itself sets ic.m_file, but only once validated,
            // so a name that resolves to neither a blob nor a file (e.g. an unrelated rename
            // elsewhere in the registry) throws cleanly without leaving m_file holding a
            // bogus non-path string.
            loadFile(name, context.contextDir);
        }
    }

    // Re-register the new ports in the outer portMap under the original keys
    // so that subsequent connection deserialization can find them.
    //
    // When the embedded sub-circuit declares fewer ports than the outer
    // element header did (malformed/fuzzed input), the leftover keys still
    // resolved to ports that loadFromBlob() destroyed in setPortSize().
    // Evict those stale entries so a later Connection::load() cannot
    // dereference the freed pointer.  Surfaced by libFuzzer.
    for (int i = 0; i < savedInputKeys.size(); ++i) {
        if (i < inputs().size()) {
            context.portMap[savedInputKeys[i]] = inputs()[i];
        } else {
            context.portMap.remove(savedInputKeys[i]);
        }
    }
    for (int i = 0; i < savedOutputKeys.size(); ++i) {
        if (i < outputs().size()) {
            context.portMap[savedOutputKeys[i]] = outputs()[i];
        } else {
            context.portMap.remove(savedOutputKeys[i]);
        }
    }
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
    ICLoader::loadFile(*this, fileName, contextDir);
}

void IC::loadFromBlob(const QByteArray &blob, const QString &contextDir)
{
    ICLoader::loadFromBlob(*this, blob, contextDir);
}

void IC::loadFromDrop(const QString &fileName, const QString &contextDir)
{
    loadFile(fileName, contextDir);
}

IC::PortMetadata IC::buildPortMetadata(const QVector<GraphicElement *> &elements)
{
    return ICLoader::buildPortMetadata(elements);
}

void IC::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    // Hide the preview popup immediately on double-click so it doesn't overlap the sub-circuit tab
    emit previewCancelRequested(this);
    event->accept();
    emit requestOpenSubCircuit(id(), m_blobName, m_file);
}

// --- Hover preview ---

QString IC::displayName() const
{
    if (!m_blobName.isEmpty()) {
        return m_blobName;
    }
    return m_file.isEmpty() ? QString() : QFileInfo(m_file).fileName();
}

bool IC::isCursorOverPort(const QPointF &localPos) const
{
    const auto ports = allPorts();
    return std::any_of(ports.cbegin(), ports.cend(), [&](const Port *port) {
        return port->contains(mapToItem(port, localPos));
    });
}

void IC::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    // The preview belongs to the IC body only — the ports are child items that
    // don't consume hover events, so the IC also receives them over the pins.
    if (isCursorOverPort(event->pos())) {
        emit previewHideRequested();
        return;
    }

    emit previewRequested(this, event->screenPos());
}

void IC::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (isCursorOverPort(event->pos())) {
        emit previewHideRequested();
        return;
    }

    // Over the body: keep tracking the cursor while a show is pending, but
    // re-arm the show when the cursor returns from a port within the same hover.
    // (The "already active" vs. "needs re-arming" distinction is made by whoever
    // drives the popup off this signal — see SceneUiBinder.)
    emit previewMoved(this, event->screenPos());
}

void IC::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    emit previewHideRequested();
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
        painter->setBrush(m_appearance.selectionBrush());
        painter->setPen(QPen(m_appearance.selectionPen(), 0.5, Qt::SolidLine));
        painter->drawRoundedRect(boundingRect(), 5, 5);
        painter->restore();
    }

    // Draw the body as vectors (crisp at any zoom) rather than blitting a fixed-resolution pixmap.
    ICRenderer::drawBody(*this, painter);
}

void IC::initializeSimulation()
{
    ICSimulation::initialize(*this);
}

void IC::updateLogic()
{
    ICSimulation::update(*this);
}

void IC::resettleCombinational()
{
    ICSimulation::resettle(*this);
}

void IC::refresh()
{
}
