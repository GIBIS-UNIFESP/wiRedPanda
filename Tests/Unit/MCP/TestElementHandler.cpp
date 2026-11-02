// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/MCP/TestElementHandler.h"

#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSizeF>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTest>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementTypeDescriptor.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Commands.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MainWindow.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "MCP/Server/Core/JsonRpcError.h"
#include "MCP/Server/Core/MCPValidator.h"
#include "MCP/Server/Handlers/ElementHandler.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

namespace {

int createClock(ElementHandler &handler)
{
    const QJsonObject params{{"type", "Clock"}, {"x", 0}, {"y", 0}};
    const QJsonObject response = handler.handleCommand("create_element", params, {});
    return response["result"].toObject()["element_id"].toInt();
}

int createElement(ElementHandler &handler, const QString &type, double x = 0, double y = 0)
{
    const QJsonObject params{{"type", type}, {"x", x}, {"y", y}};
    const QJsonObject response = handler.handleCommand("create_element", params, {});
    return response["result"].toObject()["element_id"].toInt();
}

Element *elementById(MainWindow &window, int id)
{
    return dynamic_cast<Element *>(window.currentTab()->scene()->itemById(id));
}

// `value` with every width and height replaced by "font", for a golden that holds a Text element: its size is its
// text's, which follows the font.
QJsonValue maskFontSizes(const QJsonValue &value)
{
    if (value.isArray()) {
        QJsonArray array;
        for (const auto &item : value.toArray()) {
            array.append(maskFontSizes(item));
        }
        return array;
    }
    if (!value.isObject()) {
        return value;
    }
    QJsonObject object = value.toObject();
    for (auto it = object.begin(); it != object.end(); ++it) {
        const bool size = (it.key() == QLatin1String("width")) || (it.key() == QLatin1String("height"));
        it.value() = size ? QJsonValue("font") : maskFontSizes(it.value());
    }
    return object;
}

/// Builds a file-backed IC on \a scene whose file then changes to bytes that do not read, its cache dropped, so a
/// snapshot of it no longer reads.
IC *icWithABrokenFile(Scene *scene, const QTemporaryDir &dir)
{
    const QString path = dir.filePath("sub.panda");
    {
        WorkSpace sub;
        sub.scene()->addItem(new InputSwitch());
        sub.scene()->addItem(ElementFactory::buildElement(ElementType::Led));
        sub.save(path);
    }
    auto *ic = new IC();
    scene->addItem(ic);
    ic->loadFile(path, dir.path());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return nullptr;
    }
    file.write("not a circuit");
    file.close();
    scene->icRegistry()->invalidate(path);
    return ic;
}

} // namespace

namespace TestElementHandlerHelpers {

// The element's whole state, as an undo snapshot holds it
QByteArray snapshot(const Element *element)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    element->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    return data;
}

// Whether set_element_properties with `params` answers a validation error and leaves the element and the undo stack
// as they were
bool refuses(MainWindow &window, ElementHandler &handler, const QJsonObject &params)
{
    const Element *element = elementById(window, params["element_id"].toInt());
    const QByteArray before = snapshot(element);
    const int commands = window.currentTab()->scene()->undoStack()->count();
    const QJsonObject response = handler.handleCommand("set_element_properties", params, 1);
    if (!response.contains("error") || (response["error"].toObject()["code"].toInt() != JsonRpcError::ValidationError)) {
        qWarning() << "Not refused:" << QJsonDocument(response).toJson(QJsonDocument::Compact);
        return false;
    }
    return (snapshot(element) == before) && (window.currentTab()->scene()->undoStack()->count() == commands);
}

} // namespace TestElementHandlerHelpers

using TestElementHandlerHelpers::refuses;
using TestElementHandlerHelpers::snapshot;

void TestElementHandler::testSetElementPropertiesAcceptsNegativeClockDelay()
{
    // Regression: the clock's delay spec clamps to [-1, 1] (negative delays advance the clock —
    // see Clock.cpp), but this handler's validation only ever accepted delay >= 0,
    // so a legitimate negative phase delay was rejected outright via the MCP path even
    // though the GUI property editor and a direct C++ call both accept it.
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const int clockId = createClock(handler);
    QVERIFY(clockId != 0);

    const QJsonObject params{{"element_id", clockId}, {"delay", -0.25}};
    const QJsonObject response = handler.handleCommand("set_element_properties", params, {});
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));

    auto *element = dynamic_cast<Element *>(window.currentTab()->scene()->itemById(clockId));
    QVERIFY(element);
    QCOMPARE(element->delay(), -0.25);
}

void TestElementHandler::testSetElementPropertiesAnswersAPushThatDropsWithAnError()
{
    // A rotation is a snapshot of the element, which a file-backed IC whose file no longer reads cannot load: the push
    // drops and the handler throws, which the processor answers as an error (MCPProcessor::processCommand()), the undo
    // stack as it was, the label sent with it included
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const IC *ic = icWithABrokenFile(scene, dir);
    QVERIFY(ic);
    const int commands = scene->undoStack()->count();

    const QJsonObject params{{"element_id", ic->id()}, {"label", "renamed"}, {"rotation", 90}};
    QVERIFY_THROWS_EXCEPTION(std::exception, handler.handleCommand("set_element_properties", params, {}));
    QCOMPARE(scene->undoStack()->count(), commands);
}

void TestElementHandler::testSetElementPropertiesRelabelsAnICWithoutReadingItsFile()
{
    // A label alone is the property command's, which holds the values, not a snapshot: no file is read
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const IC *ic = icWithABrokenFile(scene, dir);
    QVERIFY(ic);
    const int commands = scene->undoStack()->count();
    const QString label = ic->label();

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", ic->id()}, {"label", "renamed"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(scene->undoStack()->count(), commands + 1);
    QCOMPARE(ic->label(), QString("renamed"));
    scene->undoStack()->undo();
    QCOMPARE(ic->label(), label);
}

void TestElementHandler::testSetElementPropertiesRejectsClockDelayOutOfRange()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const int clockId = createClock(handler);
    QVERIFY(clockId != 0);

    const QJsonObject tooHigh{{"element_id", clockId}, {"delay", 1.5}};
    QVERIFY2(handler.handleCommand("set_element_properties", tooHigh, {}).contains("error"),
             "delay=1.5 is outside the clock's delay range, [-1, 1], and must be rejected");

    const QJsonObject tooLow{{"element_id", clockId}, {"delay", -1.5}};
    QVERIFY2(handler.handleCommand("set_element_properties", tooLow, {}).contains("error"),
             "delay=-1.5 is outside the clock's delay range, [-1, 1], and must be rejected");
}

// ============================================================================
// create_element
// ============================================================================

