/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "enums.h"

#include <QFrame>
#include <QLabel>
#include <QMimeData>

class ElementLabel : public QFrame
{
    Q_OBJECT

public:
    explicit ElementLabel(const QPixmap *pixmap, ElementType type, const QString &icFileName, QWidget *parent = nullptr);
    explicit ElementLabel(const QPixmap &pixmap, ElementType type, const QString &icFileName, QWidget *parent = nullptr);

    QMimeData *mimeData();
    QString icFileName() const;
    QString name() const;
    const QPixmap &pixmap() const;
    void startDrag();
    void updateName();
    void updateTheme();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    ElementType m_elementType = ElementType::Unknown;
    QLabel m_iconLabel;
    QLabel m_nameLabel;
    QPixmap m_pixmap;
    QString m_icFileName;
};
