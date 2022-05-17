/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "label.h"

#include <QFrame>

class ListItemWidget : public QFrame
{
    Q_OBJECT

public:
    explicit ListItemWidget(const QPixmap *pixmap, ElementType type, const QString &icFileName, QWidget *parent = nullptr);
    explicit ListItemWidget(const QPixmap &pixmap, ElementType type, const QString &icFileName, QWidget *parent = nullptr);

    Label *label() const;
    void updateName();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    Label *m_label;
    QLabel *m_nameLabel;
};

