// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Integrated Circuit (IC) graphic element that encapsulates a sub-circuit file.
 */

#pragma once

#include <memory>

#include <QFileInfo>
#include <QFileSystemWatcher>

#include "App/Element/GraphicElement.h"
#include "App/Simulation/ElementMapping.h"

/**
 * \class IC
 * \brief Graphic element representing an Integrated Circuit (sub-circuit) box.
 *
 * \details An IC element wraps an external .panda file, exposing its input and
 * output elements as ports on the IC box.  It monitors the file for changes via
 * QFileSystemWatcher and reloads automatically.  The internal simulation mapping
 * is built lazily via generateMap().
 */
class IC : public GraphicElement
{
    Q_OBJECT

    friend class TestArduino;

public:
    // --- Lifecycle ---

    /// Constructs an IC element without loading a file.
    explicit IC(QGraphicsItem *parent = nullptr);

    /// Destructor; releases the internal ElementMapping.
    ~IC() override;

    /// Copy-constructs by delegating to the parent item constructor.
    IC(const IC &other) : IC(other.parentItem()) {}

    // --- File loading ---

    /**
     * \brief Loads the sub-circuit from \a fileName relative to \a contextDir.
     * \param fileName   Path to the .panda sub-circuit file.
     * \param contextDir Directory used to resolve relative paths (optional).
     */
    void loadFile(const QString &fileName, const QString &contextDir = {});

    /**
     * \brief Loads the IC from \a fileName after a drag-drop operation.
     * \param fileName   Dropped .panda file path.
     * \param contextDir Context directory for relative path resolution.
     */
    void loadFromDrop(const QString &fileName, const QString &contextDir) override;

    // --- File utilities ---

    /**
     * \brief Copies IC-related files from \a srcPath to \a destPath.
     * \param srcPath  Source .panda file info.
     * \param destPath Destination .panda file info.
     */
    static void copyFiles(const QFileInfo &srcPath, const QFileInfo &destPath);

    // --- Simulation ---

    /**
     * \brief Builds and returns the flat LogicElement mapping for simulation.
     * \return Vector of shared_ptr LogicElements representing the IC's internals.
     */
    const QVector<std::shared_ptr<LogicElement>> generateMap();

    /// Returns all logic elements from the IC's internal mapping.
    QVector<std::shared_ptr<LogicElement>> getLogicElementsForMapping() override;

    // --- Logic access ---

    /// \reimp
    LogicElement *getInputLogic(int portIndex) const override;
    /// \reimp
    LogicElement *getOutputLogic(int portIndex) const override;
    /// \reimp
    int getInputIndexForPort(int portIndex) const override;
    /// \reimp
    int getOutputIndexForPort(int portIndex) const override;

    // --- Port naming ---

    /// Returns \c true; IC elements support custom port naming.
    bool canSetPortNames() const override;

    /**
     * \brief Sets the name of input \a port to \a name.
     * \param port Port index.
     * \param name New port name.
     */
    void setInputPortName(int port, const QString &name) override;

    /**
     * \brief Sets the name of output \a port to \a name.
     * \param port Port index.
     * \param name New port name.
     */
    void setOutputPortName(int port, const QString &name) override;

    const QString &icFile() const { return m_file; }
    const QVector<GraphicElement *> &icElements() const { return m_icElements; }
    const QVector<QNEPort *> &icInputs() const { return m_icInputs; }
    const QVector<QNEPort *> &icOutputs() const { return m_icOutputs; }

    // --- Visual ---

    /// Paints the IC box with its generated pixmap.
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// Regenerates the IC pixmap and port layout.
    void refresh() override;

    // --- Serialization ---

    /// Deserializes the IC (file reference and port configuration) from \a stream.
    void load(QDataStream &stream, SerializationContext &context) override;

    /// Serializes the IC file reference and port configuration to \a stream.
    void save(QDataStream &stream) const override;

protected:
    // --- Qt event overrides ---

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    // --- Port sorting helpers ---

    static bool comparePorts(QNEPort *port1, QNEPort *port2);
    static void sortPorts(QVector<QNEPort *> &map);

    // --- Internal loading helpers ---

    void loadInputElement(GraphicElement *elm);
    void loadInputs();
    void loadInputsLabels();
    void loadOutputElement(GraphicElement *elm);
    void loadOutputs();
    void loadOutputsLabels();

    // --- Simulation helpers ---

    LogicElement *inputLogic(int index);
    LogicElement *outputLogic(int index);

    // --- Visual helpers ---

    void generatePixmap();

    // --- File copy helper ---

    void copyFile(const CopyOperation &op);

    // --- Members ---

    // Simulation
    std::unique_ptr<ElementMapping> m_mapping;

    // File watching
    QFileSystemWatcher m_fileWatcher;
    QString m_file;

    // IC internals
    QVector<GraphicElement *> m_icElements;
    QVector<QNEPort *> m_icInputs;
    QVector<QNEPort *> m_icOutputs;
    QVector<QString> m_icInputLabels;
    QVector<QString> m_icOutputLabels;
};
