// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementhandler.h"

#include "commands.h"
#include "elementfactory.h"
#include "enums.h"
#include "graphicelement.h"
#include "graphicelementinput.h"
#include "scene.h"
#include "simulation.h"
#include "qneport.h"

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
