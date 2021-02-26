/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LISTITEMWIDGET_H
#define LISTITEMWIDGET_H

#include <QFrame>

#include "label.h"

class QMenu;

class ListItemWidget : public QFrame
{
    Q_OBJECT
private:
    Label *m_label;
    QLabel *m_nameLabel;
    QMenu *m_menu;

public:
    explicit ListItemWidget(const QPixmap &pixmap, ElementType elementType, const QString &icFileName, QWidget *parent = nullptr);

    Label *getLabel() const;

    void updateName();

    QString getFileName();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void setupIcMenu();

signals:
    void removeIC(ListItemWidget *item);

public slots:
};

#endif /* LISTITEMWIDGET_H */
