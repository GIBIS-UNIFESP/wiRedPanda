// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/MCP/TestHistoryHandler.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTabWidget>
#include <QTest>
#include <QUndoStack>

#include "App/Element/GraphicElements/And.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MainWindow.h"
#include "MCP/Server/Core/JsonRpcError.h"
#include "MCP/Server/Handlers/HistoryHandler.h"

void TestHistoryHandler::testHandleUndoRejectsNoScene()
{
    MainWindow window;
    auto *tabs = window.findChild<QTabWidget *>();
    QVERIFY(tabs);
    emit tabs->tabCloseRequested(0);
    QVERIFY(!window.currentTab());

    HistoryHandler handler(&window, nullptr);
    const QJsonObject response = handler.handleCommand("undo", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestHistoryHandler::testHandleRedoRejectsNoScene()
{
    MainWindow window;
    auto *tabs = window.findChild<QTabWidget *>();
    QVERIFY(tabs);
    emit tabs->tabCloseRequested(0);
    QVERIFY(!window.currentTab());

    HistoryHandler handler(&window, nullptr);
    const QJsonObject response = handler.handleCommand("redo", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestHistoryHandler::testHandleGetUndoStackRejectsNoScene()
{
    MainWindow window;
    auto *tabs = window.findChild<QTabWidget *>();
    QVERIFY(tabs);
    emit tabs->tabCloseRequested(0);
    QVERIFY(!window.currentTab());

    HistoryHandler handler(&window, nullptr);
    const QJsonObject response = handler.handleCommand("get_undo_stack", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestHistoryHandler::testHandleUndoReportsNothingToUndo()
{
    MainWindow window;
    HistoryHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("undo", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();
    QCOMPARE(result["success"].toBool(), false);
    QCOMPARE(result["message"].toString(), QStringLiteral("Nothing to undo"));
}

void TestHistoryHandler::testHandleUndoPerformsRealUndo()
{
    MainWindow window;
    auto *scene = window.currentTab()->scene();
    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand({andGate}, scene));
    const int elementId = andGate->id();
    QVERIFY(scene->itemById(elementId));

    HistoryHandler handler(&window, nullptr);
    const QJsonObject response = handler.handleCommand("undo", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();
    QCOMPARE(result["success"].toBool(), true);
    QCOMPARE(result["can_undo"].toBool(), false);
    QCOMPARE(result["can_redo"].toBool(), true);

    // The real undo effect: the element must actually be gone from the scene.
    QVERIFY(!scene->itemById(elementId));
}

void TestHistoryHandler::testHandleRedoReportsNothingToRedo()
{
    MainWindow window;
    HistoryHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("redo", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();
    QCOMPARE(result["success"].toBool(), false);
    QCOMPARE(result["message"].toString(), QStringLiteral("Nothing to redo"));
}

void TestHistoryHandler::testHandleRedoPerformsRealRedo()
{
    MainWindow window;
    auto *scene = window.currentTab()->scene();
    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand({andGate}, scene));
    const int elementId = andGate->id();

    scene->undoStack()->undo();
    QVERIFY(!scene->itemById(elementId));

    HistoryHandler handler(&window, nullptr);
    const QJsonObject response = handler.handleCommand("redo", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();
    QCOMPARE(result["success"].toBool(), true);
    QCOMPARE(result["can_undo"].toBool(), true);
    QCOMPARE(result["can_redo"].toBool(), false);

    // The real redo effect: the element must be back in the scene.
    QVERIFY(scene->itemById(elementId));
}

void TestHistoryHandler::testHandleGetUndoStackReportsRealState()
{
    MainWindow window;
    auto *scene = window.currentTab()->scene();
    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand({andGate}, scene));

    HistoryHandler handler(&window, nullptr);
    const QJsonObject response = handler.handleCommand("get_undo_stack", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();

    const QUndoStack *undoStack = scene->undoStack();
    QCOMPARE(result["can_undo"].toBool(), undoStack->canUndo());
    QCOMPARE(result["can_redo"].toBool(), undoStack->canRedo());
    QCOMPARE(result["undo_text"].toString(), undoStack->undoText());
    QCOMPARE(result["undo_count"].toInt(), undoStack->count());
    QCOMPARE(result["undo_index"].toInt(), undoStack->index());
    QCOMPARE(result["undo_limit"].toInt(), undoStack->undoLimit());
}

void TestHistoryHandler::testHandleCommandRejectsUnknownCommand()
{
    MainWindow window;
    HistoryHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("totally_made_up", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::MethodNotFound);
}
