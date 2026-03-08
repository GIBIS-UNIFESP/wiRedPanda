// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Integrated Circuit (IC) graphic element that encapsulates a sub-circuit file.
 */

#pragma once

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
    /// Constructs an IC element without loading a file.
    explicit IC(QGraphicsItem *parent = nullptr);

    /// Destructor; releases the internal ElementMapping.
    ~IC() override;

    /// Copy-constructs by delegating to the parent item constructor.
    IC(const IC &other) : IC(other.parentItem()) {}

    /**
     * \brief Copies IC-related files from \a srcPath to \a destPath.
     * \param srcPath  Source .panda file info.
     * \param destPath Destination .panda file info.
     */
    static void copyFiles(const QFileInfo &srcPath, const QFileInfo &destPath);
    /// Loads the IC circuit from \a fileName and rebuilds the logic mapping.
    void loadFile(const QString &fileName);

    // --- Logic mapping ---

    /**
     * \brief Builds and returns the flat LogicElement mapping for simulation.
     * \return Vector of shared_ptr LogicElements representing the IC's internals.
     */
    const QVector<std::shared_ptr<LogicElement>> generateMap();
    /// \reimp
    QVector<std::shared_ptr<LogicElement>> getLogicElementsForMapping() override;

    // --- Logic access ---

    /// \reimp
    LogicElement *getInputLogic(int portIndex) const override;
    /// \reimp
    LogicElement *getOutputLogic(int portIndex) const override;
    // --- Port information ---

    /// \reimp
    int getInputIndexForPort(int portIndex) const override;
    /// \reimp
    int getOutputIndexForPort(int portIndex) const override;

    // --- Port naming ---

    /// \reimp Returns \c true; IC port names are editable.
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

    /// \reimp
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    /// \reimp
    void refresh() override;

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
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

    // --- File copy helper ---

    void copyFile();

    // --- Logic access ---

    LogicElement *inputLogic(int index);
    LogicElement *outputLogic(int index);

    // --- Loading helpers ---

    void loadInputElement(GraphicElement *elm);
    void loadInputs();
    void loadInputsLabels();
    void loadOutputElement(GraphicElement *elm);
    void loadOutputs();
    void loadOutputsLabels();

    // --- Visual helpers ---

    void generatePixmap();

    // --- Static file paths ---

    inline static QString destPath_;
    inline static QString srcPath_;
    inline static bool needToCopyFiles = false;

    // --- Members ---

    ElementMapping *m_mapping = nullptr;
    QFileSystemWatcher m_fileWatcher;
    QString m_file;
    QVector<GraphicElement *> m_icElements;
    QVector<QNEPort *> m_icInputs;
    QVector<QNEPort *> m_icOutputs;
    QVector<QString> m_icInputLabels;
    QVector<QString> m_icOutputLabels;
};
