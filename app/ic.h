/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class Editor;
class ICPrototype;

class IC : public GraphicElement
{
    Q_OBJECT

    friend class CodeGenerator;

public:
    explicit IC(QGraphicsItem *parent = nullptr);
    ~IC() override;

    void save(QDataStream &ds) const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    void loadFile(const QString &fname);
    QString getFile() const;
    ICPrototype *getPrototype();
    QVector<GraphicElement *> getElements() const;
    void setSkin(bool defaultSkin, const QString &filename) override;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QString m_file;

    void loadInputs(ICPrototype *prototype);
    void loadOutputs(ICPrototype *prototype);
};
