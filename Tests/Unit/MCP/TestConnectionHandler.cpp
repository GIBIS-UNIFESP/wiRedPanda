// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/MCP/TestConnectionHandler.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTabWidget>
#include <QTest>

#include "App/Element/GraphicElements/And.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MainWindow.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "MCP/Server/Core/JsonRpcError.h"
#include "MCP/Server/Handlers/ConnectionHandler.h"

namespace {

int addAnd(Scene *scene)
{
    auto *elm = new And();
    scene->receiveCommand(new AddItemsCommand({elm}, scene));
    return elm->id();
}

} // namespace

void TestConnectionHandler::testConnectElementsRejectsMissingParams()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("connect_elements", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestConnectionHandler::testConnectElementsRejectsUnknownSourceOrTarget()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);

    const QJsonObject params1{{"source_id", 999999}, {"target_id", a}};
    QCOMPARE(handler.handleCommand("connect_elements", params1, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ElementNotFound);

    const QJsonObject params2{{"source_id", a}, {"target_id", 999999}};
    QCOMPARE(handler.handleCommand("connect_elements", params2, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ElementNotFound);
}

void TestConnectionHandler::testConnectElementsRejectsMissingPortSpec()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);

    const QJsonObject params{{"source_id", a}, {"target_id", b}};
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::PortNotFound);
}

void TestConnectionHandler::testConnectElementsRejectsInvalidPortLabel()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port_label", "NoSuchPort"}, {"target_port", 0}};
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::PortNotFound);
}

void TestConnectionHandler::testConnectElementsRejectsEmptyPortLabel()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port_label", ""}, {"target_port", 0}};
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::PortNotFound);
}

void TestConnectionHandler::testConnectElementsRejectsNegativePortIndex()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port", -1}, {"target_port", 0}};
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::PortNotFound);
}

void TestConnectionHandler::testConnectElementsRejectsOutOfRangePortIndex()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port", 0}, {"target_port", 999}};
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::PortNotFound);
}

void TestConnectionHandler::testConnectElementsResolvesPortByLabel()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);

    auto *sourceElm = dynamic_cast<GraphicElement *>(scene->itemById(a));
    auto *targetElm = dynamic_cast<GraphicElement *>(scene->itemById(b));
    sourceElm->outputPort(0)->setName("Q");
    targetElm->inputPort(0)->setName("A");

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port_label", "Q"}, {"target_port_label", "A"}};
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(targetElm->inputPort(0)->connections().size(), 1);
}

void TestConnectionHandler::testConnectElementsRejectsDisallowedConnection()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port", 0}, {"target_port", 0}};
    QVERIFY2(handler.handleCommand("connect_elements", params, 1).contains("result"), "first connection must succeed");

    // Same pair/ports again: already connected -> ConnectionManager::isConnectionAllowed() rejects it.
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestConnectionHandler::testConnectElementsCreatesRealConnection()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);
    auto *targetElm = dynamic_cast<GraphicElement *>(scene->itemById(b));

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port", 0}, {"target_port", 0}};
    const QJsonObject response = handler.handleCommand("connect_elements", params, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(targetElm->inputPort(0)->connections().size(), 1);
}

void TestConnectionHandler::testDisconnectElementsRejectsMissingParams()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("disconnect_elements", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestConnectionHandler::testDisconnectElementsRejectsUnknownElements()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);

    const QJsonObject params{{"source_id", a}, {"target_id", 999999}};
    const QJsonObject response = handler.handleCommand("disconnect_elements", params, 1);
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestConnectionHandler::testDisconnectElementsRejectsUnknownSourceElement()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int b = addAnd(scene);

    const QJsonObject params{{"source_id", 999999}, {"target_id", b}};
    const QJsonObject response = handler.handleCommand("disconnect_elements", params, 1);
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestConnectionHandler::testDisconnectElementsRejectsWhenNoConnectionExists()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);

    const QJsonObject params{{"source_id", a}, {"target_id", b}};
    const QJsonObject response = handler.handleCommand("disconnect_elements", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ConnectionFailed);
}

void TestConnectionHandler::testDisconnectElementsSkipsInProgressConnection()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);
    auto *targetElm = dynamic_cast<GraphicElement *>(scene->itemById(b));

    // A wire with no ports set at all -- the loop must skip it, not crash on it, while still
    // finding and removing the real connection.
    auto *inProgress = new Connection();
    scene->addItem(inProgress);

    const QJsonObject connectParams{{"source_id", a}, {"target_id", b},
                                    {"source_port", 0}, {"target_port", 0}};
    QVERIFY(handler.handleCommand("connect_elements", connectParams, 1).contains("result"));

    const QJsonObject disconnectParams{{"source_id", a}, {"target_id", b}};
    const QJsonObject response = handler.handleCommand("disconnect_elements", disconnectParams, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(targetElm->inputPort(0)->connections().size(), 0);
}

