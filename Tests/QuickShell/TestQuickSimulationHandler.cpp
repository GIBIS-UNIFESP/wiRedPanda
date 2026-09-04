// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickSimulationHandler.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QTemporaryDir>
#include <QTest>

#include "App/BeWavedDolphin/SignalModel.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/QuickShell/Dolphin/QuickDolphinController.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "MCP/Server/Core/JsonRpcError.h"
#include "MCP/Server/Core/MCPValidator.h"
#include "MCP/Server/Handlers/QuickSimulationHandler.h"

namespace {

void addInputAndLed(CanvasItem *canvas, const QString &inputLabel = "sw1")
{
    auto *sw = new InputSwitch();
    sw->setLabel(inputLabel);
    auto *led = new Led();
    canvas->receiveCommand(new CanvasAddItemsCommand({sw, led}, canvas));
}

} // namespace

void TestQuickSimulationHandler::testSimulationControlRejectsMissingAction()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("simulation_control", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickSimulationHandler::testSimulationControlRejectsEmptyAction()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("simulation_control", {{"action", ""}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickSimulationHandler::testSimulationControlRejectsNoScene()
{
    QuickAppController controller;
    QVERIFY(!controller.currentTab());

    QuickSimulationHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand("simulation_control", {{"action", "start"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestQuickSimulationHandler::testSimulationControlStartStopRestartUpdate()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);
    auto *simulation = controller.currentTab()->canvas()->simulation();

    QVERIFY(handler.handleCommand("simulation_control", {{"action", "stop"}}, 1).contains("result"));
    QVERIFY(!simulation->isRunning());

    QVERIFY(handler.handleCommand("simulation_control", {{"action", "start"}}, 1).contains("result"));
    QVERIFY(simulation->isRunning());

    QVERIFY(handler.handleCommand("simulation_control", {{"action", "restart"}}, 1).contains("result"));
    QVERIFY(simulation->isRunning());

    QVERIFY(handler.handleCommand("simulation_control", {{"action", "update"}}, 1).contains("result"));
}

void TestQuickSimulationHandler::testSimulationControlRejectsInvalidAction()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("simulation_control", {{"action", "not_a_real_action"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestQuickSimulationHandler::testCreateWaveformRejectsNoScene()
{
    QuickAppController controller;
    QVERIFY(!controller.currentTab());

    QuickSimulationHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand("create_waveform", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestQuickSimulationHandler::testCreateWaveformRejectsInvalidDuration()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);
    addInputAndLed(controller.currentTab()->canvas());

    QCOMPARE(handler.handleCommand("create_waveform", {{"duration", 0}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
    QCOMPARE(handler.handleCommand("create_waveform", {{"duration", -1}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
    QCOMPARE(handler.handleCommand("create_waveform", {{"duration", 999999}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
    // Boundary: kMaxColumns+1 must still be rejected -- an off-by-one in the
    // "duration > kMaxColumns" comparison would let this slip through.
    QCOMPARE(handler.handleCommand("create_waveform", {{"duration", SignalModel::kMaxColumns + 1}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
}

void TestQuickSimulationHandler::testCreateWaveformAcceptsMaxDuration()
{
    // The other half of the boundary: kMaxColumns itself is the top of the valid range and
    // must succeed, not just fail one-past-the-end.
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);
    addInputAndLed(controller.currentTab()->canvas());

    const QJsonObject response = handler.handleCommand("create_waveform", {{"duration", SignalModel::kMaxColumns}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["actual_duration"].toInt(), SignalModel::kMaxColumns);
}

void TestQuickSimulationHandler::testCreateWaveformCreatesRealWaveform()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);
    addInputAndLed(controller.currentTab()->canvas(), "sw1");

    const QJsonObject response = handler.handleCommand("create_waveform", {{"duration", 8}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();
    QCOMPARE(result["actual_duration"].toInt(), 8);

    const QJsonObject waveformData = result["waveform_data"].toObject();
    QVERIFY(!waveformData["inputs"].toArray().isEmpty());
    QVERIFY(!waveformData["outputs"].toArray().isEmpty());
    QCOMPARE(waveformData["inputs"].toArray()[0].toObject()["label"].toString(), QStringLiteral("sw1"));
}

void TestQuickSimulationHandler::testCreateWaveformCreatesRealWaveformWithInputPatterns()
{
    // Drives a real valid input pattern through the label->row resolution and per-cell
    // setCellValue() write loop and confirms it lands in the response's waveform data.
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);
    addInputAndLed(controller.currentTab()->canvas(), "sw1");

    const QJsonObject params{{"duration", 4}, {"input_patterns", QJsonObject{{"sw1", QJsonArray{0, 1, 0, 1}}}}};
    const QJsonObject response = handler.handleCommand("create_waveform", params, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));

    const QJsonObject result = response["result"].toObject();
    const QJsonArray inputs = result["waveform_data"].toObject()["inputs"].toArray();
    QVERIFY(!inputs.isEmpty());
    QCOMPARE(inputs[0].toObject()["label"].toString(), QStringLiteral("sw1"));
    QCOMPARE(inputs[0].toObject()["values"].toArray(), QJsonArray({0, 1, 0, 1}));
}

void TestQuickSimulationHandler::testCreateWaveformRejectsUnknownInputLabel()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);
    addInputAndLed(controller.currentTab()->canvas(), "sw1");

    const QJsonObject params{{"duration", 4}, {"input_patterns", QJsonObject{{"no_such_input", QJsonArray{0, 1, 0, 1}}}}};
    const QJsonObject response = handler.handleCommand("create_waveform", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestQuickSimulationHandler::testCreateWaveformRejectsPatternLengthMismatch()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);
    addInputAndLed(controller.currentTab()->canvas(), "sw1");

    const QJsonObject params{{"duration", 4}, {"input_patterns", QJsonObject{{"sw1", QJsonArray{0, 1}}}}};
    const QJsonObject response = handler.handleCommand("create_waveform", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestQuickSimulationHandler::testCreateWaveformRejectsInvalidPatternValue()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);
    addInputAndLed(controller.currentTab()->canvas(), "sw1");

    const QJsonObject params{{"duration", 4}, {"input_patterns", QJsonObject{{"sw1", QJsonArray{0, 1, 2, 1}}}}};
    const QJsonObject response = handler.handleCommand("create_waveform", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestQuickSimulationHandler::testCreateWaveformReplacesExistingDolphin()
{
    // A raw pointer-identity comparison (the Widgets original's own technique, valid there
    // since BewavedDolphin's much larger QWidget-based construction makes an immediate
    // same-address reuse unlikely) isn't reliable for QuickDolphinController: a plain QObject
    // delete immediately followed by a same-size new very often gets the exact same heap
    // block back from the allocator, which would make this assertion pass even if the handler
    // leaked the old instance and just happened to overwrite the pointer. A QPointer tracks
    // the *actual* destruction of the first instance instead, which is what "replace, don't
    // reuse or leak" really means.
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);
    addInputAndLed(controller.currentTab()->canvas(), "sw1");

    QVERIFY(handler.handleCommand("create_waveform", {{"duration", 4}}, 1).contains("result"));
    QPointer<QuickDolphinController> firstDolphin = handler.m_persistentDolphin;
    QVERIFY(firstDolphin);

    QVERIFY(handler.handleCommand("create_waveform", {{"duration", 4}}, 1).contains("result"));
    QVERIFY2(firstDolphin.isNull(), "a second create_waveform call must actually delete the previous instance, not just overwrite the pointer");
    QVERIFY(handler.m_persistentDolphin);
}

void TestQuickSimulationHandler::testCreateWaveformResponseValidatesAgainstTheSchema()
{
    // The schema constrains a waveform row's per-column values to the four-state vocabulary
    // (output rows) and to 0/1 (input rows). MCPProcessor validates its own responses before
    // sending, so if a real sweep could produce anything outside that vocabulary, the
    // constraint would turn a working call into an internal error.
    //
    // The circuit is a self-feeding NOT: the engine canonicalises an oscillating region to
    // Unknown, so the LED reading it reports -1. An UNDRIVEN LED would not do -- its input
    // port falls back to a defaultValue() of Inactive and reports a plain 0, which would leave
    // this guard unable to distinguish a correct vocabulary from a two-state one.
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);
    CanvasItem *canvas = controller.currentTab()->canvas();

    auto *sw = new InputSwitch();
    sw->setLabel("sw1");
    auto *notGate = new Not();
    auto *led = new Led();
    canvas->receiveCommand(new CanvasAddItemsCommand({sw, notGate, led}, canvas));

    auto *loop = new Connection();
    canvas->addItem(loop);
    loop->setStartPort(notGate->outputPort(0));
    loop->setEndPort(notGate->inputPort(0));

    auto *tap = new Connection();
    canvas->addItem(tap);
    tap->setStartPort(notGate->outputPort(0));
    tap->setEndPort(led->inputPort(0));

    const QJsonObject response = handler.handleCommand("create_waveform", {{"duration", 4}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));

    QStringList seen;
    const QJsonObject waveformData = response["result"].toObject()["waveform_data"].toObject();
    for (const QJsonValue &row : waveformData["outputs"].toArray()) {
        for (const QJsonValue &value : row.toObject()["values"].toArray()) {
            seen.append(QString::number(value.toInt()));
        }
    }
    // Precondition: without a non-binary value in the sweep this guard proves only that 0
    // validates, which every two-state schema would also allow.
    QVERIFY2(seen.contains(QStringLiteral("-1")),
             qPrintable("precondition: the sweep must produce an unknown output; seen: " + seen.join(", ")));

    MCPValidator validator(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("schema-mcp.json"));
    QVERIFY(validator.isSchemaLoaded());
    const ValidationResult outcome = validator.validateResponse(response, "create_waveform");
    QVERIFY2(outcome.isValid, qPrintable(outcome.errorMessage + " -- output values seen: " + seen.join(", ")));
    QCOMPARE(outcome.schemaUsed, QStringLiteral("create_waveform_response"));
}

void TestQuickSimulationHandler::testExportWaveformRejectsMissingParams()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("export_waveform", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickSimulationHandler::testExportWaveformRejectsInvalidFormat()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);

    const QJsonObject params{{"filename", "out.xyz"}, {"format", "xyz"}};
    const QJsonObject response = handler.handleCommand("export_waveform", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestQuickSimulationHandler::testExportWaveformRejectsNoWaveformYet()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);

    const QJsonObject params{{"filename", "out.txt"}, {"format", "txt"}};
    const QJsonObject response = handler.handleCommand("export_waveform", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SimulationError);
}

void TestQuickSimulationHandler::testExportWaveformExportsRealTxtFile()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);
    addInputAndLed(controller.currentTab()->canvas(), "sw1");
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

void TestQuickSimulationHandler::testExportWaveformRejectsFileOpenFailure()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);
    addInputAndLed(controller.currentTab()->canvas(), "sw1");
    QVERIFY(handler.handleCommand("create_waveform", {{"duration", 4}}, 1).contains("result"));

    const QJsonObject params{{"filename", "/nonexistent/directory/waveform.txt"}, {"format", "txt"}};
    const QJsonObject response = handler.handleCommand("export_waveform", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::FileError);
}

void TestQuickSimulationHandler::testExportWaveformExportsRealPngFile()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);
    addInputAndLed(controller.currentTab()->canvas(), "sw1");
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

void TestQuickSimulationHandler::testExportWaveformRejectsPngExportFailure()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);
    addInputAndLed(controller.currentTab()->canvas(), "sw1");
    QVERIFY(handler.handleCommand("create_waveform", {{"duration", 4}}, 1).contains("result"));

    const QJsonObject params{{"filename", "/nonexistent/directory/waveform.png"}, {"format", "png"}};
    const QJsonObject response = handler.handleCommand("export_waveform", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::FileError);
}

void TestQuickSimulationHandler::testHandleCommandRejectsUnknownCommand()
{
    QuickAppController controller;
    controller.newTab();
    QuickSimulationHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("totally_made_up", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::MethodNotFound);
}

void TestQuickSimulationHandler::testHeapAllocationDeletesCleanly()
{
    // Every other test stack-allocates QuickSimulationHandler, which only ever invokes the
    // base-object destructor. `delete` through a pointer instead dispatches through the
    // virtual-destructor's separate "deleting destructor" entry point, and also exercises the
    // real body here (deleting a persistent m_persistentDolphin, if any).
    QuickAppController controller;
    controller.newTab();
    auto *handler = new QuickSimulationHandler(&controller, nullptr);
    addInputAndLed(controller.currentTab()->canvas(), "sw1");
    QVERIFY(handler->handleCommand("create_waveform", {{"duration", 4}}, 1).contains("result"));
    delete handler;
}
