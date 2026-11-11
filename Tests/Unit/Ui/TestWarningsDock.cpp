// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestWarningsDock.h"

#include <QDir>
#include <QImage>
#include <QLabel>
#include <QMainWindow>
#include <QPainter>
#include <QSignalSpy>
#include <QSplitter>
#include <QTemporaryFile>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/GraphicElements/Display16.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/Element/GraphicElements/JKFlipFlop.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/SRFlipFlop.h"
#include "App/Element/GraphicElements/TFlipFlop.h"
#include "App/Element/ICPreviewPopup.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/ElementPalette.h"
#include "App/UI/MainWindow.h"
#include "App/UI/MainWindowUI.h"
#include "App/UI/SceneUiBinder.h"
#include "App/UI/WarningsPanel.h"
#include "App/Wiring/Connection.h"

void TestWarningsDock::initTestCase()
{
}

void TestWarningsDock::cleanupTestCase()
{
}

void TestWarningsDock::testRowsAndCount()
{
    Scene scene;
    WarningsPanel panel;
    panel.setScene(&scene);

    QCOMPARE(panel.issueCount(), 0);
    QCOMPARE(panel.countLabel()->text(), QStringLiteral("0 issues"));
    QCOMPARE(panel.warningsTree()->topLevelItemCount(), 0);
    QVERIFY(panel.warningsTree()->isHidden());

    auto *andGate = new And();
    andGate->setLabel(QStringLiteral("U1"));
    scene.addItem(andGate);
    panel.refresh();

    // AND gate has 2 required unconnected inputs
    QCOMPARE(panel.issueCount(), 2);
    QCOMPARE(panel.countLabel()->text(), QStringLiteral("2 issues"));
    QCOMPARE(panel.warningsTree()->topLevelItemCount(), 1);
    QVERIFY(!panel.warningsTree()->isHidden());

    auto *item = panel.warningsTree()->topLevelItem(0);
    QVERIFY(item != nullptr);
    QVERIFY(item->text(0).contains(QStringLiteral("U1 (And)")));
    QVERIFY(item->text(0).contains(QStringLiteral("2 issues")));
    // Expandable details: by default stay hidden (collapsed) to reduce clutter
    QVERIFY(!item->isExpanded());
    QCOMPARE(item->childCount(), 2);
    QCOMPARE(item->child(0)->text(0), QStringLiteral("input 1: Not connected"));
    QCOMPARE(item->child(1)->text(0), QStringLiteral("input 2: Not connected"));

    // Clicking top-level item toggles expansion
    emit panel.warningsTree()->itemClicked(item, 0);
    QVERIFY(item->isExpanded());
    emit panel.warningsTree()->itemClicked(item, 0);
    QVERIFY(!item->isExpanded());

    // Connect input 0: issues drop to 1
    auto *vcc = new InputVcc();
    scene.addItem(vcc);
    auto *conn1 = new Connection();
    conn1->setStartPort(vcc->outputPort(0));
    conn1->setEndPort(andGate->inputPort(0));
    scene.addItem(conn1);

    panel.refresh();
    QCOMPARE(panel.issueCount(), 1);
    QCOMPARE(panel.countLabel()->text(), QStringLiteral("1 issue"));
    QCOMPARE(panel.warningsTree()->topLevelItemCount(), 1);
    QVERIFY(item->text(0).contains(QStringLiteral("1 issue")));
    QCOMPARE(item->childCount(), 1);
    QCOMPARE(item->child(0)->text(0), QStringLiteral("input 2: Not connected"));

    // Connect input 1: all inputs valid, issues drop to 0
    auto *conn2 = new Connection();
    conn2->setStartPort(vcc->outputPort(0));
    conn2->setEndPort(andGate->inputPort(1));
    scene.addItem(conn2);

    panel.refresh();
    QCOMPARE(panel.issueCount(), 0);
    QCOMPARE(panel.countLabel()->text(), QStringLiteral("0 issues"));
    QCOMPARE(panel.warningsTree()->topLevelItemCount(), 0);
    QVERIFY(panel.warningsTree()->isHidden());
}

void TestWarningsDock::testActivation()
{
    Scene scene;
    auto *andGate = new And();
    scene.addItem(andGate);

    WarningsPanel panel;
    panel.setScene(&scene);

    QCOMPARE(panel.warningsTree()->topLevelItemCount(), 1);
    QSignalSpy spy(&panel, &WarningsPanel::warningElementActivated);

    emit panel.warningsTree()->itemClicked(panel.warningsTree()->topLevelItem(0), 0);
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

    ui.warningsPanel->setScene(scene);
    ui.warningsPanel->refresh();
    QCOMPARE(ui.warningsPanel->issueCount(), 2);

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
    QCOMPARE(ui.warningsPanel->issueCount(), 0);

    auto *andGate2 = new And();
    scene->addItem(andGate2);
    ui.warningsPanel->setScene(scene);
    QCOMPARE(ui.warningsPanel->issueCount(), 2);

    binder.unbind();
}

