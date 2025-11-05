// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "basehandler.h"

/*!
 * @class ConnectionHandler
 * @brief Handler for connection operation commands (connect, disconnect, list)
 *
 * Handles all connection-related MCP commands including connecting elements,
 * disconnecting elements, and listing existing connections.
 */
class ConnectionHandler : public BaseHandler
{
public:
    explicit ConnectionHandler(MainWindow *mainWindow, MCPValidator *validator);

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    QJsonObject handleConnectElements(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleDisconnectElements(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleListConnections(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleSplitConnection(const QJsonObject &params, const QJsonValue &requestId);
};
