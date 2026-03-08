// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief BewavedDolphinUi: hand-written UI class for the BeWavedDolphin main window.
 */

#pragma once

// Specific Qt includes instead of monolithic QtWidgets
#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

class BewavedDolphin_Ui
{
public:
    BewavedDolphin_Ui() = default;

    BewavedDolphin_Ui(const BewavedDolphin_Ui&) = delete;
    BewavedDolphin_Ui& operator=(const BewavedDolphin_Ui&) = delete;

    /**
     * \brief Creates and lays out all child widgets inside \a BewavedDolphin.
     * \param BewavedDolphin The host QMainWindow.
     */
    void setupUi(QMainWindow *BewavedDolphin);

    /**
     * \brief Updates all translatable strings in the window.
     * \param BewavedDolphin The host QMainWindow.
     */
    void retranslateUi(QMainWindow *BewavedDolphin);

    // --- Actions: File Menu ---

    QAction *actionLoad = nullptr;
    QAction *actionSave = nullptr;
    QAction *actionSaveAs = nullptr;
    QAction *actionExportToPdf = nullptr;
    QAction *actionExportToPng = nullptr;
    QAction *actionExit = nullptr;

    // --- Actions: Edit Menu ---

    QAction *actionCopy = nullptr;
    QAction *actionCut = nullptr;
    QAction *actionPaste = nullptr;
    QAction *actionClear = nullptr;
    QAction *actionInvert = nullptr;
    QAction *actionSetTo0 = nullptr;
    QAction *actionSetTo1 = nullptr;
    QAction *actionSetClockWave = nullptr;
    QAction *actionSetLength = nullptr;
    QAction *actionCombinational = nullptr;
    QAction *actionAutoCrop = nullptr;
    QAction *actionMerge = nullptr;
    QAction *actionSplit = nullptr;

    // --- Actions: View Menu ---

    QAction *actionShowNumbers = nullptr;
    QAction *actionShowWaveforms = nullptr;
    QAction *actionZoomIn = nullptr;
    QAction *actionZoomOut = nullptr;
    QAction *actionFitScreen = nullptr;
    QAction *actionResetZoom = nullptr;

    // --- Actions: About Menu ---

    QAction *actionAbout = nullptr;
    QAction *actionAboutQt = nullptr;

    // --- Window Layout Widgets ---

    QWidget *centralwidget = nullptr;
    QVBoxLayout *verticalLayout = nullptr;
    QStatusBar *statusbar = nullptr;
    QToolBar *mainToolBar = nullptr;

    // --- Menus ---

    QMenuBar *menubar = nullptr;
    QMenu *menuFile = nullptr;
    QMenu *menuAbout = nullptr;
    QMenu *menuEdit = nullptr;
    QMenu *menuView = nullptr;
};
