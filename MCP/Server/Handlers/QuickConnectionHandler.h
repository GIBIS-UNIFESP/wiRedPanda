// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "MCP/Server/Handlers/QuickBaseHandler.h"

/**
 * \class QuickConnectionHandler
 * \brief CanvasItem-side port of ConnectionHandler: connect/disconnect/list/split wires.
 */
class QuickConnectionHandler : public QuickBaseHandler
{
public:
    explicit QuickConnectionHandler(QuickAppController *appController, const MCPValidator *validator);

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    QJsonObject handleConnectElements(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleDisconnectElements(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleListConnections(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleSplitConnection(const QJsonObject &params, const QJsonValue &requestId);

    /// \copydoc ConnectionHandler::resolvePort
    bool resolvePort(const QJsonObject &params, const QString &prefix,
                     GraphicElement *element, bool isOutput,
                     int &portIndex, QString &errorMsg);
};
