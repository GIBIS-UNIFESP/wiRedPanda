// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/ICRegistry.h"

#include <algorithm>
#include <utility>

#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsItem>
#include <QSaveFile>
#include <QScopeGuard>
#include <QSet>
#include <QThread>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/ExternalFilePath.h"
#include "App/IO/VersionInfo.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Versions.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace {

struct BoundaryInput {
    OutputPort *source = nullptr;
    InputPort *destination = nullptr;
    GraphicElement *destinationElement = nullptr;
    OutputPort *boundaryPort = nullptr;
};

struct BoundaryOutput {
    OutputPort *source = nullptr;
    QList<InputPort *> destinations;
    InputPort *boundaryPort = nullptr;
};

QRectF elementContentsRect(const GraphicElement *element)
{
    return element->boundingRect().united(element->childrenBoundingRect());
}

void placeBoundaryElement(GraphicElement *element, const qreal left, const qreal centerY)
{
    const QRectF contents = elementContentsRect(element);
    element->setPos(QPointF(left - contents.left(), centerY - contents.center().y()));
}

qreal boundaryRowCenter(const qsizetype index, const qsizetype count, const qreal centerY)
{
    constexpr qreal rowSpacing = 112.0;
    return centerY + (static_cast<qreal>(index) - static_cast<qreal>(count - 1) / 2.0) * rowSpacing;
}

bool portPositionLess(const Port *first, const Port *second)
{
    auto *firstElement = first->graphicElement();
    auto *secondElement = second->graphicElement();
    if (firstElement->pos() != secondElement->pos()) {
        const QPointF firstPos = firstElement->pos();
        const QPointF secondPos = secondElement->pos();
        return firstPos.y() < secondPos.y()
            || (qFuzzyCompare(firstPos.y(), secondPos.y()) && firstPos.x() < secondPos.x());
    }
    const QPointF firstPos = first->pos();
    const QPointF secondPos = second->pos();
    return firstPos.x() < secondPos.x()
        || (qFuzzyCompare(firstPos.x(), secondPos.x()) && firstPos.y() < secondPos.y());
}

bool isGeneratedBoundary(const GraphicElement *element, const QString &prefix)
{
    // The explicit marker is authoritative for new blobs; old blobs have no marker, so
    // recognize the generated IN1/OUT1-style labels written by earlier selection-to-IC code.
    const QVariant marker = element->data(GraphicElement::GeneratedICBoundaryDataRole);
    if (marker.isValid()) {
        return marker.toBool();
    }
    const QString label = element->label();
    if (!label.startsWith(prefix)) {
        return false;
    }
    bool ok = false;
    const int index = label.mid(prefix.size()).toInt(&ok);
    return ok && index > 0;
}

QByteArray makeCircuitBlob(const QList<QGraphicsItem *> &items,
                           const QMap<QString, QByteArray> &blobs,
                           const QRectF &sceneRect)
{
    // Keep the source selection's bounds in metadata: the IC is anchored at this center,
    // and flattening uses it to place the expanded elements relative to the moved instance.
    QMap<QString, QVariant> metadata;
    metadata["dolphinFileName"] = QString();
    metadata["sceneRect"] = sceneRect;
    Serialization::serializeBlobRegistry(blobs, metadata);
    QStringList fileBackedICs;
    for (auto *item : items) {
        auto *element = qgraphicsitem_cast<GraphicElement *>(item);
        if (!element || element->elementType() != ElementType::IC || element->isEmbedded()) continue;
        const auto *ic = static_cast<const IC *>(element);
        if (ic->file().isEmpty()) continue;
        const QString path = ExternalFilePath::resolve(ic->file(), {});
        if (!fileBackedICs.contains(path)) fileBackedICs.append(path);
    }
    if (!fileBackedICs.isEmpty()) metadata["fileBackedICs"] = fileBackedICs;

    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);
    payloadStream << metadata;
    Serialization::serialize(items, payloadStream, {.purpose = SerializationPurpose::PortableFile});

    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    Serialization::writePayload(stream, payload);
    return bytes;
}

QList<QGraphicsItem *> deserializeCircuitBlob(const QByteArray &bytes, Scene *scene,
                                               const QString &contextDir,
                                               QMap<QString, QByteArray> &blobRegistry,
                                               QPointF *origin = nullptr)
{
    QByteArray data(bytes);
    QDataStream stream(&data, QIODevice::ReadOnly);
    auto preamble = Serialization::readPreamble(stream);
    blobRegistry = Serialization::deserializeBlobRegistry(preamble.metadata, preamble.version);
    if (origin) {
        // ICs generated from a selection are anchored at the captured bounds center.
        *origin = preamble.metadata.value("sceneRect").toRectF().center();
    }

    QHash<quint64, Port *> portMap;
    auto context = scene->deserializationContext(portMap, preamble.version, SerializationPurpose::PortableFile);
    context.contextDir = contextDir;
    context.blobRegistry = blobRegistry.isEmpty() ? nullptr : &blobRegistry;
    QDataStream elementsStream(&preamble.remainingPayload, QIODevice::ReadOnly);
    elementsStream.setVersion(QDataStream::Qt_5_12);
    return Serialization::deserialize(elementsStream, context);
}

