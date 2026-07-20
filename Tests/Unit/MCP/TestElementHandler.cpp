// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/MCP/TestElementHandler.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTabWidget>
#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MainWindow.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "MCP/Server/Core/JsonRpcError.h"
#include "MCP/Server/Handlers/ElementHandler.h"

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

GraphicElement *elementById(MainWindow &window, int id)
{
    return dynamic_cast<GraphicElement *>(window.currentTab()->scene()->itemById(id));
}

} // namespace

void TestElementHandler::testSetElementPropertiesAcceptsNegativeClockDelay()
{
    // Regression: Clock::setDelay() clamps to [-1, 1] (negative delays advance the clock —
    // see Clock.cpp/Clock.h), but this handler's validation only ever accepted delay >= 0,
    // so a legitimate negative phase delay was rejected outright via the MCP path even
    // though the GUI property editor and a direct C++ call both accept it.
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const int clockId = createClock(handler);
    QVERIFY(clockId != 0);

    const QJsonObject params{{"element_id", clockId}, {"delay", -0.25}};
    const QJsonObject response = handler.handleCommand("set_element_properties", params, {});
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));

    auto *element = dynamic_cast<GraphicElement *>(window.currentTab()->scene()->itemById(clockId));
    QVERIFY(element);
    QCOMPARE(element->delay(), -0.25);
}

void TestElementHandler::testSetElementPropertiesRejectsClockDelayOutOfRange()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);

    const int clockId = createClock(handler);
    QVERIFY(clockId != 0);

    const QJsonObject tooHigh{{"element_id", clockId}, {"delay", 1.5}};
    QVERIFY2(handler.handleCommand("set_element_properties", tooHigh, {}).contains("error"),
             "delay=1.5 is outside Clock::setDelay()'s [-1, 1] range and must be rejected");

    const QJsonObject tooLow{{"element_id", clockId}, {"delay", -1.5}};
    QVERIFY2(handler.handleCommand("set_element_properties", tooLow, {}).contains("error"),
             "delay=-1.5 is outside Clock::setDelay()'s [-1, 1] range and must be rejected");
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
    QCOMPARE(element->pos(), QPointF(10, 20));
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
    createElement(handler, "InputSwitch"); // hasTrigger, GraphicElementInput ("locked")
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
    for (const QJsonValue &v : elements) {
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

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", id}, {"color", "blue"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(elementById(window, id)->color(), QStringLiteral("blue"));
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

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", id}, {"trigger", "Ctrl+K"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(elementById(window, id)->trigger(), QKeySequence("Ctrl+K"));
}

void TestElementHandler::testHandleSetElementPropertiesChangesLockedState()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "InputSwitch");
    auto *inputElm = dynamic_cast<GraphicElementInput *>(elementById(window, id));
    QVERIFY(inputElm);
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
    QCOMPARE(static_cast<double>(elementById(window, id)->volume()), 0.5);
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
    auto *node = dynamic_cast<Node *>(elementById(window, nodeId));
    QVERIFY(node);

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
}

void TestElementHandler::testHandleSetElementPropertiesAcceptsWirelessModeNoneForRealNode()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();

    const int nodeId = createElement(handler, "Node");
    auto *node = dynamic_cast<Node *>(elementById(window, nodeId));
    QVERIFY(node);

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

void TestElementHandler::testHandleSetElementPropertiesIgnoresWirelessModeForNonNode()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "And");

    const QJsonObject response = handler.handleCommand("set_element_properties", {{"element_id", id}, {"wireless_mode", 1}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(!response["result"].toObject()["new_properties"].toObject().contains("wireless_mode"));
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

void TestElementHandler::testHandleSetInputValueSetsRealValue()
{
    MainWindow window;
    ElementHandler handler(&window, nullptr);
    const int id = createElement(handler, "InputSwitch");
    auto *inputElm = dynamic_cast<GraphicElementInput *>(elementById(window, id));
    QVERIFY(inputElm);
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
    auto *inputElm = dynamic_cast<GraphicElementInput *>(elementById(window, id));
    inputElm->setOn(true);

    const QJsonObject response = handler.handleCommand("get_output_value", {{"element_id", id}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["value"].toBool(), true);
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
    auto *truthTable = dynamic_cast<TruthTable *>(elementById(window, id));
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
    auto *truthTable = dynamic_cast<TruthTable *>(elementById(window, id));

    const bool before = truthTable->key().testBit(0);
    const QJsonObject response = handler.handleCommand("toggle_truth_table_output", {{"element_id", id}, {"position", 0}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(truthTable->key().testBit(0), !before);
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

void TestElementHandler::testHandleMorphElementRejectsNonGraphicElementItem()
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
