// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "MCP/Server/Handlers/BaseHandler.h"

/**
 * \class HistoryHandler
 * \brief Handler for undo/redo history commands (undo, redo, get_undo_stack).
 *
 * Split out of SimulationHandler: undo/redo are an editing-history concern, not a
 * simulation one. Operates on the current scene's QUndoStack.
 */
class HistoryHandler : public BaseHandler
{
public:
    explicit HistoryHandler(MainWindow *mainWindow, MCPValidator *validator);

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    QJsonObject handleUndo(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleRedo(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleGetUndoStack(const QJsonObject &params, const QJsonValue &requestId);
};
