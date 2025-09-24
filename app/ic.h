// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "elementmapping.h"
#include "graphicelement.h"

#include <QFileInfo>
#include <QFileSystemWatcher>

class IC : public GraphicElement
{
    Q_OBJECT

    friend class CodeGenerator;
    friend class TestArduino;

public:
    explicit IC(QGraphicsItem *parent = nullptr);
    ~IC() override;
    IC(const IC &other) : IC(other.parentItem()) {}

    static void copyFiles(const QFileInfo &srcPath, const QFileInfo &destPath);

    const QVector<std::shared_ptr<LogicElement>> generateMap();
    LogicElement *inputLogic(const int index);
    LogicElement *outputLogic(const int index);
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void loadFile(const QString &fileName);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void refresh() override;
    void save(QDataStream &stream) const override;

    // Polymorphic interface overrides
    LogicElement *getInputLogic(int portIndex) const override;
    LogicElement *getOutputLogic(int portIndex) const override;
    int getInputIndexForPort(int portIndex) const override;
    int getOutputIndexForPort(int portIndex) const override;
    QVector<std::shared_ptr<LogicElement>> getLogicElementsForMapping() override;
    bool canSetPortNames() const override;
    void setInputPortName(int port, const QString &name) override;
    void setOutputPortName(int port, const QString &name) override;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    static bool comparePorts(QNEPort *port1, QNEPort *port2);
    static void sortPorts(QVector<QNEPort *> &map);

    inline static QString destPath_;
    inline static QString srcPath_;
    inline static bool needToCopyFiles = false;

    void copyFile();
    void generatePixmap();
    void loadInputElement(GraphicElement *elm);
    void loadInputs();
    void loadInputsLabels();
    void loadOutputElement(GraphicElement *elm);
    void loadOutputs();
    void loadOutputsLabels();

    ElementMapping *mapping = nullptr;
    QFileSystemWatcher m_fileWatcher;
    QString m_file;
    QVector<GraphicElement *> m_icElements;
    QVector<QNEPort *> m_icInputs;
    QVector<QNEPort *> m_icOutputs;
    QVector<QString> m_icInputLabels;
    QVector<QString> m_icOutputLabels;
};

Q_DECLARE_METATYPE(IC)
