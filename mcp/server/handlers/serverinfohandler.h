// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "basehandler.h"

/*!
 * @class ServerInfoHandler
 * @brief Handler for server information commands
 *
 * Handles server information requests providing server capabilities,
 * version information, and status.
 */
class ServerInfoHandler : public BaseHandler
{
public:
    explicit ServerInfoHandler(MainWindow *mainWindow, MCPValidator *validator);

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    QJsonObject handleGetServerInfo(const QJsonObject &params, const QJsonValue &requestId);
};
