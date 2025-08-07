/********************************************************************************
** Form generated from reading UI file 'bewaveddolphin.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BEWAVEDDOLPHIN_H
#define UI_BEWAVEDDOLPHIN_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_BewavedDolphin
{
public:
    QAction *actionAbout;
    QAction *actionLoad;
    QAction *actionExit;
    QAction *actionCombinational;
    QAction *actionExportToPdf;
    QAction *actionSave;
    QAction *actionSaveAs;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionSetTo0;
    QAction *actionSetTo1;
    QAction *actionSetClockWave;
    QAction *actionInvert;
    QAction *actionMerge;
    QAction *actionSplit;
    QAction *actionExportToPng;
    QAction *actionSetLength;
    QAction *actionAboutQt;
    QAction *actionZoomIn;
    QAction *actionZoomOut;
    QAction *actionFitScreen;
    QAction *actionResetZoom;
    QAction *actionClear;
    QAction *actionCut;
    QAction *actionShowNumbers;
    QAction *actionShowWaveforms;
    QAction *actionAutoCrop;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QStatusBar *statusbar;
    QToolBar *mainToolBar;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuAbout;
    QMenu *menuEdit;
    QMenu *menuView;

    void setupUi(QMainWindow *BewavedDolphin)
    {
        if (BewavedDolphin->objectName().isEmpty())
            BewavedDolphin->setObjectName("BewavedDolphin");
        BewavedDolphin->resize(800, 600);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/toolbar/dolphin_icon.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        BewavedDolphin->setWindowIcon(icon);
        actionAbout = new QAction(BewavedDolphin);
        actionAbout->setObjectName("actionAbout");
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/dolphin/help.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionAbout->setIcon(icon1);
        actionLoad = new QAction(BewavedDolphin);
        actionLoad->setObjectName("actionLoad");
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/dolphin/folder.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionLoad->setIcon(icon2);
        actionExit = new QAction(BewavedDolphin);
        actionExit->setObjectName("actionExit");
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/dolphin/exit.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionExit->setIcon(icon3);
        actionCombinational = new QAction(BewavedDolphin);
        actionCombinational->setObjectName("actionCombinational");
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/toolbar/wavyIcon.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionCombinational->setIcon(icon4);
        actionExportToPdf = new QAction(BewavedDolphin);
        actionExportToPdf->setObjectName("actionExportToPdf");
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/dolphin/pdf.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionExportToPdf->setIcon(icon5);
        actionSave = new QAction(BewavedDolphin);
        actionSave->setObjectName("actionSave");
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/dolphin/save.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionSave->setIcon(icon6);
        actionSaveAs = new QAction(BewavedDolphin);
        actionSaveAs->setObjectName("actionSaveAs");
        actionSaveAs->setIcon(icon6);
        actionCopy = new QAction(BewavedDolphin);
        actionCopy->setObjectName("actionCopy");
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/toolbar/copy.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionCopy->setIcon(icon7);
        actionPaste = new QAction(BewavedDolphin);
        actionPaste->setObjectName("actionPaste");
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/dolphin/paste.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionPaste->setIcon(icon8);
        actionSetTo0 = new QAction(BewavedDolphin);
        actionSetTo0->setObjectName("actionSetTo0");
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/input/0.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionSetTo0->setIcon(icon9);
        actionSetTo1 = new QAction(BewavedDolphin);
        actionSetTo1->setObjectName("actionSetTo1");
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/input/1.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionSetTo1->setIcon(icon10);
        actionSetClockWave = new QAction(BewavedDolphin);
        actionSetClockWave->setObjectName("actionSetClockWave");
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/input/clock1.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionSetClockWave->setIcon(icon11);
        actionInvert = new QAction(BewavedDolphin);
        actionInvert->setObjectName("actionInvert");
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/basic/not.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionInvert->setIcon(icon12);
        actionMerge = new QAction(BewavedDolphin);
        actionMerge->setObjectName("actionMerge");
        actionMerge->setEnabled(false);
        actionSplit = new QAction(BewavedDolphin);
        actionSplit->setObjectName("actionSplit");
        actionSplit->setEnabled(false);
        actionExportToPng = new QAction(BewavedDolphin);
        actionExportToPng->setObjectName("actionExportToPng");
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/dolphin/png.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionExportToPng->setIcon(icon13);
        actionSetLength = new QAction(BewavedDolphin);
        actionSetLength->setObjectName("actionSetLength");
        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/dolphin/range.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionSetLength->setIcon(icon14);
        actionAboutQt = new QAction(BewavedDolphin);
        actionAboutQt->setObjectName("actionAboutQt");
        QIcon icon15;
        icon15.addFile(QString::fromUtf8(":/toolbar/helpQt.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionAboutQt->setIcon(icon15);
        actionZoomIn = new QAction(BewavedDolphin);
        actionZoomIn->setObjectName("actionZoomIn");
        QIcon icon16;
        icon16.addFile(QString::fromUtf8(":/dolphin/zoomIn.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionZoomIn->setIcon(icon16);
        actionZoomOut = new QAction(BewavedDolphin);
        actionZoomOut->setObjectName("actionZoomOut");
        QIcon icon17;
        icon17.addFile(QString::fromUtf8(":/dolphin/zoomOut.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionZoomOut->setIcon(icon17);
        actionFitScreen = new QAction(BewavedDolphin);
        actionFitScreen->setObjectName("actionFitScreen");
        QIcon icon18;
        icon18.addFile(QString::fromUtf8(":/dolphin/zoomRange.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionFitScreen->setIcon(icon18);
        actionResetZoom = new QAction(BewavedDolphin);
        actionResetZoom->setObjectName("actionResetZoom");
        QIcon icon19;
        icon19.addFile(QString::fromUtf8(":/dolphin/zoomReset.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionResetZoom->setIcon(icon19);
        actionClear = new QAction(BewavedDolphin);
        actionClear->setObjectName("actionClear");
        QIcon icon20;
        icon20.addFile(QString::fromUtf8(":/dolphin/reloadFile.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionClear->setIcon(icon20);
        actionCut = new QAction(BewavedDolphin);
        actionCut->setObjectName("actionCut");
        QIcon icon21;
        icon21.addFile(QString::fromUtf8(":/dolphin/cut.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionCut->setIcon(icon21);
        actionShowNumbers = new QAction(BewavedDolphin);
        actionShowNumbers->setObjectName("actionShowNumbers");
        actionShowWaveforms = new QAction(BewavedDolphin);
        actionShowWaveforms->setObjectName("actionShowWaveforms");
        actionAutoCrop = new QAction(BewavedDolphin);
        actionAutoCrop->setObjectName("actionAutoCrop");
        QIcon icon22;
        icon22.addFile(QString::fromUtf8(":/dolphin/autoCrop.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionAutoCrop->setIcon(icon22);
        actionAutoCrop->setMenuRole(QAction::NoRole);
        centralwidget = new QWidget(BewavedDolphin);
        centralwidget->setObjectName("centralwidget");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Maximum);
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
        BewavedDolphin->addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar);
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
    } // setupUi

    void retranslateUi(QMainWindow *BewavedDolphin)
    {
        BewavedDolphin->setWindowTitle(QCoreApplication::translate("BewavedDolphin", "MainWindow", nullptr));
        actionAbout->setText(QCoreApplication::translate("BewavedDolphin", "About", nullptr));
#if QT_CONFIG(shortcut)
        actionAbout->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+H", nullptr));
#endif // QT_CONFIG(shortcut)
        actionLoad->setText(QCoreApplication::translate("BewavedDolphin", "Load", nullptr));
#if QT_CONFIG(shortcut)
        actionLoad->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+L", nullptr));
#endif // QT_CONFIG(shortcut)
        actionExit->setText(QCoreApplication::translate("BewavedDolphin", "Exit", nullptr));
#if QT_CONFIG(shortcut)
        actionExit->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+W", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCombinational->setText(QCoreApplication::translate("BewavedDolphin", "Combinational", nullptr));
#if QT_CONFIG(shortcut)
        actionCombinational->setShortcut(QCoreApplication::translate("BewavedDolphin", "Alt+C", nullptr));
#endif // QT_CONFIG(shortcut)
        actionExportToPdf->setText(QCoreApplication::translate("BewavedDolphin", "Export to PDF", nullptr));
#if QT_CONFIG(shortcut)
        actionExportToPdf->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+P", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSave->setText(QCoreApplication::translate("BewavedDolphin", "Save", nullptr));
#if QT_CONFIG(shortcut)
        actionSave->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSaveAs->setText(QCoreApplication::translate("BewavedDolphin", "Save As...", nullptr));
#if QT_CONFIG(shortcut)
        actionSaveAs->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+Shift+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCopy->setText(QCoreApplication::translate("BewavedDolphin", "Copy", nullptr));
#if QT_CONFIG(shortcut)
        actionCopy->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+C", nullptr));
#endif // QT_CONFIG(shortcut)
        actionPaste->setText(QCoreApplication::translate("BewavedDolphin", "Paste", nullptr));
#if QT_CONFIG(shortcut)
        actionPaste->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+V", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSetTo0->setText(QCoreApplication::translate("BewavedDolphin", "Set to 0", nullptr));
#if QT_CONFIG(shortcut)
        actionSetTo0->setShortcut(QCoreApplication::translate("BewavedDolphin", "0", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSetTo1->setText(QCoreApplication::translate("BewavedDolphin", "Set to 1", nullptr));
#if QT_CONFIG(shortcut)
        actionSetTo1->setShortcut(QCoreApplication::translate("BewavedDolphin", "1", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSetClockWave->setText(QCoreApplication::translate("BewavedDolphin", "Set clock frequency", nullptr));
#if QT_CONFIG(tooltip)
        actionSetClockWave->setToolTip(QCoreApplication::translate("BewavedDolphin", "Set clock frequency", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionSetClockWave->setShortcut(QCoreApplication::translate("BewavedDolphin", "Alt+W", nullptr));
#endif // QT_CONFIG(shortcut)
        actionInvert->setText(QCoreApplication::translate("BewavedDolphin", "Invert", nullptr));
#if QT_CONFIG(shortcut)
        actionInvert->setShortcut(QCoreApplication::translate("BewavedDolphin", "Space", nullptr));
#endif // QT_CONFIG(shortcut)
        actionMerge->setText(QCoreApplication::translate("BewavedDolphin", "Merge", nullptr));
        actionSplit->setText(QCoreApplication::translate("BewavedDolphin", "Split", nullptr));
        actionExportToPng->setText(QCoreApplication::translate("BewavedDolphin", "Export to PNG", nullptr));
#if QT_CONFIG(shortcut)
        actionExportToPng->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+Shift+P", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSetLength->setText(QCoreApplication::translate("BewavedDolphin", "Set Length", nullptr));
#if QT_CONFIG(shortcut)
        actionSetLength->setShortcut(QCoreApplication::translate("BewavedDolphin", "Alt+L", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAboutQt->setText(QCoreApplication::translate("BewavedDolphin", "About Qt", nullptr));
#if QT_CONFIG(shortcut)
        actionAboutQt->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+Shift+H", nullptr));
#endif // QT_CONFIG(shortcut)
        actionZoomIn->setText(QCoreApplication::translate("BewavedDolphin", "Zoom In", nullptr));
#if QT_CONFIG(shortcut)
        actionZoomIn->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+=", nullptr));
#endif // QT_CONFIG(shortcut)
        actionZoomOut->setText(QCoreApplication::translate("BewavedDolphin", "Zoom Out", nullptr));
#if QT_CONFIG(shortcut)
        actionZoomOut->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+-", nullptr));
#endif // QT_CONFIG(shortcut)
        actionFitScreen->setText(QCoreApplication::translate("BewavedDolphin", "Fit to screen", nullptr));
#if QT_CONFIG(tooltip)
        actionFitScreen->setToolTip(QCoreApplication::translate("BewavedDolphin", "Fit to screen", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionFitScreen->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+Shift+R", nullptr));
#endif // QT_CONFIG(shortcut)
        actionResetZoom->setText(QCoreApplication::translate("BewavedDolphin", "Reset Zoom", nullptr));
#if QT_CONFIG(shortcut)
        actionResetZoom->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+Home", nullptr));
#endif // QT_CONFIG(shortcut)
        actionClear->setText(QCoreApplication::translate("BewavedDolphin", "Clear", nullptr));
#if QT_CONFIG(shortcut)
        actionClear->setShortcut(QCoreApplication::translate("BewavedDolphin", "Alt+X", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCut->setText(QCoreApplication::translate("BewavedDolphin", "Cut", nullptr));
#if QT_CONFIG(shortcut)
        actionCut->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+X", nullptr));
#endif // QT_CONFIG(shortcut)
        actionShowNumbers->setText(QCoreApplication::translate("BewavedDolphin", "Show Numbers", nullptr));
        actionShowWaveforms->setText(QCoreApplication::translate("BewavedDolphin", "Show Waveforms", nullptr));
        actionAutoCrop->setText(QCoreApplication::translate("BewavedDolphin", "AutoCrop", nullptr));
#if QT_CONFIG(tooltip)
        actionAutoCrop->setToolTip(QCoreApplication::translate("BewavedDolphin", "AutoCrop", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionAutoCrop->setShortcut(QCoreApplication::translate("BewavedDolphin", "Ctrl+A", nullptr));
#endif // QT_CONFIG(shortcut)
        mainToolBar->setWindowTitle(QCoreApplication::translate("BewavedDolphin", "toolBar", nullptr));
        menuFile->setTitle(QCoreApplication::translate("BewavedDolphin", "File", nullptr));
        menuAbout->setTitle(QCoreApplication::translate("BewavedDolphin", "Help", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("BewavedDolphin", "Edit", nullptr));
        menuView->setTitle(QCoreApplication::translate("BewavedDolphin", "View", nullptr));
    } // retranslateUi

};

namespace Ui {
    class BewavedDolphin: public Ui_BewavedDolphin {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BEWAVEDDOLPHIN_H
