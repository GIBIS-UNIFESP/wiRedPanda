// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "MCP/Server/Handlers/QuickBaseHandler.h"

/**
 * \class QuickFileHandler
 * \brief CanvasItem-side port of FileHandler: load/save/new/close/tab-count/export commands.
 */
class QuickFileHandler : public QuickBaseHandler
{
public:
    explicit QuickFileHandler(QuickAppController *appController, const MCPValidator *validator);

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    QJsonObject handleLoadCircuit(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleSaveCircuit(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleNewCircuit(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleCloseCircuit(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleGetTabCount(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleExportImage(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleExportArduino(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleExportSystemVerilog(const QJsonObject &params, const QJsonValue &requestId);
};
