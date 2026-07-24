// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/ThemeHandler.h"

#include "App/Core/ThemeManager.h"

ThemeHandler::ThemeHandler(MainWindow *mainWindow, const MCPValidator *validator)
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
        return createErrorResponse(QString("Unknown theme command: %1").arg(command),
                                   requestId, JsonRpcError::MethodNotFound);
    }
}

QJsonObject ThemeHandler::handleGetTheme(const QJsonObject &, const QJsonValue &requestId)
{
    QJsonObject result;
    result["theme"] = themeToString(ThemeManager::theme());
    return createSuccessResponse(result, requestId);
}

QJsonObject ThemeHandler::handleSetTheme(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"theme"})) {
        return createErrorResponse("Missing required parameter: theme", requestId, JsonRpcError::InvalidParams);
    }

    const QString themeStr = params["theme"].toString().toLower();
    Theme theme;

    if (themeStr == "light") {
        theme = Theme::Light;
    } else if (themeStr == "dark") {
        theme = Theme::Dark;
    } else if (themeStr == "system") {
        theme = Theme::System;
    } else {
        return createErrorResponse(QString("Invalid theme: '%1'. Expected 'light', 'dark', or 'system'.").arg(themeStr),
                                   requestId, JsonRpcError::ValidationError);
    }

    ThemeManager::setTheme(theme);

    QJsonObject result;
    result["theme"] = themeToString(theme);
    return createSuccessResponse(result, requestId);
}

QJsonObject ThemeHandler::handleGetEffectiveTheme(const QJsonObject &, const QJsonValue &requestId)
{
    QJsonObject result;
    result["effective_theme"] = themeToString(ThemeManager::effectiveTheme());
    return createSuccessResponse(result, requestId);
}

QString ThemeHandler::themeToString(Theme theme) const
{
    switch (theme) {
    case Theme::Light:
        return "light";
    case Theme::Dark:
        return "dark";
    case Theme::System:
        return "system";
    }
    Q_UNREACHABLE(); // LCOV_EXCL_LINE -- Theme is an enum class with all 3 values handled by name
}
