/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LISTITEMWIDGET_H
#define LISTITEMWIDGET_H

#include <QFrame>

#include "label.h"

class ListItemWidget : public QFrame
{
    Q_OBJECT
private:
    Label *label;
    QLabel *nameLabel;

public:
    explicit ListItemWidget(const QPixmap &pixmap, ElementType elementType, const QString &icFileName, QWidget *parent = nullptr);

    Label *getLabel() const;

    void updateName();

protected:
    void mousePressEvent(QMouseEvent *event) override;

signals:

public slots:
};

#endif /* LISTITEMWIDGET_H */
