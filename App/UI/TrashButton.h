// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPushButton>

class TrashButton : public QPushButton
{
    Q_OBJECT

public:
    explicit TrashButton(QWidget *parent = nullptr);

signals:
    void removeICFile(const QString &icFileName);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};
