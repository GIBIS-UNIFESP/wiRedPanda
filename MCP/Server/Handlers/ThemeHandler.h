// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "MCP/Server/Handlers/BaseHandler.h"

/// Handler for theme management commands
class ThemeHandler : public BaseHandler
{
public:
    explicit ThemeHandler(MainWindow *mainWindow, MCPValidator *validator);

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    QJsonObject handleGetTheme(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleSetTheme(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleGetEffectiveTheme(const QJsonObject &params, const QJsonValue &requestId);

    QString themeToString(int theme) const;
};

