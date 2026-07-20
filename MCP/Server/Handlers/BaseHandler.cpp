// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/BaseHandler.h"

#include <cmath>

#include <QJsonObject>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MainWindow.h"
#include "App/Wiring/Port.h"

BaseHandler::BaseHandler(MainWindow *mainWindow, const MCPValidator *validator)
    : m_mainWindow(mainWindow)
    , m_validator(validator)
{
}

QJsonObject BaseHandler::createSuccessResponse(const QJsonObject &result, const QJsonValue &requestId) const
{
    QJsonObject response;
    response["jsonrpc"] = "2.0";
    response["result"] = result;
    if (!requestId.isNull()) {
        response["id"] = requestId;
    }
    return response;
} // LCOV_EXCL_LINE -- compiler-generated cleanup for the returned QJsonObject (pattern 1)

QJsonObject BaseHandler::createErrorResponse(const QString &error, const QJsonValue &requestId, int code) const
{
    QJsonObject response;
    response["jsonrpc"] = "2.0";

    QJsonObject errorObject;
    errorObject["code"] = code;
    errorObject["message"] = error;
    response["error"] = errorObject;

    if (!requestId.isNull()) {
        response["id"] = requestId;
    }
    return response;
}

bool BaseHandler::validateParameters(const QJsonObject &params, const QStringList &required) const
{
    for (const QString &param : required) {
        if (!params.contains(param)) {
            return false;
        }
    }
    return true;
}

Scene *BaseHandler::currentScene()
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

const Scene *BaseHandler::currentScene() const
{
    if (!m_mainWindow) {
        return nullptr;
    }

    const WorkSpace *workspace = m_mainWindow->currentTab();
    if (!workspace) {
        return nullptr;
    }

    return workspace->scene();
}

bool BaseHandler::validatePositiveInteger(const QJsonValue &value, const QString &paramName, QString &errorMsg) const
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

bool BaseHandler::validateNonNegativeInteger(const QJsonValue &value, const QString &paramName, QString &errorMsg) const
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

bool BaseHandler::validateNonEmptyString(const QJsonValue &value, const QString &paramName, QString &errorMsg) const
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

bool BaseHandler::validateElementId(int elementId, const QString &paramName, QString &errorMsg) const
{
    if (elementId <= 0) {
        errorMsg = QString("Parameter '%1' must be a positive integer (got %2)").arg(paramName).arg(elementId);
        return false;
    }

    const Scene *scene = currentScene();
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

bool BaseHandler::validateNumeric(const QJsonValue &value, const QString &paramName, QString &errorMsg) const
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

bool BaseHandler::validatePortRange(GraphicElement *element, int portIndex, bool isOutput, const QString &paramName, QString &errorMsg) const
{
    if (!element) {
        errorMsg = QString("Invalid element for port validation");
        return false;
    }

    // Use the safe QVector size() instead of calling outputPort()/inputPort()
    qsizetype maxPorts = 0;
    if (isOutput) {
        maxPorts = element->outputs().size();
    } else {
        maxPorts = element->inputs().size();
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

GraphicElement *BaseHandler::validatedElement(const QJsonObject &params, const QString &paramName, QString &errorMsg)
{
    if (!validatePositiveInteger(params.value(paramName), paramName, errorMsg)) {
        return nullptr;
    }
    const int elementId = params.value(paramName).toInt();
    if (!validateElementId(elementId, paramName, errorMsg)) {
        return nullptr;
    }
    auto *item = currentScene()->itemById(elementId);
    auto *element = dynamic_cast<GraphicElement *>(item);
    if (!element) {
        errorMsg = QString("Item %1 is not a graphic element").arg(elementId);
        return nullptr;
    }
    return element;
}

bool BaseHandler::inputPortByLabel(GraphicElement *element, const QString &label, int &portIndex, QString &errorMsg) const
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
               .arg(label, element->objectName(), availableInputPorts(element));
    return false;
}

bool BaseHandler::outputPortByLabel(GraphicElement *element, const QString &label, int &portIndex, QString &errorMsg) const
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
               .arg(label, element->objectName(), availableOutputPorts(element));
    return false;
}

QString BaseHandler::availablePorts(GraphicElement *element, bool isOutput) const
{
    if (!element) {
        return "(element is null)";
    }

    const int count = isOutput ? element->outputSize() : element->inputSize();

    if (count == 0) {
        return isOutput ? "(no output ports)" : "(no input ports)";
    }

    QStringList ports;
    for (int i = 0; i < count; ++i) {
        auto *port = isOutput ? static_cast<Port *>(element->outputPort(i))
                              : static_cast<Port *>(element->inputPort(i));
        if (port) {
            QString portName = port->name().isEmpty() ? "(unnamed)" : port->name();
            ports.append(QString("[%1] %2").arg(i).arg(portName));
        } else {
            // Unreachable: ElementPorts::inputPort()/outputPort() only return null for an
            // out-of-range index, but this loop's own bound (count) is that same size, so i
            // is always in range -- every index up to count has a real, non-null port.
            ports.append(QString("[%1] (null)").arg(i)); // LCOV_EXCL_LINE
        }
    }

    return ports.join(", ");
}

QString BaseHandler::availableInputPorts(GraphicElement *element) const
{
    return availablePorts(element, false);
}

QString BaseHandler::availableOutputPorts(GraphicElement *element) const
{
    return availablePorts(element, true);
}
