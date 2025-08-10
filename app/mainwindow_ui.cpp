// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow_ui.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QMetaObject>

void MainWindow_Ui::setupUi(QMainWindow *MainWindow)
{
    if (MainWindow->objectName().isEmpty()) {
        MainWindow->setObjectName("MainWindow");
    }

    MainWindow->resize(886, 765);
    MainWindow->setMinimumSize(QSize(0, 150));
    MainWindow->setWindowTitle("");
    MainWindow->setWindowIcon(QIcon(":/toolbar/wpanda.svg"));
    MainWindow->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    MainWindow->setUnifiedTitleAndToolBarOnMac(true);

    actionOpen = new QAction(MainWindow);
    actionOpen->setObjectName("actionOpen");
    actionOpen->setIcon(QIcon(":/toolbar/folder.svg"));
    actionSave = new QAction(MainWindow);
    actionSave->setObjectName("actionSave");
    actionSave->setIcon(QIcon(":/toolbar/save.svg"));
    actionSaveAs = new QAction(MainWindow);
    actionSaveAs->setObjectName("actionSaveAs");
    actionSaveAs->setIcon(QIcon(":/toolbar/save.svg"));
    actionCut = new QAction(MainWindow);
    actionCut->setObjectName("actionCut");
    actionCut->setIcon(QIcon(":/toolbar/cut.svg"));
    actionCopy = new QAction(MainWindow);
    actionCopy->setObjectName("actionCopy");
    actionCopy->setIcon(QIcon(":/toolbar/copy.svg"));
    actionPaste = new QAction(MainWindow);
    actionPaste->setObjectName("actionPaste");
    actionPaste->setIcon(QIcon(":/toolbar/paste.svg"));
    actionDelete = new QAction(MainWindow);
    actionDelete->setObjectName("actionDelete");
    QIcon icon6;
    icon6.addFile(":/toolbar/delete.svg", QSize(), QIcon::Normal, QIcon::On);
    actionDelete->setIcon(icon6);
    actionNew = new QAction(MainWindow);
    actionNew->setObjectName("actionNew");
    actionNew->setIcon(QIcon(":/toolbar/new.svg"));
    actionZoomIn = new QAction(MainWindow);
    actionZoomIn->setObjectName("actionZoomIn");
    actionZoomIn->setIcon(QIcon(":/toolbar/zoomIn.svg"));
    actionZoomOut = new QAction(MainWindow);
    actionZoomOut->setObjectName("actionZoomOut");
    actionZoomOut->setIcon(QIcon(":/toolbar/zoomOut.svg"));
    actionWires = new QAction(MainWindow);
    actionWires->setObjectName("actionWires");
    actionWires->setCheckable(true);
    actionWires->setChecked(true);
    actionWires->setIcon(QIcon(":/toolbar/wires.svg"));
    actionGates = new QAction(MainWindow);
    actionGates->setObjectName("actionGates");
    actionGates->setCheckable(true);
    actionGates->setChecked(true);
    actionGates->setIcon(QIcon(":/basic/nor.svg"));
    actionAbout = new QAction(MainWindow);
    actionAbout->setObjectName("actionAbout");
    actionAbout->setIcon(QIcon(":/toolbar/help.svg"));
    actionExit = new QAction(MainWindow);
    actionExit->setObjectName("actionExit");
    actionExit->setIcon(QIcon(":/toolbar/exit.svg"));
    actionRotateRight = new QAction(MainWindow);
    actionRotateRight->setObjectName("actionRotateRight");
    actionRotateRight->setIcon(QIcon(":/toolbar/rotateR.svg"));
    actionRotateLeft = new QAction(MainWindow);
    actionRotateLeft->setObjectName("actionRotateLeft");
    actionRotateLeft->setIcon(QIcon(":/toolbar/rotateL.svg"));
    actionAboutQt = new QAction(MainWindow);
    actionAboutQt->setObjectName("actionAboutQt");
    actionAboutQt->setIcon(QIcon(":/toolbar/helpQt.svg"));
    actionSelectAll = new QAction(MainWindow);
    actionSelectAll->setObjectName("actionSelectAll");
    actionSelectAll->setIcon(QIcon(":/toolbar/selectAll.svg"));
    actionReloadFile = new QAction(MainWindow);
    actionReloadFile->setObjectName("actionReloadFile");
    actionReloadFile->setIcon(QIcon(":/toolbar/reloadFile.svg"));
    actionExportToArduino = new QAction(MainWindow);
    actionExportToArduino->setObjectName("actionExportToArduino");
    actionExportToArduino->setIcon(QIcon(":/toolbar/arduino.svg"));
    actionResetZoom = new QAction(MainWindow);
    actionResetZoom->setObjectName("actionResetZoom");
    actionResetZoom->setIcon(QIcon(":/toolbar/zoomReset.svg"));
    actionExportToPdf = new QAction(MainWindow);
    actionExportToPdf->setObjectName("actionExportToPdf");
    actionExportToPdf->setIcon(QIcon(":/toolbar/pdf.svg"));
    actionPlay = new QAction(MainWindow);
    actionPlay->setObjectName("actionPlay");
    actionPlay->setCheckable(true);
    QIcon icon22(":/toolbar/play.svg");
    icon22.addFile(":/toolbar/pause.svg", QSize(), QIcon::Normal, QIcon::On);
    actionPlay->setIcon(icon22);
    actionRename = new QAction(MainWindow);
    actionRename->setObjectName("actionRename");
    actionRename->setIcon(QIcon(":/toolbar/rename.svg"));
    actionChangeTrigger = new QAction(MainWindow);
    actionChangeTrigger->setObjectName("actionChangeTrigger");
    actionChangeTrigger->setIcon(QIcon(":/input/buttonOff.svg"));
    actionClearSelection = new QAction(MainWindow);
    actionClearSelection->setObjectName("actionClearSelection");
    actionClearSelection->setIcon(QIcon(":/toolbar/clearSelection.svg"));
    actionFastMode = new QAction(MainWindow);
    actionFastMode->setObjectName("actionFastMode");
    actionFastMode->setCheckable(true);
    actionFastMode->setIcon(QIcon(":/toolbar/fast.svg"));
    actionLightTheme = new QAction(MainWindow);
    actionLightTheme->setObjectName("actionLightTheme");
    actionLightTheme->setCheckable(true);
    actionLightTheme->setChecked(true);
    actionDarkTheme = new QAction(MainWindow);
    actionDarkTheme->setObjectName("actionDarkTheme");
    actionDarkTheme->setCheckable(true);
    actionWaveform = new QAction(MainWindow);
    actionWaveform->setObjectName("actionWaveform");
    actionWaveform->setIcon(QIcon(":/toolbar/dolphin_icon.svg"));
    actionExportToImage = new QAction(MainWindow);
    actionExportToImage->setObjectName("actionExportToImage");
    actionExportToImage->setIcon(QIcon(":/toolbar/png.svg"));
    actionFlipHorizontally = new QAction(MainWindow);
    actionFlipHorizontally->setObjectName("actionFlipHorizontally");
    actionFlipVertically = new QAction(MainWindow);
    actionFlipVertically->setObjectName("actionFlipVertically");
    actionFullscreen = new QAction(MainWindow);
    actionFullscreen->setObjectName("actionFullscreen");
    actionMute = new QAction(MainWindow);
    actionMute->setObjectName("actionMute");
    actionMute->setCheckable(true);
    QIcon icon29(":/output/buzzer/BuzzerOff.svg");
    icon29.addFile(":/toolbar/mute.svg", QSize(), QIcon::Normal, QIcon::On);
    actionMute->setIcon(icon29);
    actionLabelsUnderIcons = new QAction(MainWindow);
    actionLabelsUnderIcons->setObjectName("actionLabelsUnderIcons");
    actionLabelsUnderIcons->setCheckable(true);
    actionAboutThisVersion = new QAction(MainWindow);
    actionAboutThisVersion->setObjectName("actionAboutThisVersion");
    actionRestart = new QAction(MainWindow);
    actionRestart->setObjectName("actionRestart");
    actionRestart->setIcon(QIcon(":/toolbar/reset.svg"));
    actionBackground_Simulation = new QAction(MainWindow);
    actionBackground_Simulation->setObjectName("actionBackground_Simulation");
    actionBackground_Simulation->setCheckable(true);
    actionBackground_Simulation->setIcon(QIcon::fromTheme("media-playlist-repeat"));
    actionShortcutsAndTips = new QAction(MainWindow);
    actionShortcutsAndTips->setObjectName("actionShortcutsAndTips");
    actionShortcutsAndTips->setIcon(QIcon::fromTheme("help-about"));
    actionReportTranslationError = new QAction(MainWindow);
    actionReportTranslationError->setObjectName("actionReportTranslationError");
    actionReportTranslationError->setIcon(QIcon(":/toolbar/help.svg"));
    centralWidget = new QWidget(MainWindow);
    centralWidget->setObjectName("centralWidget");
    gridLayout_8 = new QGridLayout(centralWidget);
    gridLayout_8->setSpacing(6);
    gridLayout_8->setContentsMargins(11, 11, 11, 11);
    gridLayout_8->setObjectName("gridLayout_8");
    splitter = new QSplitter(centralWidget);
    splitter->setObjectName("splitter");
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
    splitter->setSizePolicy(sizePolicy);
    splitter->setFrameShape(QFrame::NoFrame);
    splitter->setLineWidth(1);
    splitter->setMidLineWidth(2);
    splitter->setOrientation(Qt::Horizontal);
    splitter->setOpaqueResize(false);
    splitter->setHandleWidth(8);
    splitter->setChildrenCollapsible(false);
    leftPannel = new QWidget(splitter);
    leftPannel->setObjectName("leftPannel");
    leftPannel->setMinimumSize(QSize(280, 0));
    leftPannel->setMaximumSize(QSize(600, 16777215));
    verticalLayout_5 = new QVBoxLayout(leftPannel);
    verticalLayout_5->setSpacing(0);
    verticalLayout_5->setContentsMargins(11, 11, 11, 11);
    verticalLayout_5->setObjectName("verticalLayout_5");
    verticalLayout_5->setContentsMargins(0, 0, 0, 0);
    gridLayout = new QGridLayout();
    gridLayout->setSpacing(6);
    gridLayout->setObjectName("gridLayout");
    tabElements = new QTabWidget(leftPannel);
    tabElements->setObjectName("tabElements");
    sizePolicy.setHeightForWidth(tabElements->sizePolicy().hasHeightForWidth());
    tabElements->setSizePolicy(sizePolicy);
    tabElements->setMinimumSize(QSize(220, 0));
    tabElements->setStyleSheet("QTabBar::tab:disabled {\n"
"	 width: 0;\n"
"	 height: 0;\n"
"	 margin: 0;\n"
"	 padding: 0;\n"
"	 border: none; \n"
"}\n"
"\n"
"QTabWidget::pane {\n"
"	margin: 0px 0px 0px 0px; \n"
"}\n");
    io = new QWidget();
    io->setObjectName("io");
    gridLayout_2 = new QGridLayout(io);
    gridLayout_2->setSpacing(6);
    gridLayout_2->setContentsMargins(11, 11, 11, 11);
    gridLayout_2->setObjectName("gridLayout_2");
    scrollAreaInOut = new QScrollArea(io);
    scrollAreaInOut->setObjectName("scrollAreaInOut");
    scrollAreaInOut->setFrameShape(QFrame::NoFrame);
    scrollAreaInOut->setWidgetResizable(true);
    scrollAreaWidgetContents_InOut = new QWidget();
    scrollAreaWidgetContents_InOut->setObjectName("scrollAreaWidgetContents_InOut");
    scrollAreaWidgetContents_InOut->setGeometry(QRect(0, 0, 260, 584));
    verticalLayout = new QVBoxLayout(scrollAreaWidgetContents_InOut);
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(11, 11, 11, 11);
    verticalLayout->setObjectName("verticalLayout");
    verticalSpacer_InOut = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout->addItem(verticalSpacer_InOut);

    scrollAreaInOut->setWidget(scrollAreaWidgetContents_InOut);

    gridLayout_2->addWidget(scrollAreaInOut, 0, 0, 1, 1);

    tabElements->addTab(io, QIcon(":/input/buttonOff.svg"), QString());
    gates = new QWidget();
    gates->setObjectName("gates");
    gridLayout_6 = new QGridLayout(gates);
    gridLayout_6->setSpacing(6);
    gridLayout_6->setContentsMargins(11, 11, 11, 11);
    gridLayout_6->setObjectName("gridLayout_6");
    scrollAreaGates = new QScrollArea(gates);
    scrollAreaGates->setObjectName("scrollAreaGates");
    scrollAreaGates->setFrameShape(QFrame::NoFrame);
    scrollAreaGates->setWidgetResizable(true);
    scrollAreaWidgetContents_Gates = new QWidget();
    scrollAreaWidgetContents_Gates->setObjectName("scrollAreaWidgetContents_Gates");
    scrollAreaWidgetContents_Gates->setGeometry(QRect(0, 0, 260, 584));
    verticalLayout_2 = new QVBoxLayout(scrollAreaWidgetContents_Gates);
    verticalLayout_2->setSpacing(6);
    verticalLayout_2->setContentsMargins(11, 11, 11, 11);
    verticalLayout_2->setObjectName("verticalLayout_2");
    verticalSpacer_Gates = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout_2->addItem(verticalSpacer_Gates);

    scrollAreaGates->setWidget(scrollAreaWidgetContents_Gates);

    gridLayout_6->addWidget(scrollAreaGates, 0, 1, 1, 1);

    tabElements->addTab(gates, QIcon(":/basic/xor.svg"), QString());
    combinational = new QWidget();
    combinational->setObjectName("combinational");
    gridLayout_10 = new QGridLayout(combinational);
    gridLayout_10->setSpacing(6);
    gridLayout_10->setContentsMargins(11, 11, 11, 11);
    gridLayout_10->setObjectName("gridLayout_10");
    scrollArea_Combinational = new QScrollArea(combinational);
    scrollArea_Combinational->setObjectName("scrollArea_Combinational");
    scrollArea_Combinational->setFrameShape(QFrame::NoFrame);
    scrollArea_Combinational->setWidgetResizable(true);
    scrollAreaWidgetContents_Combinational = new QWidget();
    scrollAreaWidgetContents_Combinational->setObjectName("scrollAreaWidgetContents_Combinational");
    scrollAreaWidgetContents_Combinational->setGeometry(QRect(0, 0, 260, 582));
    QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(scrollAreaWidgetContents_Combinational->sizePolicy().hasHeightForWidth());
    scrollAreaWidgetContents_Combinational->setSizePolicy(sizePolicy1);
    verticalLayout_8 = new QVBoxLayout(scrollAreaWidgetContents_Combinational);
    verticalLayout_8->setSpacing(6);
    verticalLayout_8->setContentsMargins(11, 11, 11, 11);
    verticalLayout_8->setObjectName("verticalLayout_8");
    verticalSpacer_Combinational = new QSpacerItem(20, 560, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout_8->addItem(verticalSpacer_Combinational);

    scrollArea_Combinational->setWidget(scrollAreaWidgetContents_Combinational);

    gridLayout_10->addWidget(scrollArea_Combinational, 0, 0, 1, 1);

    tabElements->addTab(combinational, QIcon(":/basic/truthtable-rotated.svg"), QString());
    memory = new QWidget();
    memory->setObjectName("memory");
    gridLayout_5 = new QGridLayout(memory);
    gridLayout_5->setSpacing(6);
    gridLayout_5->setContentsMargins(11, 11, 11, 11);
    gridLayout_5->setObjectName("gridLayout_5");
    scrollAreaMemory = new QScrollArea(memory);
    scrollAreaMemory->setObjectName("scrollAreaMemory");
    scrollAreaMemory->setFrameShape(QFrame::NoFrame);
    scrollAreaMemory->setWidgetResizable(true);
    scrollAreaWidgetContents_Memory = new QWidget();
    scrollAreaWidgetContents_Memory->setObjectName("scrollAreaWidgetContents_Memory");
    scrollAreaWidgetContents_Memory->setGeometry(QRect(0, 0, 260, 584));
    verticalLayout_3 = new QVBoxLayout(scrollAreaWidgetContents_Memory);
    verticalLayout_3->setSpacing(6);
    verticalLayout_3->setContentsMargins(11, 11, 11, 11);
    verticalLayout_3->setObjectName("verticalLayout_3");
    verticalSpacer_Memory = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout_3->addItem(verticalSpacer_Memory);

    scrollAreaMemory->setWidget(scrollAreaWidgetContents_Memory);

    gridLayout_5->addWidget(scrollAreaMemory, 0, 1, 1, 1);

    tabElements->addTab(memory, QIcon(":/memory/light/D-flipflop.svg"), QString());
    ic = new QWidget();
    ic->setObjectName("ic");
    gridLayout_4 = new QGridLayout(ic);
    gridLayout_4->setSpacing(6);
    gridLayout_4->setContentsMargins(11, 11, 11, 11);
    gridLayout_4->setObjectName("gridLayout_4");
    scrollArea_Box = new QScrollArea(ic);
    scrollArea_Box->setObjectName("scrollArea_Box");
    scrollArea_Box->setFrameShape(QFrame::NoFrame);
    scrollArea_Box->setWidgetResizable(true);
    scrollAreaWidgetContents_IC = new QWidget();
    scrollAreaWidgetContents_IC->setObjectName("scrollAreaWidgetContents_IC");
    scrollAreaWidgetContents_IC->setGeometry(QRect(0, 0, 260, 552));
    verticalLayout_4 = new QVBoxLayout(scrollAreaWidgetContents_IC);
    verticalLayout_4->setSpacing(6);
    verticalLayout_4->setContentsMargins(11, 11, 11, 11);
    verticalLayout_4->setObjectName("verticalLayout_4");
    verticalSpacer_IC = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout_4->addItem(verticalSpacer_IC);

    scrollArea_Box->setWidget(scrollAreaWidgetContents_IC);

    gridLayout_4->addWidget(scrollArea_Box, 1, 0, 1, 1);

    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(6);
    horizontalLayout->setObjectName("horizontalLayout");
    pushButtonAddIC = new QPushButton(ic);
    pushButtonAddIC->setObjectName("pushButtonAddIC");

    horizontalLayout->addWidget(pushButtonAddIC);

    pushButtonRemoveIC = new TrashButton(ic);
    pushButtonRemoveIC->setObjectName("pushButtonRemoveIC");
    QSizePolicy sizePolicy2(QSizePolicy::Maximum, QSizePolicy::Fixed);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(pushButtonRemoveIC->sizePolicy().hasHeightForWidth());
    pushButtonRemoveIC->setSizePolicy(sizePolicy2);
    pushButtonRemoveIC->setIcon(QIcon(":/toolbar/trashcan.svg"));

    horizontalLayout->addWidget(pushButtonRemoveIC);

    gridLayout_4->addLayout(horizontalLayout, 0, 0, 1, 1);

    tabElements->addTab(ic, QIcon(":/basic/ic-panda.svg"), QString());
    misc = new QWidget();
    misc->setObjectName("misc");
    gridLayout_3 = new QGridLayout(misc);
    gridLayout_3->setSpacing(6);
    gridLayout_3->setContentsMargins(11, 11, 11, 11);
    gridLayout_3->setObjectName("gridLayout_3");
    scrollArea_Misc = new QScrollArea(misc);
    scrollArea_Misc->setObjectName("scrollArea_Misc");
    scrollArea_Misc->setFrameShape(QFrame::NoFrame);
    scrollArea_Misc->setWidgetResizable(true);
    scrollAreaWidgetContents_Misc = new QWidget();
    scrollAreaWidgetContents_Misc->setObjectName("scrollAreaWidgetContents_Misc");
    scrollAreaWidgetContents_Misc->setGeometry(QRect(0, 0, 260, 584));
    QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(scrollAreaWidgetContents_Misc->sizePolicy().hasHeightForWidth());
    scrollAreaWidgetContents_Misc->setSizePolicy(sizePolicy3);
    verticalLayout_6 = new QVBoxLayout(scrollAreaWidgetContents_Misc);
    verticalLayout_6->setSpacing(6);
    verticalLayout_6->setContentsMargins(11, 11, 11, 11);
    verticalLayout_6->setObjectName("verticalLayout_6");
    verticalSpacer_Misc = new QSpacerItem(210, 227, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout_6->addItem(verticalSpacer_Misc);

    scrollArea_Misc->setWidget(scrollAreaWidgetContents_Misc);

    gridLayout_3->addWidget(scrollArea_Misc, 0, 0, 1, 1);

    tabElements->addTab(misc, QIcon(":/misc/text.png"), QString());
    search = new QWidget();
    search->setObjectName("search");
    gridLayout_9 = new QGridLayout(search);
    gridLayout_9->setSpacing(6);
    gridLayout_9->setContentsMargins(11, 11, 11, 11);
    gridLayout_9->setObjectName("gridLayout_9");
    scrollArea_Search = new QScrollArea(search);
    scrollArea_Search->setObjectName("scrollArea_Search");
    scrollArea_Search->setWidgetResizable(true);
    scrollAreaWidgetContents_Search = new QWidget();
    scrollAreaWidgetContents_Search->setObjectName("scrollAreaWidgetContents_Search");
    scrollAreaWidgetContents_Search->setGeometry(QRect(0, 0, 258, 582));
    verticalLayout_7 = new QVBoxLayout(scrollAreaWidgetContents_Search);
    verticalLayout_7->setSpacing(6);
    verticalLayout_7->setContentsMargins(11, 11, 11, 11);
    verticalLayout_7->setObjectName("verticalLayout_7");
    verticalSpacer_Search = new QSpacerItem(20, 234, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout_7->addItem(verticalSpacer_Search);

    scrollArea_Search->setWidget(scrollAreaWidgetContents_Search);

    gridLayout_9->addWidget(scrollArea_Search, 0, 0, 1, 1);

    tabElements->addTab(search, QString());

    gridLayout->addWidget(tabElements, 1, 0, 1, 3);

    elementEditor = new ElementEditor(leftPannel);
    elementEditor->setObjectName("elementEditor");
    QSizePolicy sizePolicy4(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(elementEditor->sizePolicy().hasHeightForWidth());
    elementEditor->setSizePolicy(sizePolicy4);
    elementEditor->setMinimumSize(QSize(0, 0));
    elementEditor->setMaximumSize(QSize(16777215, 16777215));

    gridLayout->addWidget(elementEditor, 2, 0, 1, 3);

    label = new QLabel(leftPannel);
    label->setObjectName("label");
    QSizePolicy sizePolicy5(QSizePolicy::Fixed, QSizePolicy::Preferred);
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(0);
    sizePolicy5.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
    label->setSizePolicy(sizePolicy5);

    gridLayout->addWidget(label, 0, 0, 1, 1);

    lineEditSearch = new QLineEdit(leftPannel);
    lineEditSearch->setObjectName("lineEditSearch");
    QSizePolicy sizePolicy6(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy6.setHorizontalStretch(0);
    sizePolicy6.setVerticalStretch(0);
    sizePolicy6.setHeightForWidth(lineEditSearch->sizePolicy().hasHeightForWidth());
    lineEditSearch->setSizePolicy(sizePolicy6);
    lineEditSearch->setClearButtonEnabled(true);

    gridLayout->addWidget(lineEditSearch, 0, 1, 1, 2);

    verticalLayout_5->addLayout(gridLayout);

    splitter->addWidget(leftPannel);
    tab = new QTabWidget(splitter);
    tab->setObjectName("tab");
    QSizePolicy sizePolicy7(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy7.setHorizontalStretch(255);
    sizePolicy7.setVerticalStretch(0);
    sizePolicy7.setHeightForWidth(tab->sizePolicy().hasHeightForWidth());
    tab->setSizePolicy(sizePolicy7);
    tab->setMinimumSize(QSize(570, 0));
    tab->setMaximumSize(QSize(16777215, 16777215));
    tab->setTabsClosable(true);
    splitter->addWidget(tab);

    gridLayout_8->addWidget(splitter, 0, 0, 1, 1);

    MainWindow->setCentralWidget(centralWidget);
    mainToolBar = new QToolBar(MainWindow);
    mainToolBar->setObjectName("mainToolBar");
    mainToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    mainToolBar->setToolButtonStyle(Qt::ToolButtonFollowStyle);
    MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
    statusBar = new QStatusBar(MainWindow);
    statusBar->setObjectName("statusBar");
    MainWindow->setStatusBar(statusBar);
    menuBar = new QMenuBar(MainWindow);
    menuBar->setObjectName("menuBar");
    menuBar->setGeometry(QRect(0, 0, 886, 20));
    menuFile = new QMenu(menuBar);
    menuFile->setObjectName("menuFile");
    menuFile->setToolTipsVisible(false);
    menuRecentFiles = new QMenu(menuFile);
    menuRecentFiles->setObjectName("menuRecentFiles");
    menuRecentFiles->setEnabled(false);
    menuRecentFiles->setIcon(QIcon(":/toolbar/recentFiles.svg"));
    menuEdit = new QMenu(menuBar);
    menuEdit->setObjectName("menuEdit");
    menuHelp = new QMenu(menuBar);
    menuHelp->setObjectName("menuHelp");
    menuTranslation = new QMenu(menuBar);
    menuTranslation->setObjectName("menuTranslation");
    menuView = new QMenu(menuBar);
    menuView->setObjectName("menuView");
    menuTheme = new QMenu(menuView);
    menuTheme->setObjectName("menuTheme");
    menuLanguage = new QMenu(menuBar);
    menuLanguage->setObjectName("menuLanguage");
    menuSimulation = new QMenu(menuBar);
    menuSimulation->setObjectName("menuSimulation");
    menuExamples = new QMenu(menuBar);
    menuExamples->setObjectName("menuExamples");
    MainWindow->setMenuBar(menuBar);
    QWidget::setTabOrder(lineEditSearch, tabElements);
    QWidget::setTabOrder(tabElements, scrollAreaInOut);
    QWidget::setTabOrder(scrollAreaInOut, scrollAreaMemory);
    QWidget::setTabOrder(scrollAreaMemory, scrollArea_Box);
    QWidget::setTabOrder(scrollArea_Box, scrollAreaGates);

    mainToolBar->addAction(actionNew);
    mainToolBar->addAction(actionOpen);
    mainToolBar->addAction(actionSave);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionRotateLeft);
    mainToolBar->addAction(actionRotateRight);
    mainToolBar->addAction(actionCut);
    mainToolBar->addAction(actionCopy);
    mainToolBar->addAction(actionPaste);
    mainToolBar->addAction(actionDelete);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionZoomIn);
    mainToolBar->addAction(actionZoomOut);
    mainToolBar->addAction(actionResetZoom);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionPlay);
    mainToolBar->addAction(actionRestart);
    mainToolBar->addAction(actionWaveform);
    menuBar->addAction(menuFile->menuAction());
    menuBar->addAction(menuEdit->menuAction());
    menuBar->addAction(menuView->menuAction());
    menuBar->addAction(menuSimulation->menuAction());
    menuBar->addAction(menuExamples->menuAction());
    menuBar->addAction(menuLanguage->menuAction());
    menuBar->addAction(menuTranslation->menuAction());
    menuBar->addAction(menuHelp->menuAction());
    menuFile->addAction(actionNew);
    menuFile->addAction(actionOpen);
    menuFile->addSeparator();
    menuFile->addAction(menuRecentFiles->menuAction());
    menuFile->addAction(actionReloadFile);
    menuFile->addSeparator();
    menuFile->addAction(actionSave);
    menuFile->addAction(actionSaveAs);
    menuFile->addAction(actionExportToArduino);
    menuFile->addAction(actionExportToPdf);
    menuFile->addAction(actionExportToImage);
    menuFile->addSeparator();
    menuFile->addAction(actionExit);
    menuFile->addSeparator();
    menuRecentFiles->addSeparator();
    menuEdit->addSeparator();
    menuEdit->addAction(actionCut);
    menuEdit->addAction(actionCopy);
    menuEdit->addAction(actionPaste);
    menuEdit->addAction(actionDelete);
    menuEdit->addSeparator();
    menuEdit->addAction(actionRename);
    menuEdit->addAction(actionChangeTrigger);
    menuEdit->addSeparator();
    menuEdit->addAction(actionRotateLeft);
    menuEdit->addAction(actionRotateRight);
    menuEdit->addAction(actionFlipHorizontally);
    menuEdit->addAction(actionFlipVertically);
    menuEdit->addSeparator();
    menuEdit->addAction(actionSelectAll);
    menuEdit->addAction(actionClearSelection);
    menuHelp->addAction(actionAbout);
    menuHelp->addAction(actionAboutQt);
    menuHelp->addAction(actionAboutThisVersion);
    menuHelp->addAction(actionShortcutsAndTips);
    menuTranslation->addAction(actionReportTranslationError);
    menuView->addAction(actionZoomIn);
    menuView->addAction(actionZoomOut);
    menuView->addAction(actionResetZoom);
    menuView->addSeparator();
    menuView->addAction(actionWires);
    menuView->addAction(actionGates);
    menuView->addSeparator();
    menuView->addAction(actionFastMode);
    menuView->addSeparator();
    menuView->addAction(menuTheme->menuAction());
    menuView->addAction(actionFullscreen);
    menuView->addAction(actionLabelsUnderIcons);
    menuTheme->addAction(actionLightTheme);
    menuTheme->addAction(actionDarkTheme);
    menuSimulation->addAction(actionPlay);
    menuSimulation->addAction(actionRestart);
    menuSimulation->addAction(actionWaveform);
    menuSimulation->addAction(actionMute);
    menuSimulation->addSeparator();
    menuSimulation->addAction(actionBackground_Simulation);

    retranslateUi();

    tabElements->setCurrentIndex(0);
    tab->setCurrentIndex(-1);

    QMetaObject::connectSlotsByName(MainWindow);
}

void MainWindow_Ui::retranslateUi()
{
    actionOpen->setText(QCoreApplication::translate("MainWindow", "&Open..."));
    actionOpen->setToolTip(QCoreApplication::translate("MainWindow", "Open"));
    actionOpen->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O"));
    actionSave->setText(QCoreApplication::translate("MainWindow", "&Save"));
    actionSave->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S"));
    actionSaveAs->setText(QCoreApplication::translate("MainWindow", "Save &As..."));
    actionSaveAs->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+S"));
    actionCut->setText(QCoreApplication::translate("MainWindow", "Cu&t"));
    actionCut->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+X"));
    actionCopy->setText(QCoreApplication::translate("MainWindow", "&Copy"));
    actionCopy->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+C"));
    actionPaste->setText(QCoreApplication::translate("MainWindow", "&Paste"));
    actionPaste->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+V"));
    actionDelete->setText(QCoreApplication::translate("MainWindow", "&Delete"));
    actionDelete->setShortcut(QCoreApplication::translate("MainWindow", "Del"));
    actionNew->setText(QCoreApplication::translate("MainWindow", "&New"));
    actionNew->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+N"));
    actionZoomIn->setText(QCoreApplication::translate("MainWindow", "Zoom &in"));
    actionZoomIn->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+="));
    actionZoomOut->setText(QCoreApplication::translate("MainWindow", "Zoom &out"));
    actionZoomOut->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+-"));
    actionWires->setText(QCoreApplication::translate("MainWindow", "&Wires"));
    actionWires->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+1"));
    actionGates->setText(QCoreApplication::translate("MainWindow", "&Gates"));
    actionGates->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+2"));
    actionAbout->setText(QCoreApplication::translate("MainWindow", "&About"));
    actionAbout->setShortcut(QCoreApplication::translate("MainWindow", "F1"));
    actionExit->setText(QCoreApplication::translate("MainWindow", "&Exit"));
    actionExit->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Q"));
    actionRotateRight->setText(QCoreApplication::translate("MainWindow", "R&otate right"));
    actionRotateRight->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+R"));
    actionRotateLeft->setText(QCoreApplication::translate("MainWindow", "Rotate &left"));
    actionRotateLeft->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+R"));
    actionAboutQt->setText(QCoreApplication::translate("MainWindow", "About &Qt"));
    actionSelectAll->setText(QCoreApplication::translate("MainWindow", "&Select all"));
    actionSelectAll->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+A"));
    actionReloadFile->setText(QCoreApplication::translate("MainWindow", "Re&load File"));
    actionReloadFile->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+F5"));
    actionExportToArduino->setText(QCoreApplication::translate("MainWindow", "E&xport to Arduino"));
    actionExportToArduino->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+I"));
    actionResetZoom->setText(QCoreApplication::translate("MainWindow", "&Reset Zoom"));
    actionResetZoom->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+0"));
    actionExportToPdf->setText(QCoreApplication::translate("MainWindow", "Export to &PDF"));
    actionExportToPdf->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+P"));
    actionPlay->setText(QCoreApplication::translate("MainWindow", "&Play/Pause"));
    actionPlay->setToolTip(QCoreApplication::translate("MainWindow", "Play or Pause simulation."));
    actionPlay->setShortcut(QCoreApplication::translate("MainWindow", "F5"));
    actionRename->setText(QCoreApplication::translate("MainWindow", "&Rename"));
    actionRename->setShortcut(QCoreApplication::translate("MainWindow", "F2"));
    actionChangeTrigger->setText(QCoreApplication::translate("MainWindow", "Cha&nge Trigger"));
    actionChangeTrigger->setShortcut(QCoreApplication::translate("MainWindow", "F3"));
    actionClearSelection->setText(QCoreApplication::translate("MainWindow", "Cl&ear selection"));
    actionClearSelection->setShortcut(QCoreApplication::translate("MainWindow", "Esc"));
    actionFastMode->setText(QCoreApplication::translate("MainWindow", "&Fast Mode"));
    actionLightTheme->setText(QCoreApplication::translate("MainWindow", "&Panda Light"));
    actionDarkTheme->setText(QCoreApplication::translate("MainWindow", "Panda &Dark"));
    actionWaveform->setText(QCoreApplication::translate("MainWindow", "&Waveform"));
    actionWaveform->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+W"));
    actionExportToImage->setText(QCoreApplication::translate("MainWindow", "Export to &Image"));
    actionExportToImage->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+E"));
    actionFlipHorizontally->setText(QCoreApplication::translate("MainWindow", "&Flip horizontally"));
    actionFlipHorizontally->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+H"));
    actionFlipVertically->setText(QCoreApplication::translate("MainWindow", "Flip &vertically"));
    actionFullscreen->setText(QCoreApplication::translate("MainWindow", "F&ullscreen"));
    actionFullscreen->setShortcut(QCoreApplication::translate("MainWindow", "F11"));
    actionMute->setText(QCoreApplication::translate("MainWindow", "Mute"));
    actionMute->setStatusTip(QCoreApplication::translate("MainWindow", "Mutes and unmutes circuit buzzers"));
    actionMute->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+M"));
    actionLabelsUnderIcons->setText(QCoreApplication::translate("MainWindow", "Labels under icons"));
    actionAboutThisVersion->setText(QCoreApplication::translate("MainWindow", "About this version"));
    actionRestart->setText(QCoreApplication::translate("MainWindow", "&Restart"));
    actionRestart->setToolTip(QCoreApplication::translate("MainWindow", "Restart simulation."));
    actionBackground_Simulation->setText(QCoreApplication::translate("MainWindow", "Background Simulation"));
    actionShortcutsAndTips->setText(QCoreApplication::translate("MainWindow", "Shortcuts and Tips"));
    actionReportTranslationError->setText(QCoreApplication::translate("MainWindow", "Report Translation Error"));
    actionReportTranslationError->setToolTip(QCoreApplication::translate("MainWindow", "Report translation errors or help improve translations on Weblate"));
    tabElements->setTabText(tabElements->indexOf(io), QString());
    tabElements->setTabToolTip(tabElements->indexOf(io), QCoreApplication::translate("MainWindow", "Inputs/Outputs"));
    tabElements->setTabText(tabElements->indexOf(gates), QString());
    tabElements->setTabToolTip(tabElements->indexOf(gates), QCoreApplication::translate("MainWindow", "Gates"));
    tabElements->setTabText(tabElements->indexOf(combinational), QString());
    tabElements->setTabToolTip(tabElements->indexOf(combinational), QCoreApplication::translate("MainWindow", "Combinational"));
    tabElements->setTabText(tabElements->indexOf(memory), QString());
    tabElements->setTabToolTip(tabElements->indexOf(memory), QCoreApplication::translate("MainWindow", "Memory"));
    pushButtonAddIC->setText(QCoreApplication::translate("MainWindow", "Add IC files"));
    pushButtonRemoveIC->setText(QCoreApplication::translate("MainWindow", "Remove IC"));
    tabElements->setTabText(tabElements->indexOf(ic), QString());
    tabElements->setTabToolTip(tabElements->indexOf(ic), QCoreApplication::translate("MainWindow", "Integrated Circuits"));
    tabElements->setTabText(tabElements->indexOf(misc), QString());
    tabElements->setTabToolTip(tabElements->indexOf(misc), QCoreApplication::translate("MainWindow", "Miscellaneous"));
    tabElements->setTabText(tabElements->indexOf(search), QString());
    label->setText(QCoreApplication::translate("MainWindow", "Search:"));
    menuFile->setTitle(QCoreApplication::translate("MainWindow", "&File"));
    menuRecentFiles->setTitle(QCoreApplication::translate("MainWindow", "&Recent files:"));
    menuEdit->setTitle(QCoreApplication::translate("MainWindow", "&Edit"));
    menuHelp->setTitle(QCoreApplication::translate("MainWindow", "&Help"));
    menuTranslation->setTitle(QCoreApplication::translate("MainWindow", "&Help Translate"));
    menuView->setTitle(QCoreApplication::translate("MainWindow", "&View"));
    menuTheme->setTitle(QCoreApplication::translate("MainWindow", "&Theme"));
    menuLanguage->setTitle(QCoreApplication::translate("MainWindow", "&Language"));
    menuSimulation->setTitle(QCoreApplication::translate("MainWindow", "Sim&ulation"));
    menuExamples->setTitle(QCoreApplication::translate("MainWindow", "Examples"));
}
