// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/ElementHandler.h"

#include <QIODevice>
#include <QJsonArray>

#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/IO/Serialization.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"

ElementHandler::ElementHandler(MainWindow *mainWindow, MCPValidator *validator)
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
        return createErrorResponse(QString("Unknown element command: %1").arg(command), requestId);
    }
}

QJsonObject ElementHandler::handleCreateElement(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"type", "x", "y"})) {
        return createErrorResponse("Missing required parameters: type, x, y", requestId);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("type"), "type", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validateNumeric(params.value("x"), "x", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validateNumeric(params.value("y"), "y", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    QString typeStr = params.value("type").toString();
    double x = params.value("x").toDouble();
    double y = params.value("y").toDouble();
    QString label = params.value("label").toString();

    ElementType type = ElementFactory::textToType(typeStr);
    if (type == ElementType::Unknown) {
        return createErrorResponse(QString("Invalid element type: %1").arg(typeStr), requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    GraphicElement *element = nullptr;
    try {
        element = ElementFactory::buildElement(type);
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to create element of type %1: %2").arg(typeStr, e.what()), requestId);
    } catch (...) {
        return createErrorResponse(QString("Failed to create element of type: %1").arg(typeStr), requestId);
    }

    if (!element) {
        return createErrorResponse(QString("Failed to create element of type: %1").arg(typeStr), requestId);
    }

    element->setPos(x, y);
    if (!label.isEmpty()) {
        element->setLabel(label);
    }

    try {
        scene->receiveCommand(new AddItemsCommand({element}, scene));
    } catch (const std::exception &e) {
        delete element;
        return createErrorResponse(QString("Failed to add element to scene: %1").arg(e.what()), requestId);
    } catch (...) {
        delete element;
        return createErrorResponse("Failed to add element to scene: Unknown exception", requestId);
    }

    QJsonObject result;
    result["element_id"] = element->id();

    return createSuccessResponse(result, requestId);
}

QJsonObject ElementHandler::handleDeleteElement(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id"})) {
        return createErrorResponse("Missing required parameter: element_id", requestId);
    }

    QString errorMsg;
    if (!validatePositiveInteger(params.value("element_id"), "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    int elementId = params.value("element_id").toInt();

    if (!validateElementId(elementId, "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    auto *item = getCurrentScene()->itemById(elementId);

    auto *element = dynamic_cast<GraphicElement *>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    try {
        scene->receiveCommand(new DeleteItemsCommand({element}, scene));
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to delete element: %1").arg(e.what()), requestId);
    } catch (...) {
        return createErrorResponse("Failed to delete element: Unknown exception", requestId);
    }

    return createSuccessResponse(QJsonObject(), requestId);
}

QJsonObject ElementHandler::handleListElements(const QJsonObject &, const QJsonValue &requestId)
{
    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
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
        return createErrorResponse("Missing required parameters: element_id, x, y", requestId);
    }

    QString errorMsg;
    if (!validatePositiveInteger(params.value("element_id"), "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validateNumeric(params.value("x"), "x", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validateNumeric(params.value("y"), "y", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    int elementId = params.value("element_id").toInt();
    const int snap = Scene::gridSize / 2;
    const int x = qRound(params.value("x").toDouble() / snap) * snap;
    const int y = qRound(params.value("y").toDouble() / snap) * snap;

    if (!validateElementId(elementId, "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    auto *item = getCurrentScene()->itemById(elementId);

    auto *element = dynamic_cast<GraphicElement *>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
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
        return createErrorResponse(QString("Failed to move element: %1").arg(e.what()), requestId);
    } catch (...) {
        element->setPos(oldPos);
        return createErrorResponse("Failed to move element: Unknown exception", requestId);
    }

    QJsonObject result;
    result["old_position"] = QJsonObject{{"x", oldPos.x()}, {"y", oldPos.y()}};
    result["new_position"] = QJsonObject{{"x", newPos.x()}, {"y", newPos.y()}};

    return createSuccessResponse(result, requestId);
}

QJsonObject ElementHandler::handleSetElementProperties(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id"})) {
        return createErrorResponse("Missing required parameter: element_id", requestId);
    }

    QString errorMsg;
    if (!validatePositiveInteger(params.value("element_id"), "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    int elementId = params.value("element_id").toInt();

    if (!validateElementId(elementId, "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    auto *item = getCurrentScene()->itemById(elementId);

    auto *element = dynamic_cast<GraphicElement *>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    // Reject structural changes before any mutation — these require dedicated commands.
    if (params.contains("input_size") || params.contains("output_size")) {
        return createErrorResponse("Use change_input_size / change_output_size commands to change port counts", requestId);
    }

    // Snapshot current state before any mutation so UpdateCommand can restore it on undo.
    QByteArray oldData;
    {
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        element->save(stream);
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
        double oldFreq = element->frequency();
        double newFreq = params.value("frequency").toDouble();

        oldProperties["frequency"] = oldFreq;
        newProperties["frequency"] = newFreq;

        element->setFrequency(newFreq);
    }

    if (params.contains("rotation") && element->isRotatable()) {
        qreal oldRotation = element->rotation();
        qreal newRotation = params.value("rotation").toDouble();

        oldProperties["rotation"] = oldRotation;
        newProperties["rotation"] = newRotation;

        element->setRotation(newRotation);
    }

    if (params.contains("delay") && element->hasDelay()) {
        double oldDelay = element->delay();
        double newDelay = params.value("delay").toDouble();

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
        float oldVolume = element->volume();
        float newVolume = static_cast<float>(params.value("volume").toDouble());

        oldProperties["volume"] = static_cast<double>(oldVolume);
        newProperties["volume"] = static_cast<double>(newVolume);

        element->setVolume(newVolume);
    }

    if (params.contains("skin") && element->canChangeSkin()) {
        QString skinPath = params.value("skin").toString();
        bool useDefault = skinPath.isEmpty();

        newProperties["skin"] = skinPath;
        newProperties["skin_default"] = useDefault;

        element->setSkin(useDefault, skinPath);
    }

    // Wireless mode is only meaningful for Node elements; non-Nodes are silently ignored.
    QList<QGraphicsItem *> wirelessConnsToDelete;
    if (params.contains("wireless_mode")) {
        if (auto *node = qobject_cast<Node *>(element)) {
            int modeInt = params.value("wireless_mode").toInt();
            if (modeInt < 0 || modeInt > 2) {
                return createErrorResponse("Invalid wireless_mode. Must be 0 (None), 1 (Tx), or 2 (Rx)", requestId);
            }
            auto oldMode = static_cast<int>(node->wirelessMode());
            auto newMode = static_cast<WirelessMode>(modeInt);

            oldProperties["wireless_mode"] = oldMode;
            newProperties["wireless_mode"] = modeInt;

            // Collect connections on the port that will be hidden, so they can
            // be deleted in an undo macro (same pattern as ElementEditor::apply).
            QNEPort *port = (newMode == WirelessMode::Rx) ? static_cast<QNEPort *>(node->inputPort())
                          : (newMode == WirelessMode::Tx) ? static_cast<QNEPort *>(node->outputPort())
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
        auto *scene = getCurrentScene();
        if (needsMacro) {
            scene->undoStack()->beginMacro(QStringLiteral("Change wireless mode"));
        }
        scene->receiveCommand(new UpdateCommand({element}, oldData, scene));
        if (needsMacro) {
            scene->receiveCommand(new DeleteItemsCommand(wirelessConnsToDelete, scene));
            scene->undoStack()->endMacro();
        }
    }

    result["old_properties"] = oldProperties;
    result["new_properties"] = newProperties;

    return createSuccessResponse(result, requestId);
}

QJsonObject ElementHandler::handleSetInputValue(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id", "value"})) {
        return createErrorResponse("Missing required parameters: element_id, value", requestId);
    }

    QString errorMsg;
    if (!validatePositiveInteger(params.value("element_id"), "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    int elementId = params.value("element_id").toInt();
    bool value = params.value("value").toBool();

    if (!validateElementId(elementId, "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    auto *item = getCurrentScene()->itemById(elementId);

    auto *element = dynamic_cast<GraphicElement *>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    auto *inputElement = dynamic_cast<GraphicElementInput *>(element);
    if (inputElement) {
        inputElement->setOn(value);

        Scene *scene = getCurrentScene();
        if (scene && scene->simulation()) {
            scene->simulation()->update();
        }
    } else {
        return createErrorResponse("Element is not an input element", requestId);
    }

    return createSuccessResponse(QJsonObject(), requestId);
}

QJsonObject ElementHandler::handleGetOutputValue(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id"})) {
        return createErrorResponse("Missing required parameter: element_id", requestId);
    }

    QString errorMsg;
    if (!validatePositiveInteger(params.value("element_id"), "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    int elementId = params.value("element_id").toInt();

    if (!validateElementId(elementId, "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    auto *item = getCurrentScene()->itemById(elementId);

    auto *element = dynamic_cast<GraphicElement *>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    // Optional port index (defaults to 0).
    int portIndex = 0;
    if (params.contains("port")) {
        if (!validateNonNegativeInteger(params.value("port"), "port", errorMsg)) {
            return createErrorResponse(errorMsg, requestId);
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
                return createErrorResponse(errorMsg, requestId);
            }
            QNEInputPort *inPort = element->inputPort(portIndex);
            result["value"] = inPort ? (inPort->status() == Status::Active) : false;
        } else {
            if (!validatePortRange(element, portIndex, true, "port", errorMsg)) {
                return createErrorResponse(errorMsg, requestId);
            }
            QNEOutputPort *outPort = element->outputPort(portIndex);
            result["value"] = outPort ? (outPort->status() == Status::Active) : false;
        }
    }

    return createSuccessResponse(result, requestId);
}

QJsonObject ElementHandler::handleRotateElement(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id", "angle"})) {
        return createErrorResponse("Missing required parameters: element_id, angle", requestId);
    }

    QString errorMsg;
    if (!validatePositiveInteger(params.value("element_id"), "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validateNumeric(params.value("angle"), "angle", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    int elementId = params.value("element_id").toInt();
    int angle = params.value("angle").toInt();

    if (!validateElementId(elementId, "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    auto *item = getCurrentScene()->itemById(elementId);
    auto *element = dynamic_cast<GraphicElement *>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    if (!element->isRotatable()) {
        return createErrorResponse(QString("Element %1 is not rotatable").arg(elementId), requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    // Normalize angle to 0-360
    angle = ((angle % 360) + 360) % 360;

    try {
        scene->receiveCommand(new RotateCommand({element}, angle, scene));
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to rotate element: %1").arg(e.what()), requestId);
    } catch (...) {
        return createErrorResponse("Failed to rotate element: Unknown exception", requestId);
    }

    QJsonObject result;
    result["element_id"] = elementId;
    result["angle"] = angle;

    return createSuccessResponse(result, requestId);
}

QJsonObject ElementHandler::handleFlipElement(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id", "axis"})) {
        return createErrorResponse("Missing required parameters: element_id, axis", requestId);
    }

    QString errorMsg;
    if (!validatePositiveInteger(params.value("element_id"), "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validateNonNegativeInteger(params.value("axis"), "axis", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    int elementId = params.value("element_id").toInt();
    int axis = params.value("axis").toInt();

    if (axis != 0 && axis != 1) {
        return createErrorResponse("axis must be 0 (horizontal) or 1 (vertical)", requestId);
    }

    if (!validateElementId(elementId, "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    auto *item = getCurrentScene()->itemById(elementId);
    auto *element = dynamic_cast<GraphicElement *>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    try {
        scene->receiveCommand(new FlipCommand({element}, axis, scene));
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to flip element: %1").arg(e.what()), requestId);
    } catch (...) {
        return createErrorResponse("Failed to flip element: Unknown exception", requestId);
    }

    QJsonObject result;
    result["element_id"] = elementId;
    result["axis"] = (axis == 0 ? "horizontal" : "vertical");

    return createSuccessResponse(result, requestId);
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
        return createErrorResponse("Missing required parameters: element_id, size", requestId);
    }

    QString errorMsg;
    if (!validatePositiveInteger(params.value("element_id"), "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validatePositiveInteger(params.value("size"), "size", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    int elementId = params.value("element_id").toInt();
    int newSize = params.value("size").toInt();

    if (!validateElementId(elementId, "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    auto *item = getCurrentScene()->itemById(elementId);
    auto *element = dynamic_cast<GraphicElement *>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    if (newSize < element->minInputSize() || newSize > element->maxInputSize()) {
        return createErrorResponse(QString("Invalid input size %1. Must be between %2 and %3")
                                   .arg(newSize).arg(element->minInputSize()).arg(element->maxInputSize()), requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    try {
        scene->receiveCommand(new ChangeInputSizeCommand({element}, newSize, scene));
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to change input size: %1").arg(e.what()), requestId);
    } catch (...) {
        return createErrorResponse("Failed to change input size: Unknown exception", requestId);
    }

    QJsonObject result;
    result["element_id"] = elementId;
    result["new_size"] = newSize;

    return createSuccessResponse(result, requestId);
}

QJsonObject ElementHandler::handleChangeOutputSize(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id", "size"})) {
        return createErrorResponse("Missing required parameters: element_id, size", requestId);
    }

    QString errorMsg;
    if (!validatePositiveInteger(params.value("element_id"), "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validatePositiveInteger(params.value("size"), "size", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    int elementId = params.value("element_id").toInt();
    int newSize = params.value("size").toInt();

    if (!validateElementId(elementId, "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    auto *item = getCurrentScene()->itemById(elementId);
    auto *element = dynamic_cast<GraphicElement *>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    if (newSize < element->minOutputSize() || newSize > element->maxOutputSize()) {
        return createErrorResponse(QString("Invalid output size %1. Must be between %2 and %3")
                                   .arg(newSize).arg(element->minOutputSize()).arg(element->maxOutputSize()), requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    try {
        scene->receiveCommand(new ChangeOutputSizeCommand({element}, newSize, scene));
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to change output size: %1").arg(e.what()), requestId);
    } catch (...) {
        return createErrorResponse("Failed to change output size: Unknown exception", requestId);
    }

    QJsonObject result;
    result["element_id"] = elementId;
    result["new_size"] = newSize;

    return createSuccessResponse(result, requestId);
}

QJsonObject ElementHandler::handleToggleTruthTableOutput(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id", "position"})) {
        return createErrorResponse("Missing required parameters: element_id, position", requestId);
    }

    QString errorMsg;
    if (!validatePositiveInteger(params.value("element_id"), "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    if (!validateNonNegativeInteger(params.value("position"), "position", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    int elementId = params.value("element_id").toInt();
    int position = params.value("position").toInt();

    if (!validateElementId(elementId, "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    auto *item = getCurrentScene()->itemById(elementId);
    if (!item) {
        return createErrorResponse(QString("Item %1 not found").arg(elementId), requestId);
    }

    auto *element = dynamic_cast<GraphicElement *>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    auto *truthTable = dynamic_cast<TruthTable *>(element);
    if (!truthTable) {
        return createErrorResponse(QString("Element %1 is not a TruthTable").arg(elementId), requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    try {
        // ToggleTruthTableOutputCommand flips one cell in the truth table's output column.
        scene->receiveCommand(new ToggleTruthTableOutputCommand(truthTable, position, scene));
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to toggle truth table output: %1").arg(e.what()), requestId);
    } catch (...) {
        return createErrorResponse("Failed to toggle truth table output: Unknown exception", requestId);
    }

    QJsonObject result;
    result["element_id"] = elementId;
    result["position"] = position;

    return createSuccessResponse(result, requestId);
}

QJsonObject ElementHandler::handleMorphElement(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_ids", "target_type"})) {
        return createErrorResponse("Missing required parameters: element_ids, target_type", requestId);
    }

    QString errorMsg;

    // Validate element_ids is an array
    if (!params.value("element_ids").isArray()) {
        return createErrorResponse("element_ids must be an array", requestId);
    }

    QJsonArray elementIdsArray = params.value("element_ids").toArray();
    if (elementIdsArray.empty()) {
        return createErrorResponse("element_ids array cannot be empty", requestId);
    }

    // Validate target_type
    if (!validateNonEmptyString(params.value("target_type"), "target_type", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    QString typeStr = params.value("target_type").toString();
    ElementType targetType = ElementFactory::textToType(typeStr);
    if (targetType == ElementType::Unknown) {
        return createErrorResponse(QString("Invalid target element type: %1").arg(typeStr), requestId);
    }

    // Collect elements to morph
    QList<GraphicElement *> elementsToMorph;
    for (const QJsonValue &idValue : std::as_const(elementIdsArray)) {
        if (!idValue.isDouble()) {
            return createErrorResponse("element_ids must contain only integers", requestId);
        }

        int elementId = idValue.toInt();
        if (!validateElementId(elementId, "element_ids", errorMsg)) {
            return createErrorResponse(errorMsg, requestId);
        }

        auto *item = getCurrentScene()->itemById(elementId);
        auto *element = dynamic_cast<GraphicElement *>(item);
        if (!element) {
            return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
        }

        elementsToMorph.append(element);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    // Capture IDs before morph — element pointers become invalid after receiveCommand.
    QJsonArray morphedIds;
    for (const auto *element : elementsToMorph) {
        morphedIds.append(element->id());
    }

    try {
        scene->receiveCommand(new MorphCommand(elementsToMorph, targetType, scene));
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to morph elements: %1").arg(e.what()), requestId);
    } catch (...) {
        return createErrorResponse("Failed to morph elements: Unknown exception", requestId);
    }

    // MorphCommand preserves element IDs via updateItemId(), so the morphed
    // elements keep their original IDs.
    QJsonObject result;
    result["morphed_elements"] = morphedIds;
    result["target_type"] = typeStr;

    return createSuccessResponse(result, requestId);
}

