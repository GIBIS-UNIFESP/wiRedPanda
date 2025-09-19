// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// Specific Qt includes instead of monolithic QtWidgets
#include <QMainWindow>
#include <QAction>
#include <QWidget>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QToolBar>
#include <QMenuBar>
#include <QMenu>

class BewavedDolphin_Ui
{
public:
    // Default constructor (required for instantiation)
    BewavedDolphin_Ui() = default;

    // Delete copy constructor and assignment operator
    BewavedDolphin_Ui(const BewavedDolphin_Ui&) = delete;
    BewavedDolphin_Ui& operator=(const BewavedDolphin_Ui&) = delete;

    void setupUi(QMainWindow *BewavedDolphin);
    void retranslateUi(QMainWindow *BewavedDolphin);

    QAction *actionAbout = nullptr;
    QAction *actionLoad = nullptr;
    QAction *actionExit = nullptr;
    QAction *actionCombinational = nullptr;
    QAction *actionExportToPdf = nullptr;
    QAction *actionSave = nullptr;
    QAction *actionSaveAs = nullptr;
    QAction *actionCopy = nullptr;
    QAction *actionPaste = nullptr;
    QAction *actionSetTo0 = nullptr;
    QAction *actionSetTo1 = nullptr;
    QAction *actionSetClockWave = nullptr;
    QAction *actionInvert = nullptr;
    QAction *actionMerge = nullptr;
    QAction *actionSplit = nullptr;
    QAction *actionExportToPng = nullptr;
    QAction *actionSetLength = nullptr;
    QAction *actionAboutQt = nullptr;
    QAction *actionZoomIn = nullptr;
    QAction *actionZoomOut = nullptr;
    QAction *actionFitScreen = nullptr;
    QAction *actionResetZoom = nullptr;
    QAction *actionClear = nullptr;
    QAction *actionCut = nullptr;
    QAction *actionShowNumbers = nullptr;
    QAction *actionShowWaveforms = nullptr;
    QAction *actionAutoCrop = nullptr;
    QWidget *centralwidget = nullptr;
    QVBoxLayout *verticalLayout = nullptr;
    QStatusBar *statusbar = nullptr;
    QToolBar *mainToolBar = nullptr;
    QMenuBar *menubar = nullptr;
    QMenu *menuFile = nullptr;
    QMenu *menuAbout = nullptr;
    QMenu *menuEdit = nullptr;
    QMenu *menuView = nullptr;
};
