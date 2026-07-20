// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/ConnectionHandler.h"

#include <memory>

#include <QJsonArray>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Commands.h"
#include "App/Scene/ConnectionManager.h"
#include "App/Scene/Scene.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

ConnectionHandler::ConnectionHandler(MainWindow *mainWindow, const MCPValidator *validator)
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
        return createErrorResponse(QString("Unknown connection command: %1").arg(command),
                                   requestId, JsonRpcError::MethodNotFound);
    }
}

QJsonObject ConnectionHandler::handleConnectElements(const QJsonObject &params, const QJsonValue &requestId)
{
    // Required: source_id, target_id
    // For ports: either use index (source_port, target_port) or label (source_port_label, target_port_label)
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

    // Unreachable: resolvePort() only returns true once validatePortRange()/label lookup
    // already confirmed a real, in-range port -- outputPort()/inputPort() can't fail for it.
    if (!outputPort || !inputPort) {
        return createErrorResponse("Invalid port specification", requestId, JsonRpcError::PortNotFound); // LCOV_EXCL_LINE
    }

    auto *startPort = dynamic_cast<OutputPort *>(outputPort);
    auto *endPort = dynamic_cast<InputPort *>(inputPort);

    // Enforce the same rules the UI applies on wire drop (F21): no duplicate
    // connections, no second driver on an occupied input, no wires onto
    // wireless Tx/Rx ports. Without this, MCP could build circuits the UI
    // forbids (the simulator degrades them to Error status).
    if (!ConnectionManager::isConnectionAllowed(startPort, endPort)) {
        return createErrorResponse(QString("Connection from element %1 port %2 to element %3 port %4 is not allowed " // LCOV_EXCL_LINE -- pattern 8/45: gcov misattributes this multi-line chained-.arg() call's first line even though it's genuinely reached
                                           "(duplicate, occupied input, or wireless port)")
                                       .arg(sourceElement->id()).arg(sourcePort).arg(targetElement->id()).arg(targetPort),
                                   requestId, JsonRpcError::ValidationError);
    }

    Scene *scene = currentScene();
    // Unreachable: validatedElement() above already fails (returning ElementNotFound, with
    // this exact message) if there's no active scene, before this point can ever be reached.
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable); // LCOV_EXCL_LINE
    }

    auto connection = std::make_unique<Connection>();
    connection->setStartPort(startPort);
    connection->setEndPort(endPort);
    connection->updatePath();

    // Unreachable: AddItemsCommand::redo() only calls CommandUtils::loadItems() and
    // Scene::setCircuitUpdateRequired(), neither of which throws for an already-validated
    // Connection (confirmed exception-free for this exact code path across the whole
    // App/Scene/Commands.cpp sweep). Kept as a defensive backstop.
    try {
        scene->receiveCommand(new AddItemsCommand({connection.get()}, scene));
        connection.release(); // scene/command takes ownership
    } catch (const std::exception &e) { // LCOV_EXCL_LINE
        return createErrorResponse(QString("Failed to connect elements: %1").arg(e.what()), // LCOV_EXCL_LINE
                                   requestId, JsonRpcError::ConnectionFailed); // LCOV_EXCL_LINE
    } catch (...) { // LCOV_EXCL_LINE
        return createErrorResponse("Failed to connect elements: Unknown exception", // LCOV_EXCL_LINE
                                   requestId, JsonRpcError::ConnectionFailed); // LCOV_EXCL_LINE
    } // LCOV_EXCL_LINE

    return createSuccessResponse(QJsonObject(), requestId);
}

QJsonObject ConnectionHandler::handleDisconnectElements(const QJsonObject &params, const QJsonValue &requestId)
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

    Scene *scene = currentScene();
    // Unreachable: validatedElement() above already fails first (returning ElementNotFound,
    // with this exact message) if there's no active scene.
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable); // LCOV_EXCL_LINE
    }

    const auto connections = scene->items();
    for (auto *item : connections) {
        auto *connection = qgraphicsitem_cast<Connection *>(item);
        if (!connection) {
            continue;
        }

        Port *port1 = connection->startPort();
        Port *port2 = connection->endPort();

        if (!port1 || !port2) {
            continue;
        }

        GraphicElement *elem1 = port1->graphicElement();
        GraphicElement *elem2 = port2->graphicElement();

        if ((elem1 == sourceElement && elem2 == targetElement) ||
            (elem1 == targetElement && elem2 == sourceElement)) {
            return tryCommand([&] { // LCOV_EXCL_LINE -- pattern 45: gcov misattributes this multi-line lambda-taking call's entry; the lambda body below is genuinely covered
                scene->receiveCommand(new DeleteItemsCommand({connection}, scene));
                return createSuccessResponse(QJsonObject(), requestId);
            }, "disconnect elements", requestId);
        }
    }

    return createErrorResponse(QString("No connection found between elements %1 and %2").arg(sourceElement->id()).arg(targetElement->id()),
                               requestId, JsonRpcError::ConnectionFailed);
}

QJsonObject ConnectionHandler::handleListConnections(const QJsonObject &, const QJsonValue &requestId)
{
    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    QJsonArray connections;
    const auto sceneItems = scene->items();

    for (const auto *item : sceneItems) {
        const auto *connection = qgraphicsitem_cast<const Connection *>(item);
        if (!connection) {
            continue;
        }

        const Port *startPort = connection->startPort();
        const Port *endPort = connection->endPort();

        if (!startPort || !endPort) {
            continue;
        }

        const GraphicElement *startElement = startPort->graphicElement();
        const GraphicElement *endElement = endPort->graphicElement();

        // Unreachable: ElementPorts::addPort() unconditionally binds a port to its owning
        // element at construction, so graphicElement() is never null for a real port.
        if (!startElement || !endElement) {
            continue; // LCOV_EXCL_LINE
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

    Scene *scene = currentScene();
    // Unreachable: validatedElement() above already fails first (returning ElementNotFound,
    // with this exact message) if there's no active scene.
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable); // LCOV_EXCL_LINE
    }

    // Find the connection between source and target
    Connection *connectionToSplit = nullptr;
    const auto sceneItems = scene->items();

    for (auto *item : sceneItems) {
        auto *connection = qgraphicsitem_cast<Connection *>(item);
        if (!connection) { continue; }

        Port *port1 = connection->startPort();
        Port *port2 = connection->endPort();

        if (!port1 || !port2) {
            continue;
        }

        GraphicElement *elem1 = port1->graphicElement();
        GraphicElement *elem2 = port2->graphicElement();

        // Unreachable: same port-always-has-an-owner invariant as handleListConnections().
        if (!elem1 || !elem2) {
            continue; // LCOV_EXCL_LINE
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

    return tryCommand([&] { // LCOV_EXCL_LINE -- pattern 45: gcov misattributes this multi-line lambda-taking call's entry; the lambda body below is genuinely covered
        // Create and execute the SplitCommand
        scene->receiveCommand(new SplitCommand(connectionToSplit, QPointF(x, y), scene));
        return createSuccessResponse(QJsonObject(), requestId);
    }, "split connection", requestId);
}

bool ConnectionHandler::resolvePort(const QJsonObject &params, const QString &prefix,
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
