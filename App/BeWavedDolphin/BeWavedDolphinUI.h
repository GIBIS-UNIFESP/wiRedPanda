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

/**
 * \class BewavedDolphinUi
 * \brief Hand-written UI descriptor for the BeWavedDolphin main window.
 *
 * \details Replaces the Qt Designer .ui workflow.  Call setupUi() once after
 * construction to build the widget hierarchy, and retranslateUi() whenever the
 * application language changes.
 */
class BewavedDolphinUi
{
public:
    BewavedDolphinUi() = default;

    BewavedDolphinUi(const BewavedDolphinUi&) = delete;
    BewavedDolphinUi& operator=(const BewavedDolphinUi&) = delete;

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
