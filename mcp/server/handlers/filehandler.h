// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "basehandler.h"

/*!
 * @class FileHandler
 * @brief Handler for file operation commands (load, save, export, etc.)
 *
 * Handles all file-related MCP commands including circuit loading,
 * saving, tab management, and image export operations.
 */
class FileHandler : public BaseHandler
{
public:
    explicit FileHandler(MainWindow *mainWindow, MCPValidator *validator);

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    QJsonObject handleLoadCircuit(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleSaveCircuit(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleNewCircuit(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleCloseCircuit(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleGetTabCount(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleExportImage(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleExportArduino(const QJsonObject &params, const QJsonValue &requestId);
};
