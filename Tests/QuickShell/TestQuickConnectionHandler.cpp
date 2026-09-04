// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickConnectionHandler.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

#include "App/Element/GraphicElements/And.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "MCP/Server/Core/JsonRpcError.h"
#include "MCP/Server/Handlers/QuickConnectionHandler.h"

namespace {

int addAnd(CanvasItem *canvas)
{
    auto *elm = new And();
    canvas->receiveCommand(new CanvasAddItemsCommand({elm}, canvas));
    return elm->id();
}

} // namespace

void TestQuickConnectionHandler::testConnectElementsRejectsMissingParams()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("connect_elements", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickConnectionHandler::testConnectElementsRejectsUnknownSourceOrTarget()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);

    const QJsonObject params1{{"source_id", 999999}, {"target_id", a}};
    QCOMPARE(handler.handleCommand("connect_elements", params1, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ElementNotFound);

    const QJsonObject params2{{"source_id", a}, {"target_id", 999999}};
    QCOMPARE(handler.handleCommand("connect_elements", params2, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ElementNotFound);
}

void TestQuickConnectionHandler::testConnectElementsRejectsMissingPortSpec()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);

    const QJsonObject params{{"source_id", a}, {"target_id", b}};
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::PortNotFound);
}

void TestQuickConnectionHandler::testConnectElementsRejectsInvalidPortLabel()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port_label", "NoSuchPort"}, {"target_port", 0}};
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::PortNotFound);
}

void TestQuickConnectionHandler::testConnectElementsRejectsEmptyPortLabel()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port_label", ""}, {"target_port", 0}};
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::PortNotFound);
}

void TestQuickConnectionHandler::testConnectElementsRejectsNegativePortIndex()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port", -1}, {"target_port", 0}};
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::PortNotFound);
}

void TestQuickConnectionHandler::testConnectElementsRejectsOutOfRangePortIndex()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port", 0}, {"target_port", 999}};
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::PortNotFound);
}

void TestQuickConnectionHandler::testConnectElementsResolvesPortByLabel()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);

    auto *sourceElm = dynamic_cast<GraphicElement *>(canvas->itemById(a));
    auto *targetElm = dynamic_cast<GraphicElement *>(canvas->itemById(b));
    sourceElm->outputPort(0)->setName("Q");
    targetElm->inputPort(0)->setName("A");

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port_label", "Q"}, {"target_port_label", "A"}};
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(targetElm->inputPort(0)->connections().size(), 1);
}

void TestQuickConnectionHandler::testConnectElementsRejectsDisallowedConnection()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port", 0}, {"target_port", 0}};
    QVERIFY2(handler.handleCommand("connect_elements", params, 1).contains("result"), "first connection must succeed");

    // Same pair/ports again: already connected -> ConnectionManager::isConnectionAllowed() rejects it.
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestQuickConnectionHandler::testConnectElementsCreatesRealConnection()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);
    auto *targetElm = dynamic_cast<GraphicElement *>(canvas->itemById(b));

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port", 0}, {"target_port", 0}};
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(targetElm->inputPort(0)->connections().size(), 1);
}

void TestQuickConnectionHandler::testDisconnectElementsRejectsMissingParams()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("disconnect_elements", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickConnectionHandler::testDisconnectElementsRejectsUnknownElements()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);

    const QJsonObject params{{"source_id", a}, {"target_id", 999999}};
    const QJsonObject response = handler.handleCommand("disconnect_elements", params, 1);
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestQuickConnectionHandler::testDisconnectElementsRejectsUnknownSourceElement()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int b = addAnd(canvas);

    const QJsonObject params{{"source_id", 999999}, {"target_id", b}};
    const QJsonObject response = handler.handleCommand("disconnect_elements", params, 1);
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestQuickConnectionHandler::testDisconnectElementsRejectsWhenNoConnectionExists()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);

    const QJsonObject params{{"source_id", a}, {"target_id", b}};
    const QJsonObject response = handler.handleCommand("disconnect_elements", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ConnectionFailed);
}

