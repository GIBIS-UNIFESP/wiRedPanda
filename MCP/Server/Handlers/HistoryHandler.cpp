// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/HistoryHandler.h"

#include <QUndoStack>

#include "App/Scene/Scene.h"

HistoryHandler::HistoryHandler(MainWindow *mainWindow, const MCPValidator *validator)
    : BaseHandler(mainWindow, validator)
{
}

QJsonObject HistoryHandler::handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId)
{
    if (command == "undo") {
        return handleUndo(params, requestId);
    } else if (command == "redo") {
        return handleRedo(params, requestId);
    } else if (command == "get_undo_stack") {
        return handleGetUndoStack(params, requestId);
    } else {
        return createErrorResponse(QString("Unknown history command: %1").arg(command),
                                   requestId, JsonRpcError::MethodNotFound);
    }
}

QJsonObject HistoryHandler::handleUndo(const QJsonObject &params, const QJsonValue &requestId)
{
    (void)params;  // Parameter not used
    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    QUndoStack *undoStack = scene->undoStack();
    // Unreachable: Scene::undoStack() returns the address of a value member, never null.
    if (!undoStack) {
        return createErrorResponse("Undo stack not available", requestId, JsonRpcError::InternalError); // LCOV_EXCL_LINE
    }

    return tryCommand([&]() -> QJsonObject { // LCOV_EXCL_LINE -- gcov misattributes this multi-line lambda-taking call's entry (pattern 45); the lambda body below is genuinely covered
        if (!undoStack->canUndo()) {
            QJsonObject result;
            result["success"] = false;
            result["message"] = "Nothing to undo";
            return createSuccessResponse(result, requestId);
        }

        undoStack->undo();

        QJsonObject result;
        result["success"] = true;
        result["message"] = "Undo operation completed";
        result["can_undo"] = undoStack->canUndo();
        result["can_redo"] = undoStack->canRedo();
        result["undo_text"] = undoStack->undoText();
        result["redo_text"] = undoStack->redoText();

        return createSuccessResponse(result, requestId);
    }, "undo", requestId);
}

QJsonObject HistoryHandler::handleRedo(const QJsonObject &params, const QJsonValue &requestId)
{
    (void)params;  // Parameter not used
    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    QUndoStack *undoStack = scene->undoStack();
    // Unreachable: Scene::undoStack() returns the address of a value member, never null.
    if (!undoStack) {
        return createErrorResponse("Undo stack not available", requestId, JsonRpcError::InternalError); // LCOV_EXCL_LINE
    }

    return tryCommand([&]() -> QJsonObject { // LCOV_EXCL_LINE -- gcov misattributes this multi-line lambda-taking call's entry (pattern 45); the lambda body below is genuinely covered
        if (!undoStack->canRedo()) {
            QJsonObject result;
            result["success"] = false;
            result["message"] = "Nothing to redo";
            return createSuccessResponse(result, requestId);
        }

        undoStack->redo();

        QJsonObject result;
        result["success"] = true;
        result["message"] = "Redo operation completed";
        result["can_undo"] = undoStack->canUndo();
        result["can_redo"] = undoStack->canRedo();
        result["undo_text"] = undoStack->undoText();
        result["redo_text"] = undoStack->redoText();

        return createSuccessResponse(result, requestId);
    }, "redo", requestId);
}

QJsonObject HistoryHandler::handleGetUndoStack(const QJsonObject &params, const QJsonValue &requestId)
{
    (void)params;  // Parameter not used
    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    QUndoStack *undoStack = scene->undoStack();
    // Unreachable: Scene::undoStack() returns the address of a value member, never null.
    if (!undoStack) {
        return createErrorResponse("Undo stack not available", requestId, JsonRpcError::InternalError); // LCOV_EXCL_LINE
    }

    return tryCommand([&]() -> QJsonObject { // LCOV_EXCL_LINE -- gcov misattributes this multi-line lambda-taking call's entry (pattern 45); the lambda body below is genuinely covered
        QJsonObject result;
        result["can_undo"] = undoStack->canUndo();
        result["can_redo"] = undoStack->canRedo();
        result["undo_text"] = undoStack->undoText();
        result["redo_text"] = undoStack->redoText();
        result["undo_limit"] = undoStack->undoLimit();
        result["undo_count"] = undoStack->count();
        result["undo_index"] = undoStack->index();

        return createSuccessResponse(result, requestId);
    }, "get undo stack info", requestId);
}
