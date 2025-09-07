// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "serverinfohandler.h"

#include <QJsonArray>

ServerInfoHandler::ServerInfoHandler(MainWindow *mainWindow, MCPValidator *validator)
    : BaseHandler(mainWindow, validator)
{
}

QJsonObject ServerInfoHandler::handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId)
{
    if (command == "get_server_info") {
        return handleGetServerInfo(params, requestId);
    } else {
        return createErrorResponse(QString("Unknown server info command: %1").arg(command), requestId);
    }
}

QJsonObject ServerInfoHandler::handleGetServerInfo(const QJsonObject &, const QJsonValue &requestId)
{
    QJsonObject result;
    result["server_name"] = "wiRedPanda MCP Server";
    result["version"] = APP_VERSION;
    result["protocol_version"] = "1.0";
    result["status"] = "ready";

    QJsonArray capabilities;
    capabilities.append("circuit_design");
    capabilities.append("simulation");
    capabilities.append("file_operations");
    capabilities.append("image_export");
    capabilities.append("waveform_generation");
    capabilities.append("ic_management");
    result["capabilities"] = capabilities;

    return createSuccessResponse(result, requestId);
}
