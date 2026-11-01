// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/ElementHandler.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

#include <QIODevice>
#include <QJsonArray>
#include <QPixmap>
#include <QRegularExpression>
#include <QScopeGuard>

#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInput.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace ElementHandlerProperties {

/// A property MCP reads and writes through the element, under its key in schema-mcp.json.
struct KeyedProperty {
    const char *key;
    ElementProperty property;
};

/// The properties list_elements reports and set_element_properties sets, each where it applies to the element
/// (SetPropertyCommand::appliesTo()): the label to every element, a parameter where the element offers it.
const QList<KeyedProperty> &keyedProperties()
{
    static const QList<KeyedProperty> properties{
        {"label", ElementProperty::label()},
        {"color", ElementProperty::parameter(ParamId::Colour)},
        {"frequency", ElementProperty::parameter(ParamId::Frequency)},
        {"delay", ElementProperty::parameter(ParamId::Delay)},
        {"trigger", ElementProperty::trigger()},
        {"volume", ElementProperty::parameter(ParamId::Volume)},
        {"locked", ElementProperty::parameter(ParamId::Locked)},
        {"wireless_mode", ElementProperty::parameter(ParamId::Wireless)},
    };
    return properties;
}

/// Returns \a value as JSON; no keyed property holds a truth table's columns.
QJsonValue toJson(const ParamValue &value)
{
    return std::visit([](const auto &held) {
        if constexpr (std::is_same_v<std::decay_t<decltype(held)>, QBitArray>) {
            return QJsonValue(); // LCOV_EXCL_LINE — no keyed property is a truth table's columns
        } else {
            return QJsonValue(held);
        }
    }, value);
}

/// Returns \a json as a value of the kind of \a kind, or nothing when it is no such value: a whole number for an int,
/// a finite one for a double.
std::optional<ParamValue> fromJson(const QJsonValue &json, const ParamValue &kind)
{
    return std::visit([&json](const auto &held) -> std::optional<ParamValue> {
        using Kind = std::decay_t<decltype(held)>;
        if constexpr (std::is_same_v<Kind, bool>) {
            if (json.isBool()) {
                return json.toBool();
            }
        } else if constexpr (std::is_same_v<Kind, int>) {
            constexpr qint64 notWhole = std::numeric_limits<qint64>::min();
            if (const qint64 whole = json.toInteger(notWhole);
                json.isDouble() && (whole >= std::numeric_limits<int>::min()) && (whole <= std::numeric_limits<int>::max())) {
                return static_cast<int>(whole);
            }
        } else if constexpr (std::is_same_v<Kind, double>) {
            if (json.isDouble() && std::isfinite(json.toDouble())) {
                return json.toDouble();
            }
        } else if constexpr (std::is_same_v<Kind, QString>) {
            if (json.isString()) {
                return json.toString();
            }
        }
        return std::nullopt;
    }, kind);
}

/// Returns what a value of the kind of \a kind is, for a refusal.
QString kindName(const ParamValue &kind)
{
    return std::visit([](const auto &held) {
        using Kind = std::decay_t<decltype(held)>;
        if constexpr (std::is_same_v<Kind, bool>) {
            return QStringLiteral("a boolean");
        } else if constexpr (std::is_same_v<Kind, int>) {
            return QStringLiteral("a whole number");
        } else if constexpr (std::is_same_v<Kind, double>) {
            return QStringLiteral("a finite number");
        } else {
            return QStringLiteral("a string");
        }
    }, kind);
}

/// Returns \a value as a number, or nothing when it is of another kind.
std::optional<double> numberOf(const ParamValue &value)
{
    if (const int *whole = std::get_if<int>(&value)) {
        return *whole;
    }
    if (const double *real = std::get_if<double>(&value)) {
        return *real;
    }
    return std::nullopt;
}

/// Returns \a value as text, for a refusal: a number as itself, a boolean as true or false, a text quoted.
QString valueText(const ParamValue &value)
{
    const QJsonValue json = toJson(value);
    return json.isDouble() ? QString::number(json.toDouble())
           : json.isBool() ? QString(json.toBool() ? "true" : "false")
                           : QString("\"%1\"").arg(json.toString());
}

