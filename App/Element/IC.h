// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFileInfo>
#include <QFileSystemWatcher>

#include "App/Element/GraphicElement.h"
#include "App/Simulation/ElementMapping.h"

class IC : public GraphicElement
{
    Q_OBJECT

    friend class ArduinoCodeGen;

public:
    // --- Lifecycle ---

    explicit IC(QGraphicsItem *parent = nullptr);
    ~IC() override;
    IC(const IC &other) : IC(other.parentItem()) {}

    // --- File operations ---

    static void copyFiles(const QFileInfo &srcPath, const QFileInfo &destPath);
    void loadFile(const QString &fileName);

    // --- Logic mapping ---

    const QVector<std::shared_ptr<LogicElement>> generateMap();
    QVector<std::shared_ptr<LogicElement>> getLogicElementsForMapping() override;

    // --- Logic access ---

    LogicElement *getInputLogic(int portIndex) const override;
    LogicElement *getOutputLogic(int portIndex) const override;

    // --- Port information ---

    int getInputIndexForPort(int portIndex) const override;
    int getOutputIndexForPort(int portIndex) const override;

    // --- Port naming ---

    bool canSetPortNames() const override;
    void setInputPortName(int port, const QString &name) override;
    void setOutputPortName(int port, const QString &name) override;

    // --- Visual ---

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void refresh() override;

    // --- Serialization ---

    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void save(QDataStream &stream) const override;

protected:
    // --- Event handlers ---

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    // --- Utility methods ---

    static bool comparePorts(QNEPort *port1, QNEPort *port2);
    static void sortPorts(QVector<QNEPort *> &map);

    // --- File copy helper ---

    LogicElement *inputLogic(int index);
    LogicElement *outputLogic(int index);
    void copyFile();

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

Q_DECLARE_METATYPE(IC)

