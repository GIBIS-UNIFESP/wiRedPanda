// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/QuickConnectionHandler.h"

#include <memory>

#include <QJsonArray>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/Scene/ConnectionManager.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

QuickConnectionHandler::QuickConnectionHandler(QuickAppController *appController, const MCPValidator *validator)
    : QuickBaseHandler(appController, validator)
{
}

QJsonObject QuickConnectionHandler::handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId)
{
    if (command == "connect_elements") {
        return handleConnectElements(params, requestId);
    } else if (command == "disconnect_elements") {
        return handleDisconnectElements(params, requestId);
    } else if (command == "list_connections") {
        return handleListConnections(params, requestId);
    } else if (command == "split_connection") {
        return handleSplitConnection(params, requestId);
    } else {
        return createErrorResponse(QString("Unknown connection command: %1").arg(command),
                                   requestId, JsonRpcError::MethodNotFound);
    }
}

QJsonObject QuickConnectionHandler::handleConnectElements(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"source_id", "target_id"})) {
        return createErrorResponse("Missing required parameters: source_id, target_id", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    auto *sourceElement = validatedElement(params, "source_id", errorMsg);
    if (!sourceElement) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }
    auto *targetElement = validatedElement(params, "target_id", errorMsg);
    if (!targetElement) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }

    int sourcePort = -1;
    if (!resolvePort(params, "source", sourceElement, true, sourcePort, errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::PortNotFound);
    }

    int targetPort = -1;
    if (!resolvePort(params, "target", targetElement, false, targetPort, errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::PortNotFound);
    }

    Port *outputPort = sourceElement->outputPort(sourcePort);
    Port *inputPort = targetElement->inputPort(targetPort);

    if (!outputPort || !inputPort) {
        return createErrorResponse("Invalid port specification", requestId, JsonRpcError::PortNotFound);
    }

    auto *startPort = dynamic_cast<OutputPort *>(outputPort);
    auto *endPort = dynamic_cast<InputPort *>(inputPort);

    // Enforce the same rules the UI applies on wire drop (F21): no duplicate
    // connections, no second driver on an occupied input, no wires onto
    // wireless Tx/Rx ports. Without this, MCP could build circuits the UI
    // forbids (the simulator degrades them to Error status).
    if (!ConnectionManager::isConnectionAllowed(startPort, endPort)) {
        return createErrorResponse(QString("Connection from element %1 port %2 to element %3 port %4 is not allowed "
                                           "(duplicate, occupied input, or wireless port)")
                                       .arg(sourceElement->id()).arg(sourcePort).arg(targetElement->id()).arg(targetPort),
                                   requestId, JsonRpcError::ValidationError);
    }

    CanvasItem *canvas = currentCanvas();
    if (!canvas) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    auto connection = std::make_unique<Connection>();
    connection->setStartPort(startPort);
    connection->setEndPort(endPort);
    connection->updatePath();

    try {
        canvas->receiveCommand(new CanvasAddItemsCommand({connection.get()}, canvas));
        connection.release(); // canvas/command takes ownership
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to connect elements: %1").arg(e.what()),
                                   requestId, JsonRpcError::ConnectionFailed);
    } catch (...) {
        return createErrorResponse("Failed to connect elements: Unknown exception",
                                   requestId, JsonRpcError::ConnectionFailed);
    }

    return createSuccessResponse(QJsonObject(), requestId);
}

QJsonObject QuickConnectionHandler::handleDisconnectElements(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"source_id", "target_id"})) {
        return createErrorResponse("Missing required parameters: source_id, target_id", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    auto *sourceElement = validatedElement(params, "source_id", errorMsg);
    if (!sourceElement) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }
    auto *targetElement = validatedElement(params, "target_id", errorMsg);
    if (!targetElement) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }

    CanvasItem *canvas = currentCanvas();
    if (!canvas) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    for (auto *connection : canvas->connections()) {
        Port *port1 = connection->startPort();
        Port *port2 = connection->endPort();

        if (!port1 || !port2) {
            continue;
        }

        GraphicElement *elem1 = port1->graphicElement();
        GraphicElement *elem2 = port2->graphicElement();

        if ((elem1 == sourceElement && elem2 == targetElement) ||
            (elem1 == targetElement && elem2 == sourceElement)) {
            return tryCommand([&] {
                canvas->receiveCommand(new CanvasDeleteItemsCommand({connection}, canvas));
                return createSuccessResponse(QJsonObject(), requestId);
            }, "disconnect elements", requestId);
        }
    }

    return createErrorResponse(QString("No connection found between elements %1 and %2").arg(sourceElement->id()).arg(targetElement->id()),
                               requestId, JsonRpcError::ConnectionFailed);
}

