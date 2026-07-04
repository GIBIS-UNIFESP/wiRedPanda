// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "MCP/Server/Handlers/BaseHandler.h"

/**
 * \class ICHandler
 * \brief Handler for IC (Integrated Circuit) commands
 *
 * Handles all IC-related MCP commands: creating an IC from the current circuit,
 * instantiating an IC into the scene, listing available ICs, and embedding/extracting
 * IC blobs.
 */
class ICHandler : public BaseHandler
{
public:
    explicit ICHandler(MainWindow *mainWindow, const MCPValidator *validator);

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    Q_DISABLE_COPY_MOVE(ICHandler)

    QJsonObject handleCreateIC(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleInstantiateIC(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleListICs(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleEmbedIC(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleExtractIC(const QJsonObject &params, const QJsonValue &requestId);
};
