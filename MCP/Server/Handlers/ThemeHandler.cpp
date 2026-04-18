// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/ThemeHandler.h"

#include "App/Core/ThemeManager.h"

ThemeHandler::ThemeHandler(MainWindow *mainWindow, MCPValidator *validator)
    : BaseHandler(mainWindow, validator)
{
}

QJsonObject ThemeHandler::handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId)
{
    if (command == "get_theme") {
        return handleGetTheme(params, requestId);
    } else if (command == "set_theme") {
        return handleSetTheme(params, requestId);
    } else if (command == "get_effective_theme") {
        return handleGetEffectiveTheme(params, requestId);
    } else {
        return createErrorResponse(QString("Unknown theme command: %1").arg(command), requestId);
    }
}

QJsonObject ThemeHandler::handleGetTheme(const QJsonObject &, const QJsonValue &requestId)
{
    QJsonObject result;
    result["theme"] = themeToString(static_cast<int>(ThemeManager::theme()));
    return createSuccessResponse(result, requestId);
}

QJsonObject ThemeHandler::handleSetTheme(const QJsonObject &params, const QJsonValue &requestId)
{
    const QString themeStr = params["theme"].toString().toLower();
    Theme theme;

    if (themeStr == "light") {
        theme = Theme::Light;
    } else if (themeStr == "dark") {
        theme = Theme::Dark;
    } else if (themeStr == "system") {
        theme = Theme::System;
    } else {
        return createErrorResponse(QString("Invalid theme: '%1'. Expected 'light', 'dark', or 'system'.").arg(themeStr), requestId);
    }

    ThemeManager::setTheme(theme);

    QJsonObject result;
    result["theme"] = themeToString(static_cast<int>(theme));
    return createSuccessResponse(result, requestId);
}

QJsonObject ThemeHandler::handleGetEffectiveTheme(const QJsonObject &, const QJsonValue &requestId)
{
    QJsonObject result;
    result["effective_theme"] = themeToString(static_cast<int>(ThemeManager::effectiveTheme()));
    return createSuccessResponse(result, requestId);
}

QString ThemeHandler::themeToString(int theme) const
{
    switch (static_cast<Theme>(theme)) {
    case Theme::Light:
        return "light";
    case Theme::Dark:
        return "dark";
    case Theme::System:
        return "system";
    default:
        return "unknown";
    }
}

