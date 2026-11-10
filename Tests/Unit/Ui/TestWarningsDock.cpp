// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestWarningsDock.h"

#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QSignalSpy>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/Element/ICPreviewPopup.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/ElementPalette.h"
#include "App/UI/MainWindowUI.h"
#include "App/UI/SceneUiBinder.h"
#include "App/UI/WarningsDock.h"
#include "App/Wiring/Connection.h"

void TestWarningsDock::testRowsAndCount()
{
    Scene scene;
    WarningsDock dock;
    dock.setScene(&scene);

    QCOMPARE(dock.issueCount(), 0);
    QCOMPARE(dock.countLabel()->text(), QStringLiteral("0 issues"));
    QCOMPARE(dock.warningsList()->count(), 0);
    QVERIFY(dock.warningsList()->isHidden());

    auto *andGate = new And();
    andGate->setLabel(QStringLiteral("U1"));
    scene.addItem(andGate);
    dock.refresh();

    // AND gate has 2 required unconnected inputs
    QCOMPARE(dock.issueCount(), 2);
    QCOMPARE(dock.countLabel()->text(), QStringLiteral("2 issues"));
    QCOMPARE(dock.warningsList()->count(), 1);
    QVERIFY(!dock.warningsList()->isHidden());

    auto *item = dock.warningsList()->item(0);
    QVERIFY(item != nullptr);
    QVERIFY(item->text().contains(QStringLiteral("U1 (And)")));
    QVERIFY(item->text().contains(QStringLiteral("2 issues")));
    QVERIFY(item->text().contains(QStringLiteral("input 1: Not connected")));
    QVERIFY(item->text().contains(QStringLiteral("input 2: Not connected")));

    // Connect input 0: issues drop to 1
    auto *vcc = new InputVcc();
    scene.addItem(vcc);
    auto *conn1 = new Connection();
    conn1->setStartPort(vcc->outputPort(0));
    conn1->setEndPort(andGate->inputPort(0));
    scene.addItem(conn1);

    dock.refresh();
    QCOMPARE(dock.issueCount(), 1);
    QCOMPARE(dock.countLabel()->text(), QStringLiteral("1 issue"));
    QCOMPARE(dock.warningsList()->count(), 1);
    QVERIFY(item->text().contains(QStringLiteral("1 issue")));
    QVERIFY(item->text().contains(QStringLiteral("input 2: Not connected")));

    // Connect input 1: all inputs valid, issues drop to 0
    auto *conn2 = new Connection();
    conn2->setStartPort(vcc->outputPort(0));
    conn2->setEndPort(andGate->inputPort(1));
    scene.addItem(conn2);

    dock.refresh();
    QCOMPARE(dock.issueCount(), 0);
    QCOMPARE(dock.countLabel()->text(), QStringLiteral("0 issues"));
    QCOMPARE(dock.warningsList()->count(), 0);
    QVERIFY(dock.warningsList()->isHidden());
}

void TestWarningsDock::testActivation()
{
    Scene scene;
    auto *andGate = new And();
    scene.addItem(andGate);

    WarningsDock dock;
    dock.setScene(&scene);

    QCOMPARE(dock.warningsList()->count(), 1);
    QSignalSpy spy(&dock, &WarningsDock::warningElementActivated);

    emit dock.warningsList()->itemClicked(dock.warningsList()->item(0));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toInt(), andGate->id());
}

void TestWarningsDock::testRefreshAfterCircuitHasChangedAndTabContentLoaded()
{
    QMainWindow window;
    MainWindowUi ui;
    ui.setupUi(&window);

    WorkSpace ws;
    auto *scene = ws.scene();
    auto *andGate = new And();
    scene->addItem(andGate);

    ui.warningsDock->setScene(scene);
    ui.warningsDock->refresh();
    QCOMPARE(ui.warningsDock->issueCount(), 2);

    auto *vcc = new InputVcc();
    scene->addItem(vcc);
    auto *conn1 = new Connection();
    conn1->setStartPort(vcc->outputPort(0));
    conn1->setEndPort(andGate->inputPort(0));
    scene->addItem(conn1);
    auto *conn2 = new Connection();
    conn2->setStartPort(vcc->outputPort(0));
    conn2->setEndPort(andGate->inputPort(1));
    scene->addItem(conn2);

    ElementPalette palette(&ui);
    ICPreviewPopup previewPopup;
    SceneUiBinder binder(&ui, &palette, &previewPopup, &window);
    binder.bind(&ws);

    emit scene->circuitHasChanged();
    QCOMPARE(ui.warningsDock->issueCount(), 0);

    auto *andGate2 = new And();
    scene->addItem(andGate2);
    ui.warningsDock->setScene(scene);
    QCOMPARE(ui.warningsDock->issueCount(), 2);

    binder.unbind();
}

void TestWarningsDock::testPersistenceThroughSaveState()
{
    QMainWindow window;
    auto *dock = new WarningsDock(&window);
    window.addDockWidget(Qt::RightDockWidgetArea, dock);

    dock->show();
    QVERIFY(!dock->isHidden());

    const QByteArray stateVisible = window.saveState();

    dock->hide();
    QVERIFY(dock->isHidden());

    const QByteArray stateHidden = window.saveState();

    window.restoreState(stateVisible);
    QVERIFY(!dock->isHidden());

    window.restoreState(stateHidden);
    QVERIFY(dock->isHidden());
}
