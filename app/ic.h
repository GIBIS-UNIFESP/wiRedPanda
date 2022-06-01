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
    Q_PROPERTY(QString pixmapPath MEMBER m_pixmapPath CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

    friend class CodeGenerator;

public:
    explicit IC(QGraphicsItem *parent = nullptr);
    ~IC() override;

    ICPrototype *prototype();
    QString file() const;
    QVector<GraphicElement *> elements() const;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version) override;
    void loadFile(const QString &fileName);
    void save(QDataStream &stream) const override;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    void loadInputs(ICPrototype *prototype);
    void loadOutputs(ICPrototype *prototype);

    QString m_file;
    const QString m_pixmapPath = ":/basic/ic.png";
    const QString m_titleText = tr("<b>INTEGRATED CIRCUIT</b>");
    const QString m_translatedName = tr("IC");
};

Q_DECLARE_METATYPE(IC)
