// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "MCP/Server/Handlers/QuickBaseHandler.h"

/**
 * \class QuickHistoryHandler
 * \brief CanvasItem-side port of HistoryHandler: undo/redo commands.
 */
class QuickHistoryHandler : public QuickBaseHandler
{
public:
    explicit QuickHistoryHandler(QuickAppController *appController, const MCPValidator *validator);

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    Q_DISABLE_COPY_MOVE(QuickHistoryHandler)

    QJsonObject handleUndo(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleRedo(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleGetUndoStack(const QJsonObject &params, const QJsonValue &requestId);
};
