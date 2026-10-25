// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/ElementHandler.h"

#include <algorithm>
#include <cmath>

#include <QIODevice>
#include <QJsonArray>
#include <QPixmap>
#include <QRegularExpression>

#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace ElementHandlerProperties {

/// The colours the property editor offers, by their untranslated names.
const QStringList kColors{"White", "Red", "Green", "Blue", "Purple"};

/// Returns the first reason set_element_properties refuses \a params for \a element, or an empty string when every
/// value passes: the property exists on the type, the value has its kind, lies in its range and is one the type takes,
/// an appearance index comes with an appearance and names one of the element's states, a picture loads, and no
/// second Tx would share a label.
QString refusal(const GraphicElement &element, const QJsonObject &params, const Scene *scene)
{
    const auto lacks = [&element](const QString &property) {
        return QString("Element %1 (%2) has no property '%3'")
            .arg(QString::number(element.id()), ElementFactory::typeToText(element.elementType()), property);
    };
    const auto has = [&element](const ElementProperty property) { return SetPropertyCommand::appliesTo(element, property); };
    const auto number = [&params](const QString &key) {
        const QJsonValue value = params.value(key);
        return value.isDouble() && std::isfinite(value.toDouble());
    };
    const auto whole = [&params, &number](const QString &key) {
        return number(key) && (params.value(key).toDouble() == std::floor(params.value(key).toDouble()));
    };
    const auto kind = [](const QString &key, const QString &what) {
        return QString("Parameter '%1' must be %2").arg(key, what);
    };

    // Every element takes a label and a rotation, a label shown or not: MCP's callers label gates too
    if (params.contains("label") && !params.value("label").isString()) {
        return kind("label", "a string");
    }
    if (params.contains("rotation") && !number("rotation")) {
        return kind("rotation", "a finite number");
    }
    if (params.contains("color")) {
        if (!has(ElementProperty::Color)) {
            return lacks("color");
        }
        if (!params.value("color").isString() || !kColors.contains(params.value("color").toString())) {
            return kind("color", "one of " + kColors.join(", "));
        }
    }
    if (params.contains("frequency")) {
        if (!has(ElementProperty::Frequency)) {
            return lacks("frequency");
        }
        if (!number("frequency")) {
            return kind("frequency", "a finite number");
        }
        if (!element.acceptsFrequency(params.value("frequency").toDouble())) {
            return QString("Parameter 'frequency' %1 is not a frequency this element takes")
                .arg(params.value("frequency").toDouble());
        }
    }
    if (params.contains("delay")) {
        if (!has(ElementProperty::Delay)) {
            return lacks("delay");
        }
        if (!number("delay") || (params.value("delay").toDouble() < -1.0) || (params.value("delay").toDouble() > 1.0)) {
            return kind("delay", "a number in [-1, 1] (fraction of period; negative advances the clock)");
        }
    }
    if (params.contains("trigger")) {
        if (!has(ElementProperty::Trigger)) {
            return lacks("trigger");
        }
        // The editor's field takes one letter, digit or space, or none
        static const QRegularExpression editorKey("^[A-Za-z0-9 ]?$");
        if (!params.value("trigger").isString() || !editorKey.match(params.value("trigger").toString()).hasMatch()) {
            return kind("trigger", "one letter, digit or space, or empty");
        }
    }
    if (params.contains("audio")) {
        if (!has(ElementProperty::Audio)) {
            return lacks("audio");
        }
        // LCOV_EXCL_START — no element type has audio (hasAudio()), so the property is refused above
        if (!params.value("audio").isString()) {
            return kind("audio", "a string");
        }
        // LCOV_EXCL_STOP
    }
    if (params.contains("locked")) {
        if (!has(ElementProperty::Locked)) {
            return lacks("locked");
        }
        if (!params.value("locked").isBool()) {
            return kind("locked", "a boolean");
        }
    }
    if (params.contains("volume")) {
        if (!has(ElementProperty::Volume)) {
            return lacks("volume");
        }
        if (!number("volume") || (params.value("volume").toDouble() < 0.0) || (params.value("volume").toDouble() > 1.0)) {
            return kind("volume", "a number between 0.0 and 1.0");
        }
    }
    if (params.contains("appearance")) {
        if (!element.canChangeAppearance()) {
            return lacks("appearance");
        }
        if (!params.value("appearance").isString()) {
            return kind("appearance", "a string");
        }
        if (const QString path = params.value("appearance").toString(); !path.isEmpty() && QPixmap(path).isNull()) {
            return QString("Picture '%1' does not load").arg(path);
        }
    }
    if (params.contains("appearance_index")) {
        if (!params.contains("appearance")) {
            return "Parameter 'appearance_index' needs 'appearance'";
        }
        const auto states = element.appearanceStates();
        const bool named = whole("appearance_index") && std::any_of(states.cbegin(), states.cend(), [&params](const auto &state) {
            return state.first == params.value("appearance_index").toInt();
        });
        if (!named) {
            return kind("appearance_index", "the index of one of the element's appearance states");
        }
    }
    if (params.contains("wireless_mode")) {
        if (!has(ElementProperty::Wireless)) {
            return lacks("wireless_mode");
        }
        if (!whole("wireless_mode") || (params.value("wireless_mode").toInt() < 0) || (params.value("wireless_mode").toInt() > 2)) {
            return "Invalid wireless_mode. Must be 0 (None), 1 (Tx), or 2 (Rx)";
        }
    }

    // A Tx node's label is its channel, and a channel has one Tx
    if (element.hasWirelessMode() && scene && (params.contains("label") || params.contains("wireless_mode"))) {
        const QString label = params.contains("label") ? params.value("label").toString() : element.label();
        const auto mode = params.contains("wireless_mode") ? static_cast<WirelessMode>(params.value("wireless_mode").toInt())
                                                           : element.wirelessMode();
        if (const QString duplicate = scene->duplicateTxLabel({{&element, label, mode}}); !duplicate.isEmpty()) {
            return QString("A Tx node with label \"%1\" already exists: each wireless channel has one").arg(duplicate);
        }
    }
    return {};
}

} // namespace ElementHandlerProperties

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
    // Unreachable: ElementFactory::buildElement() only throws for ElementType::Unknown (already
    // rejected above) or a type with no registered creator -- every real type reaching here has
    // one (confirmed by reading ElementFactory.cpp's creator map).
    try {
        element = ElementFactory::buildElement(type);
    } catch (const std::exception &e) { // LCOV_EXCL_LINE
        return createErrorResponse(QString("Failed to create element of type %1: %2").arg(typeStr, e.what()), // LCOV_EXCL_LINE
                                   requestId, JsonRpcError::OperationFailed); // LCOV_EXCL_LINE
    } catch (...) { // LCOV_EXCL_LINE
        return createErrorResponse(QString("Failed to create element of type: %1").arg(typeStr), // LCOV_EXCL_LINE
                                   requestId, JsonRpcError::OperationFailed); // LCOV_EXCL_LINE
    } // LCOV_EXCL_LINE

    // Unreachable: a `new` expression (which buildElement() uses internally) never returns
    // null without throwing -- the catch blocks above already handle every throwing path.
    if (!element) { // LCOV_EXCL_LINE
        return createErrorResponse(QString("Failed to create element of type: %1").arg(typeStr), // LCOV_EXCL_LINE
                                   requestId, JsonRpcError::OperationFailed); // LCOV_EXCL_LINE
    } // LCOV_EXCL_LINE

    element->setPos(GraphicElement::snapToGrid({x, y}));
    if (!label.isEmpty()) {
        element->setLabel(label);
    }

    // Match the interactive drag-and-drop path (SceneDropHandler::handleDrop): clear the
    // previous selection before adding the new element so it alone ends up selected,
    // rather than accumulating onto every element ever created in this session. Without
    // this, a long MCP-driven build (many create_element calls with nothing else ever
    // clearing selection) leaves the entire circuit selected, and every selection-tracking
    // consumer (e.g. ElementEditor::selectionChanged) pays for the whole growing selection
    // on every single element added — O(N) per call, O(N^2) over the full build.
    scene->clearSelection();

    // Unreachable: scene->receiveCommand() with a well-formed AddItemsCommand never throws
    // (confirmed across the whole App/Scene/Commands.cpp sweep).
    try {
        scene->receiveCommand(new AddItemsCommand({element}, scene));
    } catch (const std::exception &e) { // LCOV_EXCL_LINE
        delete element; // LCOV_EXCL_LINE
        return createErrorResponse(QString("Failed to add element to scene: %1").arg(e.what()), // LCOV_EXCL_LINE
                                   requestId, JsonRpcError::OperationFailed); // LCOV_EXCL_LINE
    } catch (...) { // LCOV_EXCL_LINE
        delete element; // LCOV_EXCL_LINE
        return createErrorResponse("Failed to add element to scene: Unknown exception", // LCOV_EXCL_LINE
                                   requestId, JsonRpcError::OperationFailed); // LCOV_EXCL_LINE
    } // LCOV_EXCL_LINE

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
    // Unreachable: validatedElement() above already fails (returning ElementNotFound, with
    // this exact message) if there's no active scene, before this point can ever be reached.
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable); // LCOV_EXCL_LINE
    }

    return tryCommand([&] { // LCOV_EXCL_LINE -- pattern 45: gcov misattributes this multi-line lambda-taking call's entry; the lambda body below is genuinely covered
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
            elementObj["audio"] = element->audio(); // LCOV_EXCL_LINE -- hasAudio() is always false across every GraphicElement type, so this branch is dead
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

    const QPointF snapped = GraphicElement::snapToGrid({params.value("x").toDouble(), params.value("y").toDouble()});
    const int x = qRound(snapped.x());
    const int y = qRound(snapped.y());

    auto *element = validatedElement(params, "element_id", errorMsg);
    if (!element) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }

    Scene *scene = currentScene();
    // Unreachable: validatedElement() above already fails (returning ElementNotFound, with
    // this exact message) if there's no active scene, before this point can ever be reached.
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable); // LCOV_EXCL_LINE
    }

    QPointF oldPos = element->pos();
    QPointF newPos(x, y);

    element->setPos(newPos);

    QList<GraphicElement *> elements = {element};
    QList<QPointF> oldPositions = {oldPos};

    // Unreachable: scene->receiveCommand() with a well-formed MoveCommand never throws
    // (confirmed across the whole App/Scene/Commands.cpp sweep).
    try {
        scene->receiveCommand(new MoveCommand(elements, oldPositions, scene));
    } catch (const std::exception &e) { // LCOV_EXCL_LINE
        element->setPos(oldPos); // LCOV_EXCL_LINE
        return createErrorResponse(QString("Failed to move element: %1").arg(e.what()), // LCOV_EXCL_LINE
                                   requestId, JsonRpcError::OperationFailed); // LCOV_EXCL_LINE
    } catch (...) { // LCOV_EXCL_LINE
        element->setPos(oldPos); // LCOV_EXCL_LINE
        return createErrorResponse("Failed to move element: Unknown exception", // LCOV_EXCL_LINE
                                   requestId, JsonRpcError::OperationFailed); // LCOV_EXCL_LINE
    } // LCOV_EXCL_LINE

    QJsonObject result;
    result["old_position"] = QJsonObject{{"x", oldPos.x()}, {"y", oldPos.y()}};
    result["new_position"] = QJsonObject{{"x", newPos.x()}, {"y", newPos.y()}};

    return createSuccessResponse(result, requestId);
}

