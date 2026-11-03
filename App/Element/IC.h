// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Integrated Circuit (IC) graphic element that encapsulates a sub-circuit file.
 */

#pragma once

#include <memory>

#include <QByteArray>
#include <QMap>
#include <QPixmap>
#include <QPoint>
#include <QSet>

#include "App/Element/Element.h"
#include "App/Element/PinOrder.h"
#include "App/IO/SerializationContext.h"

class Connection;
class ICDefinition;
class ICFileCache;
class Scene;

/**
 * \class IC
 * \brief Graphic element representing an Integrated Circuit (sub-circuit) box.
 *
 * \details An IC element wraps a sub-circuit, exposing its input and output
 * elements as ports on the IC box.  File watching is handled by ICRegistry
 * (centralized per-scene, one watcher per unique file path).  The internal
 * simulation graph is built lazily via initializeSimulation().
 */
class IC : public Element
{
    Q_OBJECT

    friend class TestArduino;
    friend class TestIC;
    friend class TestDanglingPointer;
    friend class TestICController;

    /// Reach ic's internal-element/port vectors and protected Element members
    /// (portsBoundingRect(), pictureReplaced(), setMax/MinInputSize/OutputSize(), etc.) directly;
    /// see ICLoader / ICRenderer / ICSimulation.
    friend class ICLoader;
    friend class ICRenderer;
    friend class ICSimulation;
    /// Reads the inner ports behind the IC's pins for the characterization goldens.
    friend class SnapshotAccess;

public:
    /// Constructs an IC element without loading a file.
    explicit IC(QGraphicsItem *parent = nullptr);

    ~IC() override;

public:

    /// Loads the IC circuit from \a fileName and rebuilds the logic mapping.
    void loadFile(const QString &fileName, const QString &contextDir = {},
                  const QMap<QString, QByteArray> *fileCopies = nullptr);

    /// Loads the IC from in-memory blob bytes (full .panda file format).
    void loadFromBlob(const QByteArray &blob, const QString &contextDir);

    /// Loads the IC from \a bytes, a document's copy of the definition of the file \a fileName, as loadFile() loads it.
    void loadFileCopy(const QString &fileName, const QByteArray &bytes, const QString &contextDir,
                      const QMap<QString, QByteArray> *fileCopies = nullptr);

    /// Returns the file reference save() writes for this file-backed IC, written for \a purpose.
    QString fileReference(SerializationPurpose purpose) const;

    /// Returns the definition this file-backed IC holds, as its file was last read, or nothing when it holds none.
    QByteArray definitionFileBytes() const;

    /// Returns the files this IC's circuit nests, as it read them, other than its own file (absolute paths).
    QStringList nestedFiles() const;
    /// Returns the bytes this IC's circuit read from each file, its own and the nested ones, keyed by absolute path.
    const QMap<QString, QByteArray> &filesRead() const { return m_filesRead; }
    /// Lets this IC, a copy a snapshot is read into before \a live is (CommandUtils::readCopies()), keep the circuit
    /// \a live built instead of building it again from the same bytes: they load, and the copy is thrown away.
    void adoptLoadedCircuit(const IC &live);

    /// Returns the file cache this IC reads its file through, or nothing when it has none.
    ICFileCache *fileCache() const { return m_filesCache; }

    /// Sets the file cache this IC reads its file through. The scene gives its registry as the IC is added.
    void setFileCache(ICFileCache *cache) { m_filesCache = cache; }

    void loadFromDrop(const QString &fileName, const QString &contextDir) override;

    const QString &file() const { return m_file; }

    /// Name shown in the hover preview header: the blob name for embedded ICs,
    /// otherwise the file's base name with extension.  Empty if neither is set.
    QString displayName() const;

    /// True when \a localPos (in IC item coordinates) lies over one of this IC's ports, where the scene shows no hover
    /// preview (Scene::trackHover()).
    bool isCursorOverPort(const QPointF &localPos) const;

    /// Returns the blob name for embedded ICs, empty if file-backed.
    const QString &blobName() const override { return m_blobName; }
    /// Names the blob the IC's definition is embedded as, empty for a file-backed IC; reports a change of a parameter.
    void setBlobName(const QString &name);
    bool isEmbedded() const override { return !m_blobName.isEmpty(); }

