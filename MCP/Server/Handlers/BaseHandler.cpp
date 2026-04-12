// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/BaseHandler.h"

#include <cmath>

#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MainWindow.h"
#include "MCP/Server/Core/MCPValidator.h"

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

    Scene *scene = getCurrentScene();
    if (!scene) {
        errorMsg = "No active circuit scene available";
        return false;
    }

    auto *item = scene->itemById(elementId);
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
        maxPorts = static_cast<int>(element->outputs().size());
    } else {
        maxPorts = static_cast<int>(element->inputs().size());
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

GraphicElement *BaseHandler::getValidatedElement(const QJsonObject &params, const QString &paramName, QString &errorMsg)
{
    if (!validatePositiveInteger(params.value(paramName), paramName, errorMsg)) {
        return nullptr;
    }
    const int elementId = params.value(paramName).toInt();
    if (!validateElementId(elementId, paramName, errorMsg)) {
        return nullptr;
    }
    auto *item = getCurrentScene()->itemById(elementId);
    auto *element = dynamic_cast<GraphicElement *>(item);
    if (!element) {
        errorMsg = QString("Item %1 is not a graphic element").arg(elementId);
        return nullptr;
    }
    return element;
}

bool BaseHandler::getInputPortByLabel(GraphicElement *element, const QString &label, int &portIndex, QString &errorMsg)
{
    if (!element) {
        errorMsg = "Element is null";
        return false;
    }

    int inputCount = element->inputSize();
    for (int i = 0; i < inputCount; ++i) {
        auto *port = element->inputPort(i);
        if (port && port->name() == label) {
            portIndex = i;
            return true;
        }
    }

    errorMsg = QString("Input port '%1' not found on element '%2'. Available input ports: %3")
               .arg(label, element->objectName(), getAvailableInputPorts(element));
    return false;
}

bool BaseHandler::getOutputPortByLabel(GraphicElement *element, const QString &label, int &portIndex, QString &errorMsg)
{
    if (!element) {
        errorMsg = "Element is null";
        return false;
    }

    int outputCount = element->outputSize();
    for (int i = 0; i < outputCount; ++i) {
        auto *port = element->outputPort(i);
        if (port && port->name() == label) {
            portIndex = i;
            return true;
        }
    }

    errorMsg = QString("Output port '%1' not found on element '%2'. Available output ports: %3")
               .arg(label, element->objectName(), getAvailableOutputPorts(element));
    return false;
}

QString BaseHandler::getAvailableInputPorts(GraphicElement *element)
{
    if (!element) {
        return "(element is null)";
    }

    QStringList ports;
    int inputCount = element->inputSize();

    if (inputCount == 0) {
        return "(no input ports)";
    }

    for (int i = 0; i < inputCount; ++i) {
        auto *port = element->inputPort(i);
        if (port) {
            QString portName = port->name().isEmpty() ? "(unnamed)" : port->name();
            ports.append(QString("[%1] %2").arg(i).arg(portName));
        } else {
            ports.append(QString("[%1] (null)").arg(i));
        }
    }

    return ports.join(", ");
}

QString BaseHandler::getAvailableOutputPorts(GraphicElement *element)
{
    if (!element) {
        return "(element is null)";
    }

    QStringList ports;
    int outputCount = element->outputSize();

    if (outputCount == 0) {
        return "(no output ports)";
    }

    for (int i = 0; i < outputCount; ++i) {
        auto *port = element->outputPort(i);
        if (port) {
            QString portName = port->name().isEmpty() ? "(unnamed)" : port->name();
            ports.append(QString("[%1] %2").arg(i).arg(portName));
        } else {
            ports.append(QString("[%1] (null)").arg(i));
        }
    }

    return ports.join(", ");
}

