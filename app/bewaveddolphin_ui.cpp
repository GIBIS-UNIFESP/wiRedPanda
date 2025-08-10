// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bewaveddolphin_ui.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QMetaObject>

void BewavedDolphin_Ui::setupUi(QMainWindow *BewavedDolphin)
{
    if (BewavedDolphin->objectName().isEmpty()) {
        BewavedDolphin->setObjectName("BewavedDolphin");
    }

    BewavedDolphin->resize(800, 600);

    BewavedDolphin->setWindowIcon(QIcon(":/toolbar/dolphin_icon.svg"));

    actionAbout = new QAction(BewavedDolphin);
    actionAbout->setObjectName("actionAbout");
    actionAbout->setIcon(QIcon(":/dolphin/help.svg"));

    actionLoad = new QAction(BewavedDolphin);
    actionLoad->setObjectName("actionLoad");
    actionLoad->setIcon(QIcon(":/dolphin/folder.svg"));

    actionExit = new QAction(BewavedDolphin);
    actionExit->setObjectName("actionExit");
    actionExit->setIcon(QIcon(":/dolphin/exit.svg"));

    actionCombinational = new QAction(BewavedDolphin);
    actionCombinational->setObjectName("actionCombinational");
    actionCombinational->setIcon(QIcon(":/toolbar/wavyIcon.svg"));

    actionExportToPdf = new QAction(BewavedDolphin);
    actionExportToPdf->setObjectName("actionExportToPdf");
    actionExportToPdf->setIcon(QIcon(":/dolphin/pdf.svg"));

    actionSave = new QAction(BewavedDolphin);
    actionSave->setObjectName("actionSave");
    actionSave->setIcon(QIcon(":/dolphin/save.svg"));

    actionSaveAs = new QAction(BewavedDolphin);
    actionSaveAs->setObjectName("actionSaveAs");
    actionSaveAs->setIcon(QIcon(":/dolphin/save.svg"));

    actionCopy = new QAction(BewavedDolphin);
    actionCopy->setObjectName("actionCopy");
    actionCopy->setIcon(QIcon(":/toolbar/copy.svg"));

    actionPaste = new QAction(BewavedDolphin);
    actionPaste->setObjectName("actionPaste");
    actionPaste->setIcon(QIcon(":/dolphin/paste.svg"));

    actionSetTo0 = new QAction(BewavedDolphin);
    actionSetTo0->setObjectName("actionSetTo0");
    actionSetTo0->setIcon(QIcon(":/input/0.svg"));

    actionSetTo1 = new QAction(BewavedDolphin);
    actionSetTo1->setObjectName("actionSetTo1");
    actionSetTo1->setIcon(QIcon(":/input/1.svg"));

    actionSetClockWave = new QAction(BewavedDolphin);
    actionSetClockWave->setObjectName("actionSetClockWave");
    actionSetClockWave->setIcon(QIcon(":/input/clock1.svg"));

    actionInvert = new QAction(BewavedDolphin);
    actionInvert->setObjectName("actionInvert");
    actionInvert->setIcon(QIcon(":/basic/not.svg"));

    actionMerge = new QAction(BewavedDolphin);
    actionMerge->setObjectName("actionMerge");
    actionMerge->setEnabled(false);

    actionSplit = new QAction(BewavedDolphin);
    actionSplit->setObjectName("actionSplit");
    actionSplit->setEnabled(false);

    actionExportToPng = new QAction(BewavedDolphin);
    actionExportToPng->setObjectName("actionExportToPng");
    actionExportToPng->setIcon(QIcon(":/dolphin/png.svg"));

    actionSetLength = new QAction(BewavedDolphin);
    actionSetLength->setObjectName("actionSetLength");
    actionSetLength->setIcon(QIcon(":/dolphin/range.svg"));

    actionAboutQt = new QAction(BewavedDolphin);
    actionAboutQt->setObjectName("actionAboutQt");
    actionAboutQt->setIcon(QIcon(":/toolbar/helpQt.svg"));

    actionZoomIn = new QAction(BewavedDolphin);
    actionZoomIn->setObjectName("actionZoomIn");
    actionZoomIn->setIcon(QIcon(":/dolphin/zoomIn.svg"));

    actionZoomOut = new QAction(BewavedDolphin);
    actionZoomOut->setObjectName("actionZoomOut");
    actionZoomOut->setIcon(QIcon(":/dolphin/zoomOut.svg"));

    actionFitScreen = new QAction(BewavedDolphin);
    actionFitScreen->setObjectName("actionFitScreen");
    actionFitScreen->setIcon(QIcon(":/dolphin/zoomRange.svg"));

    actionResetZoom = new QAction(BewavedDolphin);
    actionResetZoom->setObjectName("actionResetZoom");
    actionResetZoom->setIcon(QIcon(":/dolphin/zoomReset.svg"));

    actionClear = new QAction(BewavedDolphin);
    actionClear->setObjectName("actionClear");
    actionClear->setIcon(QIcon(":/dolphin/reloadFile.svg"));

    actionCut = new QAction(BewavedDolphin);
    actionCut->setObjectName("actionCut");
    actionCut->setIcon(QIcon(":/dolphin/cut.svg"));

    actionShowNumbers = new QAction(BewavedDolphin);
    actionShowNumbers->setObjectName("actionShowNumbers");

    actionShowWaveforms = new QAction(BewavedDolphin);
    actionShowWaveforms->setObjectName("actionShowWaveforms");

    actionAutoCrop = new QAction(BewavedDolphin);
    actionAutoCrop->setObjectName("actionAutoCrop");
    actionAutoCrop->setIcon(QIcon(":/dolphin/autoCrop.svg"));
    actionAutoCrop->setMenuRole(QAction::NoRole);

    centralwidget = new QWidget(BewavedDolphin);
    centralwidget->setObjectName("centralwidget");
    QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(centralwidget->sizePolicy().hasHeightForWidth());
    centralwidget->setSizePolicy(sizePolicy);

    verticalLayout = new QVBoxLayout(centralwidget);
    verticalLayout->setSpacing(0);
    verticalLayout->setObjectName("verticalLayout");
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    BewavedDolphin->setCentralWidget(centralwidget);

    statusbar = new QStatusBar(BewavedDolphin);
    statusbar->setObjectName("statusbar");
    BewavedDolphin->setStatusBar(statusbar);

    mainToolBar = new QToolBar(BewavedDolphin);
    mainToolBar->setObjectName("mainToolBar");
    BewavedDolphin->addToolBar(Qt::TopToolBarArea, mainToolBar);

    menubar = new QMenuBar(BewavedDolphin);
    menubar->setObjectName("menubar");
    menubar->setGeometry(QRect(0, 0, 800, 27));

    menuFile = new QMenu(menubar);
    menuFile->setObjectName("menuFile");

    menuAbout = new QMenu(menubar);
    menuAbout->setObjectName("menuAbout");

    menuEdit = new QMenu(menubar);
    menuEdit->setObjectName("menuEdit");

    menuView = new QMenu(menubar);
    menuView->setObjectName("menuView");
    BewavedDolphin->setMenuBar(menubar);

    mainToolBar->addAction(actionLoad);
    mainToolBar->addAction(actionSave);
    mainToolBar->addAction(actionExportToPdf);
    mainToolBar->addAction(actionExportToPng);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionCut);
    mainToolBar->addAction(actionCopy);
    mainToolBar->addAction(actionPaste);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionClear);
    mainToolBar->addAction(actionCombinational);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionSetTo0);
    mainToolBar->addAction(actionSetTo1);
    mainToolBar->addAction(actionInvert);
    mainToolBar->addAction(actionSetClockWave);
    mainToolBar->addAction(actionAutoCrop);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionZoomIn);
    mainToolBar->addAction(actionZoomOut);
    mainToolBar->addAction(actionResetZoom);
    mainToolBar->addAction(actionFitScreen);
    mainToolBar->addAction(actionSetLength);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionExit);
    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuEdit->menuAction());
    menubar->addAction(menuView->menuAction());
    menubar->addAction(menuAbout->menuAction());
    menuFile->addAction(actionLoad);
    menuFile->addSeparator();
    menuFile->addAction(actionSave);
    menuFile->addAction(actionSaveAs);
    menuFile->addAction(actionExportToPdf);
    menuFile->addAction(actionExportToPng);
    menuFile->addSeparator();
    menuFile->addAction(actionExit);
    menuAbout->addAction(actionAbout);
    menuAbout->addAction(actionAboutQt);
    menuEdit->addAction(actionCut);
    menuEdit->addAction(actionCopy);
    menuEdit->addAction(actionPaste);
    menuEdit->addSeparator();
    menuEdit->addAction(actionClear);
    menuEdit->addAction(actionCombinational);
    menuEdit->addAction(actionSetTo0);
    menuEdit->addAction(actionSetTo1);
    menuEdit->addAction(actionInvert);
    menuEdit->addAction(actionSetClockWave);
    menuEdit->addSeparator();
    menuEdit->addAction(actionMerge);
    menuEdit->addAction(actionSplit);
    menuView->addAction(actionZoomOut);
    menuView->addAction(actionZoomIn);
    menuView->addAction(actionResetZoom);
    menuView->addAction(actionFitScreen);
    menuView->addAction(actionSetLength);
    menuView->addSeparator();
    menuView->addAction(actionShowNumbers);
    menuView->addAction(actionShowWaveforms);

    retranslateUi(BewavedDolphin);

    QMetaObject::connectSlotsByName(BewavedDolphin);
}