void TestQuickConnectionHandler::testDisconnectElementsSkipsInProgressConnection()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);
    auto *targetElm = dynamic_cast<GraphicElement *>(canvas->itemById(b));

    // A wire with no ports set at all -- the loop must skip it, not crash on it, while still
    // finding and removing the real connection.
    auto *inProgress = new Connection();
    canvas->addItem(inProgress);

    const QJsonObject connectParams{{"source_id", a}, {"target_id", b},
                                    {"source_port", 0}, {"target_port", 0}};
    QVERIFY(handler.handleCommand("connect_elements", connectParams, 1).contains("result"));

    const QJsonObject disconnectParams{{"source_id", a}, {"target_id", b}};
    const QJsonObject response = handler.handleCommand("disconnect_elements", disconnectParams, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(targetElm->inputPort(0)->connections().size(), 0);
}

void TestQuickConnectionHandler::testDisconnectElementsMatchesReversedSourceTarget()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);
    auto *targetElm = dynamic_cast<GraphicElement *>(canvas->itemById(b));

    const QJsonObject connectParams{{"source_id", a}, {"target_id", b},
                                    {"source_port", 0}, {"target_port", 0}};
    QVERIFY(handler.handleCommand("connect_elements", connectParams, 1).contains("result"));

    // source_id/target_id swapped relative to how the wire was actually created -- disconnect
    // must still find it (order-independent).
    const QJsonObject disconnectParams{{"source_id", b}, {"target_id", a}};
    const QJsonObject response = handler.handleCommand("disconnect_elements", disconnectParams, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(targetElm->inputPort(0)->connections().size(), 0);
}

void TestQuickConnectionHandler::testDisconnectElementsRemovesRealConnection()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);
    auto *targetElm = dynamic_cast<GraphicElement *>(canvas->itemById(b));

    const QJsonObject connectParams{{"source_id", a}, {"target_id", b},
                                    {"source_port", 0}, {"target_port", 0}};
    QVERIFY(handler.handleCommand("connect_elements", connectParams, 1).contains("result"));
    QCOMPARE(targetElm->inputPort(0)->connections().size(), 1);

    const QJsonObject disconnectParams{{"source_id", a}, {"target_id", b}};
    const QJsonObject response = handler.handleCommand("disconnect_elements", disconnectParams, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(targetElm->inputPort(0)->connections().size(), 0);
}

void TestQuickConnectionHandler::testListConnectionsRejectsNoScene()
{
    QuickAppController controller;
    QVERIFY(!controller.currentTab());

    QuickConnectionHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand("list_connections", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestQuickConnectionHandler::testListConnectionsReturnsEmptyWhenNoneExist()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("list_connections", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(response["result"].toObject()["connections"].toArray().isEmpty());
}

void TestQuickConnectionHandler::testListConnectionsReturnsRealConnections()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);

    const QJsonObject connectParams{{"source_id", a}, {"target_id", b},
                                    {"source_port", 0}, {"target_port", 0}};
    QVERIFY(handler.handleCommand("connect_elements", connectParams, 1).contains("result"));

    const QJsonObject response = handler.handleCommand("list_connections", {}, 1);
    const QJsonArray connections = response["result"].toObject()["connections"].toArray();
    QCOMPARE(connections.size(), 1);
    QCOMPARE(connections[0].toObject()["source_id"].toInt(), a);
    QCOMPARE(connections[0].toObject()["source_port"].toInt(), 0);
    QCOMPARE(connections[0].toObject()["target_id"].toInt(), b);
    QCOMPARE(connections[0].toObject()["target_port"].toInt(), 0);
}

void TestQuickConnectionHandler::testListConnectionsSkipsInProgressConnection()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();

    // A wire with no ports set at all -- the same shape as a connection mid-drag,
    // before either endpoint has been attached.
    auto *inProgress = new Connection();
    canvas->addItem(inProgress);

    const QJsonObject response = handler.handleCommand("list_connections", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(response["result"].toObject()["connections"].toArray().isEmpty());
}

