// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Integrated Circuit (IC) graphic element that encapsulates a sub-circuit file.
 */

#pragma once

#include <QFileInfo>
#include <QSet>

#include "App/Element/GraphicElement.h"
#include "App/IO/SerializationContext.h"

class ICDefinition;

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

public:
    /// Constructs an IC element without loading a file.
    explicit IC(QGraphicsItem *parent = nullptr);

    ~IC() override;

    /// Copy-constructs by delegating to the parent item constructor.
    IC(const IC &other) : IC(other.parentItem()) {}

signals:
    /// Emitted on double-click to request opening the sub-circuit in a new tab.
    void requestOpenSubCircuit(int elementId, const QString &blobName, const QString &filePath);

public:

    /// Loads the IC circuit from \a fileName and rebuilds the logic mapping.
    void loadFile(const QString &fileName, const QString &contextDir = {});

    /// Loads the IC from in-memory blob bytes (full .panda file format).
    void loadFromBlob(const QByteArray &blob, const QString &contextDir);

    /// Loads the IC from a shared definition (delegates to loadFromBlob).
    void loadFromDefinition(const ICDefinition *def, const QString &contextDir);

    /// Returns the current definition, or nullptr if not set.
    const ICDefinition *definition() const { return m_definition; }

    void loadFromDrop(const QString &fileName, const QString &contextDir) override;

    const QString &file() const { return m_file; }

    /// Returns the blob name for embedded ICs, empty if file-backed.
    const QString &blobName() const override { return m_blobName; }
    void setBlobName(const QString &name) { m_blobName = name; }
    bool isEmbedded() const override { return !m_blobName.isEmpty(); }

    const QVector<GraphicElement *> &internalElements() const { return m_internalElements; }
    const QVector<QNEPort *> &internalInputs() const { return m_internalInputs; }
    const QVector<QNEPort *> &internalOutputs() const { return m_internalOutputs; }

    // --- Visual ---

    /// \reimp Simulates the IC's internal circuit and propagates results.
    void updateLogic() override;

    /// Builds the internal simulation graph (connection graph + sort) for direct simulation.
    void initializeSimulation();

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
    void save(QDataStream &stream) const override;

protected:
    // --- Event handlers ---

    /// \reimp Opens the IC sub-circuit for editing on double-click.
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    // --- Utility methods ---

    static bool comparePorts(QNEPort *port1, QNEPort *port2);
    static void sortPorts(QVector<QNEPort *> &map);

    void loadFileDirectly(const QFileInfo &fileInfo);
    void migrateFile(const QFileInfo &fileInfo, const QList<QGraphicsItem *> &items,
                     const QVersionNumber &version, const QMap<QString, QByteArray> &fileRegistry);
    void resetInternalState();

    // --- Loading helpers ---

    void processLoadedItems(const QList<QGraphicsItem *> &items);
    void loadBoundaryElement(GraphicElement *elm, bool isInput);
    void loadBoundaryPorts(bool isInput, const QVector<QString> &labels);
    void buildPortLabels(const QVector<QNEPort *> &ports, QVector<QString> &labels);

    // --- Visual helpers ---

    void generatePixmap();

    // --- Members ---

    const ICDefinition *m_definition = nullptr;
    QString m_file;
    QString m_blobName;
    QVector<GraphicElement *> m_internalElements;
    QVector<QNEPort *> m_internalInputs;
    QVector<QNEPort *> m_internalOutputs;

    // --- Members: Direct simulation ---

    QVector<GraphicElement *> m_sortedInternalElements;
    QSet<GraphicElement *> m_boundaryInputElements;
    bool m_internalHasFeedback = false;
};

