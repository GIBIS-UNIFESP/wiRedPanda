// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/ElementHandler.h"

#include <QIODevice>
#include <QJsonArray>

#include "App/Core/Common.h"
#include "App/Core/Constants.h"
#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

ElementHandler::ElementHandler(MainWindow *mainWindow, const MCPValidator *validator)
    : BaseHandler(mainWindow, validator)
{
}

QJsonObject ElementHandler::handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId)
{
    if (command == "create_element") {
        return handleCreateElement(params, requestId);
    } else if (command == "delete_element") {
        return handleDeleteElement(params, requestId);
    } else if (command == "list_elements") {
        return handleListElements(params, requestId);
    } else if (command == "move_element") {
        return handleMoveElement(params, requestId);
    } else if (command == "set_element_properties") {
        return handleSetElementProperties(params, requestId);
    } else if (command == "set_input_value") {
        return handleSetInputValue(params, requestId);
    } else if (command == "get_output_value") {
        return handleGetOutputValue(params, requestId);
    } else if (command == "rotate_element") {
        return handleRotateElement(params, requestId);
    } else if (command == "flip_element") {
        return handleFlipElement(params, requestId);
    } else if (command == "update_element") {
        return handleUpdateElement(params, requestId);
    } else if (command == "change_input_size") {
        return handleChangeInputSize(params, requestId);
    } else if (command == "change_output_size") {
        return handleChangeOutputSize(params, requestId);
    } else if (command == "toggle_truth_table_output") {
        return handleToggleTruthTableOutput(params, requestId);
    } else if (command == "morph_element") {
        return handleMorphElement(params, requestId);
    } else {
        return createErrorResponse(QString("Unknown element command: %1").arg(command),
                                   requestId, JsonRpcError::MethodNotFound);
    }
}