void deleteCircuitItems(QList<QGraphicsItem *> &items)
{
    for (auto &item : items) {
        if (item->type() == Connection::Type) {
            delete item;
            item = nullptr;
        }
    }
    qDeleteAll(items);
    items.clear();
}

} // namespace

ICRegistry::ICRegistry(Scene *scene)
    : QObject(scene)
    , m_scene(scene)
{
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &ICRegistry::onFileChanged, Qt::QueuedConnection);
}

const QByteArray &ICRegistry::cachedFileBytes(const QString &filePath)
{
    Q_ASSERT(QCoreApplication::instance()->thread() == QThread::currentThread());

    if (!m_fileCache.contains(filePath)) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            m_fileCache[filePath] = file.readAll();
        } else {
            // Do NOT insert into m_fileCache on failure: QMap::operator[] would
            // default-construct and permanently cache an empty entry, silently
            // masking the failure on every subsequent lookup (including if the
            // file becomes readable later).
            qCWarning(zero) << "ICRegistry: cannot open IC file:" << filePath;
            static const QByteArray empty;
            return empty;
        }
    }
    return m_fileCache[filePath];
}

void ICRegistry::invalidate(const QString &filePath)
{
    m_fileCache.remove(filePath);
}

void ICRegistry::watchFile(const QString &filePath)
{
    if (!m_fileWatcher.files().contains(filePath)) {
        m_fileWatcher.addPath(filePath);
    }
}

QList<GraphicElement *> ICRegistry::findICsByFile(const QString &fileName) const
{
    const QFileInfo target(fileName);
    QList<GraphicElement *> result;
    for (auto *elm : m_scene->elements()) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = static_cast<IC *>(elm);
            if (QFileInfo(ic->file()) == target) {
                result.append(elm);
            }
        }
    }
    return result;
}

void ICRegistry::onFileChanged(const QString &filePath)
{
    // Guarded because this is a queued slot that deliberately rethrows below, so an escaping
    // exception crosses Qt's signal-slot dispatch -- where the upstream catch in
    // Application::notify cannot be relied on. On macOS it is structurally unreachable (Qt's
    // Exception Safety documentation, and QTBUG-15197): the unwinder reaches std::terminate
    // mid-stack before any upstream frame runs. Debian's own buildd reproduces the same abort
    // natively on armhf, hppa and sparc64 with Qt 6.10.2 (issue #525) -- no emulation
    // involved -- while amd64 catches the same throw and carries on. Only the behaviour is
    // established off amd64, not the mechanism. Deleting a watched IC file therefore
    // terminated the application on those platforms instead of reporting the load error.
    Application::guardedSlot(this, [this, &filePath] {
        qCDebug(zero) << "IC file changed:" << filePath;

        // Invalidate the cached definition so it's rebuilt on next access
        invalidate(filePath);

        // Re-add the watch (some OS remove it after a file change event)
        if (!m_fileWatcher.files().contains(filePath) && QFileInfo::exists(filePath)) {
            m_fileWatcher.addPath(filePath);
        }

        // Reload all IC instances referencing this file
        const auto targets = findICsByFile(filePath);
        if (targets.isEmpty()) {
            emit definitionChanged(filePath);
            return;
        }

        // Capture pre-reload state so the undo command can restore both the
        // ICs' element data and the scene wires that touch their ports.
        // Without this the wires get cascade-deleted by setInputSize/setOutputSize
        // inside loadFile and Cluster D throws on the next undo lookup.
        const auto connections = UpdateBlobCommand::captureConnections(targets);
        const QByteArray oldData = captureSnapshot(targets);

        try {
            reloadTargetsAtomically(targets, oldData, [&](IC *ic) { ic->loadFile(filePath); });
        } catch (...) {
            m_scene->setCircuitUpdateRequired();
            emit definitionChanged(filePath);
            throw;
        }
        m_scene->setCircuitUpdateRequired();

        auto *cmd = new UpdateBlobCommand(targets, oldData, connections, m_scene);
        m_scene->undoStack()->push(cmd);

        emit definitionChanged(filePath);
    });
}

// --- Embedded IC blob storage ---

bool ICRegistry::hasBlob(const QString &name) const
{
    return m_blobs.contains(name);
}

QByteArray ICRegistry::blob(const QString &name) const
{
    return m_blobs.value(name);
}

void ICRegistry::setBlob(const QString &name, const QByteArray &data)
{
    m_blobs[name] = data;
}

void ICRegistry::registerBlob(const QString &name, const QByteArray &data)
{
    QSet<QString> visited;
    QMap<QString, QByteArray> workingBlobs;
    workingBlobs[name] = data;
    makeBlobSelfContained(name, visited, workingBlobs);
    m_blobs[name] = workingBlobs[name];
}

void ICRegistry::removeBlob(const QString &name)
{
    m_blobs.remove(name);
}

