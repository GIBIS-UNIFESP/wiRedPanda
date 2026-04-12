// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/ConnectionHandler.h"

#include <QJsonArray>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"

ConnectionHandler::ConnectionHandler(MainWindow *mainWindow, MCPValidator *validator)
    : BaseHandler(mainWindow, validator)
{
}

QJsonObject ConnectionHandler::handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId)
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
        return createErrorResponse(QString("Unknown connection command: %1").arg(command), requestId);
    }
}

QJsonObject ConnectionHandler::handleConnectElements(const QJsonObject &params, const QJsonValue &requestId)
{
    // Required: source_id, target_id
    // For ports: either use index (source_port, target_port) or label (source_port_label, target_port_label)
    if (!validateParameters(params, {"source_id", "target_id"})) {
        return createErrorResponse("Missing required parameters: source_id, target_id", requestId);
    }

    // Check that we have at least one way to specify each port
    bool hasSourcePort = params.contains("source_port");
    bool hasSourceLabel = params.contains("source_port_label");
    bool hasTargetPort = params.contains("target_port");
    bool hasTargetLabel = params.contains("target_port_label");

    if (!hasSourcePort && !hasSourceLabel) {
        return createErrorResponse("Missing source port: provide either 'source_port' (index) or 'source_port_label' (name)", requestId);
    }
    if (!hasTargetPort && !hasTargetLabel) {
        return createErrorResponse("Missing target port: provide either 'target_port' (index) or 'target_port_label' (name)", requestId);
    }

    QString errorMsg;
    auto *sourceElement = getValidatedElement(params, "source_id", errorMsg);
    if (!sourceElement) {
        return createErrorResponse(errorMsg, requestId);
    }
    auto *targetElement = getValidatedElement(params, "target_id", errorMsg);
    if (!targetElement) {
        return createErrorResponse(errorMsg, requestId);
    }

    // Resolve source port (prefer label if provided)
    int sourcePort = -1;
    if (hasSourceLabel) {
        if (!validateNonEmptyString(params.value("source_port_label"), "source_port_label", errorMsg)) {
            return createErrorResponse(errorMsg, requestId);
        }
        QString label = params.value("source_port_label").toString();
        if (!getOutputPortByLabel(sourceElement, label, sourcePort, errorMsg)) {
            return createErrorResponse(errorMsg, requestId);
        }
    } else {
        if (!validateNonNegativeInteger(params.value("source_port"), "source_port", errorMsg)) {
            return createErrorResponse(errorMsg, requestId);
        }
        sourcePort = params.value("source_port").toInt();
        if (!validatePortRange(sourceElement, sourcePort, true, "source_port", errorMsg)) {
            return createErrorResponse(errorMsg, requestId);
        }
    }

    // Resolve target port (prefer label if provided)
    int targetPort = -1;
    if (hasTargetLabel) {
        if (!validateNonEmptyString(params.value("target_port_label"), "target_port_label", errorMsg)) {
            return createErrorResponse(errorMsg, requestId);
        }
        QString label = params.value("target_port_label").toString();
        if (!getInputPortByLabel(targetElement, label, targetPort, errorMsg)) {
            return createErrorResponse(errorMsg, requestId);
        }
    } else {
        if (!validateNonNegativeInteger(params.value("target_port"), "target_port", errorMsg)) {
            return createErrorResponse(errorMsg, requestId);
        }
        targetPort = params.value("target_port").toInt();
        if (!validatePortRange(targetElement, targetPort, false, "target_port", errorMsg)) {
            return createErrorResponse(errorMsg, requestId);
        }
    }

    QNEPort *outputPort = sourceElement->outputPort(sourcePort);
    QNEPort *inputPort = targetElement->inputPort(targetPort);

    if (!outputPort || !inputPort) {
        return createErrorResponse("Invalid port specification", requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    auto *connection = new QNEConnection();
    connection->setStartPort(dynamic_cast<QNEOutputPort *>(outputPort));
    connection->setEndPort(dynamic_cast<QNEInputPort *>(inputPort));
    connection->updatePath();

    try {
        scene->receiveCommand(new AddItemsCommand({connection}, scene));
    } catch (const std::exception &e) {
        delete connection;
        return createErrorResponse(QString("Failed to connect elements: %1").arg(e.what()), requestId);
    } catch (...) {
        delete connection;
        return createErrorResponse("Failed to connect elements: Unknown exception", requestId);
    }

    return createSuccessResponse(QJsonObject(), requestId);
}

QJsonObject ConnectionHandler::handleDisconnectElements(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"source_id", "target_id"})) {
        return createErrorResponse("Missing required parameters: source_id, target_id", requestId);
    }

    QString errorMsg;
    auto *sourceElement = getValidatedElement(params, "source_id", errorMsg);
    if (!sourceElement) {
        return createErrorResponse(errorMsg, requestId);
    }
    auto *targetElement = getValidatedElement(params, "target_id", errorMsg);
    if (!targetElement) {
        return createErrorResponse(errorMsg, requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    const auto connections = scene->items();
    for (auto *item : connections) {
        auto *connection = qgraphicsitem_cast<QNEConnection *>(item);
        if (!connection) {
            continue;
        }

        QNEPort *port1 = connection->startPort();
        QNEPort *port2 = connection->endPort();

        if (!port1 || !port2) {
            continue;
        }

        GraphicElement *elem1 = port1->graphicElement();
        GraphicElement *elem2 = port2->graphicElement();

        if ((elem1 == sourceElement && elem2 == targetElement) ||
            (elem1 == targetElement && elem2 == sourceElement)) {
            return tryCommand([&] {
                scene->receiveCommand(new DeleteItemsCommand({connection}, scene));
                return createSuccessResponse(QJsonObject(), requestId);
            }, "disconnect elements", requestId);
        }
    }

    return createErrorResponse(QString("No connection found between elements %1 and %2").arg(sourceElement->id()).arg(targetElement->id()), requestId);
}

QJsonObject ConnectionHandler::handleListConnections(const QJsonObject &, const QJsonValue &requestId)
{
    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    QJsonArray connections;
    const auto sceneItems = scene->items();

    for (const auto *item : sceneItems) {
        const auto *connection = qgraphicsitem_cast<const QNEConnection *>(item);
        if (!connection) {
            continue;
        }

        const QNEPort *startPort = connection->startPort();
        const QNEPort *endPort = connection->endPort();

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

QJsonObject ConnectionHandler::handleSplitConnection(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"source_id", "source_port", "target_id", "target_port", "x", "y"})) {
        return createErrorResponse("Missing required parameters: source_id, source_port, target_id, target_port, x, y", requestId);
    }

    QString errorMsg;

    if (!validateNonNegativeInteger(params.value("source_port"), "source_port", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }
    if (!validateNonNegativeInteger(params.value("target_port"), "target_port", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }
    if (!validateNumeric(params.value("x"), "x", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }
    if (!validateNumeric(params.value("y"), "y", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    const int sourcePort = params.value("source_port").toInt();
    const int targetPort = params.value("target_port").toInt();
    const double x = params.value("x").toDouble();
    const double y = params.value("y").toDouble();

    auto *sourceElement = getValidatedElement(params, "source_id", errorMsg);
    if (!sourceElement) {
        return createErrorResponse(errorMsg, requestId);
    }
    auto *targetElement = getValidatedElement(params, "target_id", errorMsg);
    if (!targetElement) {
        return createErrorResponse(errorMsg, requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    // Find the connection between source and target
    QNEConnection *connectionToSplit = nullptr;
    const auto sceneItems = scene->items();

    for (auto *item : sceneItems) {
        auto *connection = qgraphicsitem_cast<QNEConnection *>(item);
        if (!connection) { continue; }

        QNEPort *port1 = connection->startPort();
        QNEPort *port2 = connection->endPort();

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
        return createErrorResponse("Connection not found between specified source and target", requestId);
    }

    return tryCommand([&] {
        // Create and execute the SplitCommand
        scene->receiveCommand(new SplitCommand(connectionToSplit, QPointF(x, y), scene));
        return createSuccessResponse(QJsonObject(), requestId);
    }, "split connection", requestId);
}