void TestConnectionHandler::testDisconnectElementsMatchesReversedSourceTarget()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);
    auto *targetElm = dynamic_cast<GraphicElement *>(scene->itemById(b));

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

void TestConnectionHandler::testDisconnectElementsRemovesRealConnection()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);
    auto *targetElm = dynamic_cast<GraphicElement *>(scene->itemById(b));

    const QJsonObject connectParams{{"source_id", a}, {"target_id", b},
                                    {"source_port", 0}, {"target_port", 0}};
    QVERIFY(handler.handleCommand("connect_elements", connectParams, 1).contains("result"));
    QCOMPARE(targetElm->inputPort(0)->connections().size(), 1);

    const QJsonObject disconnectParams{{"source_id", a}, {"target_id", b}};
    const QJsonObject response = handler.handleCommand("disconnect_elements", disconnectParams, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(targetElm->inputPort(0)->connections().size(), 0);
}

void TestConnectionHandler::testListConnectionsRejectsNoScene()
{
    MainWindow window;
    auto *tabs = window.findChild<QTabWidget *>();
    QVERIFY(tabs);
    emit tabs->tabCloseRequested(0);
    QVERIFY(!window.currentTab());

    ConnectionHandler handler(&window, nullptr);
    const QJsonObject response = handler.handleCommand("list_connections", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestConnectionHandler::testListConnectionsReturnsEmptyWhenNoneExist()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("list_connections", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(response["result"].toObject()["connections"].toArray().isEmpty());
}

void TestConnectionHandler::testListConnectionsReturnsRealConnections()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);

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

void TestConnectionHandler::testListConnectionsSkipsInProgressConnection()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();

    // A wire with no ports set at all -- the same shape as a connection mid-drag,
    // before either endpoint has been attached.
    auto *inProgress = new Connection();
    scene->addItem(inProgress);

    const QJsonObject response = handler.handleCommand("list_connections", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(response["result"].toObject()["connections"].toArray().isEmpty());
}

void TestConnectionHandler::testSplitConnectionRejectsMissingParams()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("split_connection", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestConnectionHandler::testSplitConnectionRejectsInvalidPortsOrCoordinates()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);

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

void TestConnectionHandler::testSplitConnectionRejectsUnknownSourceOrTarget()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);

    const QJsonObject unknownSource{{"source_id", 999999}, {"target_id", b},
                                    {"source_port", 0}, {"target_port", 0}, {"x", 0}, {"y", 0}};
    QCOMPARE(handler.handleCommand("split_connection", unknownSource, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ElementNotFound);

    const QJsonObject unknownTarget{{"source_id", a}, {"target_id", 999999},
                                    {"source_port", 0}, {"target_port", 0}, {"x", 0}, {"y", 0}};
    QCOMPARE(handler.handleCommand("split_connection", unknownTarget, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ElementNotFound);
}

void TestConnectionHandler::testSplitConnectionRejectsWhenNotFound()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);

    const QJsonObject params{{"source_id", a}, {"target_id", b},
                             {"source_port", 0}, {"target_port", 0}, {"x", 10}, {"y", 10}};
    const QJsonObject response = handler.handleCommand("split_connection", params, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ConnectionFailed);
}

void TestConnectionHandler::testSplitConnectionSkipsInProgressConnection()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);

    auto *inProgress = new Connection();
    scene->addItem(inProgress);

    const QJsonObject connectParams{{"source_id", a}, {"target_id", b},
                                    {"source_port", 0}, {"target_port", 0}};
    QVERIFY(handler.handleCommand("connect_elements", connectParams, 1).contains("result"));

    const qsizetype itemCountBefore = scene->items().size();
    const QJsonObject splitParams{{"source_id", a}, {"target_id", b},
                                  {"source_port", 0}, {"target_port", 0}, {"x", 50}, {"y", 50}};
    const QJsonObject response = handler.handleCommand("split_connection", splitParams, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(scene->items().size() > itemCountBefore);
}

void TestConnectionHandler::testSplitConnectionPerformsRealSplit()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = addAnd(scene);
    const int b = addAnd(scene);

    const QJsonObject connectParams{{"source_id", a}, {"target_id", b},
                                    {"source_port", 0}, {"target_port", 0}};
    QVERIFY(handler.handleCommand("connect_elements", connectParams, 1).contains("result"));

    const qsizetype itemCountBefore = scene->items().size();

    const QJsonObject splitParams{{"source_id", a}, {"target_id", b},
                                  {"source_port", 0}, {"target_port", 0}, {"x", 50}, {"y", 50}};
    const QJsonObject response = handler.handleCommand("split_connection", splitParams, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));

    // A split inserts a new Node plus a second wire -- the scene must have grown.
    QVERIFY(scene->items().size() > itemCountBefore);
}

void TestConnectionHandler::testHandleCommandRejectsUnknownCommand()
{
    MainWindow window;
    ConnectionHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("totally_made_up", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::MethodNotFound);
}