void ICRegistry::renameBlob(const QString &oldName, const QString &newName)
{
    // Defense in depth: the primary guard is at the caller (ElementEditor rejects a colliding
    // rename before ever constructing a command), but no-op here too rather than silently
    // overwriting an unrelated blob's bytes, so any future/other caller can't corrupt one IC's
    // data by renaming a different one onto it.
    if (!m_blobs.contains(oldName) || oldName == newName || m_blobs.contains(newName)) {
        return;
    }

    m_blobs[newName] = m_blobs.take(oldName);

    // Update all IC instances on the scene referencing the old name
    for (auto *elm : m_scene->elements()) {
        if (elm->isEmbedded() && elm->blobName() == oldName) {
            auto *ic = static_cast<IC *>(elm);
            ic->setBlobName(newName);
        }
    }

    // Update embedded IC references inside other blobs' metadata so that parent
    // blobs that contain the renamed blob as a nested dependency stay consistent.
    for (auto it = m_blobs.begin(); it != m_blobs.end(); ++it) {
        renameBlobReference(it.value(), oldName, newName);
    }

    emit blobRenamed(oldName, newName);
}

void ICRegistry::clearBlobs()
{
    m_blobs.clear();
}

QList<GraphicElement *> ICRegistry::findICsByBlobName(const QString &blobName) const
{
    QList<GraphicElement *> result;
    for (auto *elm : m_scene->elements()) {
        if (elm->isEmbedded() && elm->blobName() == blobName) {
            result.append(elm);
        }
    }
    return result;
} // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned QList<GraphicElement *>, never reached after the return above.

bool ICRegistry::initEmbeddedIC(IC *ic, const QString &blobName)
{
    if (!hasBlob(blobName)) {
        return false;
    }
    ic->setBlobName(blobName);
    ic->loadFromBlob(blob(blobName), m_scene->contextDir());
    if (ic->label().isEmpty()) {
        ic->setLabel(blobName.toUpper());
    }
    return true;
}

QString ICRegistry::uniqueBlobName(const QString &baseName) const
{
    if (!hasBlob(baseName)) {
        return baseName;
    }
    for (int i = 2; ; ++i) {
        const QString candidate = baseName + "_" + QString::number(i);
        if (!hasBlob(candidate)) {
            return candidate;
        }
    }
}

IC *ICRegistry::createEmbeddedIC(const QString &blobName, const QByteArray &fileBytes, const QString &contextDir)
{
    auto *ic = new IC();
    ic->setBlobName(blobName);

    m_scene->undoStack()->beginMacro(QCoreApplication::tr("Add embedded IC"));
    m_scene->receiveCommand(new RegisterBlobCommand(blobName, fileBytes, m_scene));
    ic->loadFromBlob(fileBytes, contextDir);
    m_scene->receiveCommand(new AddItemsCommand({ic}, m_scene));
    m_scene->undoStack()->endMacro();

    return ic;
}

