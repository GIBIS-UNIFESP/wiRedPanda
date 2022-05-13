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
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)

    friend class CodeGenerator;

public:
    explicit IC(QGraphicsItem *parent = nullptr);
    ~IC() override;

    ICPrototype *getPrototype();
    QString getFile() const;
    QVector<GraphicElement *> getElements() const;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    void loadFile(const QString &fname);
    void save(QDataStream &ds) const override;
    void setSkin(bool defaultSkin, const QString &filename) override;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    void loadInputs(ICPrototype *prototype);
    void loadOutputs(ICPrototype *prototype);

    QString m_file;
    const QString m_titleText = tr("<b>INTEGRATED CIRCUIT</b>");
    const QString m_translatedName = tr("IC");
    const QString m_pixmap = ":/basic/box.png";
};

Q_DECLARE_METATYPE(IC)