void TestElementHandler::testHandleCreateElementRejectsMissingParams()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("create_element", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleCreateElementRejectsEmptyType()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("create_element", {{"type", ""}, {"x", 0}, {"y", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleCreateElementRejectsNonNumericX()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("create_element", {{"type", "And"}, {"x", "bad"}, {"y", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleCreateElementRejectsNonNumericY()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("create_element", {{"type", "And"}, {"x", 0}, {"y", "bad"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleCreateElementRejectsInvalidType()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("create_element", {{"type", "NotARealType"}, {"x", 0}, {"y", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestElementHandler::testHandleCreateElementRejectsNoScene()
{
    MainWindow window;
    auto *tabs = window.findChild<QTabWidget *>();
    QVERIFY(tabs);
    emit tabs->tabCloseRequested(0);
    QVERIFY(!window.currentTab());

    ElementHandler handler(&window, nullptr);
    const QJsonObject response = handler.handleCommand("create_element", {{"type", "And"}, {"x", 0}, {"y", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestElementHandler::testHandleCreateElementCreatesRealElement()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("create_element", {{"type", "And"}, {"x", 10}, {"y", 20}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const int id = response["result"].toObject()["element_id"].toInt();
    QVERIFY(id != 0);
    QVERIFY(response["result"].toObject()["width"].toDouble() > 0);

    auto *element = elementById(window, id);
    QVERIFY(element);
    // Placed on the half grid, as every edit that decides a place puts an element
    QCOMPARE(element->pos(), QPointF(8, 24));
}

void TestElementHandler::testHandleCreateElementAppliesLabel()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("create_element", {{"type", "And"}, {"x", 0}, {"y", 0}, {"label", "MyGate"}}, 1);
    const int id = response["result"].toObject()["element_id"].toInt();
    QCOMPARE(elementById(window, id)->label(), QStringLiteral("MyGate"));
}

// ============================================================================
// delete_element
// ============================================================================

void TestElementHandler::testHandleDeleteElementRejectsMissingParams()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("delete_element", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleDeleteElementRejectsUnknownElement()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("delete_element", {{"element_id", 999999}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestElementHandler::testHandleDeleteElementDeletesRealElement()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");
    QVERIFY(elementById(window, id));

    const QJsonObject response = handler.handleCommand("delete_element", {{"element_id", id}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(!window.currentTab()->scene()->itemById(id));
}

// ============================================================================
// list_elements
// ============================================================================

void TestElementHandler::testHandleListElementsRejectsNoScene()
{
    MainWindow window;
    auto *tabs = window.findChild<QTabWidget *>();
    QVERIFY(tabs);
    emit tabs->tabCloseRequested(0);
    QVERIFY(!window.currentTab());

    ElementHandler handler(&window, nullptr);
    const QJsonObject response = handler.handleCommand("list_elements", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestElementHandler::testHandleListElementsReturnsRealElements()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    createElement(handler, "And", 5, 5);
    const int clockId = createClock(handler);
    createElement(handler, "InputSwitch"); // hasTrigger, an input ("locked")
    createElement(handler, "Led", 50, 50); // hasColors
    createElement(handler, "Buzzer", 100, 100); // hasVolume

    const QJsonObject response = handler.handleCommand("list_elements", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonArray elements = response["result"].toObject()["elements"].toArray();
    QCOMPARE(elements.size(), 5);

    bool foundClockWithFrequencyAndDelay = false;
    bool foundInputSwitchWithLockedAndTrigger = false;
    bool foundLedWithColor = false;
    bool foundBuzzerWithVolume = false;
    for (const auto &v : elements) {
        const QJsonObject obj = v.toObject();
        QVERIFY(obj.contains("element_id"));
        QVERIFY(obj.contains("type"));
        if (obj["element_id"].toInt() == clockId) {
            QVERIFY(obj.contains("frequency"));
            QVERIFY(obj.contains("delay"));
            foundClockWithFrequencyAndDelay = true;
        }
        if (obj["type"].toString() == "InputSwitch") {
            QVERIFY(obj.contains("locked"));
            QVERIFY(obj.contains("trigger"));
            foundInputSwitchWithLockedAndTrigger = true;
        }
        if (obj["type"].toString() == "Led") {
            QVERIFY(obj.contains("color"));
            foundLedWithColor = true;
        }
        if (obj["type"].toString() == "Buzzer") {
            QVERIFY(obj.contains("volume"));
            foundBuzzerWithVolume = true;
        }
    }
    QVERIFY(foundClockWithFrequencyAndDelay);
    QVERIFY(foundInputSwitchWithLockedAndTrigger);
    QVERIFY(foundLedWithColor);
    QVERIFY(foundBuzzerWithVolume);
}

void TestElementHandler::testHandleListElementsReportsAWirelessNodesOutline()
{
    // A wireless node's indicator is drawn beyond its outline, and the size reported is the outline's, a plain node's
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int plainId = createElement(handler, "Node");
    const int wirelessId = createElement(handler, "Node", 96, 96);
    const QJsonObject set = handler.handleCommand("set_element_properties", {{"element_id", wirelessId}, {"wireless_mode", 1}}, 1);
    QVERIFY2(set.contains("result"), qPrintable(QJsonDocument(set).toJson()));

    const QJsonObject response = handler.handleCommand("list_elements", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QHash<int, QSizeF> sizes;
    for (const auto &value : response["result"].toObject()["elements"].toArray()) {
        const QJsonObject element = value.toObject();
        sizes.insert(element["element_id"].toInt(), QSizeF(element["width"].toDouble(), element["height"].toDouble()));
    }
    QCOMPARE(sizes.value(wirelessId), sizes.value(plainId));
    QCOMPARE(sizes.value(wirelessId), elementById(window, wirelessId)->outlineRect().size());
}

// ============================================================================
// move_element
// ============================================================================

void TestElementHandler::testHandleMoveElementRejectsMissingParams()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("move_element", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleMoveElementRejectsInvalidCoordinates()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("move_element", {{"element_id", id}, {"x", "bad"}, {"y", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleMoveElementRejectsNonNumericY()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("move_element", {{"element_id", id}, {"x", 0}, {"y", "bad"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleMoveElementRejectsUnknownElement()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("move_element", {{"element_id", 999999}, {"x", 0}, {"y", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestElementHandler::testHandleMoveElementMovesRealElementSnappedToGrid()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And", 0, 0);

    // gridSize is 16, snap unit is gridSize/2 == 8: 103 -> round(12.875)*8 == 104, 97 -> round(12.125)*8 == 96.
    const QJsonObject response = handler.handleCommand("move_element", {{"element_id", id}, {"x", 103}, {"y", 97}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject newPos = response["result"].toObject()["new_position"].toObject();
    QCOMPARE(newPos["x"].toDouble(), 104.0);
    QCOMPARE(newPos["y"].toDouble(), 96.0);

    auto *element = elementById(window, id);
    QCOMPARE(element->pos(), QPointF(104, 96));
}

// ============================================================================
// set_element_properties
// ============================================================================

void TestElementHandler::testHandleSetElementPropertiesRejectsMissingElementId()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("set_element_properties", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleSetElementPropertiesRejectsUnknownElement()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", 999999}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestElementHandler::testHandleSetElementPropertiesRejectsPortSizeParams()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", id}, {"input_size", 3}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestElementHandler::testGetOutputValueReportsFourStateStatus()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");
    auto *element = elementById(window, id);
    QVERIFY(element);

    const auto statusFor = [&handler, id](int) {
        const QJsonObject response =
            handler.handleCommand("get_output_value", {{"element_id", id}, {"port", 0}}, 1);
        return response["result"].toObject();
    };

    element->outputPort(0)->setStatus(Status::Active);
    QCOMPARE(statusFor(0)["status"].toString(), QStringLiteral("high"));
    QCOMPARE(statusFor(0)["value"].toBool(), true);

    element->outputPort(0)->setStatus(Status::Inactive);
    QCOMPARE(statusFor(0)["status"].toString(), QStringLiteral("low"));

    // The cases `value` alone cannot express: both report false, indistinguishable from a
    // genuine logic low.
    element->outputPort(0)->setStatus(Status::Unknown);
    QCOMPARE(statusFor(0)["status"].toString(), QStringLiteral("unknown"));
    QCOMPARE(statusFor(0)["value"].toBool(), false);

    element->outputPort(0)->setStatus(Status::Error);
    QCOMPARE(statusFor(0)["status"].toString(), QStringLiteral("error"));
    QCOMPARE(statusFor(0)["value"].toBool(), false);
}

void TestElementHandler::testHandleSetElementPropertiesChangesLabel()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", id}, {"label", "NewLabel"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["new_properties"].toObject()["label"].toString(), QStringLiteral("NewLabel"));
    QCOMPARE(elementById(window, id)->label(), QStringLiteral("NewLabel"));
}

void TestElementHandler::testHandleSetElementPropertiesChangesColorWhenSupported()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "Led"); // hasColors

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", id}, {"color", "Blue"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(elementById(window, id)->color(), QStringLiteral("Blue"));
}

void TestElementHandler::testHandleSetElementPropertiesChangesFrequencyWhenSupported()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createClock(handler); // hasFrequency

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", id}, {"frequency", 2.5}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(elementById(window, id)->frequency(), 2.5);
}

void TestElementHandler::testHandleSetElementPropertiesRejectsInvalidFrequency()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createClock(handler);

    QCOMPARE(handler.handleCommand("set_element_properties", {{"element_id", id}, {"frequency", "bad"}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
    QCOMPARE(handler.handleCommand("set_element_properties", {{"element_id", id}, {"frequency", 0}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
    QCOMPARE(handler.handleCommand("set_element_properties", {{"element_id", id}, {"frequency", -1}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
}

void TestElementHandler::testHandleSetElementPropertiesRejectsNonNumericDelay()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createClock(handler); // hasDelay

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", id}, {"delay", "bad"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestElementHandler::testHandleSetElementPropertiesChangesRotation()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", id}, {"rotation", 90}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(elementById(window, id)->rotation(), 90.0);
}

void TestElementHandler::testHandleSetElementPropertiesChangesTriggerWhenSupported()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "InputSwitch"); // hasTrigger

    // A key the editor's field takes, stored as the field stores it: a key sequence names a letter's key uppercased
    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", id}, {"trigger", "k"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(elementById(window, id)->trigger(), QKeySequence("K"));
    QCOMPARE(response["result"].toObject()["new_properties"].toObject()["trigger"].toString(), QString("K"));
}

void TestElementHandler::testHandleSetElementPropertiesChangesLockedState()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "InputSwitch");
    auto *inputElm = elementById(window, id);
    QVERIFY(inputElm);
    QCOMPARE(inputElm->elementGroup(), ElementGroup::Input);
    QVERIFY(!inputElm->isLocked());

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", id}, {"locked", true}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(inputElm->isLocked());
}

void TestElementHandler::testHandleSetElementPropertiesChangesVolumeWhenSupported()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "Buzzer"); // hasVolume

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", id}, {"volume", 0.5}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(elementById(window, id)->volume(), 0.5);
}

void TestElementHandler::testHandleSetElementPropertiesRejectsInvalidVolume()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "Buzzer");

    QCOMPARE(handler.handleCommand("set_element_properties", {{"element_id", id}, {"volume", "bad"}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
    QCOMPARE(handler.handleCommand("set_element_properties", {{"element_id", id}, {"volume", -0.1}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
    QCOMPARE(handler.handleCommand("set_element_properties", {{"element_id", id}, {"volume", 1.5}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
}

void TestElementHandler::testHandleSetElementPropertiesChangesAppearance()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And"); // canChangeAppearance

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", id}, {"appearance", ""}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["new_properties"].toObject()["appearance_default"].toBool(), true);
}

void TestElementHandler::testHandleSetElementPropertiesChangesAppearanceAtIndex()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("set_element_properties",
        {{"element_id", id}, {"appearance", ""}, {"appearance_index", 0}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["new_properties"].toObject()["appearance_index"].toInt(), 0);
}

void TestElementHandler::testHandleSetElementPropertiesChangesWirelessModeAndSeversConnections()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();

    const int nodeId = createElement(handler, "Node");
    auto *node = elementById(window, nodeId);
    QVERIFY(node);
    QCOMPARE(node->elementType(), ElementType::Node);

    const int andId = createElement(handler, "And", 100, 100);
    auto *andGate = elementById(window, andId);

    auto connection = std::make_unique<Connection>();
    connection->setStartPort(node->outputPort());
    connection->setEndPort(andGate->inputPort(0));
    scene->receiveCommand(new AddItemsCommand({connection.release()}, scene));
    QCOMPARE(node->outputPort()->connections().size(), 1);

    // Tx mode means the Node broadcasts its output wirelessly -- any wired connection on
    // that now-redundant output port must be severed (Rx would sever the *input* port instead).
    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", nodeId}, {"wireless_mode", 1}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(node->wirelessMode(), WirelessMode::Tx);
    QCOMPARE(node->outputPort()->connections().size(), 0);

    // The mode and the wire it severed are one undo step
    scene->undoStack()->undo();
    QCOMPARE(node->wirelessMode(), WirelessMode::None);
    QCOMPARE(node->outputPort()->connections().size(), 1);
}

void TestElementHandler::testHandleSetElementPropertiesRxModeSeversInputConnection()
{
    // The other half of the ternary this file's Tx test exercises: Rx mode means the Node
    // receives wirelessly, so a wired connection on its now-redundant *input* port must be
    // severed (mirrors the Tx test, which severs the output port instead).
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();

    const int nodeId = createElement(handler, "Node");
    auto *node = elementById(window, nodeId);
    QVERIFY(node);
    QCOMPARE(node->elementType(), ElementType::Node);

    const int swId = createElement(handler, "InputSwitch", 100, 100);
    auto *sw = elementById(window, swId);

    auto connection = std::make_unique<Connection>();
    connection->setStartPort(sw->outputPort());
    connection->setEndPort(node->inputPort());
    scene->receiveCommand(new AddItemsCommand({connection.release()}, scene));
    QCOMPARE(node->inputPort()->connections().size(), 1);

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", nodeId}, {"wireless_mode", 2}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(node->wirelessMode(), WirelessMode::Rx);
    QCOMPARE(node->inputPort()->connections().size(), 0);
}

void TestElementHandler::testHandleSetElementPropertiesAcceptsWirelessModeNoneForRealNode()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();

    const int nodeId = createElement(handler, "Node");
    auto *node = elementById(window, nodeId);
    QVERIFY(node);
    QCOMPARE(node->elementType(), ElementType::Node);

    const int andId = createElement(handler, "And", 100, 100);
    auto *andGate = elementById(window, andId);

    auto connection = std::make_unique<Connection>();
    connection->setStartPort(node->outputPort());
    connection->setEndPort(andGate->inputPort(0));
    scene->receiveCommand(new AddItemsCommand({connection.release()}, scene));
    QCOMPARE(node->outputPort()->connections().size(), 1);

    // wireless_mode 0 (None) matches neither the Tx nor Rx ternary branch, so the port
    // pointer stays null and no connection is severed.
    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", nodeId}, {"wireless_mode", 0}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(node->wirelessMode(), WirelessMode::None);
    QCOMPARE(node->outputPort()->connections().size(), 1);
}

void TestElementHandler::testHandleSetElementPropertiesRejectsInvalidWirelessMode()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int nodeId = createElement(handler, "Node");

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", nodeId}, {"wireless_mode", 99}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestElementHandler::testHandleSetElementPropertiesRefusesWirelessModeForNonNode()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    QVERIFY(refuses(window, handler, {{"element_id", id}, {"wireless_mode", 1}}));
}

void TestElementHandler::testHandleSetElementPropertiesPushesUndoableCommand()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");
    auto *scene = window.currentTab()->scene();
    auto *element = elementById(window, id);

    QVERIFY(handler.handleCommand("set_element_properties", {{"element_id", id}, {"label", "Changed"}}, 1).contains("result"));
    QCOMPARE(element->label(), QStringLiteral("Changed"));

    QVERIFY(scene->undoStack()->canUndo());
    scene->undoStack()->undo();
    QVERIFY(element->label() != QStringLiteral("Changed"));
}

void TestElementHandler::testSetElementPropertiesRefusesAPropertyTheTypeLacks()
{
    // An And has no colour, frequency, delay, volume, trigger, lock or sound to change, and a label sent with one is
    // not applied either
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    QVERIFY(refuses(window, handler, {{"element_id", id}, {"label", "L"}, {"color", "Red"}}));
    QVERIFY(refuses(window, handler, {{"element_id", id}, {"frequency", 2}}));
    QVERIFY(refuses(window, handler, {{"element_id", id}, {"delay", 0.25}}));
    QVERIFY(refuses(window, handler, {{"element_id", id}, {"volume", 0.5}}));
    QVERIFY(refuses(window, handler, {{"element_id", id}, {"trigger", "A"}}));
    QVERIFY(refuses(window, handler, {{"element_id", id}, {"locked", true}}));
}

void TestElementHandler::testSetElementPropertiesRefusesAValueOfTheWrongKind()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int input = createElement(handler, "InputSwitch");
    const int node = createElement(handler, "Node", 100, 0);

    QVERIFY(refuses(window, handler, {{"element_id", input}, {"label", 5}}));
    QVERIFY(refuses(window, handler, {{"element_id", input}, {"locked", "yes"}}));
    QVERIFY(refuses(window, handler, {{"element_id", input}, {"trigger", 1}}));
    QVERIFY(refuses(window, handler, {{"element_id", input}, {"rotation", "90"}}));
    QVERIFY(refuses(window, handler, {{"element_id", node}, {"wireless_mode", "Tx"}}));
    QVERIFY(refuses(window, handler, {{"element_id", node}, {"wireless_mode", 1.5}}));
}

void TestElementHandler::testSetElementPropertiesRefusesAnInvalidValueBeforeChangingAnything()
{
    // The label comes before the delay, which is out of range: neither is applied, and no step is pushed
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createClock(handler);

    QVERIFY(refuses(window, handler, {{"element_id", id}, {"label", "L"}, {"delay", 2}}));
    QVERIFY(refuses(window, handler, {{"element_id", id}, {"label", "L"}, {"frequency", -1}}));
    QVERIFY(elementById(window, id)->label().isEmpty());
}

void TestElementHandler::testSetElementPropertiesRefusesAFrequencyTheClockCannotTake()
{
    // A half period under a microsecond: the clock keeps its frequency, so the request is refused, not reported as set
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createClock(handler);

    QVERIFY(refuses(window, handler, {{"element_id", id}, {"frequency", 1e7}}));
}

void TestElementHandler::testSetElementPropertiesRefusesAColourTheEditorDoesNotOffer()
{
    // The spec's choices, for an LED and each display
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    for (const QString type : {"Led", "Display7", "Display14", "Display16"}) {
        const int id = createElement(handler, type);
        QVERIFY2(refuses(window, handler, {{"element_id", id}, {"color", "Orange"}}), qPrintable(type));
        QVERIFY2(refuses(window, handler, {{"element_id", id}, {"color", "blue"}}), qPrintable(type));
        QVERIFY2(refuses(window, handler, {{"element_id", id}, {"color", ""}}), qPrintable(type));
    }
}

void TestElementHandler::testSetElementPropertiesRefusesATriggerTheEditorDoesNotTake()
{
    // The editor's field takes one letter, digit or space
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "InputSwitch");

    QVERIFY(refuses(window, handler, {{"element_id", id}, {"trigger", "Ctrl+K"}}));
    QVERIFY(refuses(window, handler, {{"element_id", id}, {"trigger", "AB"}}));
    QVERIFY(refuses(window, handler, {{"element_id", id}, {"trigger", "%"}}));
}

void TestElementHandler::testSetElementPropertiesRefusesAnAppearanceIndexAlone()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "Led");

    QVERIFY(refuses(window, handler, {{"element_id", id}, {"appearance_index", 0}}));
}

void TestElementHandler::testSetElementPropertiesRefusesAnAppearanceIndexOutOfRange()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "Led");
    const QString picture = ":/Components/Output/Led/RedLed.svg";

    QVERIFY(refuses(window, handler, {{"element_id", id}, {"appearance", picture}, {"appearance_index", 99}}));
    QVERIFY(refuses(window, handler, {{"element_id", id}, {"appearance", picture}, {"appearance_index", -2}}));
}

void TestElementHandler::testSetElementPropertiesRefusesAPictureThatDoesNotLoad()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "Led");

    QVERIFY(refuses(window, handler, {{"element_id", id}, {"label", "L"}, {"appearance", "/no/such/picture.png"}}));
}

void TestElementHandler::testSetElementPropertiesRefusesASecondTxOnALabel()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int first = createElement(handler, "Node");
    const int second = createElement(handler, "Node", 100, 0);
    QVERIFY(handler.handleCommand("set_element_properties", {{"element_id", first}, {"label", "x"}, {"wireless_mode", 1}}, 1)
                .contains("result"));
    QVERIFY(handler.handleCommand("set_element_properties", {{"element_id", second}, {"label", "y"}, {"wireless_mode", 1}}, 1)
                .contains("result"));

    QVERIFY(refuses(window, handler, {{"element_id", second}, {"label", "x"}}));
    QVERIFY(handler.handleCommand("set_element_properties", {{"element_id", second}, {"wireless_mode", 0}}, 1).contains("result"));
    QVERIFY(handler.handleCommand("set_element_properties", {{"element_id", second}, {"label", "x"}}, 1).contains("result"));
    QVERIFY(refuses(window, handler, {{"element_id", second}, {"wireless_mode", 1}}));

    // The property command refuses the channel: a label and a mode that make the second Tx, with a rotation the same
    // request sets after them, leave all three as they were
    QVERIFY(refuses(window, handler, {{"element_id", second}, {"label", "x"}, {"wireless_mode", 1}, {"rotation", 90}}));
    QCOMPARE(elementById(window, second)->rotation(), 0.0);
}

void TestElementHandler::testSetElementPropertiesPushesNothingWhenNothingChanges()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "InputSwitch");
    auto *stack = window.currentTab()->scene()->undoStack();
    const int commands = stack->count();

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", id}, {"label", ""}, {"locked", false}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(stack->count(), commands);

    // Nor does a rotation the element has, through the snapshot
    QVERIFY(handler.handleCommand("set_element_properties", {{"element_id", id}, {"label", ""}, {"rotation", 0}}, 1).contains("result"));
    QCOMPARE(stack->count(), commands);
}

void TestElementHandler::testSetElementPropertiesIsOneUndoStep()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createClock(handler);
    auto *stack = window.currentTab()->scene()->undoStack();
    const int commands = stack->count();
    const QByteArray before = snapshot(elementById(window, id));

    // The properties go through the property command and the rotation through a snapshot: still one step
    const QJsonObject response = handler.handleCommand("set_element_properties",
        {{"element_id", id}, {"label", "L"}, {"frequency", 4}, {"delay", 0.25}, {"locked", true}, {"rotation", 90}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(stack->count(), commands + 1);
    const QByteArray after = snapshot(elementById(window, id));

    stack->undo();
    QCOMPARE(snapshot(elementById(window, id)), before);
    stack->redo();
    QCOMPARE(snapshot(elementById(window, id)), after);
}

void TestElementHandler::testSetElementPropertiesReadsBackWhatTheElementTook()
{
    // The response says the value the element holds, here the volume as given
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "Buzzer");

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", id}, {"volume", 0.3}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["new_properties"].toObject()["volume"].toDouble(),
             elementById(window, id)->volume());
    QCOMPARE(elementById(window, id)->volume(), 0.3);
}

// ============================================================================
// set_input_value
// ============================================================================

void TestElementHandler::testHandleSetInputValueRejectsMissingParams()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("set_input_value", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleSetInputValueRejectsUnknownElement()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("set_input_value", {{"element_id", 999999}, {"value", true}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestElementHandler::testHandleSetInputValueRejectsNonInputElement()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("set_input_value", {{"element_id", id}, {"value", true}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestElementHandler::testHandleSetInputValueRefusesARotary()
{
    // A rotary selects one output and has no level: either value is refused, and its selection stays
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "InputRotary");
    auto *rotary = elementById(window, id);
    QVERIFY(rotary);
    QVERIFY(rotary->setParam(ParamId::Position, 1));
    for (const bool value : {true, false}) {
        const QJsonObject response = handler.handleCommand("set_input_value", {{"element_id", id}, {"value", value}}, 1);
        QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
        QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
        QCOMPARE(rotary->param<int>(ParamId::Position), 1);
    }
}

void TestElementHandler::testHandleSetInputValueSetsRealValue()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "InputSwitch");
    auto *inputElm = elementById(window, id);
    QVERIFY(inputElm);
    QCOMPARE(inputElm->elementGroup(), ElementGroup::Input);
    QVERIFY(!inputElm->isOn());

    const QJsonObject response = handler.handleCommand("set_input_value", {{"element_id", id}, {"value", true}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(inputElm->isOn());
}

// ============================================================================
// get_output_value
// ============================================================================

void TestElementHandler::testHandleGetOutputValueRejectsMissingParams()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("get_output_value", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleGetOutputValueRejectsUnknownElement()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("get_output_value", {{"element_id", 999999}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestElementHandler::testHandleGetOutputValueReturnsInputElementValue()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "InputSwitch");
    auto *inputElm = elementById(window, id);
    inputElm->setOn(true);

    const QJsonObject response = handler.handleCommand("get_output_value", {{"element_id", id}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["value"].toBool(), true);
}

void TestElementHandler::testHandleGetOutputValueOfAnInputReportsEachPortsStatus()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    MCPValidator validator(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("schema-mcp.json"));
    QVERIFY(validator.isSchemaLoaded());

    const auto read = [&handler](int id, int port) {
        return handler.handleCommand("get_output_value", {{"element_id", id}, {"port", port}}, 1);
    };

    const int switchId = createElement(handler, "InputSwitch");
    auto *inputSwitch = elementById(window, switchId);
    QVERIFY(inputSwitch);
    QCOMPARE(inputSwitch->elementType(), ElementType::InputSwitch);
    inputSwitch->setOn(false);
    QJsonObject response = read(switchId, 0);
    ValidationResult valid = validator.validateResponse(response, "get_output_value");
    QVERIFY2(valid.isValid, qPrintable(valid.errorMessage));
    QCOMPARE(response["result"].toObject()["status"].toString(), QStringLiteral("low"));

    inputSwitch->setOn(true);
    response = read(switchId, 0);
    QCOMPARE(response["result"].toObject()["status"].toString(), QStringLiteral("high"));
    QCOMPARE(response["result"].toObject()["value"].toBool(), true);

    // A rotary is on at one port only: each port reads its own state
    const int rotaryId = createElement(handler, "InputRotary");
    auto *rotary = elementById(window, rotaryId);
    QVERIFY(rotary);
    QCOMPARE(rotary->elementType(), ElementType::InputRotary);
    rotary->setOn(true, 1);
    response = read(rotaryId, 1);
    valid = validator.validateResponse(response, "get_output_value");
    QVERIFY2(valid.isValid, qPrintable(valid.errorMessage));
    QCOMPARE(response["result"].toObject()["status"].toString(), QStringLiteral("high"));
    QCOMPARE(read(rotaryId, 0)["result"].toObject()["status"].toString(), QStringLiteral("low"));

    const QJsonObject outOfRange = handler.handleCommand("get_output_value", {{"element_id", rotaryId}, {"port", 2}}, 1);
    QCOMPARE(outOfRange["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestElementHandler::testHandleGetOutputValueReturnsGenericElementValue()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And"); // generic (non-input, non-Output-group) element

    const QJsonObject response = handler.handleCommand("get_output_value", {{"element_id", id}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(response["result"].toObject().contains("value"));
}

void TestElementHandler::testHandleGetOutputValueReturnsOutputGroupValue()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "Led"); // ElementGroup::Output

    const QJsonObject response = handler.handleCommand("get_output_value", {{"element_id", id}, {"port", 0}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(response["result"].toObject().contains("value"));
}

void TestElementHandler::testHandleGetOutputValueRejectsInvalidPort()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("get_output_value", {{"element_id", id}, {"port", -1}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleGetOutputValueRejectsOutOfRangePort()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int andId = createElement(handler, "And");
    QCOMPARE(handler.handleCommand("get_output_value", {{"element_id", andId}, {"port", 99}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);

    const int ledId = createElement(handler, "Led");
    QCOMPARE(handler.handleCommand("get_output_value", {{"element_id", ledId}, {"port", 99}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
}

// ============================================================================
// rotate_element
// ============================================================================

void TestElementHandler::testHandleRotateElementRejectsMissingParams()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("rotate_element", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleRotateElementRejectsInvalidAngle()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("rotate_element", {{"element_id", id}, {"angle", "bad"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleRotateElementRejectsUnknownElement()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("rotate_element", {{"element_id", 999999}, {"angle", 90}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestElementHandler::testHandleRotateElementNormalizesAngle()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("rotate_element", {{"element_id", id}, {"angle", 450}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["angle"].toInt(), 90);
    QCOMPARE(elementById(window, id)->rotation(), 90.0);

    const QJsonObject negResponse = handler.handleCommand("rotate_element", {{"element_id", id}, {"angle", -90}}, 1);
    QCOMPARE(negResponse["result"].toObject()["angle"].toInt(), 270);
}

// ============================================================================
// flip_element
// ============================================================================

void TestElementHandler::testHandleFlipElementRejectsMissingParams()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("flip_element", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleFlipElementRejectsInvalidAxis()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    QCOMPARE(handler.handleCommand("flip_element", {{"element_id", id}, {"axis", "bad"}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::InvalidParams);
    QCOMPARE(handler.handleCommand("flip_element", {{"element_id", id}, {"axis", 2}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
}

void TestElementHandler::testHandleFlipElementRejectsUnknownElement()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("flip_element", {{"element_id", 999999}, {"axis", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestElementHandler::testHandleFlipElementFlipsRealElement()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject hResponse = handler.handleCommand("flip_element", {{"element_id", id}, {"axis", 0}}, 1);
    QVERIFY2(hResponse.contains("result"), qPrintable(QJsonDocument(hResponse).toJson()));
    QCOMPARE(hResponse["result"].toObject()["axis"].toString(), QStringLiteral("horizontal"));

    const QJsonObject vResponse = handler.handleCommand("flip_element", {{"element_id", id}, {"axis", 1}}, 1);
    QCOMPARE(vResponse["result"].toObject()["axis"].toString(), QStringLiteral("vertical"));
}

// ============================================================================
// update_element
// ============================================================================

void TestElementHandler::testHandleUpdateElementDelegatesAndSimplifiesResponse()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("update_element", {{"element_id", id}, {"label", "Updated"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["element_id"].toInt(), id);
    QVERIFY(!response["result"].toObject().contains("new_properties"));
    QCOMPARE(elementById(window, id)->label(), QStringLiteral("Updated"));
}

void TestElementHandler::testHandleUpdateElementPropagatesErrors()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("update_element", {{"element_id", 999999}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

// ============================================================================
// change_input_size / change_output_size
// ============================================================================

void TestElementHandler::testHandleChangeInputSizeRejectsMissingParams()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("change_input_size", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleChangeInputSizeRejectsInvalidSize()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("change_input_size", {{"element_id", id}, {"size", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleChangeInputSizeRejectsOutOfRangeSize()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And"); // input range [2, 8]

    QCOMPARE(handler.handleCommand("change_input_size", {{"element_id", id}, {"size", 1}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
    QCOMPARE(handler.handleCommand("change_input_size", {{"element_id", id}, {"size", 99}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
}

void TestElementHandler::testHandleChangeInputSizeRejectsUnknownElement()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("change_input_size", {{"element_id", 999999}, {"size", 4}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestElementHandler::testHandleChangeInputSizeChangesRealSize()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("change_input_size", {{"element_id", id}, {"size", 4}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["new_size"].toInt(), 4);
    QCOMPARE(elementById(window, id)->inputSize(), 4);
}

void TestElementHandler::testHandleChangeOutputSizeRejectsMissingParams()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("change_output_size", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleChangeOutputSizeRejectsInvalidSize()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "Demux");

    const QJsonObject response = handler.handleCommand("change_output_size", {{"element_id", id}, {"size", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleChangeOutputSizeRejectsOutOfRangeSize()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "Demux"); // output range [2, 8]

    QCOMPARE(handler.handleCommand("change_output_size", {{"element_id", id}, {"size", 1}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
    QCOMPARE(handler.handleCommand("change_output_size", {{"element_id", id}, {"size", 99}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::ValidationError);
}

void TestElementHandler::testHandleChangeOutputSizeRejectsUnknownElement()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("change_output_size", {{"element_id", 999999}, {"size", 4}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestElementHandler::testHandleChangeOutputSizeChangesRealSize()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "Demux");

    const QJsonObject response = handler.handleCommand("change_output_size", {{"element_id", id}, {"size", 4}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["new_size"].toInt(), 4);
    QCOMPARE(elementById(window, id)->outputSize(), 4);
}

// ============================================================================
// toggle_truth_table_output
// ============================================================================

void TestElementHandler::testHandleToggleTruthTableOutputRejectsMissingParams()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("toggle_truth_table_output", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleToggleTruthTableOutputRejectsInvalidPosition()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "TruthTable");

    const QJsonObject response = handler.handleCommand("toggle_truth_table_output", {{"element_id", id}, {"position", -1}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleToggleTruthTableOutputRejectsUnknownElement()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("toggle_truth_table_output", {{"element_id", 999999}, {"position", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestElementHandler::testHandleToggleTruthTableOutputRejectsNonTruthTable()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("toggle_truth_table_output", {{"element_id", id}, {"position", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestElementHandler::testHandleToggleTruthTableOutputRejectsOutOfRangePosition()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "TruthTable");
    Element *truthTable = elementById(window, id);
    const int maxPosition = 256 * truthTable->outputSize();

    const QJsonObject response = handler.handleCommand("toggle_truth_table_output", {{"element_id", id}, {"position", maxPosition}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestElementHandler::testHandleToggleTruthTableOutputTogglesRealCell()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "TruthTable");
    const Element *truthTable = elementById(window, id);

    const bool before = truthTable->param<QBitArray>(ParamId::Table).testBit(0);
    const QJsonObject response = handler.handleCommand("toggle_truth_table_output", {{"element_id", id}, {"position", 0}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(truthTable->param<QBitArray>(ParamId::Table).testBit(0), !before);
}

// ============================================================================
// morph_element
// ============================================================================

void TestElementHandler::testHandleMorphElementRejectsMissingParams()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("morph_element", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleMorphElementRejectsNonArrayElementIds()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("morph_element", {{"element_ids", 5}, {"target_type", "Or"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleMorphElementRejectsEmptyElementIds()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("morph_element", {{"element_ids", QJsonArray()}, {"target_type", "Or"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleMorphElementRejectsEmptyTargetType()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("morph_element", {{"element_ids", QJsonArray{id}}, {"target_type", ""}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleMorphElementRejectsInvalidTargetType()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("morph_element", {{"element_ids", QJsonArray{id}}, {"target_type", "NotReal"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestElementHandler::testHandleMorphElementRejectsNonIntegerElementId()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("morph_element", {{"element_ids", QJsonArray{"not a number"}}, {"target_type", "Or"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestElementHandler::testHandleMorphElementRejectsUnknownElementId()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("morph_element", {{"element_ids", QJsonArray{999999}}, {"target_type", "Or"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestElementHandler::testHandleMorphElementRejectsNonElementItem()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    const int a = createElement(handler, "And");
    const int b = createElement(handler, "And", 100, 100);

    auto connection = std::make_unique<Connection>();
    connection->setStartPort(elementById(window, a)->outputPort());
    connection->setEndPort(elementById(window, b)->inputPort(0));
    scene->addItem(connection.get());

    const QJsonObject response = handler.handleCommand("morph_element",
        {{"element_ids", QJsonArray{connection->id()}}, {"target_type", "Or"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestElementHandler::testHandleMorphElementMorphsRealElement()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("morph_element", {{"element_ids", QJsonArray{id}}, {"target_type", "Or"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonArray morphedIds = response["result"].toObject()["morphed_elements"].toArray();
    QCOMPARE(morphedIds.size(), 1);
    QCOMPARE(morphedIds[0].toInt(), id);

    auto *morphed = elementById(window, id);
    QVERIFY(morphed);
    QCOMPARE(ElementFactory::typeToText(morphed->elementType()), QStringLiteral("Or"));
}

void TestElementHandler::testHandleCommandRejectsUnknownCommand()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("totally_made_up", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::MethodNotFound);
}

void TestElementHandler::testSchemaMatchesTheKeyedProperties()
{
    // schema-mcp.json's element properties are the ones MCP reads and writes (ElementHandler::keyedProperties()):
    // set_element_properties, update_element and the Element of list_elements hold every keyed property and nothing
    // else but their own keys. A command's is of its ParamSpec's kind, range and choices, which every type that has the
    // parameter agrees on; the Element's states the kind alone, since an element holds what its spec's accept rule
    // takes, which a file's value outside the range or the choices can be
    QFile file(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("schema-mcp.json"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QJsonObject schema = QJsonDocument::fromJson(file.readAll()).object();
    const QJsonObject commands = schema["properties"]["commands"]["properties"].toObject();
    const auto paramsOf = [&commands](const QString &command) {
        return commands[command]["properties"]["params"]["properties"].toObject();
    };
    const QList<std::pair<QString, QJsonObject>> objects{
        {"set_element_properties", paramsOf("set_element_properties")},
        {"update_element", paramsOf("update_element")},
        {"Element", schema["definitions"]["Element"]["properties"].toObject()},
    };
    // What each object holds besides the keyed properties
    const QStringList commandKeys{"element_id", "rotation", "appearance", "appearance_index"};
    const QStringList elementKeys{"element_id", "type", "x", "y", "width", "height", "rotation", "input_size", "output_size"};

    // A spec's bounds are inclusive, so a property's schema holds them as its minimum and maximum, and its choices as
    // its enum; an open bound and no choices are no keyword
    struct Expected {
        QString type;
        QJsonValue minimum{QJsonValue::Undefined};
        QJsonValue maximum{QJsonValue::Undefined};
        QJsonValue choices{QJsonValue::Undefined};
        bool operator==(const Expected &other) const = default;
    };
    const auto jsonType = [](const ParamValue &value) {
        return std::visit([](const auto &held) {
            using Kind = std::decay_t<decltype(held)>;
            return std::is_same_v<Kind, bool>     ? QString("boolean")
                   : std::is_same_v<Kind, int>    ? QString("integer")
                   : std::is_same_v<Kind, double> ? QString("number")
                                                  : QString("string");
        }, value);
    };
    const auto bound = [](const std::optional<double> &value) {
        return value ? QJsonValue(*value) : QJsonValue(QJsonValue::Undefined);
    };
    QHash<QString, Expected> expected;
    for (const auto &keyed : ElementHandler::keyedProperties()) {
        const QString key = QString::fromLatin1(keyed.key);
        if (keyed.property.kind != ElementProperty::Kind::Parameter) {
            expected[key] = {.type = "string"};
            continue;
        }
        std::optional<Expected> found;
        for (const ElementType type : TestUtils::dumpedTypes()) {
            if (!ElementFactory::hasCreator(type)) {
                continue;
            }
            for (const ParamSpec &spec : ElementTypeDescriptor::of(type).parameters) {
                if (spec.id != keyed.property.id) {
                    continue;
                }
                const Expected own{jsonType(spec.defaultValue), bound(spec.range.minimum), bound(spec.range.maximum),
                                   spec.choices.isEmpty() ? QJsonValue(QJsonValue::Undefined)
                                                          : QJsonValue(QJsonArray::fromStringList(spec.choices))};
                QVERIFY2(!found || (*found == own),
                         qPrintable(QString("%1's '%2' differs from another type's").arg(TestUtils::snapshotType(type), key)));
                found = own;
            }
        }
        QVERIFY2(found, qPrintable(QString("no type has '%1'").arg(key)));
        expected[key] = *found;
    }

    for (const auto &[name, properties] : objects) {
        QVERIFY2(!properties.isEmpty(), qPrintable(name));
        const QStringList &others = (name == "Element") ? elementKeys : commandKeys;
        for (const QString &key : properties.keys()) {
            QVERIFY2(expected.contains(key) || others.contains(key),
                     qPrintable(QString("%1 has '%2', which MCP does not read or write").arg(name, key)));
        }
        for (auto it = expected.cbegin(); it != expected.cend(); ++it) {
            const QString where = QString("%1's '%2'").arg(name, it.key());
            QVERIFY2(properties.contains(it.key()), qPrintable(where + " is missing"));
            const QJsonObject property = properties[it.key()].toObject();
            QVERIFY2(property["type"].toString() == it->type, qPrintable(where + " is not a " + it->type));
            QVERIFY2(!property.contains("exclusiveMinimum") && !property.contains("exclusiveMaximum"),
                     qPrintable(where + " has an exclusive bound, which no range has"));
            if (name == "Element") {
                QVERIFY2(!property.contains("minimum") && !property.contains("maximum") && !property.contains("enum"),
                         qPrintable(where + " bounds what an element reports"));
                continue;
            }
            QVERIFY2(property["minimum"] == it->minimum, qPrintable(where + "'s minimum differs from its range"));
            QVERIFY2(property["maximum"] == it->maximum, qPrintable(where + "'s maximum differs from its range"));
            QVERIFY2(property["enum"] == it->choices, qPrintable(where + "'s enum differs from its choices"));
        }
    }
}

void TestElementHandler::testListElementsReportsAFilesValuesOutsideWhatMCPSets()
{
    // A file keeps a colour off the choices and a volume past the range, which list_elements reports as they are, its
    // response still the schema's
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    MCPValidator validator(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("schema-mcp.json"));
    QVERIFY(validator.isSchemaLoaded());
    elementById(window, createElement(handler, "Led"))->setColor("Orange");
    elementById(window, createElement(handler, "Buzzer", 100, 0))->setVolume(1.5);

    const QJsonObject response = handler.handleCommand("list_elements", {}, 1);
    const ValidationResult valid = validator.validateResponse(response, "list_elements");
    QVERIFY2(valid.isValid, qPrintable(valid.errorMessage));
    const QJsonArray elements = response["result"].toObject()["elements"].toArray();
    QCOMPARE(elements.size(), 2);
    QCOMPARE(elements.at(0).toObject()["color"].toString(), QString("Orange"));
    QCOMPARE(elements.at(1).toObject()["volume"].toDouble(), 1.5);
}

void TestElementHandler::testResponsesMatchGolden()
{
    // Every creatable type, alone in a new tab so its id is always 1: what each element command responds, result or
    // error, in a fixed order that ends with a write of every property and the element's deletion. morph_element and
    // update_element are left out, since they echo their parameters; each property is set by a request of its own. A
    // Text element's size is its text's, which follows the font, so the golden holds its sizes masked. Each response
    // must also pass the schema, as the server checks every response before sending it.
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    MCPValidator validator(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("schema-mcp.json"));
    QVERIFY(validator.isSchemaLoaded());

    QStringList lines;
    QStringList invalid;
    for (const ElementType type : TestUtils::dumpedTypes()) {
        const QString typeName = TestUtils::snapshotType(type);
        if (!ElementFactory::hasCreator(type)) {
            lines << QString("%1 %2").arg(QLatin1String(TestUtils::kNoCreator), typeName);
            continue;
        }
        window.createNewTab();
        const auto send = [&](const QString &command, const QJsonObject &params) {
            const QJsonObject response = handler.handleCommand(command, params, 1);
            const ValidationResult valid = validator.validateResponse(response, command);
            if (!valid.isValid) {
                invalid << QString("%1 %2: %3").arg(typeName, command, valid.errorMessage.trimmed());
            }
            const bool failed = response.contains("error");
            const QJsonValue raw = response[failed ? "error" : "result"];
            const QJsonObject body = ((type == ElementType::Text) ? maskFontSizes(raw) : raw).toObject();
            lines << QString("T %1 %2 %3=%4")
                         .arg(typeName, command, failed ? "error" : "result",
                              QString::fromUtf8(QJsonDocument(body).toJson(QJsonDocument::Compact)));
            return response;
        };

        const int id = send("create_element", {{"type", ElementFactory::typeToText(type)}, {"x", 0}, {"y", 0}})["result"]
                           .toObject()["element_id"]
                           .toInt();
        QCOMPARE(id, 1);
        const Element *element = elementById(window, id);
        QVERIFY(element);
        send("list_elements", {});
        send("get_output_value", {{"element_id", id}});
        send("set_input_value", {{"element_id", id}, {"value", true}});
        send("get_output_value", {{"element_id", id}});
        send("rotate_element", {{"element_id", id}, {"angle", 90}});
        send("flip_element", {{"element_id", id}, {"axis", 0}});
        send("change_input_size", {{"element_id", id}, {"size", element->inputSize() + 1}});
        send("change_output_size", {{"element_id", id}, {"size", element->outputSize() + 1}});
        send("toggle_truth_table_output", {{"element_id", id}, {"position", 0}});
        send("list_elements", {});
        // Each property in its own request, so the golden holds each one's result or refusal for the type
        const QList<std::pair<QString, QJsonValue>> properties{
            {"label", "Edited"}, {"color", "Green"}, {"frequency", 2}, {"delay", 0.25}, {"trigger", "Q"},
            {"volume", 0.75}, {"locked", true}, {"appearance", ""}, {"wireless_mode", 1}, {"rotation", 180}};
        for (const auto &[property, value] : properties) {
            send("set_element_properties", {{"element_id", id}, {property, value}});
        }
        send("delete_element", {{"element_id", id}});
    }

    QVERIFY2(invalid.isEmpty(), qPrintable(invalid.join('\n')));

    const QString golden = QString(QUOTE(CURRENTDIR)) + "/Integration/Properties/mcp_responses.txt";
    const QString mismatch = TestUtils::compareWithGolden(golden, lines.join('\n') + '\n');
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}
