// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/MCP/TestSimulationHandler.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTest>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/UI/MainWindow.h"
#include "MCP/Server/Core/JsonRpcError.h"
#include "MCP/Server/Handlers/SimulationHandler.h"

namespace {

void addInputAndLed(Scene *scene, const QString &inputLabel = "sw1")
{
    auto *sw = new InputSwitch();
    sw->setLabel(inputLabel);
    auto *led = new Led();
    scene->receiveCommand(new AddItemsCommand({sw, led}, scene));
}

} // namespace

void TestSimulationHandler::testSimulationControlRejectsMissingAction()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("simulation_control", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestSimulationHandler::testSimulationControlRejectsEmptyAction()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("simulation_control", {{"action", ""}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestSimulationHandler::testSimulationControlRejectsNoScene()
{
    MainWindow window;
    auto *tabs = window.findChild<QTabWidget *>();
    QVERIFY(tabs);
    emit tabs->tabCloseRequested(0);
    QVERIFY(!window.currentTab());

    SimulationHandler handler(&window, nullptr);
    const QJsonObject response = handler.handleCommand("simulation_control", {{"action", "start"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestSimulationHandler::testSimulationControlStartStopRestartUpdate()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);
    auto *simulation = window.currentTab()->scene()->simulation();

    QVERIFY(handler.handleCommand("simulation_control", {{"action", "stop"}}, 1).contains("result"));
    QVERIFY(!simulation->isRunning());

    QVERIFY(handler.handleCommand("simulation_control", {{"action", "start"}}, 1).contains("result"));
    QVERIFY(simulation->isRunning());

    QVERIFY(handler.handleCommand("simulation_control", {{"action", "restart"}}, 1).contains("result"));
    QVERIFY(simulation->isRunning());

    QVERIFY(handler.handleCommand("simulation_control", {{"action", "update"}}, 1).contains("result"));
}

void TestSimulationHandler::testSimulationControlRejectsInvalidAction()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("simulation_control", {{"action", "not_a_real_action"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestSimulationHandler::testCreateWaveformRejectsNoScene()
{
    MainWindow window;
    auto *tabs = window.findChild<QTabWidget *>();
    QVERIFY(tabs);
    emit tabs->tabCloseRequested(0);
    QVERIFY(!window.currentTab());

    SimulationHandler handler(&window, nullptr);
    const QJsonObject response = handler.handleCommand("create_waveform", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestSimulationHandler::testCreateWaveformRejectsInvalidDuration()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);
    addInputAndLed(window.currentTab()->scene());

    QCOMPARE(handler.handleCommand("create_waveform", {{"duration", 0}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
    QCOMPARE(handler.handleCommand("create_waveform", {{"duration", -1}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
    QCOMPARE(handler.handleCommand("create_waveform", {{"duration", 999999}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
}

void TestSimulationHandler::testCreateWaveformCreatesRealWaveform()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);
    addInputAndLed(window.currentTab()->scene(), "sw1");

    const QJsonObject response = handler.handleCommand("create_waveform", {{"duration", 8}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();
    QCOMPARE(result["actual_duration"].toInt(), 8);

    const QJsonObject waveformData = result["waveform_data"].toObject();
    QVERIFY(!waveformData["inputs"].toArray().isEmpty());
    QVERIFY(!waveformData["outputs"].toArray().isEmpty());
    QCOMPARE(waveformData["inputs"].toArray()[0].toObject()["label"].toString(), QStringLiteral("sw1"));
}

void TestSimulationHandler::testCreateWaveformRejectsUnknownInputLabel()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);
    addInputAndLed(window.currentTab()->scene(), "sw1");

    const QJsonObject params{{"duration", 4}, {"input_patterns", QJsonObject{{"no_such_input", QJsonArray{0, 1, 0, 1}}}}};
    const QJsonObject response = handler.handleCommand("create_waveform", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestSimulationHandler::testCreateWaveformRejectsPatternLengthMismatch()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);
    addInputAndLed(window.currentTab()->scene(), "sw1");

    const QJsonObject params{{"duration", 4}, {"input_patterns", QJsonObject{{"sw1", QJsonArray{0, 1}}}}};
    const QJsonObject response = handler.handleCommand("create_waveform", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestSimulationHandler::testCreateWaveformRejectsInvalidPatternValue()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);
    addInputAndLed(window.currentTab()->scene(), "sw1");

    const QJsonObject params{{"duration", 4}, {"input_patterns", QJsonObject{{"sw1", QJsonArray{0, 1, 2, 1}}}}};
    const QJsonObject response = handler.handleCommand("create_waveform", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestSimulationHandler::testCreateWaveformReplacesExistingDolphin()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);
    addInputAndLed(window.currentTab()->scene(), "sw1");

    QVERIFY(handler.handleCommand("create_waveform", {{"duration", 4}}, 1).contains("result"));
    auto *firstDolphin = handler.m_persistentDolphin;
    QVERIFY(firstDolphin);

    QVERIFY(handler.handleCommand("create_waveform", {{"duration", 4}}, 1).contains("result"));
    QVERIFY2(handler.m_persistentDolphin != firstDolphin, "a second create_waveform call must replace the previous instance, not reuse or leak it");
}

void TestSimulationHandler::testExportWaveformRejectsMissingParams()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("export_waveform", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestSimulationHandler::testExportWaveformRejectsInvalidFormat()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);

    const QJsonObject params{{"filename", "out.xyz"}, {"format", "xyz"}};
    const QJsonObject response = handler.handleCommand("export_waveform", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestSimulationHandler::testExportWaveformRejectsNoWaveformYet()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);

    const QJsonObject params{{"filename", "out.txt"}, {"format", "txt"}};
    const QJsonObject response = handler.handleCommand("export_waveform", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SimulationError);
}

void TestSimulationHandler::testExportWaveformExportsRealTxtFile()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);
    addInputAndLed(window.currentTab()->scene(), "sw1");
    QVERIFY(handler.handleCommand("create_waveform", {{"duration", 4}}, 1).contains("result"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("waveform.txt");

    const QJsonObject params{{"filename", path}, {"format", "TXT"}}; // uppercase -> lowercased internally
    const QJsonObject response = handler.handleCommand("export_waveform", params, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(QFile::exists(path));
    QVERIFY(QFileInfo(path).size() > 0);
}

void TestSimulationHandler::testExportWaveformRejectsFileOpenFailure()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);
    addInputAndLed(window.currentTab()->scene(), "sw1");
    QVERIFY(handler.handleCommand("create_waveform", {{"duration", 4}}, 1).contains("result"));

    const QJsonObject params{{"filename", "/nonexistent/directory/waveform.txt"}, {"format", "txt"}};
    const QJsonObject response = handler.handleCommand("export_waveform", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::FileError);
}

void TestSimulationHandler::testExportWaveformExportsRealPngFile()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);
    addInputAndLed(window.currentTab()->scene(), "sw1");
    QVERIFY(handler.handleCommand("create_waveform", {{"duration", 4}}, 1).contains("result"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("waveform.png");

    const QJsonObject params{{"filename", path}, {"format", "png"}};
    const QJsonObject response = handler.handleCommand("export_waveform", params, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(QFile::exists(path));
    QVERIFY(QFileInfo(path).size() > 0);
}

void TestSimulationHandler::testExportWaveformRejectsPngExportFailure()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);
    addInputAndLed(window.currentTab()->scene(), "sw1");
    QVERIFY(handler.handleCommand("create_waveform", {{"duration", 4}}, 1).contains("result"));

    const QJsonObject params{{"filename", "/nonexistent/directory/waveform.png"}, {"format", "png"}};
    const QJsonObject response = handler.handleCommand("export_waveform", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::FileError);
}

void TestSimulationHandler::testHandleCommandRejectsUnknownCommand()
{
    MainWindow window;
    SimulationHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("totally_made_up", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::MethodNotFound);
}

void TestSimulationHandler::testHeapAllocationDeletesCleanly()
{
    // Every other test stack-allocates SimulationHandler, which only ever invokes the
    // base-object destructor. `delete` through a pointer instead dispatches through the
    // virtual-destructor's separate "deleting destructor" entry point (pattern 35), and also
    // exercises the real body here (deleting a persistent m_persistentDolphin, if any).
    MainWindow window;
    auto *handler = new SimulationHandler(&window, nullptr);
    addInputAndLed(window.currentTab()->scene(), "sw1");
    QVERIFY(handler->handleCommand("create_waveform", {{"duration", 4}}, 1).contains("result"));
    delete handler;
}
