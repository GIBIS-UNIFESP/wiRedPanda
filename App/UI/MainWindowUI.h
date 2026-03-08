// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief MainWindowUi: hand-written UI class for the MainWindow.
 */

#pragma once

#include "App/UI/ElementEditor.h"
#include "App/UI/TrashButton.h"

// Specific Qt includes instead of monolithic QtWidgets
#include <QMainWindow>
#include <QAction>
#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QScrollArea>
#include <QSpacerItem>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>

class MainWindowUi
{
public:
    // --- Lifecycle ---

    MainWindowUi() = default;

    MainWindowUi(const MainWindowUi&) = delete;
    MainWindowUi& operator=(const MainWindowUi&) = delete;

    // --- Setup ---

    /**
     * \brief Creates and lays out all child widgets inside \a MainWindow.
     * \param MainWindow The host QMainWindow.
     */
    void setupUi(QMainWindow *MainWindow);

    /// Updates all translatable strings in the window.
    void retranslateUi();

    // --- Actions ---

    // File actions
    QAction *actionNew = nullptr;
    QAction *actionOpen = nullptr;
    QAction *actionSave = nullptr;
    QAction *actionSaveAs = nullptr;
    QAction *actionReloadFile = nullptr;
    QAction *actionExit = nullptr;

    // Edit actions
    QAction *actionCut = nullptr;
    QAction *actionCopy = nullptr;
    QAction *actionPaste = nullptr;
    QAction *actionDelete = nullptr;
    QAction *actionSelectAll = nullptr;
    QAction *actionClearSelection = nullptr;
    QAction *actionRename = nullptr;
    QAction *actionChangeTrigger = nullptr;

    // Transform actions
    QAction *actionRotateRight = nullptr;
    QAction *actionRotateLeft = nullptr;
    QAction *actionFlipHorizontally = nullptr;
    QAction *actionFlipVertically = nullptr;

    // View actions
    QAction *actionZoomIn = nullptr;
    QAction *actionZoomOut = nullptr;
    QAction *actionResetZoom = nullptr;
    QAction *actionFullscreen = nullptr;
    QAction *actionFastMode = nullptr;
    QAction *actionGates = nullptr;
    QAction *actionWires = nullptr;
    QAction *actionLabelsUnderIcons = nullptr;
    QAction *actionLightTheme = nullptr;
    QAction *actionDarkTheme = nullptr;

    // Simulation actions
    QAction *actionPlay = nullptr;
    QAction *actionWaveform = nullptr;
    QAction *actionMute = nullptr;
    QAction *actionRestart = nullptr;
    QAction *actionBackground_Simulation = nullptr;

    // Export actions
    QAction *actionExportToArduino = nullptr;
    QAction *actionExportToPdf = nullptr;
    QAction *actionExportToImage = nullptr;

    // Help / about actions
    QAction *actionAbout = nullptr;
    QAction *actionAboutQt = nullptr;
    QAction *actionAboutThisVersion = nullptr;
    QAction *actionShortcutsAndTips = nullptr;
    QAction *actionReportTranslationError = nullptr;

    // --- Central Layout ---

    QWidget *centralWidget = nullptr;
    QGridLayout *gridLayout_8 = nullptr;
    QSplitter *splitter = nullptr;
    QTabWidget *tab = nullptr;

    // --- Left Panel (Palette) ---

    QWidget *leftPannel = nullptr;
    QVBoxLayout *verticalLayout_5 = nullptr;
    QGridLayout *gridLayout = nullptr;
    QTabWidget *tabElements = nullptr;

    // I/O tab
    QWidget *io = nullptr;
    QGridLayout *gridLayout_2 = nullptr;
    QScrollArea *scrollAreaInOut = nullptr;
    QWidget *scrollAreaWidgetContents_InOut = nullptr;
    QVBoxLayout *verticalLayout = nullptr;
    QSpacerItem *verticalSpacer_InOut = nullptr;

    // Gates tab
    QWidget *gates = nullptr;
    QGridLayout *gridLayout_6 = nullptr;
    QScrollArea *scrollAreaGates = nullptr;
    QWidget *scrollAreaWidgetContents_Gates = nullptr;
    QVBoxLayout *verticalLayout_2 = nullptr;
    QSpacerItem *verticalSpacer_Gates = nullptr;

    // Combinational tab
    QWidget *combinational = nullptr;
    QGridLayout *gridLayout_10 = nullptr;
    QScrollArea *scrollArea_Combinational = nullptr;
    QWidget *scrollAreaWidgetContents_Combinational = nullptr;
    QVBoxLayout *verticalLayout_8 = nullptr;
    QSpacerItem *verticalSpacer_Combinational = nullptr;

    // Memory tab
    QWidget *memory = nullptr;
    QGridLayout *gridLayout_5 = nullptr;
    QScrollArea *scrollAreaMemory = nullptr;
    QWidget *scrollAreaWidgetContents_Memory = nullptr;
    QVBoxLayout *verticalLayout_3 = nullptr;
    QSpacerItem *verticalSpacer_Memory = nullptr;

    // IC tab
    QWidget *ic = nullptr;
    QGridLayout *gridLayout_4 = nullptr;
    QScrollArea *scrollArea_Box = nullptr;
    QWidget *scrollAreaWidgetContents_IC = nullptr;
    QVBoxLayout *verticalLayout_4 = nullptr;
    QSpacerItem *verticalSpacer_IC = nullptr;
    QHBoxLayout *horizontalLayout = nullptr;
    QPushButton *pushButtonAddIC = nullptr;
    TrashButton *pushButtonRemoveIC = nullptr;

    // Misc tab
    QWidget *misc = nullptr;
    QGridLayout *gridLayout_3 = nullptr;
    QScrollArea *scrollArea_Misc = nullptr;
    QWidget *scrollAreaWidgetContents_Misc = nullptr;
    QVBoxLayout *verticalLayout_6 = nullptr;
    QSpacerItem *verticalSpacer_Misc = nullptr;

    // Search tab
    QWidget *search = nullptr;
    QGridLayout *gridLayout_9 = nullptr;
    QScrollArea *scrollArea_Search = nullptr;
    QWidget *scrollAreaWidgetContents_Search = nullptr;
    QVBoxLayout *verticalLayout_7 = nullptr;
    QSpacerItem *verticalSpacer_Search = nullptr;

    // Search bar
    QLabel *label = nullptr;
    QLineEdit *lineEditSearch = nullptr;

    // --- Element Editor Panel ---

    ElementEditor *elementEditor = nullptr;

    // --- Toolbars & Status ---

    QToolBar *mainToolBar = nullptr;
    QStatusBar *statusBar = nullptr;

    // --- Menu Bar & Menus ---

    QMenuBar *menuBar = nullptr;
    QMenu *menuFile = nullptr;
    QMenu *menuRecentFiles = nullptr;
    QMenu *menuEdit = nullptr;
    QMenu *menuView = nullptr;
    QMenu *menuTheme = nullptr;
    QMenu *menuLanguage = nullptr;
    QMenu *menuTranslation = nullptr;
    QMenu *menuSimulation = nullptr;
    QMenu *menuExamples = nullptr;
    QMenu *menuHelp = nullptr;

};
