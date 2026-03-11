// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFrame>
#include <QLabel>
#include <QMimeData>

#include "App/Core/Enums.h"

class ElementLabel : public QFrame
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit ElementLabel(const QPixmap *pixmap, ElementType type, const QString &icFileName, QWidget *parent = nullptr);
    explicit ElementLabel(const QPixmap &pixmap, ElementType type, const QString &icFileName, QWidget *parent = nullptr);

    // --- Queries ---

    const ElementType &elementType() const;
    QString icFileName() const;
    QString name() const;
    const QPixmap &pixmap() const;

    // --- Drag support ---

    QMimeData *mimeData();
    void startDrag();

    // --- Display updates ---

    void updateName();
    void updateTheme();

protected:
    // --- Qt event overrides ---

    void mousePressEvent(QMouseEvent *event) override;

private:
    // --- Members ---

    ElementType m_elementType = ElementType::Unknown;
    QLabel m_iconLabel;
    QLabel m_nameLabel;
    QPixmap m_pixmap;
    QString m_icFileName;
};
