// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "MCP/Server/Handlers/QuickBaseHandler.h"

/**
 * \class QuickICHandler
 * \brief CanvasItem-side port of ICHandler: create/instantiate/list/embed/extract ICs.
 */
class QuickICHandler : public QuickBaseHandler
{
public:
    explicit QuickICHandler(QuickAppController *appController, const MCPValidator *validator);

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    Q_DISABLE_COPY_MOVE(QuickICHandler)

    QJsonObject handleCreateIC(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleInstantiateIC(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleListICs(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleEmbedIC(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleExtractIC(const QJsonObject &params, const QJsonValue &requestId);
};