QJsonObject QuickConnectionHandler::handleListConnections(const QJsonObject &, const QJsonValue &requestId)
{
    CanvasItem *canvas = currentCanvas();
    if (!canvas) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    QJsonArray connections;

    for (const auto *connection : canvas->connections()) {
        const Port *startPort = connection->startPort();
        const Port *endPort = connection->endPort();

        if (!startPort || !endPort) {
            continue;
        }

        const GraphicElement *startElement = startPort->graphicElement();
        const GraphicElement *endElement = endPort->graphicElement();

        if (!startElement || !endElement) {
            continue;
        }

        QJsonObject connectionObj;
        connectionObj["source_id"] = startElement->id();
        connectionObj["source_port"] = startPort->index();
        connectionObj["target_id"] = endElement->id();
        connectionObj["target_port"] = endPort->index();

        connections.append(connectionObj);
    }

    QJsonObject result;
    result["connections"] = connections;

    return createSuccessResponse(result, requestId);
}

QJsonObject QuickConnectionHandler::handleSplitConnection(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"source_id", "source_port", "target_id", "target_port", "x", "y"})) {
        return createErrorResponse("Missing required parameters: source_id, source_port, target_id, target_port, x, y",
                                   requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;

    if (!validateNonNegativeInteger(params.value("source_port"), "source_port", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }
    if (!validateNonNegativeInteger(params.value("target_port"), "target_port", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }
    if (!validateNumeric(params.value("x"), "x", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }
    if (!validateNumeric(params.value("y"), "y", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    const int sourcePort = params.value("source_port").toInt();
    const int targetPort = params.value("target_port").toInt();
    const double x = params.value("x").toDouble();
    const double y = params.value("y").toDouble();

    auto *sourceElement = validatedElement(params, "source_id", errorMsg);
    if (!sourceElement) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }
    auto *targetElement = validatedElement(params, "target_id", errorMsg);
    if (!targetElement) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }

    CanvasItem *canvas = currentCanvas();
    if (!canvas) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    // Find the connection between source and target
    Connection *connectionToSplit = nullptr;

    for (auto *connection : canvas->connections()) {
        Port *port1 = connection->startPort();
        Port *port2 = connection->endPort();

        if (!port1 || !port2) {
            continue;
        }

        GraphicElement *elem1 = port1->graphicElement();
        GraphicElement *elem2 = port2->graphicElement();

        if (!elem1 || !elem2) {
            continue;
        }

        // Check if this connection matches source->target
        if (elem1 == sourceElement && elem2 == targetElement &&
            port1->index() == sourcePort && port2->index() == targetPort) {
            connectionToSplit = connection;
            break;
        }
    }

    if (!connectionToSplit) {
        return createErrorResponse("Connection not found between specified source and target",
                                   requestId, JsonRpcError::ConnectionFailed);
    }

    return tryCommand([&] {
        // Create and execute the CanvasSplitCommand
        canvas->receiveCommand(new CanvasSplitCommand(connectionToSplit, QPointF(x, y), canvas));
        return createSuccessResponse(QJsonObject(), requestId);
    }, "split connection", requestId);
}

bool QuickConnectionHandler::resolvePort(const QJsonObject &params, const QString &prefix,
                                         GraphicElement *element, bool isOutput,
                                         int &portIndex, QString &errorMsg)
{
    const QString labelParam = prefix + "_port_label";
    const QString indexParam = prefix + "_port";

    if (!params.contains(labelParam) && !params.contains(indexParam)) {
        errorMsg = QString("Missing %1 port: provide either '%2' (index) or '%3' (name)")
                   .arg(prefix, indexParam, labelParam);
        return false;
    }

    if (params.contains(labelParam)) {
        if (!validateNonEmptyString(params.value(labelParam), labelParam, errorMsg)) {
            return false;
        }
        const QString label = params.value(labelParam).toString();
        return isOutput ? outputPortByLabel(element, label, portIndex, errorMsg)
                        : inputPortByLabel(element, label, portIndex, errorMsg);
    }

    if (!validateNonNegativeInteger(params.value(indexParam), indexParam, errorMsg)) {
        return false;
    }
    portIndex = params.value(indexParam).toInt();
    return validatePortRange(element, portIndex, isOutput, indexParam, errorMsg);
}