    const QVector<Element *> &internalElements() const { return m_internalElements; }
    const QVector<Port *> &internalInputs() const { return m_internalInputs; }
    const QVector<Port *> &internalOutputs() const { return m_internalOutputs; }

    /// Returns the hover preview of the designed circuit, so it shows what the user placed (buttons, switches, LEDs),
    /// not the proxy Nodes that replace them. It is rendered once per definition and directory (ICPreviewCache). Null
    /// when the IC holds no circuit, the circuit is empty or too large, or it no longer loads.
    QPixmap preview();

    // --- Port metadata ---

    /// Port count and label metadata extracted from Input/Output elements.
    struct PortMetadata {
        int inputCount = 0;
        int outputCount = 0;
        QStringList inputLabels;
        QStringList outputLabels;
    };

    /// Scans \a elements for Input/Output groups, sorts by Y/X position, and builds labels.
    static PortMetadata buildPortMetadata(const QVector<PinRef> &inputs, const QVector<PinRef> &outputs);

    // --- Visual ---

    /// \reimp Simulates the IC's internal circuit and propagates results.
    void updateLogic() override;

    /// \reimp Re-propagates committed sequential state through the IC's internal
    /// combinational logic and refreshes its output boundary, without re-clocking
    /// internal flip-flops/latches.
    void resettleCombinational() override;

    /// Builds the internal simulation graph (connection graph + sort) for direct simulation, keeping the internal
    /// elements' outputs if \a keepOutputs (a rebuild).
    void initializeSimulation(bool keepOutputs = false);

    /// \reimp Saves/restores every internal primitive's state too (recursively through nested
    /// ICs), so a BeWavedDolphin sweep hands the live circuit back exactly as it found it.
    void saveSimState(QVector<Status> &out) const override;
    void restoreSimState(const QVector<Status> &in, int &cursor) override;

    /// \reimp
    void refresh() override;
    /// None once a sub-circuit has loaded, since the hover preview names the IC; one with nothing loaded has its type's
    /// name.
    QString helpText() const override;

    // --- External file dependencies ---

    /// \reimp Returns IC .panda file path for file-backed ICs.
    QStringList externalFiles() const override;

    // --- Serialization ---

    /// \reimp
    void save(QDataStream &stream, SerializationOptions options) const override;

protected:
    /// \reimp
    void loadState(QDataStream &stream, SerializationContext &context) override;

    /// \reimp A change of scene drops the file cache: the scene the IC joins gives its own.
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    Q_DISABLE_COPY_MOVE(IC)

    // --- Utility methods ---

    /// Clears the internal element/connection/port vectors and derived simulation state,
    /// freeing the previously-loaded sub-circuit.  Called before loading a new one.
    void resetInternalState();

    // --- Members ---

    QString m_file;
    QString m_blobName;
    /// Whether a sub-circuit has been loaded, from a file or a blob.
    bool m_loaded = false;
    /// Whether this IC, a copy, took over the record of a circuit another IC built (adoptLoadedCircuit()).
    bool m_circuitAdopted = false;
    /// The file cache this IC reads its file through: its scene's, or for a copy outside any scene the live IC's
    /// (adoptLoadedCircuit()).
    ICFileCache *m_filesCache = nullptr;
    /// The definition the elements were built from, held for as long as the IC is loaded, so it stays in use.
    std::shared_ptr<const ICDefinition> m_definition;
    /// Bytes last successfully loaded via loadFromBlob(), so IC::loadState() can skip re-loading
    /// an embedded sub-circuit whose blob reference and content are both unchanged (a
    /// label-only property edit) instead of always tearing down and rebuilding it.
    QByteArray m_loadedBlobSnapshot;
    /// The IC files the circuit loaded last read, nested ones included, by absolute path: a snapshot carries them, so
    /// its restore loads one deleted since.
    QMap<QString, QByteArray> m_filesRead;
    QVector<Element *> m_internalElements;
    QVector<Connection *> m_internalConnections;
    QVector<Port *> m_internalInputs;
    QVector<Port *> m_internalOutputs;

    // --- Members: Direct simulation ---

    QVector<Element *> m_sortedInternalElements;
    QSet<Element *> m_boundaryInputElements;
    bool m_internalHasFeedback = false;

    // --- Members: Hover preview ---

    QString m_previewContextDir;       ///< The directory the file-backed ICs the circuit names resolve against.
};