QJsonObject ElementHandler::handleCreateElement(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"type", "x", "y"})) {
        return createErrorResponse("Missing required parameters: type, x, y", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("type"), "type", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    if (!validateNumeric(params.value("x"), "x", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    if (!validateNumeric(params.value("y"), "y", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    QString typeStr = params.value("type").toString();
    double x = params.value("x").toDouble();
    double y = params.value("y").toDouble();
    QString label = params.value("label").toString();

    ElementType type = ElementFactory::textToType(typeStr);
    if (type == ElementType::Unknown) {
        return createErrorResponse(QString("Invalid element type: %1").arg(typeStr),
                                   requestId, JsonRpcError::ValidationError);
    }

    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    GraphicElement *element = nullptr;
    try {
        element = ElementFactory::buildElement(type);
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to create element of type %1: %2").arg(typeStr, e.what()),
                                   requestId, JsonRpcError::OperationFailed);
    } catch (...) {
        return createErrorResponse(QString("Failed to create element of type: %1").arg(typeStr),
                                   requestId, JsonRpcError::OperationFailed);
    }

    if (!element) {
        return createErrorResponse(QString("Failed to create element of type: %1").arg(typeStr),
                                   requestId, JsonRpcError::OperationFailed);
    }

    element->setPos(x, y);
    if (!label.isEmpty()) {
        element->setLabel(label);
    }

    try {
        scene->receiveCommand(new AddItemsCommand({element}, scene));
    } catch (const std::exception &e) {
        delete element;
        return createErrorResponse(QString("Failed to add element to scene: %1").arg(e.what()),
                                   requestId, JsonRpcError::OperationFailed);
    } catch (...) {
        delete element;
        return createErrorResponse("Failed to add element to scene: Unknown exception",
                                   requestId, JsonRpcError::OperationFailed);
    }

    const QRectF bounds = element->boundingRect();

    QJsonObject result;
    result["element_id"] = element->id();
    result["width"] = bounds.width();
    result["height"] = bounds.height();

    return createSuccessResponse(result, requestId);
}

QJsonObject ElementHandler::handleDeleteElement(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id"})) {
        return createErrorResponse("Missing required parameter: element_id", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    auto *element = validatedElement(params, "element_id", errorMsg);
    if (!element) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }

    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    return tryCommand([&] {
        scene->receiveCommand(new DeleteItemsCommand({element}, scene));
        return createSuccessResponse(QJsonObject(), requestId);
    }, "delete element", requestId);
}

QJsonObject ElementHandler::handleListElements(const QJsonObject &, const QJsonValue &requestId)
{
    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    QJsonArray elements;
    const auto sceneElements = scene->elements();

    for (const auto *element : sceneElements) {
        const QRectF br = element->boundingRect();
        QJsonObject elementObj;
        elementObj["element_id"] = element->id();
        elementObj["type"] = ElementFactory::typeToText(element->elementType());
        elementObj["x"] = element->pos().x();
        elementObj["y"] = element->pos().y();
        elementObj["width"] = br.width();
        elementObj["height"] = br.height();
        elementObj["label"] = element->label();
        elementObj["rotation"] = element->rotation();
        elementObj["input_size"] = element->inputSize();
        elementObj["output_size"] = element->outputSize();

        if (element->hasColors()) {
            elementObj["color"] = element->color();
        }
        if (element->hasFrequency()) {
            elementObj["frequency"] = element->frequency();
        }
        if (element->hasDelay()) {
            elementObj["delay"] = element->delay();
        }
        if (element->hasTrigger()) {
            elementObj["trigger"] = element->trigger().toString();
        }
        if (element->hasAudio()) {
            elementObj["audio"] = element->audio();
        }
        if (element->hasVolume()) {
            elementObj["volume"] = static_cast<double>(element->volume());
        }
        if (const auto *inputElm = qobject_cast<const GraphicElementInput *>(element)) {
            elementObj["locked"] = inputElm->isLocked();
        }

        elements.append(elementObj);
    }

    QJsonObject result;
    result["elements"] = elements;

    return createSuccessResponse(result, requestId);
}

QJsonObject ElementHandler::handleMoveElement(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id", "x", "y"})) {
        return createErrorResponse("Missing required parameters: element_id, x, y", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNumeric(params.value("x"), "x", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }
    if (!validateNumeric(params.value("y"), "y", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    const int snap = Constants::gridSize / 2;
    const int x = qRound(params.value("x").toDouble() / snap) * snap;
    const int y = qRound(params.value("y").toDouble() / snap) * snap;

    auto *element = validatedElement(params, "element_id", errorMsg);
    if (!element) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }

    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    QPointF oldPos = element->pos();
    QPointF newPos(x, y);

    element->setPos(newPos);

    QList<GraphicElement *> elements = {element};
    QList<QPointF> oldPositions = {oldPos};

    try {
        scene->receiveCommand(new MoveCommand(elements, oldPositions, scene));
    } catch (const std::exception &e) {
        element->setPos(oldPos);
        return createErrorResponse(QString("Failed to move element: %1").arg(e.what()),
                                   requestId, JsonRpcError::OperationFailed);
    } catch (...) {
        element->setPos(oldPos);
        return createErrorResponse("Failed to move element: Unknown exception",
                                   requestId, JsonRpcError::OperationFailed);
    }

    QJsonObject result;
    result["old_position"] = QJsonObject{{"x", oldPos.x()}, {"y", oldPos.y()}};
    result["new_position"] = QJsonObject{{"x", newPos.x()}, {"y", newPos.y()}};

    return createSuccessResponse(result, requestId);
}

QJsonObject ElementHandler::handleSetElementProperties(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id"})) {
        return createErrorResponse("Missing required parameter: element_id", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    auto *element = validatedElement(params, "element_id", errorMsg);
    if (!element) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }

    // Reject structural changes before any mutation — these require dedicated commands.
    if (params.contains("input_size") || params.contains("output_size")) {
        return createErrorResponse("Use change_input_size / change_output_size commands to change port counts",
                                   requestId, JsonRpcError::ValidationError);
    }

    // Snapshot current state before any mutation so UpdateCommand can restore it on undo.
    QByteArray oldData;
    {
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        element->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }

    QJsonObject result;
    QJsonObject oldProperties;
    QJsonObject newProperties;

    if (params.contains("label")) {
        QString oldLabel = element->label();
        QString newLabel = params.value("label").toString();

        oldProperties["label"] = oldLabel;
        newProperties["label"] = newLabel;

        element->setLabel(newLabel);
    }

    if (params.contains("color") && element->hasColors()) {
        QString oldColor = element->color();
        QString newColor = params.value("color").toString();

        oldProperties["color"] = oldColor;
        newProperties["color"] = newColor;

        element->setColor(newColor);
    }

    if (params.contains("frequency") && element->hasFrequency()) {
        if (!validateNumeric(params.value("frequency"), "frequency", errorMsg)) {
            return createErrorResponse(errorMsg, requestId, JsonRpcError::ValidationError);
        }
        double oldFreq = element->frequency();
        double newFreq = params.value("frequency").toDouble();
        if (newFreq <= 0) {
            return createErrorResponse("Parameter 'frequency' must be a positive value", requestId, JsonRpcError::ValidationError);
        }

        oldProperties["frequency"] = oldFreq;
        newProperties["frequency"] = newFreq;

        element->setFrequency(newFreq);
    }

    if (params.contains("rotation")) {
        qreal oldRotation = element->rotation();
        qreal newRotation = params.value("rotation").toDouble();

        oldProperties["rotation"] = oldRotation;
        newProperties["rotation"] = newRotation;

        element->setRotation(newRotation);
    }

    if (params.contains("delay") && element->hasDelay()) {
        if (!validateNumeric(params.value("delay"), "delay", errorMsg)) {
            return createErrorResponse(errorMsg, requestId, JsonRpcError::ValidationError);
        }
        double oldDelay = element->delay();
        double newDelay = params.value("delay").toDouble();
        if (newDelay < 0) {
            return createErrorResponse("Parameter 'delay' must be non-negative", requestId, JsonRpcError::ValidationError);
        }

        oldProperties["delay"] = oldDelay;
        newProperties["delay"] = newDelay;

        element->setDelay(newDelay);
    }

    if (params.contains("trigger") && element->hasTrigger()) {
        QString oldTrigger = element->trigger().toString();
        QString newTrigger = params.value("trigger").toString();

        oldProperties["trigger"] = oldTrigger;
        newProperties["trigger"] = newTrigger;

        element->setTrigger(QKeySequence(newTrigger));
    }

    if (params.contains("audio") && element->hasAudio()) {
        QString oldAudio = element->audio();
        QString newAudio = params.value("audio").toString();

        oldProperties["audio"] = oldAudio;
        newProperties["audio"] = newAudio;

        element->setAudio(newAudio);
    }

    if (params.contains("locked")) {
        auto *inputElm = qobject_cast<GraphicElementInput *>(element);
        if (inputElm) {
            bool oldLocked = inputElm->isLocked();
            bool newLocked = params.value("locked").toBool();

            oldProperties["locked"] = oldLocked;
            newProperties["locked"] = newLocked;

            inputElm->setLocked(newLocked);
        }
    }

    if (params.contains("volume") && element->hasVolume()) {
        if (!validateNumeric(params.value("volume"), "volume", errorMsg)) {
            return createErrorResponse(errorMsg, requestId, JsonRpcError::ValidationError);
        }
        float oldVolume = element->volume();
        float newVolume = static_cast<float>(params.value("volume").toDouble());
        if (newVolume < 0.0f || newVolume > 1.0f) {
            return createErrorResponse("Parameter 'volume' must be between 0.0 and 1.0", requestId, JsonRpcError::ValidationError);
        }

        oldProperties["volume"] = static_cast<double>(oldVolume);
        newProperties["volume"] = static_cast<double>(newVolume);

        element->setVolume(newVolume);
    }

    if (params.contains("appearance") && element->canChangeAppearance()) {
        QString appearancePath = params.value("appearance").toString();
        bool useDefault = appearancePath.isEmpty();
        int appearanceIndex = params.contains("appearance_index") ? params.value("appearance_index").toInt() : -1;

        newProperties["appearance"] = appearancePath;
        newProperties["appearance_default"] = useDefault;

        if (appearanceIndex >= 0) {
            // Set appearance for a specific index directly (e.g., LED state)
            element->setAppearanceAt(appearanceIndex, appearancePath);
            newProperties["appearance_index"] = appearanceIndex;
        } else {
            element->setAppearance(useDefault, appearancePath);
        }
    }

    // Wireless mode is only meaningful for Node elements; non-Nodes are silently ignored.
    QList<QGraphicsItem *> wirelessConnsToDelete;
    if (params.contains("wireless_mode")) {
        if (auto *node = qobject_cast<Node *>(element)) {
            int modeInt = params.value("wireless_mode").toInt();
            if (modeInt < 0 || modeInt > 2) {
                return createErrorResponse("Invalid wireless_mode. Must be 0 (None), 1 (Tx), or 2 (Rx)",
                                           requestId, JsonRpcError::ValidationError);
            }
            auto oldMode = static_cast<int>(node->wirelessMode());
            auto newMode = static_cast<WirelessMode>(modeInt);

            oldProperties["wireless_mode"] = oldMode;
            newProperties["wireless_mode"] = modeInt;

            // Collect connections on the port that will be hidden, so they can
            // be deleted in an undo macro (same pattern as ElementEditor::apply).
            Port *port = (newMode == WirelessMode::Rx) ? static_cast<Port *>(node->inputPort())
                          : (newMode == WirelessMode::Tx) ? static_cast<Port *>(node->outputPort())
                          : nullptr;
            if (port) {
                for (auto *conn : port->connections()) {
                    wirelessConnsToDelete.append(static_cast<QGraphicsItem *>(conn));
                }
            }

            node->setWirelessMode(newMode);
        }
    }

    // Push an undo command so Ctrl+Z in the GUI can revert MCP-applied property changes.
    // When wireless mode changes sever connections, group the property update and the
    // delete into a single undo macro so both are undone together.
    if (!newProperties.isEmpty()) {
        const bool needsMacro = !wirelessConnsToDelete.isEmpty();
        auto *scene = currentScene();
        if (scene) {
            if (needsMacro) {
                scene->undoStack()->beginMacro(QStringLiteral("Change wireless mode"));
            }
            scene->receiveCommand(new UpdateCommand({element}, oldData, scene));
            if (needsMacro) {
                scene->receiveCommand(new DeleteItemsCommand(wirelessConnsToDelete, scene));
                scene->undoStack()->endMacro();
            }
        }
    }

    result["old_properties"] = oldProperties;
    result["new_properties"] = newProperties;

    return createSuccessResponse(result, requestId);
}

QJsonObject ElementHandler::handleSetInputValue(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id", "value"})) {
        return createErrorResponse("Missing required parameters: element_id, value", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    auto *element = validatedElement(params, "element_id", errorMsg);
    if (!element) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }
    const bool value = params.value("value").toBool();

    auto *inputElement = dynamic_cast<GraphicElementInput *>(element);
    if (inputElement) {
        inputElement->setOn(value);

        Scene *scene = currentScene();
        if (scene && scene->simulation()) {
            scene->simulation()->update();
        }
    } else {
        return createErrorResponse("Element is not an input element", requestId, JsonRpcError::ValidationError);
    }

    return createSuccessResponse(QJsonObject(), requestId);
}

QJsonObject ElementHandler::handleGetOutputValue(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id"})) {
        return createErrorResponse("Missing required parameter: element_id", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    auto *element = validatedElement(params, "element_id", errorMsg);
    if (!element) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }

    // Optional port index (defaults to 0).
    int portIndex = 0;
    if (params.contains("port")) {
        if (!validateNonNegativeInteger(params.value("port"), "port", errorMsg)) {
            return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
        }
        portIndex = params.value("port").toInt();
    }

    QJsonObject result;

    auto *inputElement = dynamic_cast<GraphicElementInput *>(element);
    if (inputElement) {
        result["value"] = inputElement->isOn();
    } else {
        ElementGroup group = element->elementGroup();

        if (group == ElementGroup::Output) {
            if (!validatePortRange(element, portIndex, false, "port", errorMsg)) {
                return createErrorResponse(errorMsg, requestId, JsonRpcError::ValidationError);
            }
            InputPort *inPort = element->inputPort(portIndex);
            result["value"] = inPort ? (inPort->status() == Status::Active) : false;
        } else {
            if (!validatePortRange(element, portIndex, true, "port", errorMsg)) {
                return createErrorResponse(errorMsg, requestId, JsonRpcError::ValidationError);
            }
            OutputPort *outPort = element->outputPort(portIndex);
            result["value"] = outPort ? (outPort->status() == Status::Active) : false;
        }
    }

    return createSuccessResponse(result, requestId);
}

QJsonObject ElementHandler::handleRotateElement(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id", "angle"})) {
        return createErrorResponse("Missing required parameters: element_id, angle", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNumeric(params.value("angle"), "angle", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }
    int angle = params.value("angle").toInt();

    auto *element = validatedElement(params, "element_id", errorMsg);
    if (!element) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }

    // Fixed-graphic elements (inputs/outputs/displays) rotate by repositioning their ports,
    // exactly as the GUI does — so rotation is valid for every element here.
    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    // Normalize angle to 0-360
    angle = ((angle % 360) + 360) % 360;

    return tryCommand([&]() -> QJsonObject {
        scene->receiveCommand(new RotateCommand({element}, angle, scene));
        QJsonObject result;
        result["element_id"] = element->id();
        result["angle"] = angle;
        return createSuccessResponse(result, requestId);
    }, "rotate element", requestId);
}

QJsonObject ElementHandler::handleFlipElement(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id", "axis"})) {
        return createErrorResponse("Missing required parameters: element_id, axis", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNonNegativeInteger(params.value("axis"), "axis", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }
    const int axis = params.value("axis").toInt();
    if (axis != 0 && axis != 1) {
        return createErrorResponse("axis must be 0 (horizontal) or 1 (vertical)",
                                   requestId, JsonRpcError::ValidationError);
    }

    auto *element = validatedElement(params, "element_id", errorMsg);
    if (!element) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }

    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    return tryCommand([&]() -> QJsonObject {
        scene->receiveCommand(new FlipCommand({element}, axis, scene));
        QJsonObject result;
        result["element_id"] = element->id();
        result["axis"] = (axis == 0 ? "horizontal" : "vertical");
        return createSuccessResponse(result, requestId);
    }, "flip element", requestId);
}

QJsonObject ElementHandler::handleUpdateElement(const QJsonObject &params, const QJsonValue &requestId)
{
    // update_element is a thin wrapper around set_element_properties.
    QJsonObject response = handleSetElementProperties(params, requestId);

    // Propagate errors from the underlying handler.
    if (response.contains("error")) {
        return response;
    }

    // Return a simpler response format for update_element.
    QJsonObject result;
    result["element_id"] = params.value("element_id").toInt();

    return createSuccessResponse(result, requestId);
}

QJsonObject ElementHandler::handleChangeInputSize(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id", "size"})) {
        return createErrorResponse("Missing required parameters: element_id, size", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validatePositiveInteger(params.value("size"), "size", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }
    const int newSize = params.value("size").toInt();

    auto *element = validatedElement(params, "element_id", errorMsg);
    if (!element) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }

    if (newSize < element->minInputSize() || newSize > element->maxInputSize()) {
        return createErrorResponse(QString("Invalid input size %1. Must be between %2 and %3")
                                   .arg(newSize).arg(element->minInputSize()).arg(element->maxInputSize()),
                                   requestId, JsonRpcError::ValidationError);
    }

    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    return tryCommand([&]() -> QJsonObject {
        scene->receiveCommand(new ChangePortSizeCommand({element}, newSize, scene, true));
        const QRectF bounds = element->boundingRect();
        QJsonObject result;
        result["element_id"] = element->id();
        result["new_size"] = newSize;
        result["width"] = bounds.width();
        result["height"] = bounds.height();
        return createSuccessResponse(result, requestId);
    }, "change input size", requestId);
}

QJsonObject ElementHandler::handleChangeOutputSize(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id", "size"})) {
        return createErrorResponse("Missing required parameters: element_id, size", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validatePositiveInteger(params.value("size"), "size", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }
    const int newSize = params.value("size").toInt();

    auto *element = validatedElement(params, "element_id", errorMsg);
    if (!element) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }

    if (newSize < element->minOutputSize() || newSize > element->maxOutputSize()) {
        return createErrorResponse(QString("Invalid output size %1. Must be between %2 and %3")
                                   .arg(newSize).arg(element->minOutputSize()).arg(element->maxOutputSize()),
                                   requestId, JsonRpcError::ValidationError);
    }

    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    return tryCommand([&]() -> QJsonObject {
        scene->receiveCommand(new ChangePortSizeCommand({element}, newSize, scene, false));
        const QRectF bounds = element->boundingRect();
        QJsonObject result;
        result["element_id"] = element->id();
        result["new_size"] = newSize;
        result["width"] = bounds.width();
        result["height"] = bounds.height();
        return createSuccessResponse(result, requestId);
    }, "change output size", requestId);
}

QJsonObject ElementHandler::handleToggleTruthTableOutput(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id", "position"})) {
        return createErrorResponse("Missing required parameters: element_id, position", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNonNegativeInteger(params.value("position"), "position", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }
    const int position = params.value("position").toInt();

    auto *element = validatedElement(params, "element_id", errorMsg);
    if (!element) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }

    auto *truthTable = dynamic_cast<TruthTable *>(element);
    if (!truthTable) {
        return createErrorResponse(QString("Element %1 is not a TruthTable").arg(element->id()),
                                   requestId, JsonRpcError::ValidationError);
    }

    // The key stores 256 rows per output; positions beyond the current
    // outputs address nothing the UI can ever toggle (and ≥ 2048 would be an
    // out-of-bounds write on the 2048-bit key — the command guards that too).
    const int maxPosition = 256 * truthTable->outputSize();
    if (position >= maxPosition) {
        return createErrorResponse(QString("position %1 out of range: TruthTable %2 has %3 outputs (valid positions 0..%4)")
                                       .arg(position).arg(element->id()).arg(truthTable->outputSize()).arg(maxPosition - 1),
                                   requestId, JsonRpcError::ValidationError);
    }

    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    return tryCommand([&]() -> QJsonObject {
        // ToggleTruthTableOutputCommand flips one cell in the truth table's output column.
        scene->receiveCommand(new ToggleTruthTableOutputCommand(truthTable, position, scene));
        QJsonObject result;
        result["element_id"] = element->id();
        result["position"] = position;
        return createSuccessResponse(result, requestId);
    }, "toggle truth table output", requestId);
}

QJsonObject ElementHandler::handleMorphElement(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_ids", "target_type"})) {
        return createErrorResponse("Missing required parameters: element_ids, target_type", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;

    // Validate element_ids is an array
    if (!params.value("element_ids").isArray()) {
        return createErrorResponse("element_ids must be an array", requestId, JsonRpcError::InvalidParams);
    }

    QJsonArray elementIdsArray = params.value("element_ids").toArray();
    if (elementIdsArray.empty()) {
        return createErrorResponse("element_ids array cannot be empty", requestId, JsonRpcError::InvalidParams);
    }

    // Validate target_type
    if (!validateNonEmptyString(params.value("target_type"), "target_type", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    QString typeStr = params.value("target_type").toString();
    ElementType targetType = ElementFactory::textToType(typeStr);
    if (targetType == ElementType::Unknown) {
        return createErrorResponse(QString("Invalid target element type: %1").arg(typeStr),
                                   requestId, JsonRpcError::ValidationError);
    }

    // Collect elements to morph
    QList<GraphicElement *> elementsToMorph;
    for (const QJsonValue &idValue : std::as_const(elementIdsArray)) {
        if (!idValue.isDouble()) {
            return createErrorResponse("element_ids must contain only integers", requestId, JsonRpcError::InvalidParams);
        }

        int elementId = idValue.toInt();
        if (!validateElementId(elementId, "element_ids", errorMsg)) {
            return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
        }

        auto *item = currentScene()->itemById(elementId);
        auto *element = dynamic_cast<GraphicElement *>(item);
        if (!element) {
            return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId),
                                       requestId, JsonRpcError::ValidationError);
        }

        elementsToMorph.append(element);
    }

    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    // Capture IDs before morph — element pointers become invalid after receiveCommand.
    QJsonArray morphedIds;
    for (const auto *element : elementsToMorph) {
        morphedIds.append(element->id());
    }

    return tryCommand([&]() -> QJsonObject {
        scene->receiveCommand(new MorphCommand(elementsToMorph, targetType, scene));
        // MorphCommand preserves element IDs via updateItemId(), so the morphed
        // elements keep their original IDs.
        QJsonObject result;
        result["morphed_elements"] = morphedIds;
        result["target_type"] = typeStr;
        return createSuccessResponse(result, requestId);
    }, "morph elements", requestId);
}