namespace {

/// Four-state name for a port status. `value` stays a bool for compatibility, but it cannot
/// distinguish Unknown or Error from logic 0 -- and the engine produces Unknown routinely, since
/// an oscillating region is canonicalised to it and propagated to every reader. Matches the
/// vocabulary the canvas already uses for wires (grey unknown, red error).
QString statusName(const Status status)
{
    switch (status) {
    case Status::Active:   return QStringLiteral("high");
    case Status::Inactive: return QStringLiteral("low");
    case Status::Error:    return QStringLiteral("error");
    default:               return QStringLiteral("unknown");
    }
}

} // namespace

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

    // Every value is checked before any is set, so a refused request changes nothing
    if (const QString refused = ElementHandlerProperties::refusal(*element, params, currentScene()); !refused.isEmpty()) {
        return createErrorResponse(refused, requestId, JsonRpcError::ValidationError);
    }

    const auto snapshot = [element] {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        element->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
        return data;
    };

    // Snapshot current state before any mutation so UpdateCommand can restore it on undo.
    const QByteArray oldData = snapshot();

    QJsonObject result;
    QJsonObject oldProperties;
    QJsonObject newProperties;

    // Each new value is read back from the element: the value it took
    if (params.contains("label")) {
        oldProperties["label"] = element->label();
        element->setLabel(params.value("label").toString());
        newProperties["label"] = element->label();
    }

    if (params.contains("color")) {
        oldProperties["color"] = element->color();
        element->setColor(params.value("color").toString());
        newProperties["color"] = element->color();
    }

    if (params.contains("frequency")) {
        oldProperties["frequency"] = element->frequency();
        element->setFrequency(params.value("frequency").toDouble());
        newProperties["frequency"] = element->frequency();
    }

    if (params.contains("rotation")) {
        oldProperties["rotation"] = element->rotation();
        element->setRotation(params.value("rotation").toDouble());
        newProperties["rotation"] = element->rotation();
    }

    if (params.contains("delay")) {
        oldProperties["delay"] = element->delay();
        element->setDelay(params.value("delay").toDouble());
        newProperties["delay"] = element->delay();
    }

    if (params.contains("trigger")) {
        // Stored uppercased, as the editor's field stores it
        oldProperties["trigger"] = element->trigger().toString();
        element->setTrigger(QKeySequence(params.value("trigger").toString().toUpper()));
        newProperties["trigger"] = element->trigger().toString();
    }

    // LCOV_EXCL_START — no element type has audio (hasAudio()), so refusal() refuses the property
    if (params.contains("audio")) {
        oldProperties["audio"] = element->audio();
        element->setAudio(params.value("audio").toString());
        newProperties["audio"] = element->audio();
    }
    // LCOV_EXCL_STOP

    if (params.contains("locked")) {
        if (auto *inputElm = qobject_cast<GraphicElementInput *>(element)) {
            oldProperties["locked"] = inputElm->isLocked();
            inputElm->setLocked(params.value("locked").toBool());
            newProperties["locked"] = inputElm->isLocked();
        }
    }

    if (params.contains("volume")) {
        oldProperties["volume"] = static_cast<double>(element->volume());
        element->setVolume(static_cast<float>(params.value("volume").toDouble()));
        newProperties["volume"] = static_cast<double>(element->volume());
    }

    // The appearance has no old value in the response: whether it changed is whether the element's state did
    bool appearanceChanged = false;
    if (params.contains("appearance")) {
        QString appearancePath = params.value("appearance").toString();
        bool useDefault = appearancePath.isEmpty();
        int appearanceIndex = params.contains("appearance_index") ? params.value("appearance_index").toInt() : -1;

        newProperties["appearance"] = appearancePath;
        newProperties["appearance_default"] = useDefault;

        const QByteArray beforeAppearance = snapshot();
        if (appearanceIndex >= 0) {
            // Set appearance for a specific index directly (e.g., LED state)
            element->setAppearanceAt(appearanceIndex, appearancePath);
            newProperties["appearance_index"] = appearanceIndex;
        } else {
            element->setAppearance(useDefault, appearancePath);
        }
        appearanceChanged = (snapshot() != beforeAppearance);
    }

    // Wireless mode is only meaningful for Node elements, which refusal() checked.
    QList<QGraphicsItem *> wirelessConnsToDelete;
    if (params.contains("wireless_mode")) {
        if (auto *node = qobject_cast<Node *>(element)) {
            const auto newMode = static_cast<WirelessMode>(params.value("wireless_mode").toInt());
            oldProperties["wireless_mode"] = static_cast<int>(node->wirelessMode());

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
            newProperties["wireless_mode"] = static_cast<int>(node->wirelessMode());
        }
    }

    // Push an undo command so Ctrl+Z in the GUI can revert MCP-applied property changes, unless no value changed (an
    // empty label set over a null one changes the saved bytes, not the label). When wireless mode changes sever
    // connections, group the property update and the delete into a single undo macro so both are undone together.
    QJsonObject compared = newProperties;
    compared.remove("appearance");
    compared.remove("appearance_default");
    compared.remove("appearance_index");
    if ((compared != oldProperties) || appearanceChanged || !wirelessConnsToDelete.isEmpty()) {
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
        // An input's own state, which drives its port once the simulation runs
        if (!validatePortRange(element, portIndex, true, "port", errorMsg)) {
            return createErrorResponse(errorMsg, requestId, JsonRpcError::ValidationError);
        }
        const bool on = inputElement->isOn(portIndex);
        result["value"] = on;
        result["status"] = statusName(on ? Status::Active : Status::Inactive);
    } else {
        ElementGroup group = element->elementGroup();

        if (group == ElementGroup::Output) {
            if (!validatePortRange(element, portIndex, false, "port", errorMsg)) {
                return createErrorResponse(errorMsg, requestId, JsonRpcError::ValidationError);
            }
            InputPort *inPort = element->inputPort(portIndex);
            result["value"] = inPort ? (inPort->status() == Status::Active) : false;
            result["status"] = statusName(inPort ? inPort->status() : Status::Unknown);
        } else {
            if (!validatePortRange(element, portIndex, true, "port", errorMsg)) {
                return createErrorResponse(errorMsg, requestId, JsonRpcError::ValidationError);
            }
            OutputPort *outPort = element->outputPort(portIndex);
            result["value"] = outPort ? (outPort->status() == Status::Active) : false;
            result["status"] = statusName(outPort ? outPort->status() : Status::Unknown);
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
    // Unreachable: validatedElement() above already fails (returning ElementNotFound, with
    // this exact message) if there's no active scene, before this point can ever be reached.
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable); // LCOV_EXCL_LINE
    }

    // Normalize angle to 0-360
    angle = ((angle % 360) + 360) % 360;

    return tryCommand([&]() -> QJsonObject { // LCOV_EXCL_LINE -- pattern 45: gcov misattributes this multi-line lambda-taking call's entry; the lambda body below is genuinely covered
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
    // Unreachable: validatedElement() above already fails (returning ElementNotFound, with
    // this exact message) if there's no active scene, before this point can ever be reached.
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable); // LCOV_EXCL_LINE
    }

    return tryCommand([&]() -> QJsonObject { // LCOV_EXCL_LINE -- pattern 45: gcov misattributes this multi-line lambda-taking call's entry; the lambda body below is genuinely covered
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
        return createErrorResponse(QString("Invalid input size %1. Must be between %2 and %3") // LCOV_EXCL_LINE -- pattern 8: gcov misattributes this multi-line chained-.arg() call's first line even though it's genuinely reached (see testHandleChangeInputSizeRejectsOutOfRangeSize)
                                   .arg(newSize).arg(element->minInputSize()).arg(element->maxInputSize()),
                                   requestId, JsonRpcError::ValidationError);
    }

    Scene *scene = currentScene();
    // Unreachable: validatedElement() above already fails (returning ElementNotFound, with
    // this exact message) if there's no active scene, before this point can ever be reached.
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable); // LCOV_EXCL_LINE
    }

    return tryCommand([&]() -> QJsonObject { // LCOV_EXCL_LINE -- pattern 45: gcov misattributes this multi-line lambda-taking call's entry; the lambda body below is genuinely covered
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
        return createErrorResponse(QString("Invalid output size %1. Must be between %2 and %3") // LCOV_EXCL_LINE -- pattern 8: gcov misattributes this multi-line chained-.arg() call's first line even though it's genuinely reached (see testHandleChangeOutputSizeRejectsOutOfRangeSize)
                                   .arg(newSize).arg(element->minOutputSize()).arg(element->maxOutputSize()),
                                   requestId, JsonRpcError::ValidationError);
    }

    Scene *scene = currentScene();
    // Unreachable: validatedElement() above already fails (returning ElementNotFound, with
    // this exact message) if there's no active scene, before this point can ever be reached.
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable); // LCOV_EXCL_LINE
    }

    return tryCommand([&]() -> QJsonObject { // LCOV_EXCL_LINE -- pattern 45: gcov misattributes this multi-line lambda-taking call's entry; the lambda body below is genuinely covered
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
        return createErrorResponse(QString("position %1 out of range: TruthTable %2 has %3 outputs (valid positions 0..%4)") // LCOV_EXCL_LINE -- pattern 8: gcov misattributes this multi-line chained-.arg() call's first line even though it's genuinely reached (see testHandleToggleTruthTableOutputRejectsOutOfRangePosition)
                                       .arg(position).arg(element->id()).arg(truthTable->outputSize()).arg(maxPosition - 1),
                                   requestId, JsonRpcError::ValidationError);
    }

    Scene *scene = currentScene();
    // Unreachable: validatedElement() above already fails (returning ElementNotFound, with
    // this exact message) if there's no active scene, before this point can ever be reached.
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable); // LCOV_EXCL_LINE
    }

    return tryCommand([&]() -> QJsonObject { // LCOV_EXCL_LINE -- pattern 45: gcov misattributes this multi-line lambda-taking call's entry; the lambda body below is genuinely covered
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
    // Unreachable: elementIdsArray is non-empty (checked above), so the loop above already
    // calls validateElementId() at least once, which fails first if there's no active scene.
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable); // LCOV_EXCL_LINE
    }

    // Capture IDs before morph — element pointers become invalid after receiveCommand.
    QJsonArray morphedIds;
    for (const auto *element : elementsToMorph) {
        morphedIds.append(element->id());
    }

    return tryCommand([&]() -> QJsonObject { // LCOV_EXCL_LINE -- pattern 45: gcov misattributes this multi-line lambda-taking call's entry; the lambda body below is genuinely covered
        scene->receiveCommand(new MorphCommand(elementsToMorph, targetType, scene));
        // MorphCommand preserves element IDs via updateItemId(), so the morphed
        // elements keep their original IDs.
        QJsonObject result;
        result["morphed_elements"] = morphedIds;
        result["target_type"] = typeStr;
        return createSuccessResponse(result, requestId);
    }, "morph elements", requestId);
}
