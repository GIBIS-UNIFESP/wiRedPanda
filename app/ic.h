/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class ICPrototype;

class IC : public GraphicElement
{
    Q_OBJECT

    friend class CodeGenerator;

public:
    explicit IC(QGraphicsItem *parent = nullptr);
    ~IC() override;

    ICPrototype *prototype();
    QRectF boundingRect() const override;
    QString file() const;
    QVector<GraphicElement *> elements() const;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version) override;
    void loadFile(const QString &fileName);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void save(QDataStream &stream) const override;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QRectF portsBoundingRect() const;
    void loadInputs(ICPrototype *prototype);
    void loadOutputs(ICPrototype *prototype);

    QString m_file;
};

Q_DECLARE_METATYPE(IC)
