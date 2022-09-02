// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "elementmapping.h"
#include "graphicelement.h"

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

    ElementMapping *generateMap() const;
    LogicElement *icInput(const int index);
    LogicElement *icOutput(const int index);
    QRectF boundingRect() const override;
    QString file() const;
    QVector<GraphicElement *> elements() const;
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

    QRectF portsBoundingRect() const;
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