void TestQuickConnectionHandler::testSplitConnectionRejectsMissingParams()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("split_connection", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickConnectionHandler::testSplitConnectionRejectsInvalidPortsOrCoordinates()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);

    const QJsonObject badPort{{"source_id", a}, {"target_id", b},
                              {"source_port", -1}, {"target_port", 0}, {"x", 0}, {"y", 0}};
    QCOMPARE(handler.handleCommand("split_connection", badPort, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::InvalidParams);

    const QJsonObject badTargetPort{{"source_id", a}, {"target_id", b},
                                    {"source_port", 0}, {"target_port", -1}, {"x", 0}, {"y", 0}};
    QCOMPARE(handler.handleCommand("split_connection", badTargetPort, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::InvalidParams);

    const QJsonObject badCoord{{"source_id", a}, {"target_id", b},
                               {"source_port", 0}, {"target_port", 0}, {"x", "not a number"}, {"y", 0}};
    QCOMPARE(handler.handleCommand("split_connection", badCoord, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::InvalidParams);

    const QJsonObject badY{{"source_id", a}, {"target_id", b},
                           {"source_port", 0}, {"target_port", 0}, {"x", 0}, {"y", "not a number"}};
    QCOMPARE(handler.handleCommand("split_connection", badY, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::InvalidParams);
}

void TestQuickConnectionHandler::testSplitConnectionRejectsUnknownSourceOrTarget()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);

    const QJsonObject unknownSource{{"source_id", 999999}, {"target_id", b},
                                    {"source_port", 0}, {"target_port", 0}, {"x", 0}, {"y", 0}};
    QCOMPARE(handler.handleCommand("split_connection", unknownSource, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ElementNotFound);

    const QJsonObject unknownTarget{{"source_id", a}, {"target_id", 999999},
                                    {"source_port", 0}, {"target_port", 0}, {"x", 0}, {"y", 0}};
    QCOMPARE(handler.handleCommand("split_connection", unknownTarget, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ElementNotFound);
}

void TestQuickConnectionHandler::testSplitConnectionRejectsWhenNotFound()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port", 0}, {"target_port", 0}, {"x", 10}, {"y", 10}};
    const QJsonObject response = handler.handleCommand("split_connection", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ConnectionFailed);
}

void TestQuickConnectionHandler::testSplitConnectionSkipsInProgressConnection()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);

    auto *inProgress = new Connection();
    canvas->addItem(inProgress);

    const QJsonObject connectParams{{"source_id", a}, {"target_id", b},
                                    {"source_port", 0}, {"target_port", 0}};
    QVERIFY(handler.handleCommand("connect_elements", connectParams, 1).contains("result"));

    const qsizetype elementsBefore = canvas->elements().size();
    const qsizetype connectionsBefore = canvas->connections().size();
    const QJsonObject splitParams{{"source_id", a}, {"target_id", b},
                                  {"source_port", 0}, {"target_port", 0}, {"x", 50}, {"y", 50}};
    const QJsonObject response = handler.handleCommand("split_connection", splitParams, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    // CanvasSplitCommand::redo() reuses the existing connection (rewiring its endPort) and adds
    // exactly one new Node element plus one new Connection.
    QCOMPARE(canvas->elements().size(), elementsBefore + 1);
    QCOMPARE(canvas->connections().size(), connectionsBefore + 1);
}

void TestQuickConnectionHandler::testSplitConnectionPerformsRealSplit()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);
    auto *canvas = controller.currentTab()->canvas();
    const int a = addAnd(canvas);
    const int b = addAnd(canvas);

    const QJsonObject connectParams{{"source_id", a}, {"target_id", b},
                                    {"source_port", 0}, {"target_port", 0}};
    QVERIFY(handler.handleCommand("connect_elements", connectParams, 1).contains("result"));

    const qsizetype elementsBefore = canvas->elements().size();
    const qsizetype connectionsBefore = canvas->connections().size();

    const QJsonObject splitParams{{"source_id", a}, {"target_id", b},
                                  {"source_port", 0}, {"target_port", 0}, {"x", 50}, {"y", 50}};
    const QJsonObject response = handler.handleCommand("split_connection", splitParams, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));

    // Exactly one new Node element and one new Connection (see
    // testSplitConnectionSkipsInProgressConnection for the CanvasSplitCommand::redo() reasoning).
    QCOMPARE(canvas->elements().size(), elementsBefore + 1);
    QCOMPARE(canvas->connections().size(), connectionsBefore + 1);
}

void TestQuickConnectionHandler::testHandleCommandRejectsUnknownCommand()
{
    QuickAppController controller;
    controller.newTab();
    QuickConnectionHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("totally_made_up", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::MethodNotFound);
}
