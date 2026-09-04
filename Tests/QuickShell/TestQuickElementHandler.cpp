// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickElementHandler.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "App/Element/GraphicElement.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "MCP/Server/Handlers/QuickElementHandler.h"

namespace {

int createClock(QuickElementHandler &handler)
{
    const QJsonObject params{{"type", "Clock"}, {"x", 0}, {"y", 0}};
    const QJsonObject response = handler.handleCommand("create_element", params, {});
    return response["result"].toObject()["element_id"].toInt();
}

} // namespace

void TestQuickElementHandler::testSetElementPropertiesAcceptsNegativeClockDelay()
{
    // Regression: mirrors TestElementHandler's identical check against the Widgets handler --
    // Clock::setDelay() clamps to [-1, 1] (negative delays advance the clock), and this
    // handler's validation must accept the full range, not just delay >= 0.
    QuickAppController controller;
    controller.newTab();
    QuickElementHandler handler(&controller, nullptr);

    const int clockId = createClock(handler);
    QVERIFY(clockId != 0);

    const QJsonObject params{{"element_id", clockId}, {"delay", -0.25}};
    const QJsonObject response = handler.handleCommand("set_element_properties", params, {});
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));

    auto *element = dynamic_cast<GraphicElement *>(controller.currentTab()->canvas()->itemById(clockId));
    QVERIFY(element);
    QCOMPARE(element->delay(), -0.25);
}

void TestQuickElementHandler::testSetElementPropertiesRejectsClockDelayOutOfRange()
{
    QuickAppController controller;
    controller.newTab();
    QuickElementHandler handler(&controller, nullptr);

    const int clockId = createClock(handler);
    QVERIFY(clockId != 0);

    const QJsonObject tooHigh{{"element_id", clockId}, {"delay", 1.5}};
    QVERIFY2(handler.handleCommand("set_element_properties", tooHigh, {}).contains("error"),
             "delay=1.5 is outside Clock::setDelay()'s [-1, 1] range and must be rejected");

    const QJsonObject tooLow{{"element_id", clockId}, {"delay", -1.5}};
    QVERIFY2(handler.handleCommand("set_element_properties", tooLow, {}).contains("error"),
             "delay=-1.5 is outside Clock::setDelay()'s [-1, 1] range and must be rejected");
}
