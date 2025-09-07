// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementhandler.h"

#include "commands.h"
#include "common.h"
#include "elementfactory.h"
#include "enums.h"
#include "graphicelement.h"
#include "graphicelementinput.h"
#include "scene.h"
#include "simulation.h"
#include "qneport.h"

#include <QJsonArray>

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

    GraphicElement *element = ElementFactory::buildElement(type);
    if (!element) {
        return createErrorResponse(QString("Failed to create element of type: %1").arg(typeStr), requestId);
    }

    element->setPos(x, y);
    if (!label.isEmpty()) {
        element->setLabel(label);
    }

    scene->receiveCommand(new AddItemsCommand({element}, scene));

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

    auto *item = ElementFactory::itemById(elementId);

    auto *element = dynamic_cast<GraphicElement*>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    scene->receiveCommand(new DeleteItemsCommand({element}, scene));

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
        QJsonObject elementObj;
        elementObj["element_id"] = element->id();
        elementObj["type"] = ElementFactory::typeToText(element->elementType());
        elementObj["x"] = element->pos().x();
        elementObj["y"] = element->pos().y();
        elementObj["label"] = element->label();

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
    double x = params.value("x").toDouble();
    double y = params.value("y").toDouble();

    if (!validateElementId(elementId, "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    auto *item = ElementFactory::itemById(elementId);

    auto *element = dynamic_cast<GraphicElement*>(item);
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

    QList<GraphicElement*> elements = {element};
    QList<QPointF> oldPositions = {oldPos};

    scene->receiveCommand(new MoveCommand(elements, oldPositions, scene));

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

    int elementId = params.value("element_id").toInt();

    auto *item = ElementFactory::itemById(elementId);
    if (!item) {
        return createErrorResponse(QString("Element not found: %1").arg(elementId), requestId);
    }

    auto *element = dynamic_cast<GraphicElement*>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
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

    if (params.contains("input_size")) {
        int newInputSize = params.value("input_size").toInt();
        if (newInputSize >= element->minInputSize() && newInputSize <= element->maxInputSize()) {
            int oldInputSize = element->inputSize();
            oldProperties["input_size"] = oldInputSize;
            newProperties["input_size"] = newInputSize;

            element->setInputSize(newInputSize);
        } else {
            return createErrorResponse(QString("Invalid input size. Must be between %1 and %2")
                                       .arg(element->minInputSize()).arg(element->maxInputSize()), requestId);
        }
    }

    if (params.contains("output_size")) {
        int newOutputSize = params.value("output_size").toInt();
        if (newOutputSize >= element->minOutputSize() && newOutputSize <= element->maxOutputSize()) {
            int oldOutputSize = element->outputSize();
            oldProperties["output_size"] = oldOutputSize;
            newProperties["output_size"] = newOutputSize;

            element->setOutputSize(newOutputSize);
        } else {
            return createErrorResponse(QString("Invalid output size. Must be between %1 and %2")
                                       .arg(element->minOutputSize()).arg(element->maxOutputSize()), requestId);
        }
    }

    if (params.contains("color") && element->hasColors()) {
        QString oldColor = element->color();
        QString newColor = params.value("color").toString();

        oldProperties["color"] = oldColor;
        newProperties["color"] = newColor;

        element->setColor(newColor);
    }

    if (params.contains("frequency") && element->hasFrequency()) {
        float oldFreq = element->frequency();
        float newFreq = static_cast<float>(params.value("frequency").toDouble());

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

    auto *item = ElementFactory::itemById(elementId);

    auto *element = dynamic_cast<GraphicElement*>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    auto *inputElement = dynamic_cast<GraphicElementInput*>(element);
    if (inputElement) {
        inputElement->setOn(value);

        Scene *scene = getCurrentScene();
        if (scene && scene->simulation()) {
            for (int i = 0; i < 5; ++i) {
                scene->simulation()->update();
            }
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

    auto *item = ElementFactory::itemById(elementId);

    auto *element = dynamic_cast<GraphicElement*>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    QJsonObject result;

    auto *inputElement = dynamic_cast<GraphicElementInput*>(element);
    if (inputElement) {
        result["value"] = inputElement->isOn();
    } else {
        ElementGroup group = element->elementGroup();

        if (group == ElementGroup::Output) {
            QNEInputPort *inputPort = element->inputPort(0);
            if (inputPort) {
                Status status = inputPort->status();
                result["value"] = (status == Status::Active);
            } else {
                result["value"] = false;
            }
        } else {
            QNEOutputPort *outputPort = element->outputPort(0);
            if (outputPort) {
                Status status = outputPort->status();
                result["value"] = (status == Status::Active);
            } else {
                result["value"] = false;
            }
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

    auto *item = ElementFactory::itemById(elementId);
    auto *element = dynamic_cast<GraphicElement*>(item);
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

    scene->receiveCommand(new RotateCommand({element}, angle, scene));

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

    auto *item = ElementFactory::itemById(elementId);
    auto *element = dynamic_cast<GraphicElement*>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    scene->receiveCommand(new FlipCommand({element}, axis, scene));

    QJsonObject result;
    result["element_id"] = elementId;
    result["axis"] = (axis == 0 ? "horizontal" : "vertical");

    return createSuccessResponse(result, requestId);
}

QJsonObject ElementHandler::handleUpdateElement(const QJsonObject &params, const QJsonValue &requestId)
{
    // update_element is a wrapper around set_element_properties
    // Call the actual property update but return a simpler response
    if (!validateParameters(params, {"element_id"})) {
        return createErrorResponse("Missing required parameter: element_id", requestId);
    }

    int elementId = params.value("element_id").toInt();
    QString errorMsg;

    if (!validateElementId(elementId, "element_id", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    // Call the property update via the properties handler
    handleSetElementProperties(params, requestId);

    // Return a simpler response format for update_element
    QJsonObject result;
    result["element_id"] = elementId;

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

    auto *item = ElementFactory::itemById(elementId);
    auto *element = dynamic_cast<GraphicElement*>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    // Use ChangeInputSizeCommand for undo/redo support
    scene->receiveCommand(new ChangeInputSizeCommand({element}, newSize, scene));

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

    auto *item = ElementFactory::itemById(elementId);
    auto *element = dynamic_cast<GraphicElement*>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    // Use ChangeOutputSizeCommand for undo/redo support
    scene->receiveCommand(new ChangeOutputSizeCommand({element}, newSize, scene));

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

    auto *item = ElementFactory::itemById(elementId);
    if (!item) {
        return createErrorResponse(QString("Item %1 not found").arg(elementId), requestId);
    }

    auto *element = dynamic_cast<GraphicElement*>(item);
    if (!element) {
        return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    try {
        // Use ToggleTruthTableOutputCommand for undo/redo support
        // Note: ElementEditor parameter is nullptr for MCP usage
        scene->receiveCommand(new ToggleTruthTableOutputCommand(element, position, scene, nullptr));
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
    for (const QJsonValue &idValue : elementIdsArray) {
        if (!idValue.isDouble()) {
            return createErrorResponse("element_ids must contain only integers", requestId);
        }

        int elementId = idValue.toInt();
        if (!validateElementId(elementId, "element_ids", errorMsg)) {
            return createErrorResponse(errorMsg, requestId);
        }

        auto *item = ElementFactory::itemById(elementId);
        auto *element = dynamic_cast<GraphicElement*>(item);
        if (!element) {
            return createErrorResponse(QString("Item %1 is not a graphic element").arg(elementId), requestId);
        }

        elementsToMorph.append(element);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    // Store original IDs before morphing
    QList<int> originalIds;
    for (const auto *element : elementsToMorph) {
        originalIds.append(element->id());
    }

    try {
        // Use MorphCommand for undo/redo support
        scene->receiveCommand(new MorphCommand(elementsToMorph, targetType, scene));
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to morph elements: %1").arg(e.what()), requestId);
    } catch (...) {
        return createErrorResponse("Failed to morph elements: Unknown exception", requestId);
    }

    // After morph, find the new elements with the same original IDs
    // (Note: with Approach 2, the new elements will have different IDs)
    // Find all elements of the target type and match by position in the scene
    QJsonObject result;
    QJsonArray morphedIds;

    // Get all elements in the scene and find the new morphed elements
    // We can identify them as elements of targetType that weren't there before
    auto sceneItems = scene->items();
    QList<GraphicElement *> newElements;

    for (auto *item : sceneItems) {
        if (auto *elem = dynamic_cast<GraphicElement*>(item)) {
            if (elem->elementType() == targetType) {
                // Check if this element wasn't in our original list
                bool isNew = true;
                for (int origId : originalIds) {
                    if (elem->id() == origId) {
                        isNew = false;
                        break;
                    }
                }
                if (isNew) {
                    newElements.append(elem);
                }
            }
        }
    }

    // If we found the right number of new elements, use them
    if (newElements.size() == originalIds.size()) {
        for (auto *elem : newElements) {
            morphedIds.append(elem->id());
        }
    } else {
        // Fallback: return the original IDs (may be incorrect but better than nothing)
        // This shouldn't happen in normal operation
        qCWarning(zero) << "ElementHandler::handleMorphElement - Could not find all new elements after morph";
        for (int origId : originalIds) {
            morphedIds.append(origId);
        }
    }

    result["morphed_elements"] = morphedIds;
    result["target_type"] = typeStr;

    return createSuccessResponse(result, requestId);
}
