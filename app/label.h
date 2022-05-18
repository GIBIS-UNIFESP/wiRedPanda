/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "elementtype.h"

#include <QLabel>

class Label : public QLabel
{
    Q_OBJECT

public:
    explicit Label(QWidget *parent = nullptr);
    ~Label() override = default;

    ElementType elementType();
    QString auxData() const;
    QString name() const;
    const QPixmap &pixmapData() const;
    void setAuxData(const QString &auxData);
    void setElementType(const ElementType elementType);
    void setName(const QString &name);
    void setPixmapData(const QPixmap &pixmapData);
    void startDrag(QPoint pos = {});

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    ElementType m_elementType = ElementType::Unknown;
    QPixmap m_pixmapData;
    QString m_auxData;
    QString m_name;
};

