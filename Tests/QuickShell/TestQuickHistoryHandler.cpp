// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickHistoryHandler.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUndoStack>

#include "App/Element/GraphicElements/And.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "MCP/Server/Core/JsonRpcError.h"
#include "MCP/Server/Handlers/QuickHistoryHandler.h"

void TestQuickHistoryHandler::testHandleUndoRejectsNoScene()
{
    QuickAppController controller;
    QVERIFY(!controller.currentTab());

    QuickHistoryHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand("undo", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestQuickHistoryHandler::testHandleRedoRejectsNoScene()
{
    QuickAppController controller;
    QVERIFY(!controller.currentTab());

    QuickHistoryHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand("redo", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestQuickHistoryHandler::testHandleGetUndoStackRejectsNoScene()
{
    QuickAppController controller;
    QVERIFY(!controller.currentTab());

    QuickHistoryHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand("get_undo_stack", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestQuickHistoryHandler::testHandleUndoReportsNothingToUndo()
{
    QuickAppController controller;
    controller.newTab();
    QuickHistoryHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("undo", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();
    QCOMPARE(result["success"].toBool(), false);
    QCOMPARE(result["message"].toString(), QStringLiteral("Nothing to undo"));
}

void TestQuickHistoryHandler::testHandleUndoPerformsRealUndo()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    auto *andGate = new And();
    canvas->receiveCommand(new CanvasAddItemsCommand({andGate}, canvas));
    const int elementId = andGate->id();
    QVERIFY(canvas->itemById(elementId));

    QuickHistoryHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand("undo", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();
    QCOMPARE(result["success"].toBool(), true);
    QCOMPARE(result["can_undo"].toBool(), false);
    QCOMPARE(result["can_redo"].toBool(), true);
    QCOMPARE(result["redo_text"].toString(), canvas->undoStack()->redoText());

    // The real undo effect: the element must actually be gone from the canvas.
    QVERIFY(!canvas->itemById(elementId));
}

void TestQuickHistoryHandler::testHandleRedoReportsNothingToRedo()
{
    QuickAppController controller;
    controller.newTab();
    QuickHistoryHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("redo", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();
    QCOMPARE(result["success"].toBool(), false);
    QCOMPARE(result["message"].toString(), QStringLiteral("Nothing to redo"));
}

void TestQuickHistoryHandler::testHandleRedoPerformsRealRedo()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    auto *andGate = new And();
    canvas->receiveCommand(new CanvasAddItemsCommand({andGate}, canvas));
    const int elementId = andGate->id();

    canvas->undoStack()->undo();
    QVERIFY(!canvas->itemById(elementId));

    QuickHistoryHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand("redo", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();
    QCOMPARE(result["success"].toBool(), true);
    QCOMPARE(result["can_undo"].toBool(), true);
    QCOMPARE(result["can_redo"].toBool(), false);
    QCOMPARE(result["redo_text"].toString(), canvas->undoStack()->redoText());

    // The real redo effect: the element must be back in the canvas.
    QVERIFY(canvas->itemById(elementId));
}

void TestQuickHistoryHandler::testHandleGetUndoStackReportsRealState()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    auto *andGate = new And();
    canvas->receiveCommand(new CanvasAddItemsCommand({andGate}, canvas));

    QuickHistoryHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand("get_undo_stack", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();

    const QUndoStack *undoStack = canvas->undoStack();
    QCOMPARE(result["can_undo"].toBool(), undoStack->canUndo());
    QCOMPARE(result["can_redo"].toBool(), undoStack->canRedo());
    QCOMPARE(result["undo_text"].toString(), undoStack->undoText());
    QCOMPARE(result["redo_text"].toString(), undoStack->redoText());
    QCOMPARE(result["undo_count"].toInt(), undoStack->count());
    QCOMPARE(result["undo_index"].toInt(), undoStack->index());
    QCOMPARE(result["undo_limit"].toInt(), undoStack->undoLimit());
}

void TestQuickHistoryHandler::testHandleCommandRejectsUnknownCommand()
{
    QuickAppController controller;
    controller.newTab();
    QuickHistoryHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("totally_made_up", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::MethodNotFound);
}