void TestWarningsDock::testToggleVisibility()
{
    QMainWindow window;
    MainWindowUi ui;
    ui.setupUi(&window);

    QVERIFY(ui.warningsPanel != nullptr);
    QVERIFY(ui.actionShowWarnings != nullptr);

    ui.warningsPanel->show();
    QVERIFY(!ui.warningsPanel->isHidden());

    ui.warningsPanel->hide();
    QVERIFY(ui.warningsPanel->isHidden());
}

void TestWarningsDock::testDropElementUpdatesWarningsPanel()
{
    MainWindow window;
    auto *panel = window.findChild<WarningsPanel *>("warningsPanel");
    QVERIFY(panel != nullptr);
    auto *workspace = window.currentTab();
    QVERIFY(workspace != nullptr);
    auto *scene = workspace->scene();
    QVERIFY(scene != nullptr);

    QCOMPARE(panel->issueCount(), 0);

    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand({andGate}, scene));

    QCOMPARE(panel->issueCount(), 2);
    QCOMPARE(panel->warningsTree()->topLevelItemCount(), 1);
}

void TestWarningsDock::testWithRealSettings()
{
    MainWindow window;
    window.show();
    QCoreApplication::processEvents();
    auto *panel = window.findChild<WarningsPanel *>("warningsPanel");
    QVERIFY(panel != nullptr);
    QCOMPARE(panel->sizeHint(), QSize(260, 400));
    QCOMPARE(panel->minimumSizeHint(), QSize(160, 100));
    QVERIFY(panel->isVisible());
    QVERIFY(panel->width() >= 160);
}

void TestWarningsDock::testLoadSavedFile()
{
    MainWindow window;
    window.show();
    auto *panel = window.findChild<WarningsPanel *>("warningsPanel");
    QVERIFY(panel != nullptr);

    const QString counterPath = QFileInfo(QStringLiteral("../Examples/counter.panda")).exists()
        ? QStringLiteral("../Examples/counter.panda")
        : QStringLiteral("Examples/counter.panda");

    window.loadPandaFile(counterPath);
    // counter.panda has 3 JK flip-flops with unconnected J, K, ~Preset, ~Clear inputs (4 * 3 = 12 issues)
    QCOMPARE(panel->issueCount(), 12);
    QCOMPARE(panel->warningsTree()->topLevelItemCount(), 3);

    // Now in the loaded scene, add an unconnected element
    auto *scene = window.currentTab()->scene();
    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand({andGate}, scene));

    QCOMPARE(panel->issueCount(), 14);
}

void TestWarningsDock::testElementHighlightRendering()
{
    And andGate;
    QVERIFY(andGate.hasPortIssues());

    QRectF r = andGate.boundingRect();
    QImage img(r.size().toSize() + QSize(20, 20), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);

    QPainter painter(&img);
    painter.translate(10 - r.left(), 10 - r.top());
    QStyleOptionGraphicsItem opt;
    andGate.paint(&painter, &opt, nullptr);
    painter.end();

    int redPixels = 0;
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            QColor c = img.pixelColor(x, y);
            if (c.red() > 200 && c.green() < 50 && c.blue() < 50) {
                redPixels++;
            }
        }
    }
    QVERIFY(redPixels > 0);
}

void TestWarningsDock::testLoadFileWithUnconnectedElements()
{
    QTemporaryFile tempFile(QDir::tempPath() + QStringLiteral("/unconnected_XXXXXX.panda"));
    QVERIFY(tempFile.open());
    const QString tempFileName = tempFile.fileName();
    tempFile.close();

    // Create a workspace, add an unconnected And, and save it
    {
        WorkSpace ws;
        auto *scene = ws.scene();
        auto *andGate = new And();
        scene->receiveCommand(new AddItemsCommand({andGate}, scene));
        ws.save(tempFileName);
    }

    // Now load it into MainWindow and verify the panel populates
    MainWindow window;
    window.show();
    auto *panel = window.findChild<WarningsPanel *>("warningsPanel");
    QVERIFY(panel != nullptr);

    window.loadPandaFile(tempFileName);
    QCOMPARE(panel->issueCount(), 2);
    QCOMPARE(panel->warningsTree()->topLevelItemCount(), 1);
}

