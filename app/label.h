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

    ~Label() override;
    ElementType elementType();
    void setElementType(ElementType elementType);
    QString auxData() const;
    void setAuxData(const QString &auxData);
    void startDrag(QPoint pos = QPoint());
    QString name() const;
    void setName(const QString &name);

    const QPixmap &pixmapData() const;
    void setPixmapData(const QPixmap &pixmapData);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    ElementType m_elementType;
    QPixmap m_pixmapData;
    QString m_name;
    QString m_auxData;
};

