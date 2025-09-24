// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "elementeditor.h"
#include "trashbutton.h"

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

class MainWindow_Ui
{
public:
    // Default constructor (required for instantiation)
    MainWindow_Ui() = default;

    // Delete copy constructor and assignment operator
    MainWindow_Ui(const MainWindow_Ui&) = delete;
    MainWindow_Ui& operator=(const MainWindow_Ui&) = delete;

    void setupUi(QMainWindow *MainWindow);
    void retranslateUi();

    QAction *actionOpen = nullptr;
    QAction *actionSave = nullptr;
    QAction *actionSaveAs = nullptr;
    QAction *actionCut = nullptr;
    QAction *actionCopy = nullptr;
    QAction *actionPaste = nullptr;
    QAction *actionDelete = nullptr;
    QAction *actionNew = nullptr;
    QAction *actionZoomIn = nullptr;
    QAction *actionZoomOut = nullptr;
    QAction *actionWires = nullptr;
    QAction *actionGates = nullptr;
    QAction *actionAbout = nullptr;
    QAction *actionExit = nullptr;
    QAction *actionRotateRight = nullptr;
    QAction *actionRotateLeft = nullptr;
    QAction *actionAboutQt = nullptr;
    QAction *actionSelectAll = nullptr;
    QAction *actionReloadFile = nullptr;
    QAction *actionExportToArduino = nullptr;
    QAction *actionExportToVerilog = nullptr;
    QAction *actionResetZoom = nullptr;
    QAction *actionExportToPdf = nullptr;
    QAction *actionPlay = nullptr;
    QAction *actionRename = nullptr;
    QAction *actionChangeTrigger = nullptr;
    QAction *actionClearSelection = nullptr;
    QAction *actionFastMode = nullptr;
    QAction *actionLightTheme = nullptr;
    QAction *actionDarkTheme = nullptr;
    QAction *actionWaveform = nullptr;
    QAction *actionExportToImage = nullptr;
    QAction *actionFlipHorizontally = nullptr;
    QAction *actionFlipVertically = nullptr;
    QAction *actionFullscreen = nullptr;
    QAction *actionMute = nullptr;
    QAction *actionLabelsUnderIcons = nullptr;
    QAction *actionAboutThisVersion = nullptr;
    QAction *actionRestart = nullptr;
    QAction *actionBackground_Simulation = nullptr;
    QAction *actionShortcutsAndTips = nullptr;
    QAction *actionReportTranslationError = nullptr;
    QWidget *centralWidget = nullptr;
    QGridLayout *gridLayout_8 = nullptr;
    QSplitter *splitter = nullptr;
    QWidget *leftPannel = nullptr;
    QVBoxLayout *verticalLayout_5 = nullptr;
    QGridLayout *gridLayout = nullptr;
    QTabWidget *tabElements = nullptr;
    QWidget *io = nullptr;
    QGridLayout *gridLayout_2 = nullptr;
    QScrollArea *scrollAreaInOut = nullptr;
    QWidget *scrollAreaWidgetContents_InOut = nullptr;
    QVBoxLayout *verticalLayout = nullptr;
    QSpacerItem *verticalSpacer_InOut = nullptr;
    QWidget *gates = nullptr;
    QGridLayout *gridLayout_6 = nullptr;
    QScrollArea *scrollAreaGates = nullptr;
    QWidget *scrollAreaWidgetContents_Gates = nullptr;
    QVBoxLayout *verticalLayout_2 = nullptr;
    QSpacerItem *verticalSpacer_Gates = nullptr;
    QWidget *combinational = nullptr;
    QGridLayout *gridLayout_10 = nullptr;
    QScrollArea *scrollArea_Combinational = nullptr;
    QWidget *scrollAreaWidgetContents_Combinational = nullptr;
    QVBoxLayout *verticalLayout_8 = nullptr;
    QSpacerItem *verticalSpacer_Combinational = nullptr;
    QWidget *memory = nullptr;
    QGridLayout *gridLayout_5 = nullptr;
    QScrollArea *scrollAreaMemory = nullptr;
    QWidget *scrollAreaWidgetContents_Memory = nullptr;
    QVBoxLayout *verticalLayout_3 = nullptr;
    QSpacerItem *verticalSpacer_Memory = nullptr;
    QWidget *ic = nullptr;
    QGridLayout *gridLayout_4 = nullptr;
    QScrollArea *scrollArea_Box = nullptr;
    QWidget *scrollAreaWidgetContents_IC = nullptr;
    QVBoxLayout *verticalLayout_4 = nullptr;
    QSpacerItem *verticalSpacer_IC = nullptr;
    QHBoxLayout *horizontalLayout = nullptr;
    QPushButton *pushButtonAddIC = nullptr;
    TrashButton *pushButtonRemoveIC = nullptr;
    QWidget *misc = nullptr;
    QGridLayout *gridLayout_3 = nullptr;
    QScrollArea *scrollArea_Misc = nullptr;
    QWidget *scrollAreaWidgetContents_Misc = nullptr;
    QVBoxLayout *verticalLayout_6 = nullptr;
    QSpacerItem *verticalSpacer_Misc = nullptr;
    QWidget *search = nullptr;
    QGridLayout *gridLayout_9 = nullptr;
    QScrollArea *scrollArea_Search = nullptr;
    QWidget *scrollAreaWidgetContents_Search = nullptr;
    QVBoxLayout *verticalLayout_7 = nullptr;
    QSpacerItem *verticalSpacer_Search = nullptr;
    ElementEditor *elementEditor = nullptr;
    QLabel *label = nullptr;
    QLineEdit *lineEditSearch = nullptr;
    QTabWidget *tab = nullptr;
    QToolBar *mainToolBar = nullptr;
    QStatusBar *statusBar = nullptr;
    QMenuBar *menuBar = nullptr;
    QMenu *menuFile = nullptr;
    QMenu *menuRecentFiles = nullptr;
    QMenu *menuEdit = nullptr;
    QMenu *menuHelp = nullptr;
    QMenu *menuTranslation = nullptr;
    QMenu *menuView = nullptr;
    QMenu *menuTheme = nullptr;
    QMenu *menuLanguage = nullptr;
    QMenu *menuSimulation = nullptr;
    QMenu *menuExamples = nullptr;

};
