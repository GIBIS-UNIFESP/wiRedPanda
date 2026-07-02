// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Integrated Circuit (IC) graphic element that encapsulates a sub-circuit file.
 */

#pragma once

#include <QByteArray>
#include <QPixmap>
#include <QPoint>

#include "App/Element/GraphicElement.h"
#include "App/IO/SerializationContext.h"

class QGraphicsSceneHoverEvent;
class Connection;

/**
 * \class IC
 * \brief Graphic element representing an Integrated Circuit (sub-circuit) box.
 *
 * \details An IC element wraps a sub-circuit, exposing its input and output
 * elements as ports on the IC box.  File watching is handled by ICRegistry
 * (centralized per-scene, one watcher per unique file path).  The internal
 * simulation graph is built lazily via initializeSimulation().
 */
class IC : public GraphicElement
{
    Q_OBJECT

    friend class TestArduino;
    friend class TestIC;
    friend class TestDanglingPointer;

    /// Reach ic's internal-element/port vectors and protected GraphicElement members
    /// (pixmap(), portsBoundingRect(), setMax/MinInputSize/OutputSize(), etc.) directly;
    /// see ICLoader / ICRenderer / ICSimulation.
    friend class ICLoader;
    friend class ICRenderer;
    friend class ICSimulation;

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

    void loadFromDrop(const QString &fileName, const QString &contextDir) override;

    const QString &file() const { return m_file; }

    /// Name shown in the hover preview header: the blob name for embedded ICs,
    /// otherwise the file's base name with extension.  Empty if neither is set.
    QString displayName() const;

    /// Returns the blob name for embedded ICs, empty if file-backed.
    const QString &blobName() const override { return m_blobName; }
    void setBlobName(const QString &name) { m_blobName = name; }
    bool isEmbedded() const override { return !m_blobName.isEmpty(); }

    const QVector<GraphicElement *> &internalElements() const { return m_internalElements; }
    const QVector<Port *> &internalInputs() const { return m_internalInputs; }
    const QVector<Port *> &internalOutputs() const { return m_internalOutputs; }

    /// Returns the cached preview pixmap of the designed circuit.
    /// The snapshot is taken once during load (before boundary elements are
    /// substituted with proxy Nodes), so the preview reflects what the user
    /// actually placed — buttons, switches, LEDs — not the simulation graph.
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
    static PortMetadata buildPortMetadata(const QVector<GraphicElement *> &elements);

    // --- Visual ---

    /// Wires the IC's internal primitives so Simulation::initialize() can splice them into the
    /// top-level flat netlist. The IC no longer settles itself (no updateLogic override).
    void initializeSimulation();

    /// \reimp Resets the IC's own ports AND every internal primitive (recursively through
    /// nested ICs). The flat netlist simulates the internals directly, so they carry
    /// sequential state (flip-flop Q, edge detection) that a reset must not skip — e.g. a
    /// BeWavedDolphin sweep would otherwise start from whatever state the live run left.
    void resetSimState() override;

    /// \reimp
    QRectF boundingRect() const override;
    /// \reimp
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    /// \reimp
    void refresh() override;

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
    /// Bytes last successfully loaded via loadFromBlob(), so IC::load() can skip re-loading
    /// an embedded sub-circuit whose blob reference and content are both unchanged (a
    /// label-only property edit) instead of always tearing down and rebuilding it.
    QByteArray m_loadedBlobSnapshot;
    QVector<GraphicElement *> m_internalElements;
    QVector<Connection *> m_internalConnections;
    QVector<Port *> m_internalInputs;
    QVector<Port *> m_internalOutputs;

    // --- Members: Hover preview ---

    QPixmap m_previewPixmap;           ///< Snapshot of the designed circuit, rendered at load time.
};