void TestWarningsDock::testEveryComponentTypeTriggersWarningsWhenInputsOpened()
{
    // Verify that every component type with open (unconnected) inputs triggers warnings
    // both on the element itself (invalidInputs, hasPortIssues) and in WarningsPanel.
    // Also verify expandable details are collapsed by default.
    Scene scene;
    WarningsPanel panel;
    panel.setScene(&scene);

    // 1. LED (1 input by default)
    auto *led = new Led();
    QCOMPARE(led->inputSize(), 1);
    QVERIFY(led->hasPortIssues());
    QCOMPARE(led->invalidInputs().size(), 1);
    scene.addItem(led);
    panel.refresh();
    QCOMPARE(panel.issueCount(), 1);

    // 2. 7-Segment Display (8 inputs: 7 required segments + 1 optional DP)
    auto *disp7 = new Display7();
    QCOMPARE(disp7->inputSize(), 8);
    QVERIFY(disp7->hasPortIssues());
    QCOMPARE(disp7->invalidInputs().size(), 7);
    scene.addItem(disp7);
    panel.refresh();
    QCOMPARE(panel.issueCount(), 1 + 7);

    // 3. 14-Segment Display (15 inputs: 14 required + 1 optional DP)
    auto *disp14 = new Display14();
    QCOMPARE(disp14->inputSize(), 15);
    QVERIFY(disp14->hasPortIssues());
    QCOMPARE(disp14->invalidInputs().size(), 14);
    scene.addItem(disp14);
    panel.refresh();
    QCOMPARE(panel.issueCount(), 1 + 7 + 14);

    // 4. 16-Segment Display (17 inputs: 16 required + 1 optional DP)
    auto *disp16 = new Display16();
    QCOMPARE(disp16->inputSize(), 17);
    QVERIFY(disp16->hasPortIssues());
    QCOMPARE(disp16->invalidInputs().size(), 16);
    scene.addItem(disp16);
    panel.refresh();
    QCOMPARE(panel.issueCount(), 1 + 7 + 14 + 16);

    // 5. D Flip-Flop (4 inputs: Data, Clock, ~Preset, ~Clear)
    auto *dff = new DFlipFlop();
    QCOMPARE(dff->inputSize(), 4);
    QVERIFY(dff->hasPortIssues());
    QCOMPARE(dff->invalidInputs().size(), 4);
    scene.addItem(dff);
    panel.refresh();
    QCOMPARE(panel.issueCount(), 1 + 7 + 14 + 16 + 4);

    // 6. JK Flip-Flop (5 inputs: J, Clock, K, ~Preset, ~Clear)
    auto *jkff = new JKFlipFlop();
    QCOMPARE(jkff->inputSize(), 5);
    QVERIFY(jkff->hasPortIssues());
    QCOMPARE(jkff->invalidInputs().size(), 5);
    scene.addItem(jkff);
    panel.refresh();
    QCOMPARE(panel.issueCount(), 1 + 7 + 14 + 16 + 4 + 5);

    // 7. SR Flip-Flop (5 inputs: S, Clock, R, ~Preset, ~Clear)
    auto *srff = new SRFlipFlop();
    QCOMPARE(srff->inputSize(), 5);
    QVERIFY(srff->hasPortIssues());
    QCOMPARE(srff->invalidInputs().size(), 5);
    scene.addItem(srff);
    panel.refresh();
    QCOMPARE(panel.issueCount(), 1 + 7 + 14 + 16 + 4 + 5 + 5);

    // 8. T Flip-Flop (4 inputs: T, Clock, ~Preset, ~Clear)
    auto *tff = new TFlipFlop();
    QCOMPARE(tff->inputSize(), 4);
    QVERIFY(tff->hasPortIssues());
    QCOMPARE(tff->invalidInputs().size(), 4);
    scene.addItem(tff);
    panel.refresh();
    QCOMPARE(panel.issueCount(), 1 + 7 + 14 + 16 + 4 + 5 + 5 + 4);

    // 9. AND gate (2 inputs)
    auto *andGate = new And();
    QCOMPARE(andGate->inputSize(), 2);
    QVERIFY(andGate->hasPortIssues());
    QCOMPARE(andGate->invalidInputs().size(), 2);
    scene.addItem(andGate);
    panel.refresh();
    QCOMPARE(panel.issueCount(), 1 + 7 + 14 + 16 + 4 + 5 + 5 + 4 + 2);

    // Total: 9 elements in warnings panel
    QCOMPARE(panel.warningsTree()->topLevelItemCount(), 9);

    // All items must be collapsed by default to avoid clutter
    for (int i = 0; i < panel.warningsTree()->topLevelItemCount(); ++i) {
        auto *item = panel.warningsTree()->topLevelItem(i);
        QVERIFY(!item->isExpanded());
        QVERIFY(item->childCount() > 0);
    }
}
