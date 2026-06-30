// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/BeWavedDolphinUI.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QMetaObject>
#include <QtGui/QIcon>

void BewavedDolphinUi::setupUi(QMainWindow *BewavedDolphin)
{
    if (BewavedDolphin->objectName().isEmpty()) {
        BewavedDolphin->setObjectName("BewavedDolphin");
    }

    BewavedDolphin->setWindowIcon(QIcon(":/Interface/Toolbar/dolphin_icon.svg"));

    actionAbout = new QAction(BewavedDolphin);
    actionAbout->setObjectName("actionAbout");
    actionAbout->setIcon(QIcon(":/Interface/Dolphin/help.svg"));

    actionLoad = new QAction(BewavedDolphin);
    actionLoad->setObjectName("actionLoad");
    actionLoad->setIcon(QIcon(":/Interface/Dolphin/folder.svg"));

    actionExit = new QAction(BewavedDolphin);
    actionExit->setObjectName("actionExit");
    actionExit->setIcon(QIcon(":/Interface/Dolphin/exit.svg"));

    actionCombinational = new QAction(BewavedDolphin);
    actionCombinational->setObjectName("actionCombinational");
    actionCombinational->setIcon(QIcon(":/Interface/Toolbar/wavyIcon.svg"));

    actionExportToPdf = new QAction(BewavedDolphin);
    actionExportToPdf->setObjectName("actionExportToPdf");
    actionExportToPdf->setIcon(QIcon(":/Interface/Dolphin/pdf.svg"));

    actionSave = new QAction(BewavedDolphin);
    actionSave->setObjectName("actionSave");
    actionSave->setIcon(QIcon(":/Interface/Dolphin/save.svg"));

    actionSaveAs = new QAction(BewavedDolphin);
    actionSaveAs->setObjectName("actionSaveAs");
    actionSaveAs->setIcon(QIcon(":/Interface/Dolphin/save.svg"));

    actionUndo = new QAction(BewavedDolphin);
    actionUndo->setObjectName("actionUndo");
    actionUndo->setIcon(QIcon(":/Interface/Toolbar/undo.svg"));
    actionUndo->setEnabled(false);

    actionRedo = new QAction(BewavedDolphin);
    actionRedo->setObjectName("actionRedo");
    actionRedo->setIcon(QIcon(":/Interface/Toolbar/redo.svg"));
    actionRedo->setEnabled(false);

    actionCopy = new QAction(BewavedDolphin);
    actionCopy->setObjectName("actionCopy");
    actionCopy->setIcon(QIcon(":/Interface/Toolbar/copy.svg"));

    actionPaste = new QAction(BewavedDolphin);
    actionPaste->setObjectName("actionPaste");
    actionPaste->setIcon(QIcon(":/Interface/Dolphin/paste.svg"));

    actionSetTo0 = new QAction(BewavedDolphin);
    actionSetTo0->setObjectName("actionSetTo0");
    actionSetTo0->setIcon(QIcon(":/Components/Input/0.svg"));

    actionSetTo1 = new QAction(BewavedDolphin);
    actionSetTo1->setObjectName("actionSetTo1");
    actionSetTo1->setIcon(QIcon(":/Components/Input/1.svg"));

    actionSetClockWave = new QAction(BewavedDolphin);
    actionSetClockWave->setObjectName("actionSetClockWave");
    actionSetClockWave->setIcon(QIcon(":/Components/Input/clock1.svg"));
    // Gated on a non-empty selection — kept in sync by on_tableView_selectionChanged.
    actionSetClockWave->setEnabled(false);

    actionInvert = new QAction(BewavedDolphin);
    actionInvert->setObjectName("actionInvert");
    actionInvert->setIcon(QIcon(":/Components/Logic/not.svg"));

    // Merge/Split are reserved for future multi-bit bus functionality and intentionally
    // disabled until the feature is implemented
    actionMerge = new QAction(BewavedDolphin);
    actionMerge->setObjectName("actionMerge");
    actionMerge->setEnabled(false);

    actionSplit = new QAction(BewavedDolphin);
    actionSplit->setObjectName("actionSplit");
    actionSplit->setEnabled(false);

    actionExportToPng = new QAction(BewavedDolphin);
    actionExportToPng->setObjectName("actionExportToPng");
    actionExportToPng->setIcon(QIcon(":/Interface/Dolphin/png.svg"));

    actionSetLength = new QAction(BewavedDolphin);
    actionSetLength->setObjectName("actionSetLength");
    actionSetLength->setIcon(QIcon(":/Interface/Dolphin/range.svg"));

    actionTemporalMode = new QAction(BewavedDolphin);
    actionTemporalMode->setObjectName("actionTemporalMode");
    actionTemporalMode->setCheckable(true);
    actionTemporalMode->setIcon(QIcon(":/Components/Input/clock1.svg"));

    comboTimeResolution = new QComboBox(BewavedDolphin);
    comboTimeResolution->setObjectName("comboTimeResolution");
    // ns of sim-time advanced per waveform column. Small values (relative to the 5-20 ns gate
    // delays) make a single gate's delay span several visible columns.
    comboTimeResolution->addItem("1 ns/col", 1);
    comboTimeResolution->addItem("2 ns/col", 2);
    comboTimeResolution->addItem("5 ns/col", 5);
    comboTimeResolution->addItem("10 ns/col", 10);
    comboTimeResolution->setCurrentIndex(1); // 2 ns/col

    actionAboutQt = new QAction(BewavedDolphin);
    actionAboutQt->setObjectName("actionAboutQt");
    actionAboutQt->setIcon(QIcon(":/Interface/Toolbar/helpQt.svg"));

    actionZoomIn = new QAction(BewavedDolphin);
    actionZoomIn->setObjectName("actionZoomIn");
    actionZoomIn->setIcon(QIcon(":/Interface/Dolphin/zoomIn.svg"));

    actionZoomOut = new QAction(BewavedDolphin);
    actionZoomOut->setObjectName("actionZoomOut");
    actionZoomOut->setIcon(QIcon(":/Interface/Dolphin/zoomOut.svg"));

    actionFitScreen = new QAction(BewavedDolphin);
    actionFitScreen->setObjectName("actionFitScreen");
    actionFitScreen->setIcon(QIcon(":/Interface/Dolphin/zoomRange.svg"));

    actionResetZoom = new QAction(BewavedDolphin);
    actionResetZoom->setObjectName("actionResetZoom");
    actionResetZoom->setIcon(QIcon(":/Interface/Dolphin/zoomReset.svg"));

    actionClear = new QAction(BewavedDolphin);
    actionClear->setObjectName("actionClear");
    actionClear->setIcon(QIcon(":/Interface/Dolphin/reloadFile.svg"));

    actionCut = new QAction(BewavedDolphin);
    actionCut->setObjectName("actionCut");
    actionCut->setIcon(QIcon(":/Interface/Dolphin/cut.svg"));

    actionShowNumbers = new QAction(BewavedDolphin);
    actionShowNumbers->setObjectName("actionShowNumbers");

    actionShowWaveforms = new QAction(BewavedDolphin);
    actionShowWaveforms->setObjectName("actionShowWaveforms");

    actionAutoCrop = new QAction(BewavedDolphin);
    actionAutoCrop->setObjectName("actionAutoCrop");
    actionAutoCrop->setIcon(QIcon(":/Interface/Dolphin/autoCrop.svg"));
    actionAutoCrop->setMenuRole(QAction::NoRole);

    centralwidget = new QWidget(BewavedDolphin);
    centralwidget->setObjectName("centralwidget");
    // Expanding so the waveform table (added to the layout below) fills the window.
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(centralwidget->sizePolicy().hasHeightForWidth());
    centralwidget->setSizePolicy(sizePolicy);

    // Zero spacing/margins so the waveform view fills the entire central area
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
    // Height of 27px matches the typical platform menubar height; width matches initial window size
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
    mainToolBar->addAction(actionUndo);
    mainToolBar->addAction(actionRedo);
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
    mainToolBar->addAction(actionTemporalMode);
    actionTimeResolution = mainToolBar->addWidget(comboTimeResolution);
    actionTimeResolution->setVisible(false); // shown only while Temporal mode is on
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
    menuEdit->addAction(actionUndo);
    menuEdit->addAction(actionRedo);
    menuEdit->addSeparator();
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

    // Connects signals to slots by naming convention (on_<objectName>_<signal>), but
    // BewavedDolphin manually connects all its actions, so this is a safety net only
    QMetaObject::connectSlotsByName(BewavedDolphin);
}

