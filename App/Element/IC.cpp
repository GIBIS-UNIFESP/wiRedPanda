// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/IC.h"

#include <algorithm>
#include <optional>

#include <QDataStream>
#include <QFileInfo>
#include <QStyleOptionGraphicsItem>

#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/ICLoader.h"
#include "App/Element/ICPreviewCache.h"
#include "App/Element/ICSimulation.h"
#include "App/IO/ExternalFilePath.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Scene/Scene.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<ElementType::IC> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::IC,
        .group = ElementGroup::IC,
        .hasLabel = true,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.kernel = KernelKind::Instance;
        descriptor.pixmapPath = QString();
        descriptor.titleText = QT_TRANSLATE_NOOP("IC", "INTEGRATED CIRCUIT");
        descriptor.translatedName = QT_TRANSLATE_NOOP("IC", "IC");
        descriptor.trContext = "IC";
        return descriptor;
    }

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new IC(); });
        return true;
    }();
};

IC::IC(QGraphicsItem *parent)
    : Element(ElementType::IC, parent)
{
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
} // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned QStringList, never reached after the return above.

void IC::save(QDataStream &stream, SerializationOptions options) const
{
    Element::save(stream, options);

    QMap<QString, QVariant> map;

    if (isEmbedded()) {
        map.insert("name", m_blobName);
    } else if (!m_file.isEmpty()) {
        map.insert("name", fileReference(options.purpose));
    }

    // A snapshot carries the IC files the circuit read, so its restore loads one deleted since; a file never does
    if ((options.purpose == SerializationPurpose::InMemorySnapshot) && !m_filesRead.isEmpty()) {
        QByteArray files;
        QDataStream filesStream(&files, QIODevice::WriteOnly);
        filesStream.setVersion(QDataStream::Qt_5_12);
        filesStream << m_filesRead;
        map.insert("files", files);
    }

    stream << map;
}

QString IC::fileReference(const SerializationPurpose purpose) const
{
    return ExternalFilePath::forWriting(m_file, purpose);
}

QByteArray IC::definitionFileBytes() const
{
    return m_filesRead.value(m_file);
}

QStringList IC::nestedFiles() const
{
    QStringList files;
    for (auto it = m_filesRead.cbegin(); it != m_filesRead.cend(); ++it) {
        if (it.key() != m_file) {
            files.append(it.key());
        }
    }
    return files;
}

void IC::loadFileCopy(const QString &fileName, const QByteArray &bytes, const QString &contextDir,
                      const QMap<QString, QByteArray> *fileCopies)
{
    // A definition replaces the ports and the body, and names the file: one change, also when the load throws.
    const ChangeBatch batch(*this, ElementChange::Parameter | ElementChange::Ports);
    m_filesRead = ICLoader::recordFilesRead(
        [&] { ICLoader::loadFileCopy(*this, fileName, bytes, contextDir, fileCopies); });
}

namespace {
/// Returns the document's copy of the definition of the file \a name, or nothing when the document holds none.
std::optional<QByteArray> savedFileCopy(const SerializationContext &context, const QString &name)
{
    if (!context.fileCopies) {
        return std::nullopt;
    }
    const auto packed = context.fileCopies->constFind(name);
    if (packed == context.fileCopies->constEnd()) {
        return std::nullopt;
    }
    QByteArray hash;
    QByteArray bytes;
    if (!ICLoader::unpackFileCopy(packed.value(), hash, bytes)) {
        return std::nullopt;
    }
    return bytes;
}
} // namespace

