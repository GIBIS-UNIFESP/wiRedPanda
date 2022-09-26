// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "elementmapping.h"
#include "graphicelement.h"

#include <QFileInfo>
#include <QFileSystemWatcher>

class ICPrototype;

class IC : public GraphicElement
{
    Q_OBJECT

    friend class CodeGenerator;

public:
    explicit IC(QGraphicsItem *parent = nullptr);
    ~IC() override = default;
    IC(const IC &other) : IC(other.parentItem()) {}

    static void copyFiles(const QFileInfo &srcFile);

    ElementMapping *generateMap() const;
    LogicElement *inputLogic(const int index);
    LogicElement *outputLogic(const int index);
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version) override;
    void loadFile(const QString &fileName);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void refresh() override;
    void save(QDataStream &stream) const override;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    static bool comparePorts(QNEPort *port1, QNEPort *port2);
    static void sortPorts(QVector<QNEPort *> &map);
    inline static bool needToCopyFiles = false;
    inline static QString path;

    void copyFile();
    void generatePixmap();
    void loadInputElement(GraphicElement *elm);
    void loadInputs();
    void loadInputsLabels();
    void loadOutputElement(GraphicElement *elm);
    void loadOutputs();
    void loadOutputsLabels();

    QFileSystemWatcher m_fileWatcher;
    QString m_file;
    QVector<GraphicElement *> m_icElements;
    QVector<QNEPort *> m_icInputs;
    QVector<QNEPort *> m_icOutputs;
    QVector<QString> m_icInputLabels;
    QVector<QString> m_icOutputLabels;
};

Q_DECLARE_METATYPE(IC)
