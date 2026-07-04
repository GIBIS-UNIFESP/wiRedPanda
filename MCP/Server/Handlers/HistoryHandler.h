// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "MCP/Server/Handlers/BaseHandler.h"

/**
 * \class HistoryHandler
 * \brief Handler for undo/redo commands
 *
 * Handles the MCP commands that drive the active scene's undo stack: undo, redo,
 * and querying the current undo/redo state.
 */
class HistoryHandler : public BaseHandler
{
public:
    explicit HistoryHandler(MainWindow *mainWindow, const MCPValidator *validator);

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    Q_DISABLE_COPY_MOVE(HistoryHandler)

    QJsonObject handleUndo(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleRedo(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleGetUndoStack(const QJsonObject &params, const QJsonValue &requestId);
};