void IC::loadState(QDataStream &stream, SerializationContext &context)
{
    Element::loadState(stream, context);

    // Save the portMap keys for each port that Element::loadState() just
    // registered.  Loading the sub-circuit below destroys and recreates all
    // ports, so we must re-register the new ports under the original keys
    // to keep the outer portMap valid for connection deserialization.
    // Old format uses ptr values as keys; new format uses serialIds.  The
    // keys come from the ones Element::loadState() just registered, never
    // from a search of the whole map: it holds every port loaded so far, so a
    // search per IC would make a load quadratic in the circuit's ICs.  A key
    // counts while it still names its port (a later port of the same element
    // listed under the same key takes it over).
    QVector<quint64> savedInputKeys, savedOutputKeys;
    for (int i = 0; i < context.loadedInputKeys.size(); ++i) {
        if (const quint64 key = context.loadedInputKeys.at(i); (i < inputs().size()) && (context.portMap.value(key) == inputs().at(i))) {
            savedInputKeys.append(key);
        }
    }
    for (int i = 0; i < context.loadedOutputKeys.size(); ++i) {
        if (const quint64 key = context.loadedOutputKeys.at(i); (i < outputs().size()) && (context.portMap.value(key) == outputs().at(i))) {
            savedOutputKeys.append(key);
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

        loadFile(m_file, context.contextDir, context.fileCopies);
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

        // A snapshot's restore loads the IC files it carries in place of any deleted since it was taken
        ICLoader::Files carried;
        if ((context.purpose == SerializationPurpose::InMemorySnapshot) && map.contains("files")) {
            QByteArray files = map.value("files").toByteArray();
            QDataStream filesStream(&files, QIODevice::ReadOnly);
            filesStream.setVersion(QDataStream::Qt_5_12);
            carried = Serialization::readBoundedBlobMap(filesStream);
        }
        ICLoader::withCarriedFiles(carried, context.purpose == SerializationPurpose::InMemorySnapshot, [&] {
            // Resolve: try exact name first (embedded blob names may contain dots),
            // then baseName (file-backed "chain_c.panda" with registry key "chain_c")
            const QString baseName = QFileInfo(name).baseName();
            const bool exactMatch = context.blobRegistry && context.blobRegistry->contains(name);
            const bool baseMatch = !exactMatch && context.blobRegistry && context.blobRegistry->contains(baseName);

            if (exactMatch) {
                const QByteArray blobBytes = context.blobRegistry->value(name);

                // A cosmetic property edit (e.g. the label field, live on every keystroke —
                // see ElementEditor::apply()) re-serializes and reloads this IC via UpdateCommand
                // even though neither the blob reference nor its content actually changed. For a
                // large embedded sub-circuit, resetInternalState()+loadFromBlob() below is a full
                // teardown/rebuild of every internal element and connection -- skip it when
                // nothing this IC depends on has moved. Comparing blob bytes (not just the name)
                // is required for correctness: UpdateBlobCommand's undo/redo replaces the
                // registry's bytes under this same name to restore/reapply a sub-circuit edit, and
                // that must still reload.
                const bool blobUnchanged = (m_blobName == name) && (m_loaded || m_circuitAdopted)
                                         && (m_loadedBlobSnapshot == blobBytes);

                m_blobName = name;
                if (!blobUnchanged) {
                    loadFromBlob(blobBytes, context.contextDir);
                }
            } else if (baseMatch) {
                m_blobName = baseName;
                loadFromBlob(context.blobRegistry->value(baseName), context.contextDir);
            } else if (const std::optional<QByteArray> copy = savedFileCopy(context, name); copy) {
                // The document's copy stands for the file: unchanged or gone, it is the definition; changed, it is loaded
                // over afterwards as a reload, whose undo returns to the copy (WorkSpace::load())
                loadFileCopy(name, *copy, context.contextDir, context.fileCopies);
            } else {
                // Don't assign m_file until loadFile() has confirmed name actually resolves to a
                // real file — ICLoader::loadFile() itself sets ic.m_file, but only once validated,
                // so a name that resolves to neither a blob nor a file (e.g. an unrelated rename
                // elsewhere in the registry) throws cleanly without leaving m_file holding a
                // bogus non-path string. A file whose bytes are those this IC built keeps its circuit, as a blob
                // does above: a label edit's undo builds nothing again
                const auto file = ICLoader::fileBytes(*this, name, context.contextDir);
                // Every file the snapshot carries must be the one this IC read: a nested file's change is a change too
                bool carriedUnchanged = true;
                for (auto it = carried.cbegin(); it != carried.cend(); ++it) {
                    carriedUnchanged = carriedUnchanged && (m_filesRead.value(it.key()) == it.value());
                }
                const bool fileUnchanged = file && (m_loaded || m_circuitAdopted) && !m_file.isEmpty()
                                         && (QFileInfo(m_file) == QFileInfo(file->first))
                                         && (m_filesRead.value(file->first) == file->second)
                                         && carriedUnchanged;
                if (!fileUnchanged) {
                    loadFile(name, context.contextDir, context.fileCopies);
                }
            }
        });
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

void IC::loadFile(const QString &fileName, const QString &contextDir, const QMap<QString, QByteArray> *fileCopies)
{
    // A definition replaces the ports and the body, and names the file: one change, also when the load throws.
    const ChangeBatch batch(*this, ElementChange::Parameter | ElementChange::Ports);
    m_filesRead = ICLoader::recordFilesRead([&] { ICLoader::loadFile(*this, fileName, contextDir, fileCopies); });
}

void IC::adoptLoadedCircuit(const IC &live)
{
    m_blobName = live.m_blobName;
    m_loadedBlobSnapshot = live.m_loadedBlobSnapshot;
    m_file = live.m_file;
    m_filesRead = live.m_filesRead;
    m_circuitAdopted = live.m_loaded || live.m_circuitAdopted;
    m_filesCache = live.m_filesCache;
    m_definition = live.m_definition;
}

QVariant IC::itemChange(const GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneChange) {
        m_filesCache = nullptr;
    }
    return Element::itemChange(change, value);
}

void IC::loadFromBlob(const QByteArray &blob, const QString &contextDir)
{
    const ChangeBatch batch(*this, ElementChange::Parameter | ElementChange::Ports);
    m_filesRead = ICLoader::recordFilesRead([&] { ICLoader::loadFromBlob(*this, blob, contextDir); });
    m_loadedBlobSnapshot = blob;
}

void IC::setBlobName(const QString &name)
{
    if (name == m_blobName) {
        return;
    }
    m_blobName = name;
    changed(ElementChange::Parameter);
}

QPixmap IC::preview()
{
    if (!m_loaded || !m_definition) {
        return {};
    }
    return ICPreviewCache::preview(m_definition, m_previewContextDir);
}

void IC::loadFromDrop(const QString &fileName, const QString &contextDir)
{
    loadFile(fileName, contextDir);
}

IC::PortMetadata IC::buildPortMetadata(const QVector<PinRef> &inputs, const QVector<PinRef> &outputs)
{
    return ICLoader::buildPortMetadata(inputs, outputs);
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

void IC::initializeSimulation(const bool keepOutputs)
{
    ICSimulation::initialize(*this, keepOutputs);
}

void IC::updateLogic()
{
    ICSimulation::update(*this);
}

void IC::resettleCombinational()
{
    ICSimulation::resettle(*this);
}

void IC::saveSimState(QVector<Status> &out) const
{
    // Same reason resetSimState() recurses: the IC is only structure, but its internal
    // primitives are simulated directly by the flat netlist and hold the sequential state.
    Element::saveSimState(out);
    for (auto *internal : std::as_const(m_internalElements)) {
        if (internal) {
            internal->saveSimState(out);
        }
    }
}

void IC::restoreSimState(const QVector<Status> &in, int &cursor)
{
    Element::restoreSimState(in, cursor);
    for (auto *internal : std::as_const(m_internalElements)) {
        if (internal) {
            internal->restoreSimState(in, cursor);
        }
    }
}

void IC::refresh()
{
}

QString IC::helpText() const
{
    return m_loaded ? QString() : Element::helpText();
}