IC *ICRegistry::createICFromSelection(const QList<GraphicElement *> &targets)
{
    if (targets.isEmpty()) {
        throw PANDACEPTION("Select at least one element to create an IC.");
    }

    QSet<GraphicElement *> selected;
    QRectF bounds;
    bool firstBounds = true;
    for (auto *element : targets) {
        if (!element || element->scene() != m_scene || selected.contains(element)) {
            throw PANDACEPTION("The IC selection contains an invalid element.");
        }
        selected.insert(element);
        bounds = firstBounds ? element->sceneBoundingRect() : bounds.united(element->sceneBoundingRect());
        firstBounds = false;
    }

    QList<BoundaryInput> inputs;
    QHash<OutputPort *, qsizetype> outputIndex;
    QList<BoundaryOutput> outputs;
    QSet<Connection *> internalConnections;

    for (auto *element : targets) {
        for (auto *port : element->inputs()) {
            for (auto *connection : port->connections()) {
                auto *source = connection->startPort();
                if (!source || !source->graphicElement()) continue;
                if (selected.contains(source->graphicElement())) {
                    internalConnections.insert(connection);
                } else {
                    inputs.append({source, port, element});
                }
            }
        }
        for (auto *port : element->outputs()) {
            for (auto *connection : port->connections()) {
                auto *destination = connection->endPort();
                if (!destination || !destination->graphicElement()) continue;
                if (selected.contains(destination->graphicElement())) {
                    internalConnections.insert(connection);
                } else {
                    const auto found = outputIndex.constFind(port);
                    if (found == outputIndex.constEnd()) {
                        outputIndex.insert(port, outputs.size());
                        outputs.append({port, {destination}});
                    } else {
                        outputs[found.value()].destinations.append(destination);
                    }
                }
            }
        }
    }

    std::stable_sort(inputs.begin(), inputs.end(), [](const BoundaryInput &a, const BoundaryInput &b) {
        return portPositionLess(a.destination, b.destination);
    });
    std::stable_sort(outputs.begin(), outputs.end(), [](const BoundaryOutput &a, const BoundaryOutput &b) {
        return portPositionLess(a.source, b.source);
    });

    QList<QGraphicsItem *> blobItems;
    QList<QGraphicsItem *> generatedBlobItems;
    auto generatedItemsGuard = qScopeGuard([&generatedBlobItems] { deleteCircuitItems(generatedBlobItems); });
    for (auto *element : targets) {
        if (element->elementGroup() == ElementGroup::Input || element->elementGroup() == ElementGroup::Output) {
            element->setData(GraphicElement::GeneratedICBoundaryDataRole, false);
        }
        blobItems.append(element);
    }
    for (auto *connection : std::as_const(internalConnections)) blobItems.append(connection);

    constexpr qreal boundaryGap = 32.0;
    const qreal boundaryLeft = bounds.left() - boundaryGap;
    for (int i = 0; i < inputs.size(); ++i) {
        auto *boundary = ElementFactory::buildElement(ElementType::InputSwitch);
        boundary->setLabel(QStringLiteral("IN%1").arg(i + 1));
        boundary->setData(GraphicElement::GeneratedICBoundaryDataRole, true);
        const QRectF contents = elementContentsRect(boundary);
        placeBoundaryElement(boundary, boundaryLeft - contents.width(),
                             boundaryRowCenter(i, inputs.size(), bounds.center().y()));
        inputs[i].boundaryPort = boundary->outputPort(0);
        blobItems.append(boundary);
        generatedBlobItems.append(boundary);

        auto *connection = new Connection();
        connection->setStartPort(boundary->outputPort(0));
        connection->setEndPort(inputs[i].destination);
        blobItems.append(connection);
        generatedBlobItems.append(connection);
    }

    const qreal outputBoundaryLeft = bounds.right() + boundaryGap;
    for (int i = 0; i < outputs.size(); ++i) {
        auto *boundary = ElementFactory::buildElement(ElementType::Led);
        boundary->setLabel(QStringLiteral("OUT%1").arg(i + 1));
        boundary->setData(GraphicElement::GeneratedICBoundaryDataRole, true);
        placeBoundaryElement(boundary, outputBoundaryLeft,
                             boundaryRowCenter(i, outputs.size(), bounds.center().y()));
        outputs[i].boundaryPort = boundary->inputPort(0);
        blobItems.append(boundary);
        generatedBlobItems.append(boundary);

        auto *connection = new Connection();
        connection->setStartPort(outputs[i].source);
        connection->setEndPort(boundary->inputPort(0));
        blobItems.append(connection);
        generatedBlobItems.append(connection);
    }

    QVector<OutputPort *> inputBoundaryPorts;
    QVector<InputPort *> outputBoundaryPorts;
    for (auto *item : std::as_const(blobItems)) {
        auto *element = qgraphicsitem_cast<GraphicElement *>(item);
        if (!element) continue;
        if (element->elementGroup() == ElementGroup::Input) {
            for (auto *port : element->outputs()) inputBoundaryPorts.append(port);
        } else if (element->elementGroup() == ElementGroup::Output) {
            for (auto *port : element->inputs()) outputBoundaryPorts.append(port);
        }
    }
    std::stable_sort(inputBoundaryPorts.begin(), inputBoundaryPorts.end(), portPositionLess);
    std::stable_sort(outputBoundaryPorts.begin(), outputBoundaryPorts.end(), portPositionLess);

    // The loader sorts interface ports by element position. Build the same pointer-to-index
    // mapping here so wires crossing the selection attach to the corresponding IC pin even
    // when selected boundary elements and generated boundaries are mixed.
    QHash<OutputPort *, int> inputBoundaryIndices;
    for (int i = 0; i < inputBoundaryPorts.size(); ++i) {
        inputBoundaryIndices.insert(inputBoundaryPorts.at(i), i);
    }
    QHash<InputPort *, int> outputBoundaryIndices;
    for (int i = 0; i < outputBoundaryPorts.size(); ++i) {
        outputBoundaryIndices.insert(outputBoundaryPorts.at(i), i);
    }
    for (const auto &input : std::as_const(inputs)) {
        if (!inputBoundaryIndices.contains(input.boundaryPort)) {
            throw PANDACEPTION("Generated IC input boundary was not found.");
        }
    }
    for (const auto &output : std::as_const(outputs)) {
        if (!outputBoundaryIndices.contains(output.boundaryPort)) {
            throw PANDACEPTION("Generated IC output boundary was not found.");
        }
    }

    const QString blobName = uniqueBlobName(QStringLiteral("IC"));
    const QByteArray originalBlob = makeCircuitBlob(blobItems, m_blobs, bounds);
    deleteCircuitItems(generatedBlobItems);
    generatedItemsGuard.dismiss();
    registerBlob(blobName, originalBlob);
    const QByteArray selfContainedBlob = blob(blobName);
    removeBlob(blobName);

    auto *ic = new IC();
    try {
        ic->setBlobName(blobName);
        ic->setLabel(blobName);
        ic->setPos(bounds.center());
        ic->loadFromBlob(selfContainedBlob, m_scene->contextDir());
        if (ic->inputSize() != inputBoundaryPorts.size() || ic->outputSize() != outputBoundaryPorts.size()) {
            throw PANDACEPTION("Generated IC boundary ports do not match the selected circuit.");
        }
    } catch (...) {
        delete ic;
        throw;
    }

    QList<QGraphicsItem *> replacementItems{ic};
    QList<QGraphicsItem *> itemsToDelete;
    itemsToDelete.reserve(targets.size());
    for (auto *element : targets) itemsToDelete.append(element);

    m_scene->undoStack()->beginMacro(QCoreApplication::tr("Create IC from Selection"));
    m_scene->receiveCommand(new RegisterBlobCommand(blobName, selfContainedBlob, m_scene));
    m_scene->receiveCommand(new DeleteItemsCommand(itemsToDelete, m_scene));

    for (int i = 0; i < inputs.size(); ++i) {
        auto *connection = new Connection();
        connection->setStartPort(inputs[i].source);
        connection->setEndPort(ic->inputPort(inputBoundaryIndices.value(inputs[i].boundaryPort)));
        replacementItems.append(connection);
    }
    for (int i = 0; i < outputs.size(); ++i) {
        for (auto *destination : outputs[i].destinations) {
            auto *connection = new Connection();
            connection->setStartPort(ic->outputPort(outputBoundaryIndices.value(outputs[i].boundaryPort)));
            connection->setEndPort(destination);
            replacementItems.append(connection);
        }
    }
    m_scene->receiveCommand(new AddItemsCommand(replacementItems, m_scene));
    m_scene->undoStack()->endMacro();
    m_scene->setCircuitUpdateRequired();
    return ic;
}

