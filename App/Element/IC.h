// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Integrated Circuit (IC) graphic element that encapsulates a sub-circuit file.
 */

#pragma once

#include <QByteArray>
#include <QMap>
#include <QPixmap>
#include <QPoint>
#include <QSet>

#include "App/Element/Element.h"
#include "App/IO/SerializationContext.h"

class QGraphicsSceneHoverEvent;
class Connection;
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
    /// (pixmap(), portsBoundingRect(), setMax/MinInputSize/OutputSize(), etc.) directly;
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

signals:
    /// Emitted on double-click to request opening the sub-circuit in a new tab.
    void requestOpenSubCircuit(int elementId, const QString &blobName, const QString &filePath);

    /// Hover-preview lifecycle signals. The UI layer (SceneUiBinder) drives the shared
    /// ICPreviewPopup off these, so IC needs no dependency on MainWindow. \a ic is \c this.
    void previewRequested(IC *ic, const QPoint &screenPos);
    void previewMoved(IC *ic, const QPoint &screenPos);
    void previewHideRequested();
    void previewCancelRequested(IC *ic);

public:

    /// Loads the IC circuit from \a fileName and rebuilds the logic mapping.
    void loadFile(const QString &fileName, const QString &contextDir = {});

    /// Loads the IC from in-memory blob bytes (full .panda file format).
    void loadFromBlob(const QByteArray &blob, const QString &contextDir);
    /// Lets this IC, a copy a snapshot is read into before \a live is (CommandUtils::readCopies()), keep the circuit
    /// \a live built instead of building it again from the same bytes: they load, and the copy is thrown away.
    void adoptLoadedCircuit(const IC &live);

    void loadFromDrop(const QString &fileName, const QString &contextDir) override;

    const QString &file() const { return m_file; }

    /// Name shown in the hover preview header: the blob name for embedded ICs,
    /// otherwise the file's base name with extension.  Empty if neither is set.
    QString displayName() const;

    /// Returns the blob name for embedded ICs, empty if file-backed.
    const QString &blobName() const override { return m_blobName; }
    void setBlobName(const QString &name) { m_blobName = name; }
    bool isEmbedded() const override { return !m_blobName.isEmpty(); }

    const QVector<Element *> &internalElements() const { return m_internalElements; }
    const QVector<Port *> &internalInputs() const { return m_internalInputs; }
    const QVector<Port *> &internalOutputs() const { return m_internalOutputs; }

    /// Returns the hover preview of the designed circuit, rendering it on the first call after a load from the
    /// definition the IC loaded, read again, so it shows what the user placed (buttons, switches, LEDs), not the
    /// proxy Nodes that replace them. Null when the circuit is empty or too large, or its definition no longer loads.
    const QPixmap &preview();

    /// Returns the preview rendered so far: null until preview() renders it.
    const QPixmap &previewPixmap() const { return m_previewPixmap; }

    // --- Port metadata ---

    /// Port count and label metadata extracted from Input/Output elements.
    struct PortMetadata {
        int inputCount = 0;
        int outputCount = 0;
        QStringList inputLabels;
        QStringList outputLabels;
    };

    /// Scans \a elements for Input/Output groups, sorts by Y/X position, and builds labels.
    static PortMetadata buildPortMetadata(const QVector<Element *> &elements);

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
    QRectF boundingRect() const override;
    /// \reimp
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    /// \reimp
    void refresh() override;
    /// Keeps a loaded IC's tooltip clear, since the hover preview names it; one with nothing loaded shows its type name.
    void retranslate() override;

    // --- External file dependencies ---

    /// \reimp Returns IC .panda file path for file-backed ICs.
    QStringList externalFiles() const override;

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, SerializationContext &context) override;
    /// \reimp
    void save(QDataStream &stream, SerializationOptions options) const override;

protected:
    // --- Event handlers ---

    /// \reimp Opens the IC sub-circuit for editing on double-click.
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    /// \reimp Schedules a floating preview of the internal circuit after a short hover delay.
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

    /// \reimp Updates the pending preview position so the popup appears near
    /// the current cursor when the show delay elapses, not at the entry point.
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

    /// \reimp Dismisses the floating preview with a short delay.
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    Q_DISABLE_COPY_MOVE(IC)

    // --- Utility methods ---

    /// True when \a localPos (in IC item coordinates) lies over one of this IC's
    /// ports.  Used to suppress the hover preview over the connection pins.
    bool isCursorOverPort(const QPointF &localPos) const;

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
    /// The scene whose file cache this IC, a copy outside any scene, reads its file through, as the live IC does.
    Scene *m_filesScene = nullptr;
    /// Bytes last successfully loaded via loadFromBlob(), so IC::load() can skip re-loading
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

    QPixmap m_previewPixmap;           ///< The designed circuit, rendered by preview().
    bool m_previewRendered = false;    ///< Whether preview() has rendered the circuit loaded last.
    /// The bytes the circuit loaded last came from, for preview() to read again; empty when it came from m_file.
    QByteArray m_previewSource;
    QString m_previewContextDir;       ///< The directory the file-backed ICs it names resolve against.
};