void BewavedDolphin_Ui::retranslateUi(QMainWindow *BewavedDolphin)
{
    BewavedDolphin->setWindowTitle(QCoreApplication::translate("BewavedDolphin", "MainWindow"));
    actionAbout->setText(QCoreApplication::translate("BewavedDolphin", "About"));
    actionAbout->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+H"));
    actionLoad->setText(QCoreApplication::translate("BewavedDolphin", "Load"));
    actionLoad->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+L"));
    actionExit->setText(QCoreApplication::translate("BewavedDolphin", "Exit"));
    actionExit->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+W"));
    actionCombinational->setText(QCoreApplication::translate("BewavedDolphin", "Combinational"));
    actionCombinational->setShortcut(QCoreApplication::translate("BewavedDolphin", "Alt+C"));
    actionExportToPdf->setText(QCoreApplication::translate("BewavedDolphin", "Export to PDF"));
    actionExportToPdf->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+P"));
    actionSave->setText(QCoreApplication::translate("BewavedDolphin", "Save"));
    actionSave->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+S"));
    actionSaveAs->setText(QCoreApplication::translate("BewavedDolphin", "Save As..."));
    actionSaveAs->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+Shift+S"));
    actionCopy->setText(QCoreApplication::translate("BewavedDolphin", "Copy"));
    actionCopy->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+C"));
    actionPaste->setText(QCoreApplication::translate("BewavedDolphin", "Paste"));
    actionPaste->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+V"));
    actionSetTo0->setText(QCoreApplication::translate("BewavedDolphin", "Set to 0"));
    actionSetTo0->setShortcut(QCoreApplication::translate("BewavedDolphin", "0"));
    actionSetTo1->setText(QCoreApplication::translate("BewavedDolphin", "Set to 1"));
    actionSetTo1->setShortcut(QCoreApplication::translate("BewavedDolphin", "1"));
    actionSetClockWave->setText(QCoreApplication::translate("BewavedDolphin", "Set clock frequency"));
    actionSetClockWave->setToolTip(QCoreApplication::translate("BewavedDolphin", "Set clock frequency"));
    actionSetClockWave->setShortcut(QCoreApplication::translate("BewavedDolphin", "Alt+W"));
    actionInvert->setText(QCoreApplication::translate("BewavedDolphin", "Invert"));
    actionInvert->setShortcut(QCoreApplication::translate("BewavedDolphin", "Space"));
    actionMerge->setText(QCoreApplication::translate("BewavedDolphin", "Merge"));
    actionSplit->setText(QCoreApplication::translate("BewavedDolphin", "Split"));
    actionExportToPng->setText(QCoreApplication::translate("BewavedDolphin", "Export to PNG"));
    actionExportToPng->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+Shift+P"));
    actionSetLength->setText(QCoreApplication::translate("BewavedDolphin", "Set Length"));
    actionSetLength->setShortcut(QCoreApplication::translate("BewavedDolphin", "Alt+L"));
    actionAboutQt->setText(QCoreApplication::translate("BewavedDolphin", "About Qt"));
    actionAboutQt->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+Shift+H"));
    actionZoomIn->setText(QCoreApplication::translate("BewavedDolphin", "Zoom In"));
    actionZoomIn->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+="));
    actionZoomOut->setText(QCoreApplication::translate("BewavedDolphin", "Zoom Out"));
    actionZoomOut->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+-"));
    actionFitScreen->setText(QCoreApplication::translate("BewavedDolphin", "Fit to screen"));
    actionFitScreen->setToolTip(QCoreApplication::translate("BewavedDolphin", "Fit to screen"));
    actionFitScreen->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+Shift+R"));
    actionResetZoom->setText(QCoreApplication::translate("BewavedDolphin", "Reset Zoom"));
    actionResetZoom->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+Home"));
    actionClear->setText(QCoreApplication::translate("BewavedDolphin", "Clear"));
    actionClear->setShortcut(QCoreApplication::translate("BewavedDolphin", "Alt+X"));
    actionCut->setText(QCoreApplication::translate("BewavedDolphin", "Cut"));
    actionCut->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+X"));
    actionShowNumbers->setText(QCoreApplication::translate("BewavedDolphin", "Show Numbers"));
    actionShowWaveforms->setText(QCoreApplication::translate("BewavedDolphin", "Show Waveforms"));
    actionAutoCrop->setText(QCoreApplication::translate("BewavedDolphin", "AutoCrop"));
    actionAutoCrop->setToolTip(QCoreApplication::translate("BewavedDolphin", "AutoCrop"));
    actionAutoCrop->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+A"));
    mainToolBar->setWindowTitle(QCoreApplication::translate("BewavedDolphin", "toolBar"));
    menuFile->setTitle(QCoreApplication::translate("BewavedDolphin", "File"));
    menuAbout->setTitle(QCoreApplication::translate("BewavedDolphin", "Help"));
    menuEdit->setTitle(QCoreApplication::translate("BewavedDolphin", "Edit"));
    menuView->setTitle(QCoreApplication::translate("BewavedDolphin", "View"));
}