void ICRegistry::flattenIC(IC *ic)
{
    if (!ic || ic->scene() != m_scene) {
        throw PANDACEPTION("The IC to flatten is not in this scene.");
    }
    const bool wasEmbedded = ic->isEmbedded();
    const QString blobName = ic->blobName();

    QByteArray bytes;
    QString contextDir = m_scene->contextDir();
    if (ic->isEmbedded()) {
        bytes = blob(ic->blobName());
        if (bytes.isEmpty()) {
            throw PANDACEPTION("Embedded IC definition \"%1\" was not found.", ic->blobName());
        }
    } else {
        const QString filePath = ExternalFilePath::resolve(ic->file(), contextDir);
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            throw PANDACEPTION("Could not open IC file: %1", file.errorString());
        }
        bytes = file.readAll();
        contextDir = QFileInfo(filePath).absolutePath();
    }

    QMap<QString, QByteArray> nestedBlobs;
    QPointF definitionOrigin;
    QList<QGraphicsItem *> expandedItems = deserializeCircuitBlob(bytes, m_scene, contextDir, nestedBlobs,
                                                                   &definitionOrigin);
    auto cleanup = qScopeGuard([&expandedItems] { deleteCircuitItems(expandedItems); });

    QVector<OutputPort *> boundaryInputs;
    QVector<InputPort *> boundaryOutputs;
    for (auto *item : std::as_const(expandedItems)) {
        auto *element = qgraphicsitem_cast<GraphicElement *>(item);
        if (!element) continue;
        if (element->elementGroup() == ElementGroup::Input) {
            for (auto *port : element->outputs()) boundaryInputs.append(port);
        } else if (element->elementGroup() == ElementGroup::Output) {
            for (auto *port : element->inputs()) boundaryOutputs.append(port);
        }
    }
    std::stable_sort(boundaryInputs.begin(), boundaryInputs.end(), portPositionLess);
    std::stable_sort(boundaryOutputs.begin(), boundaryOutputs.end(), portPositionLess);

    if (boundaryInputs.size() != ic->inputSize() || boundaryOutputs.size() != ic->outputSize()) {
        throw PANDACEPTION("IC definition boundary ports do not match the IC instance.");
    }

    struct ExternalInput {
        OutputPort *source;
        int portIndex;
    };
    struct ExternalOutput {
        InputPort *destination;
        int portIndex;
    };
    QList<ExternalInput> externalInputs;
    QList<ExternalOutput> externalOutputs;
    for (int i = 0; i < ic->inputSize(); ++i) {
        for (auto *connection : ic->inputPort(i)->connections()) {
            if (connection->startPort()) externalInputs.append({connection->startPort(), i});
        }
    }
    for (int i = 0; i < ic->outputSize(); ++i) {
        for (auto *connection : ic->outputPort(i)->connections()) {
            if (connection->endPort()) externalOutputs.append({connection->endPort(), i});
        }
    }

    QSet<GraphicElement *> generatedInputBoundaryElements;
    QSet<GraphicElement *> generatedOutputBoundaryElements;
    QSet<Connection *> inputBoundaryConnections;
    QSet<Connection *> outputBoundaryConnections;
    QSet<OutputPort *> generatedInputBoundaryPorts;
    QVector<QList<InputPort *>> inputDestinations(boundaryInputs.size());
    QVector<QList<OutputPort *>> outputSources(boundaryOutputs.size());
    for (int i = 0; i < boundaryInputs.size(); ++i) {
        auto *boundary = boundaryInputs.at(i);
        auto *boundaryElement = boundary->graphicElement();
        if (isGeneratedBoundary(boundaryElement, QStringLiteral("IN"))) {
            generatedInputBoundaryElements.insert(boundaryElement);
            generatedInputBoundaryPorts.insert(boundary);
        }
        for (auto *connection : boundary->connections()) {
            if (generatedInputBoundaryPorts.contains(boundary)) {
                inputBoundaryConnections.insert(connection);
            }
            if (connection->endPort()) inputDestinations[i].append(connection->endPort());
        }
    }
    for (int i = 0; i < boundaryOutputs.size(); ++i) {
        auto *boundaryElement = boundaryOutputs.at(i)->graphicElement();
        const bool generated = isGeneratedBoundary(boundaryElement, QStringLiteral("OUT"));
        if (generated) generatedOutputBoundaryElements.insert(boundaryElement);
        for (auto *connection : boundaryOutputs.at(i)->connections()) {
            if (generated) outputBoundaryConnections.insert(connection);
            if (connection->startPort()) outputSources[i].append(connection->startPort());
        }
    }

    QHash<OutputPort *, QList<OutputPort *>> externalSourcesByBoundary;
    for (const auto &external : externalInputs) {
        externalSourcesByBoundary[boundaryInputs.at(external.portIndex)].append(external.source);
    }

    for (auto it = expandedItems.begin(); it != expandedItems.end();) {
        auto *element = qgraphicsitem_cast<GraphicElement *>(*it);
        auto *connection = qgraphicsitem_cast<Connection *>(*it);
        if ((element && (generatedInputBoundaryElements.contains(element)
                         || generatedOutputBoundaryElements.contains(element)))
            || (connection && (inputBoundaryConnections.contains(connection)
                               || outputBoundaryConnections.contains(connection)))) {
            it = expandedItems.erase(it);
        } else {
            ++it;
        }
    }
    for (auto *connection : std::as_const(inputBoundaryConnections)) delete connection;
    for (auto *connection : std::as_const(outputBoundaryConnections)) delete connection;
    qDeleteAll(generatedInputBoundaryElements);
    qDeleteAll(generatedOutputBoundaryElements);

    // The blob retains the original circuit coordinates; offset its contents with the IC instance.
    const QPointF translation = ic->pos() - definitionOrigin;
    if (!translation.isNull()) {
        for (auto *item : std::as_const(expandedItems)) {
            if (auto *element = qgraphicsitem_cast<GraphicElement *>(item)) {
                element->setPos(element->pos() + translation);
            }
        }
        for (auto *item : std::as_const(expandedItems)) {
            if (auto *connection = qgraphicsitem_cast<Connection *>(item)) {
                connection->updatePosFromPorts();
            }
        }
    }

    for (auto it = nestedBlobs.cbegin(); it != nestedBlobs.cend(); ++it) {
        if (hasBlob(it.key()) && blob(it.key()) != it.value()) {
            throw PANDACEPTION("Flattening this IC would conflict with embedded IC \"%1\".", it.key());
        }
    }

    m_scene->undoStack()->beginMacro(QCoreApplication::tr("Flatten IC"));
    for (auto it = nestedBlobs.cbegin(); it != nestedBlobs.cend(); ++it) {
        if (hasBlob(it.key())) continue;
        m_scene->receiveCommand(new RegisterBlobCommand(it.key(), it.value(), m_scene));
    }
    m_scene->receiveCommand(new DeleteItemsCommand({ic}, m_scene));
    for (int i = 0; i < inputDestinations.size(); ++i) {
        if (!generatedInputBoundaryPorts.contains(boundaryInputs.at(i))) continue;
        const auto sources = externalSourcesByBoundary.value(boundaryInputs.at(i));
        for (auto *source : sources) {
            for (auto *destination : inputDestinations.at(i)) {
                auto *connection = new Connection();
                connection->setStartPort(source);
                connection->setEndPort(destination);
                expandedItems.append(connection);
            }
        }
    }
    for (const auto &external : externalOutputs) {
        for (auto *source : outputSources.at(external.portIndex)) {
            if (generatedInputBoundaryPorts.contains(source)) {
                for (auto *externalSource : externalSourcesByBoundary.value(source)) {
                    auto *connection = new Connection();
                    connection->setStartPort(externalSource);
                    connection->setEndPort(external.destination);
                    expandedItems.append(connection);
                }
            } else {
                auto *connection = new Connection();
                connection->setStartPort(source);
                connection->setEndPort(external.destination);
                expandedItems.append(connection);
            }
        }
    }
    m_scene->receiveCommand(new AddItemsCommand(expandedItems, m_scene));
    expandedItems.clear();
    cleanup.dismiss();
    if (wasEmbedded && findICsByBlobName(blobName).isEmpty()) {
        m_scene->receiveCommand(new RemoveBlobCommand(blobName, m_scene));
    }
    m_scene->undoStack()->endMacro();
    m_scene->setCircuitUpdateRequired();
}