void BewavedDolphinUi::retranslateUi(QMainWindow *BewavedDolphin)
{
    // The window title is set by BewavedDolphin's constructor (and updated on file
    // load/save), so no placeholder is applied here.
    Q_UNUSED(BewavedDolphin)
    actionAbout->setText(QCoreApplication::translate("BewavedDolphin", "About"));
    actionAbout->setShortcut(QStringLiteral("F1")); // match the main window (Ctrl+H is Flip there)
    actionLoad->setText(QCoreApplication::translate("BewavedDolphin", "Load"));
    actionLoad->setShortcut(QStringLiteral("Ctrl+L"));
    actionExit->setText(QCoreApplication::translate("BewavedDolphin", "Exit"));
    actionExit->setShortcut(QStringLiteral("Ctrl+W"));
    actionCombinational->setText(QCoreApplication::translate("BewavedDolphin", "Combinational"));
    actionCombinational->setShortcut(QStringLiteral("Alt+C"));
    actionExportToPdf->setText(QCoreApplication::translate("BewavedDolphin", "Export to PDF"));
    actionExportToPdf->setShortcut(QStringLiteral("Ctrl+P"));
    actionSave->setText(QCoreApplication::translate("BewavedDolphin", "Save"));
    actionSave->setShortcut(QStringLiteral("Ctrl+S"));
    actionSaveAs->setText(QCoreApplication::translate("BewavedDolphin", "Save As..."));
    actionSaveAs->setShortcut(QStringLiteral("Ctrl+Shift+S"));
    actionUndo->setText(QCoreApplication::translate("BewavedDolphin", "Undo"));
    actionUndo->setShortcut(QKeySequence::Undo);
    actionRedo->setText(QCoreApplication::translate("BewavedDolphin", "Redo"));
    actionRedo->setShortcut(QKeySequence::Redo);
    actionCopy->setText(QCoreApplication::translate("BewavedDolphin", "Copy"));
    actionCopy->setShortcut(QStringLiteral("Ctrl+C"));
    actionPaste->setText(QCoreApplication::translate("BewavedDolphin", "Paste"));
    actionPaste->setShortcut(QStringLiteral("Ctrl+V"));
    actionSetTo0->setText(QCoreApplication::translate("BewavedDolphin", "Set to 0"));
    actionSetTo0->setShortcut(QStringLiteral("0"));
    actionSetTo1->setText(QCoreApplication::translate("BewavedDolphin", "Set to 1"));
    actionSetTo1->setShortcut(QStringLiteral("1"));
    actionSetClockWave->setText(QCoreApplication::translate("BewavedDolphin", "Set clock period"));
    actionSetClockWave->setToolTip(QCoreApplication::translate("BewavedDolphin", "Set clock period"));
    actionSetClockWave->setShortcut(QStringLiteral("Alt+W"));
    actionInvert->setText(QCoreApplication::translate("BewavedDolphin", "Invert"));
    actionInvert->setShortcut(QStringLiteral("Space"));
    actionMerge->setText(QCoreApplication::translate("BewavedDolphin", "Merge"));
    actionSplit->setText(QCoreApplication::translate("BewavedDolphin", "Split"));
    actionExportToPng->setText(QCoreApplication::translate("BewavedDolphin", "Export to PNG"));
    actionExportToPng->setShortcut(QStringLiteral("Ctrl+Shift+P"));
    actionSetLength->setText(QCoreApplication::translate("BewavedDolphin", "Set Length"));
    actionSetLength->setShortcut(QStringLiteral("Alt+L"));
    actionTemporalMode->setText(QCoreApplication::translate("BewavedDolphin", "Temporal"));
    actionTemporalMode->setToolTip(QCoreApplication::translate("BewavedDolphin",
        "Temporal mode: simulate per-element propagation delays, so outputs lag their cause by "
        "whole columns. Hold a signal steady for at least its gate delay to see the effect "
        "(narrower pulses are absorbed)."));
    comboTimeResolution->setToolTip(QCoreApplication::translate("BewavedDolphin",
        "Simulation time advanced per waveform column (temporal mode)."));
    actionAboutQt->setText(QCoreApplication::translate("BewavedDolphin", "About Qt"));
    actionAboutQt->setShortcut(QStringLiteral("Ctrl+Shift+H"));
    actionZoomIn->setText(QCoreApplication::translate("BewavedDolphin", "Zoom In"));
    actionZoomIn->setShortcut(QStringLiteral("Ctrl+="));
    actionZoomOut->setText(QCoreApplication::translate("BewavedDolphin", "Zoom Out"));
    actionZoomOut->setShortcut(QStringLiteral("Ctrl+-"));
    actionFitScreen->setText(QCoreApplication::translate("BewavedDolphin", "Fit to screen"));
    actionFitScreen->setToolTip(QCoreApplication::translate("BewavedDolphin", "Fit to screen"));
    actionFitScreen->setShortcut(QStringLiteral("Ctrl+Shift+F")); // match the main window's Zoom to Fit (Ctrl+Shift+R is Rotate there)
    actionResetZoom->setText(QCoreApplication::translate("BewavedDolphin", "Reset Zoom"));
    actionResetZoom->setShortcut(QStringLiteral("Ctrl+Home"));
    actionClear->setText(QCoreApplication::translate("BewavedDolphin", "Clear"));
    actionClear->setShortcut(QStringLiteral("Alt+X"));
    actionCut->setText(QCoreApplication::translate("BewavedDolphin", "Cut"));
    actionCut->setShortcut(QStringLiteral("Ctrl+X"));
    actionShowNumbers->setText(QCoreApplication::translate("BewavedDolphin", "Show Numbers"));
    actionShowWaveforms->setText(QCoreApplication::translate("BewavedDolphin", "Show Waveforms"));
    actionAutoCrop->setText(QCoreApplication::translate("BewavedDolphin", "AutoCrop"));
    actionAutoCrop->setToolTip(QCoreApplication::translate("BewavedDolphin", "AutoCrop"));
    actionAutoCrop->setShortcut(QStringLiteral("Alt+A")); // fits BWD's Alt+ family; frees Ctrl+A for its universal Select-All meaning
    mainToolBar->setWindowTitle(QCoreApplication::translate("BewavedDolphin", "toolBar"));
    menuFile->setTitle(QCoreApplication::translate("BewavedDolphin", "File"));
    menuAbout->setTitle(QCoreApplication::translate("BewavedDolphin", "Help"));
    menuEdit->setTitle(QCoreApplication::translate("BewavedDolphin", "Edit"));
    menuView->setTitle(QCoreApplication::translate("BewavedDolphin", "View"));
}
