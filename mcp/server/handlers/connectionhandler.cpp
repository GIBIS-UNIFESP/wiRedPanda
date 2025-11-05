// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "connectionhandler.h"

#include "commands.h"
#include "elementfactory.h"
#include "graphicelement.h"
#include "qneconnection.h"
#include "qneport.h"
#include "scene.h"

#include <QJsonArray>

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
    if (!validateParameters(params, {"source_id", "source_port", "target_id", "target_port"})) {
        return createErrorResponse("Missing required parameters: source_id, source_port, target_id, target_port", requestId);
    }

    QString errorMsg;

    if (!validatePositiveInteger(params.value("source_id"), "source_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validatePositiveInteger(params.value("target_id"), "target_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validateNonNegativeInteger(params.value("source_port"), "source_port", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validateNonNegativeInteger(params.value("target_port"), "target_port", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    int sourceId = params.value("source_id").toInt();
    int sourcePort = params.value("source_port").toInt();
    int targetId = params.value("target_id").toInt();
    int targetPort = params.value("target_port").toInt();

    if (!validateElementId(sourceId, "source_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validateElementId(targetId, "target_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    auto *sourceItem = ElementFactory::itemById(sourceId);
    auto *targetItem = ElementFactory::itemById(targetId);

    auto *sourceElement = dynamic_cast<GraphicElement*>(sourceItem);
    auto *targetElement = dynamic_cast<GraphicElement*>(targetItem);

    if (!sourceElement || !targetElement) {
        return createErrorResponse("Source or target is not a graphic element", requestId);
    }

    if (!validatePortRange(sourceElement, sourcePort, true, "source_port", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validatePortRange(targetElement, targetPort, false, "target_port", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
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
    connection->setStartPort(dynamic_cast<QNEOutputPort*>(outputPort));
    connection->setEndPort(dynamic_cast<QNEInputPort*>(inputPort));
    connection->updatePath();

    scene->receiveCommand(new AddItemsCommand({connection}, scene));

    return createSuccessResponse(QJsonObject(), requestId);
}

QJsonObject ConnectionHandler::handleDisconnectElements(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"source_id", "target_id"})) {
        return createErrorResponse("Missing required parameters: source_id, target_id", requestId);
    }

    int sourceId = params.value("source_id").toInt();
    int targetId = params.value("target_id").toInt();

    auto *sourceItem = ElementFactory::itemById(sourceId);
    auto *targetItem = ElementFactory::itemById(targetId);

    if (!sourceItem || !targetItem) {
        return createErrorResponse("Source or target element not found", requestId);
    }

    auto *sourceElement = dynamic_cast<GraphicElement*>(sourceItem);
    auto *targetElement = dynamic_cast<GraphicElement*>(targetItem);

    if (!sourceElement || !targetElement) {
        return createErrorResponse("Source or target is not a graphic element", requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    const auto connections = scene->items();
    for (auto *item : connections) {
        auto *connection = qgraphicsitem_cast<QNEConnection*>(item);
        if (!connection) continue;

        QNEPort *port1 = connection->startPort();
        QNEPort *port2 = connection->endPort();

        if (!port1 || !port2) continue;

        GraphicElement *elem1 = port1->graphicElement();
        GraphicElement *elem2 = port2->graphicElement();

        if ((elem1 == sourceElement && elem2 == targetElement) ||
            (elem1 == targetElement && elem2 == sourceElement)) {
            scene->receiveCommand(new DeleteItemsCommand({connection}, scene));
            break;
        }
    }

    return createSuccessResponse(QJsonObject(), requestId);
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
        const auto *connection = qgraphicsitem_cast<const QNEConnection*>(item);
        if (!connection) continue;

        const QNEPort *startPort = connection->startPort();
        const QNEPort *endPort = connection->endPort();

        if (!startPort || !endPort) continue;

        const GraphicElement *startElement = startPort->graphicElement();
        const GraphicElement *endElement = endPort->graphicElement();

        if (!startElement || !endElement) continue;

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

    if (!validatePositiveInteger(params.value("source_id"), "source_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validatePositiveInteger(params.value("target_id"), "target_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    int sourceId = params.value("source_id").toInt();
    int targetId = params.value("target_id").toInt();
    int sourcePort = params.value("source_port").toInt();
    int targetPort = params.value("target_port").toInt();
    double x = params.value("x").toDouble();
    double y = params.value("y").toDouble();

    if (!validateElementId(sourceId, "source_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validateElementId(targetId, "target_id", errorMsg)) {
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
        auto *connection = qgraphicsitem_cast<QNEConnection*>(item);
        if (!connection) continue;

        QNEPort *port1 = connection->startPort();
        QNEPort *port2 = connection->endPort();

        if (!port1 || !port2) continue;

        GraphicElement *elem1 = port1->graphicElement();
        GraphicElement *elem2 = port2->graphicElement();

        if (!elem1 || !elem2) continue;

        // Check if this connection matches source->target
        if (elem1->id() == sourceId && elem2->id() == targetId &&
            port1->index() == sourcePort && port2->index() == targetPort) {
            connectionToSplit = connection;
            break;
        }
    }

    if (!connectionToSplit) {
        return createErrorResponse("Connection not found between specified source and target", requestId);
    }

    try {
        // Create and execute the SplitCommand
        scene->receiveCommand(new SplitCommand(connectionToSplit, QPointF(x, y), scene));
        return createSuccessResponse(QJsonObject(), requestId);
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Split operation failed: %1").arg(e.what()), requestId);
    }
}