int ICRegistry::embedICsByFile(const QString &fileName, const QByteArray &fileBytes,
                               const QString &blobName)
{
    const auto targets = findICsByFile(fileName);
    if (targets.isEmpty()) {
        return 0;
    }

    const auto connections = UpdateBlobCommand::captureConnections(targets);
    const QByteArray oldData = captureSnapshot(targets);

    registerBlob(blobName, fileBytes);

    try {
        reloadTargetsAtomically(targets, oldData, [&](IC *ic) {
            // loadFromBlob() first: if it throws, the element must stay fully untouched
            // (reloadTargetsAtomically only rolls back targets whose mutate() call
            // completed) -- setting the blob name first would leave a throwing element
            // half-mutated (isEmbedded() flipped true, blobName changed) with no rollback.
            ic->loadFromBlob(m_blobs[blobName], m_scene->contextDir());
            ic->setBlobName(blobName);
        });
    } catch (...) {
        removeBlob(blobName);
        throw;
    }

    auto *cmd = new UpdateBlobCommand(targets, oldData, connections, m_scene);
    // This blob is newly registered above (registerBlob() at a name these targets weren't
    // already using), not replacing prior content — leave m_oldBlob at its default-empty so
    // undo() removes it rather than restoring bytes that never existed. Explicit rather than
    // relying on the constructor default, unlike every other UpdateBlobCommand call site.
    cmd->setOldBlob(QByteArray());
    m_scene->undoStack()->push(cmd);
    return static_cast<int>(targets.size());
}

