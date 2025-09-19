// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "basehandler.h"

/*!
 * @class ElementHandler
 * @brief Handler for element operation commands (create, delete, modify, etc.)
 *
 * Handles all element-related MCP commands including creating elements,
 * deleting elements, listing elements, moving elements, and setting properties.
 */
class ElementHandler : public BaseHandler
{
public:
    explicit ElementHandler(MainWindow *mainWindow, MCPValidator *validator);

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    QJsonObject handleCreateElement(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleDeleteElement(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleListElements(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleMoveElement(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleSetElementProperties(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleSetInputValue(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleGetOutputValue(const QJsonObject &params, const QJsonValue &requestId);
};
