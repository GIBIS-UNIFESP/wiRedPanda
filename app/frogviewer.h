/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "workspace.h"

#include <QWidget>

namespace Ui {
class FrogViewer;
}

class FrogViewer : public QWidget
{
    Q_OBJECT

public:
    explicit FrogViewer(QWidget *parent = nullptr);
    ~FrogViewer();

    void setTab(WorkSpace *newTab);

private:
    void on_closeButton_clicked();
    void on_gifButton_clicked();
    void on_nextButton_clicked();
    void on_previousButton_clicked();
    void on_recButton_clicked();
    void processSignal(GraphicElementInput *elm);

    Ui::FrogViewer *ui;
    WorkSpace *m_currentTab = nullptr;
};