int ICRegistry::extractToFile(const QString &blobName, const QString &filePath)
{
    // Write blob to disk
    QSaveFile saveFile(filePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        throw PANDACEPTION("Could not open file: %1", saveFile.errorString());
    }
    saveFile.write(blob(blobName));
    if (!saveFile.commit()) {
        // Covered: TestICRegistry::testExtractToFileThrowsWhenCommitFails() forces this via
        // RLIMIT_FSIZE (ScopedTinyFsizeLimit) -- Qt defers write() errors, so a failed
        // write() above only surfaces here, at commit().
        throw PANDACEPTION("Could not save file: %1", saveFile.errorString());
    }

    // Convert all embedded ICs with this blobName to file-backed
    const auto targets = findICsByBlobName(blobName);
    if (targets.isEmpty()) {
        return 0;
    }

    const auto connections = UpdateBlobCommand::captureConnections(targets);
    const QByteArray oldData = captureSnapshot(targets);
    const QByteArray oldBlob = blob(blobName);

    const QString fileDir = QFileInfo(filePath).absolutePath();
    reloadTargetsAtomically(targets, oldData, [&](IC *ic) { ic->loadFile(filePath, fileDir); });

    removeBlob(blobName);

    auto *cmd = new UpdateBlobCommand(targets, oldData, connections, m_scene);
    cmd->setOldBlob(oldBlob);
    cmd->setBlobName(blobName);
    m_scene->undoStack()->push(cmd);
    return static_cast<int>(targets.size());
}

void ICRegistry::rollbackElements(const QList<GraphicElement *> &elements, const QByteArray &snapshot,
                                   Scene *scene)
{
    QByteArray data(snapshot);
    QDataStream stream(&data, QIODevice::ReadOnly);
    const auto version = Serialization::readPandaHeader(stream);
    QHash<quint64, Port *> portMap;
    auto ctx = scene->deserializationContext(portMap, version, SerializationPurpose::InMemorySnapshot);
    for (auto *elm : elements) {
        elm->load(stream, ctx); // LCOV_EXCL_LINE — reloadTargetsAtomically()'s three current callers (onFileChanged/embedICsByFile/extractToFile) all apply the SAME file or blob bytes to every target in a batch, and a parse failure depends only on those bytes/contextDir, never on which target element is being mutated -- so within one batch either every target's mutate() call succeeds or the very first one throws. "elements" (the already-mutated subset needing rollback) can therefore never be non-empty in practice; kept for the case where a future caller passes heterogeneous per-target mutations.
    }
}

void ICRegistry::reloadTargetsAtomically(const QList<GraphicElement *> &targets, const QByteArray &oldData,
                                          const std::function<void(IC *)> &mutate)
{
    // Stop the simulation for the entire loop, not just each individual mutation. Between
    // freeing one IC's old internal graph and rebuilding it, the scene's sorted vectors hold
    // dangling pointers; ticking on that state would fault — and that stale state persists
    // across the whole loop, since setCircuitUpdateRequired() only runs once, after every
    // target has been reloaded, not after each one.
    QList<GraphicElement *> updated;
    SimulationBlocker blocker(m_scene->simulation());
    try {
        for (auto *elm : targets) {
            mutate(static_cast<IC *>(elm));
            updated.append(elm);
        }
    } catch (...) {
        rollbackElements(updated, oldData, m_scene);
        throw;
    }
}

