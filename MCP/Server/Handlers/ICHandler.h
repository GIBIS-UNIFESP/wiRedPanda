// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "MCP/Server/Handlers/BaseHandler.h"

/**
 * \class ICHandler
 * \brief Handler for IC management commands (create, instantiate, list, embed, extract).
 *
 * Split out of SimulationHandler so all IC orchestration lives in one handler,
 * mirroring how the App side separates IC concerns. Handles create_ic,
 * instantiate_ic, list_ics, embed_ic and extract_ic.
 */
class ICHandler : public BaseHandler
{
public:
    explicit ICHandler(MainWindow *mainWindow, MCPValidator *validator);

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    QJsonObject handleCreateIC(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleInstantiateIC(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleListICs(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleEmbedIC(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleExtractIC(const QJsonObject &params, const QJsonValue &requestId);
};
