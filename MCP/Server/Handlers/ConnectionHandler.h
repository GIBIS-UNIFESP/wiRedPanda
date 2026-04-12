// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "MCP/Server/Handlers/BaseHandler.h"

/**
 * \class ConnectionHandler
 * \brief Handler for connection operation commands (connect, disconnect, list, split)
 *
 * Handles all connection-related MCP commands including connecting elements,
 * disconnecting elements, listing existing connections, and splitting connections.
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

    /**
     * \brief Resolves a port index for the element at \a prefix + "_id".
     * \details Accepts either a label (\a prefix + "_port_label") or an integer index
     * (\a prefix + "_port"). Sets \a portIndex on success, \a errorMsg on failure.
     * \param isOutput true when resolving an output port (source side), false for input (target side).
     */
    bool resolvePort(const QJsonObject &params, const QString &prefix,
                     GraphicElement *element, bool isOutput,
                     int &portIndex, QString &errorMsg);
};