void ICRegistry::makeBlobSelfContained(const QString &name, QSet<QString> &visited,
                                       QMap<QString, QByteArray> &blobs, int depth)
{
    if (depth >= kMaxBlobNestingDepth) {
        throw PANDACEPTION("Embedded IC dependency chain exceeds the maximum nesting depth (%1) while resolving '%2'",
                           QString::number(kMaxBlobNestingDepth), name);
    }

    if (visited.contains(name)) {
        qCWarning(zero) << "Circular blob reference detected:" << name << "— skipping.";
        return;
    }
    visited.insert(name);

    QByteArray blobData(blobs[name]);
    QDataStream readStream(&blobData, QIODevice::ReadOnly);
    const auto preamble = Serialization::readPreamble(readStream);

    if (!VersionInfo::hasMetadata(preamble.version)) {
        return;
    }

    auto metadata = preamble.metadata;
    auto embeddedICs = Serialization::deserializeBlobRegistry(metadata, preamble.version);
    bool modified = false;

    // Recurse into already-embedded blobs to ensure they are self-contained
    for (auto it = embeddedICs.begin(); it != embeddedICs.end(); ++it) {
        const QString &depName = it.key();
        blobs[depName] = it.value();
        makeBlobSelfContained(depName, visited, blobs, depth + 1);
        it.value() = blobs[depName];
    }

    // Resolve file-backed IC dependencies from disk and embed them
    if (metadata.contains("fileBackedICs")) {
        const QStringList files = metadata.value("fileBackedICs").toStringList();
        const QString contextDir = m_scene->contextDir();

        for (const QString &fileName : files) {
            const QString baseName = QFileInfo(fileName).baseName();
            if (embeddedICs.contains(baseName)) {
                continue;
            }

            QFileInfo fi(QDir(contextDir), fileName);
            if (!fi.exists()) {
                qCWarning(zero) << "makeBlobSelfContained: dependency" << fileName << "not found for blob" << name << "— skipping.";
                continue;
            }

            QFile file(fi.absoluteFilePath());
            if (!file.open(QIODevice::ReadOnly)) {
                qCWarning(zero) << "makeBlobSelfContained: cannot open dependency" << fi.absoluteFilePath() << "for blob" << name << "— blob will be incomplete.";
                continue;
            }
            QByteArray fileBytes = file.readAll();
            file.close();

            // Recursively make the dep self-contained before embedding
            blobs[baseName] = fileBytes;
            makeBlobSelfContained(baseName, visited, blobs, depth + 1);
            embeddedICs[baseName] = blobs[baseName];
        }

        metadata.remove("fileBackedICs");
        modified = true;
    }

    if (!modified) {
        return;
    }

    // Re-serialize the blob with updated metadata, preserving the elements/connections
    // tail byte-for-byte. Read that tail from the already-decompressed
    // preamble.remainingPayload, not readStream's device -- readPreamble() fully
    // consumed the live device to do that decompression, so the device has
    // nothing left to offer here.
    const QByteArray elements = preamble.remainingPayload;

    Serialization::serializeBlobRegistry(embeddedICs, metadata);

    // Metadata and the elements tail must be compressed together as one payload
    // (see Serialization::writePayload()), not written as two independent raw
    // writes -- readPreamble() on the other end decompresses the whole thing in
    // one shot.
    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);
    payloadStream << metadata;
    payloadStream.writeRawData(elements.constData(), static_cast<int>(elements.size()));

    QByteArray newBlob;
    QDataStream writeStream(&newBlob, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    Serialization::writePayload(writeStream, payload);

    blobs[name] = newBlob;
}

void ICRegistry::renameBlobReference(QByteArray &blobData, const QString &oldName, const QString &newName)
{
    QDataStream readStream(&blobData, QIODevice::ReadOnly);
    const auto preamble = Serialization::readPreamble(readStream);

    if (!VersionInfo::hasMetadata(preamble.version)) {
        return;
    }

    auto embeddedICs = Serialization::deserializeBlobRegistry(preamble.metadata, preamble.version);
    if (!embeddedICs.contains(oldName)) {
        return;
    }

    // Rename the key in the embedded IC map
    embeddedICs[newName] = embeddedICs.take(oldName);

    // Re-serialize the blob with updated metadata, preserving the elements/connections
    // tail byte-for-byte. Read that tail from the already-decompressed
    // preamble.remainingPayload, not readStream's device -- readPreamble() fully
    // consumed the live device to do that decompression, so the device has
    // nothing left to offer here.
    const QByteArray elements = preamble.remainingPayload;
    auto metadata = preamble.metadata;
    Serialization::serializeBlobRegistry(embeddedICs, metadata);

    // Metadata and the elements tail must be compressed together as one payload
    // (see Serialization::writePayload()), not written as two independent raw
    // writes -- readPreamble() on the other end decompresses the whole thing in
    // one shot.
    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);
    payloadStream << metadata;
    payloadStream.writeRawData(elements.constData(), static_cast<int>(elements.size()));

    QByteArray newBlob;
    QDataStream writeStream(&newBlob, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    Serialization::writePayload(writeStream, payload);

    blobData = newBlob;
}

QByteArray ICRegistry::captureSnapshot(const QList<GraphicElement *> &targets)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    for (auto *elm : targets) {
        elm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }
    return data;
}
