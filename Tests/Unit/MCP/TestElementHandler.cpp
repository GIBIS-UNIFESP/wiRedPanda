// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/MCP/TestElementHandler.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MainWindow.h"
#include "MCP/Server/Handlers/ElementHandler.h"

namespace {

int createClock(ElementHandler &handler)
{
    const QJsonObject params{{"type", "Clock"}, {"x", 0}, {"y", 0}};
    const QJsonObject response = handler.handleCommand("create_element", params, {});
    return response["result"].toObject()["element_id"].toInt();
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