/// Returns \a range as text, an open bound as an infinity.
QString rangeText(const ParamRange &range)
{
    return QString("[%1, %2]").arg(range.minimum ? QString::number(*range.minimum) : QStringLiteral("-inf"),
                                   range.maximum ? QString::number(*range.maximum) : QStringLiteral("inf"));
}

/// Returns the first reason set_element_properties refuses \a params for \a element, or an empty string when every
/// value passes: each keyed property applies to the element, its value has the property's kind and is one its spec
/// takes (within the range, among the choices, one \c accept takes); a trigger is one key, as the editor's field
/// takes it; an appearance index comes with an appearance and names one of the element's states, and a picture loads.
/// The rule that a wireless channel has one Tx is the property command's (SetPropertyCommand::apply()).
QString refusal(const Element &element, const QJsonObject &params)
{
    const auto lacks = [&element](const QString &property) {
        return QString("Element %1 (%2) has no property '%3'")
            .arg(QString::number(element.id()), ElementFactory::typeToText(element.elementType()), property);
    };
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

    for (const KeyedProperty &keyed : keyedProperties()) {
        const QString key = QString::fromLatin1(keyed.key);
        if (!params.contains(key)) {
            continue;
        }
        if (!SetPropertyCommand::appliesTo(element, keyed.property)) {
            return lacks(key);
        }
        const ParamValue current = SetPropertyCommand::read(element, keyed.property);
        const std::optional<ParamValue> value = fromJson(params.value(key), current);
        if (!value) {
            return kind(key, kindName(current));
        }
        // The editor's field takes one letter, digit or space, or none
        static const QRegularExpression editorKey("^[A-Za-z0-9 ]?$");
        if ((keyed.property == ElementProperty::trigger()) && !editorKey.match(std::get<QString>(*value)).hasMatch()) {
            return kind(key, "one letter, digit or space, or empty");
        }
        if (keyed.property.kind != ElementProperty::Kind::Parameter) {
            continue;
        }
        const ParamSpec &spec = *element.parameters().spec(keyed.property.id);
        if (const std::optional<double> numeric = numberOf(*value); numeric && !spec.range.contains(*numeric)) {
            return kind(key, "in " + rangeText(spec.range));
        }
        if (!spec.choices.isEmpty() && !spec.choices.contains(std::get<QString>(*value))) {
            return kind(key, "one of " + spec.choices.join(", "));
        }
        if (spec.accept && !spec.accept(element, *value)) {
            return QString("Parameter '%1' %2 is not a value this element takes").arg(key, valueText(*value));
        }
    }

    // Every element takes a rotation
    if (params.contains("rotation") && !number("rotation")) {
        return kind("rotation", "a finite number");
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

    Element *element = nullptr;
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

    element->setPos(Element::snapToGrid({x, y}));
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

    // Unreachable: scene->pushCommand() with a well-formed AddItemsCommand never throws
    // (confirmed across the whole App/Scene/Commands.cpp sweep).
    try {
        scene->pushCommand(new AddItemsCommand({element}, scene));
    } catch (const std::exception &e) { // LCOV_EXCL_LINE
        delete element; // LCOV_EXCL_LINE
        return createErrorResponse(QString("Failed to add element to scene: %1").arg(e.what()), // LCOV_EXCL_LINE
                                   requestId, JsonRpcError::OperationFailed); // LCOV_EXCL_LINE
    } catch (...) { // LCOV_EXCL_LINE
        delete element; // LCOV_EXCL_LINE
        return createErrorResponse("Failed to add element to scene: Unknown exception", // LCOV_EXCL_LINE
                                   requestId, JsonRpcError::OperationFailed); // LCOV_EXCL_LINE
    } // LCOV_EXCL_LINE

    const QRectF bounds = element->outlineRect();

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
        scene->pushCommand(new DeleteItemsCommand({element}, scene));
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
        const QRectF br = element->outlineRect();
        QJsonObject elementObj;
        elementObj["element_id"] = element->id();
        elementObj["type"] = ElementFactory::typeToText(element->elementType());
        elementObj["x"] = element->pos().x();
        elementObj["y"] = element->pos().y();
        elementObj["width"] = br.width();
        elementObj["height"] = br.height();
        elementObj["rotation"] = element->rotation();
        elementObj["input_size"] = element->inputSize();
        elementObj["output_size"] = element->outputSize();

        for (const auto &keyed : ElementHandlerProperties::keyedProperties()) {
            if (SetPropertyCommand::appliesTo(*element, keyed.property)) {
                elementObj[keyed.key] = ElementHandlerProperties::toJson(SetPropertyCommand::read(*element, keyed.property));
            }
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

    const QPointF snapped = Element::snapToGrid({params.value("x").toDouble(), params.value("y").toDouble()});
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

    QList<Element *> elements = {element};
    QList<QPointF> oldPositions = {oldPos};

    // Unreachable: scene->pushCommand() with a well-formed MoveCommand never throws
    // (confirmed across the whole App/Scene/Commands.cpp sweep).
    try {
        scene->pushCommand(new MoveCommand(elements, oldPositions, scene));
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
    if (const QString refused = ElementHandlerProperties::refusal(*element, params); !refused.isEmpty()) {
        return createErrorResponse(refused, requestId, JsonRpcError::ValidationError);
    }
    Scene *scene = currentScene();

    // The keyed properties, through the property command, which holds the rule that a wireless channel has one Tx
    QJsonObject oldProperties;
    QJsonObject newProperties;
    QList<PropertyEdit> edits;
    QList<const char *> keys; ///< The key of each edit.
    for (const auto &keyed : ElementHandlerProperties::keyedProperties()) {
        if (!params.contains(QLatin1String(keyed.key))) {
            continue;
        }
        const ParamValue current = SetPropertyCommand::read(*element, keyed.property);
        oldProperties[keyed.key] = ElementHandlerProperties::toJson(current);
        // refusal() took the value as of the property's kind; a trigger's text is set as the editor's field sets it
        edits.append({element, keyed.property,
                      *ElementHandlerProperties::fromJson(params.value(QLatin1String(keyed.key)), current)});
        keys.append(keyed.key);
    }

    // A Tx takes no wire on its output and an Rx none on its input: setting the mode hides the port, and a
    // DeleteItemsCommand deletes its wires, so undo restores them
    QList<QGraphicsItem *> severed;
    if (params.contains("wireless_mode")) {
        const auto newMode = static_cast<WirelessMode>(params.value("wireless_mode").toInt());
        Port *port = (newMode == WirelessMode::Rx)   ? static_cast<Port *>(element->inputPort())
                     : (newMode == WirelessMode::Tx) ? static_cast<Port *>(element->outputPort())
                                                     : nullptr;
        if (port) {
            for (auto *conn : port->connections()) {
                severed.append(conn);
            }
        }
    }

    // A request with a rotation or an appearance, which are no properties of the command, is one snapshot of the
    // element, taken before anything is set: its one command, which drops whole when the element's snapshot no longer
    // reads (a file-backed IC whose file changed)
    const auto snapshot = [element] {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        element->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
        return data;
    };
    const bool snapshotted = params.contains("rotation") || params.contains("appearance");
    const QByteArray oldData = snapshotted ? snapshot() : QByteArray();

    auto [propertyCommand, duplicate] = SetPropertyCommand::apply(edits, scene);
    if (!duplicate.isEmpty()) {
        return createErrorResponse(
            QString("A Tx node with label \"%1\" already exists: each wireless channel has one").arg(duplicate),
            requestId, JsonRpcError::ValidationError);
    }
    // Each new value is read back from the element: the value it took
    for (qsizetype i = 0; i < edits.size(); ++i) {
        newProperties[keys.at(i)] =
            ElementHandlerProperties::toJson(SetPropertyCommand::read(*element, edits.at(i).property));
    }

    if (params.contains("rotation")) {
        oldProperties["rotation"] = element->rotation();
        element->setRotation(params.value("rotation").toDouble());
        newProperties["rotation"] = element->rotation();
    }
    // The appearance has no old value in the response: whether it changed is whether the element's state did
    if (params.contains("appearance")) {
        const QString appearancePath = params.value("appearance").toString();
        newProperties["appearance"] = appearancePath;
        newProperties["appearance_default"] = appearancePath.isEmpty();
        if (params.contains("appearance_index")) {
            // Set appearance for a specific index directly (e.g., LED state)
            const int appearanceIndex = params.value("appearance_index").toInt();
            element->setAppearanceAt(appearanceIndex, appearancePath);
            newProperties["appearance_index"] = appearanceIndex;
        } else {
            element->setAppearance(appearancePath.isEmpty(), appearancePath);
        }
    }

    // One undo step: the properties' command, or the snapshot's, and the wires a mode severs. An unchanged snapshot
    // (an empty label set over a null one changes its bytes, not the label) pushes nothing
    QUndoCommand *change = propertyCommand;
    if (snapshotted) {
        delete std::exchange(propertyCommand, nullptr);
        change = (snapshot() != oldData) ? new UpdateCommand({element}, oldData, scene) : nullptr;
    }
    QUndoCommand *deletion = severed.isEmpty() ? nullptr : new DeleteItemsCommand(severed, scene);
    const bool needsMacro = change && deletion;
    if (needsMacro) {
        scene->undoStack()->beginMacro(QStringLiteral("Change wireless mode"));
    }
    // A push that drops throws: the macro is closed all the same, or every later edit would join it, and the deletion
    // not pushed is deleted
    const auto closeMacro = qScopeGuard([scene, needsMacro] {
        if (needsMacro) {
            scene->undoStack()->endMacro();
        }
    });
    if (change) {
        auto dropDeletion = qScopeGuard([deletion] { delete deletion; });
        scene->pushCommand(change);
        dropDeletion.dismiss();
    }
    if (deletion) {
        scene->pushCommand(deletion);
    }

    QJsonObject result;
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

    auto *inputElement = dynamic_cast<ElementInput *>(element);
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

    auto *inputElement = dynamic_cast<ElementInput *>(element);
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
        scene->pushCommand(new RotateCommand({element}, angle, scene));
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
        scene->pushCommand(new FlipCommand({element}, axis, scene));
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
        scene->pushCommand(new ChangePortSizeCommand({element}, newSize, scene, true));
        const QRectF bounds = element->outlineRect();
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
        scene->pushCommand(new ChangePortSizeCommand({element}, newSize, scene, false));
        const QRectF bounds = element->outlineRect();
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
        scene->pushCommand(new ToggleTruthTableOutputCommand(truthTable, position, scene));
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
    QList<Element *> elementsToMorph;
    for (const QJsonValue &idValue : std::as_const(elementIdsArray)) {
        if (!idValue.isDouble()) {
            return createErrorResponse("element_ids must contain only integers", requestId, JsonRpcError::InvalidParams);
        }

        int elementId = idValue.toInt();
        if (!validateElementId(elementId, "element_ids", errorMsg)) {
            return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
        }

        auto *item = currentScene()->itemById(elementId);
        auto *element = dynamic_cast<Element *>(item);
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

    // Capture IDs before morph — element pointers become invalid after pushCommand.
    QJsonArray morphedIds;
    for (const auto *element : elementsToMorph) {
        morphedIds.append(element->id());
    }

    return tryCommand([&]() -> QJsonObject { // LCOV_EXCL_LINE -- pattern 45: gcov misattributes this multi-line lambda-taking call's entry; the lambda body below is genuinely covered
        scene->pushCommand(new MorphCommand(elementsToMorph, targetType, scene));
        // MorphCommand preserves element IDs via updateItemId(), so the morphed
        // elements keep their original IDs.
        QJsonObject result;
        result["morphed_elements"] = morphedIds;
        result["target_type"] = typeStr;
        return createSuccessResponse(result, requestId);
    }, "morph elements", requestId);
}
