// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QJsonObject>
#include <QString>

class MainWindow;
class MCPValidator;
class Scene;
class GraphicElement;

/*!
 * @class BaseHandler
 * @brief Abstract base class for MCP command handlers
 *
 * Provides common functionality and interface for all MCP command handlers.
 * Each handler is responsible for a specific domain of MCP commands.
 */
class BaseHandler
{
public:
    explicit BaseHandler(MainWindow *mainWindow, MCPValidator *validator);
    virtual ~BaseHandler() = default;

    // Pure virtual method for handling commands - each handler implements this
    virtual QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) = 0;

    // Public utility methods available to all classes
    QJsonObject createErrorResponse(const QString &error, const QJsonValue &requestId = QJsonValue());
    QJsonObject createSuccessResponse(const QJsonObject &result = {}, const QJsonValue &requestId = QJsonValue());
    Scene *getCurrentScene();
    bool validateAndSendResponse(const QJsonObject &response, const QString &command = QString());
    bool validateElementId(int elementId, const QString &paramName, QString &errorMsg);
    bool validateNonEmptyString(const QJsonValue &value, const QString &paramName, QString &errorMsg);
    bool validateNonNegativeInteger(const QJsonValue &value, const QString &paramName, QString &errorMsg);
    bool validateNumeric(const QJsonValue &value, const QString &paramName, QString &errorMsg);
    bool validateParameters(const QJsonObject &params, const QStringList &required);
    bool validatePortRange(GraphicElement *element, int portIndex, bool isOutput, const QString &paramName, QString &errorMsg);
    bool validatePositiveInteger(const QJsonValue &value, const QString &paramName, QString &errorMsg);
    void sendResponse(const QJsonObject &response);

protected:
    // Shared resources
    MainWindow *m_mainWindow;
    MCPValidator *m_validator;
};
