// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basehandler.h"

#include "elementfactory.h"
#include "graphicelement.h"
#include "mainwindow.h"
#include "mcpvalidator.h"
#include "scene.h"
#include "workspace.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <cmath>

BaseHandler::BaseHandler(MainWindow *mainWindow, MCPValidator *validator)
    : m_mainWindow(mainWindow)
    , m_validator(validator)
{
}

QJsonObject BaseHandler::createSuccessResponse(const QJsonObject &result, const QJsonValue &requestId)
{
    QJsonObject response;
    response["jsonrpc"] = "2.0";
    response["result"] = result;
    if (!requestId.isNull()) {
        response["id"] = requestId;
    }
    return response;
}

QJsonObject BaseHandler::createErrorResponse(const QString &error, const QJsonValue &requestId)
{
    QJsonObject response;
    response["jsonrpc"] = "2.0";

    QJsonObject errorObject;
    errorObject["code"] = -32603; // Internal error
    errorObject["message"] = error;
    response["error"] = errorObject;

    if (!requestId.isNull()) {
        response["id"] = requestId;
    }
    return response;
}

bool BaseHandler::validateParameters(const QJsonObject &params, const QStringList &required)
{
    for (const QString &param : required) {
        if (!params.contains(param)) {
            return false;
        }
    }
    return true;
}

bool BaseHandler::validateAndSendResponse(const QJsonObject &response, const QString &command)
{
    // Validate response
    if (m_validator) {
        ValidationResult validationResult = m_validator->validateResponse(response, command);
        if (!validationResult.isValid) {
            QString detailedError = validationResult.errorMessage;

            if (!validationResult.errorPath.isEmpty()) {
                detailedError += QString(" (at path: %1)").arg(validationResult.errorPath);
            }

            QJsonObject errorResponse = createErrorResponse(QString("Internal validation error: %1").arg(detailedError));
            sendResponse(errorResponse);
            return false;
        }
    }

    sendResponse(response);
    return true;
}

void BaseHandler::sendResponse(const QJsonObject &response)
{
    QJsonDocument doc(response);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    // Output to stdout for MCP communication
    QTextStream out(stdout);
    out << QString::fromUtf8(data) << Qt::endl;
    out.flush();
}

Scene *BaseHandler::getCurrentScene()
{
    if (!m_mainWindow) {
        return nullptr;
    }

    WorkSpace *workspace = m_mainWindow->currentTab();
    if (!workspace) {
        return nullptr;
    }

    return workspace->scene();
}

bool BaseHandler::validatePositiveInteger(const QJsonValue &value, const QString &paramName, QString &errorMsg)
{
    if (!value.isDouble()) {
        errorMsg = QString("Parameter '%1' must be an integer").arg(paramName);
        return false;
    }

    int intVal = value.toInt();
    if (intVal <= 0) {
        errorMsg = QString("Parameter '%1' must be a positive integer (got %2)").arg(paramName).arg(intVal);
        return false;
    }

    return true;
}

bool BaseHandler::validateNonNegativeInteger(const QJsonValue &value, const QString &paramName, QString &errorMsg)
{
    if (!value.isDouble()) {
        errorMsg = QString("Parameter '%1' must be an integer").arg(paramName);
        return false;
    }

    int intVal = value.toInt();
    if (intVal < 0) {
        errorMsg = QString("Parameter '%1' must be non-negative (got %2)").arg(paramName).arg(intVal);
        return false;
    }

    return true;
}

bool BaseHandler::validateNonEmptyString(const QJsonValue &value, const QString &paramName, QString &errorMsg)
{
    if (!value.isString()) {
        errorMsg = QString("Parameter '%1' must be a string").arg(paramName);
        return false;
    }

    QString str = value.toString();
    if (str.isEmpty()) {
        errorMsg = QString("Parameter '%1' cannot be empty").arg(paramName);
        return false;
    }

    return true;
}

bool BaseHandler::validateElementId(int elementId, const QString &paramName, QString &errorMsg)
{
    if (elementId <= 0) {
        errorMsg = QString("Parameter '%1' must be a positive integer (got %2)").arg(paramName).arg(elementId);
        return false;
    }

    auto *item = ElementFactory::itemById(elementId);
    if (!item) {
        errorMsg = QString("Element not found: %1").arg(elementId);
        return false;
    }

    return true;
}

bool BaseHandler::validateNumeric(const QJsonValue &value, const QString &paramName, QString &errorMsg)
{
    if (!value.isDouble()) {
        errorMsg = QString("Parameter '%1' must be a numeric value").arg(paramName);
        return false;
    }

    double numVal = value.toDouble();
    if (std::isnan(numVal) || std::isinf(numVal)) {
        errorMsg = QString("Parameter '%1' must be a finite numeric value").arg(paramName);
        return false;
    }

    return true;
}

bool BaseHandler::validatePortRange(GraphicElement *element, int portIndex, bool isOutput, const QString &paramName, QString &errorMsg)
{
    if (!element) {
        errorMsg = QString("Invalid element for port validation");
        return false;
    }

    // Use the safe QVector size() instead of calling outputPort()/inputPort()
    int maxPorts = 0;
    if (isOutput) {
        maxPorts = element->outputs().size();  // Safe QVector size access
    } else {
        maxPorts = element->inputs().size();   // Safe QVector size access
    }

    if (portIndex >= maxPorts) {
        QString portType = isOutput ? "output" : "input";
        errorMsg = QString("Parameter '%1' port index %2 is out of range (element has %3 %4 ports)")
                   .arg(paramName)
                   .arg(portIndex)
                   .arg(maxPorts)
                   .arg(portType);
        return false;
    }

    return true;
}
