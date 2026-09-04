// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickBaseHandler.h"

#include <limits>

#include <QJsonObject>
#include <QTest>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "MCP/Server/Handlers/QuickBaseHandler.h"

namespace {

class StubHandler : public QuickBaseHandler
{
public:
    using QuickBaseHandler::QuickBaseHandler;
    QJsonObject handleCommand(const QString &, const QJsonObject &, const QJsonValue &) override { return {}; }
};

} // namespace

void TestQuickBaseHandler::testCreateSuccessResponseIncludesResultAndId()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    const QJsonObject result{{"value", 42}};
    const QJsonObject response = handler.createSuccessResponse(result, 7);

    QCOMPARE(response.value("jsonrpc").toString(), QStringLiteral("2.0"));
    QCOMPARE(response.value("result").toObject().value("value").toInt(), 42);
    QCOMPARE(response.value("id").toInt(), 7);
}

void TestQuickBaseHandler::testCreateSuccessResponseOmitsIdWhenNull()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    const QJsonObject response = handler.createSuccessResponse();
    QVERIFY(!response.contains("id"));
}

void TestQuickBaseHandler::testCreateErrorResponseIncludesCodeAndMessage()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    const QJsonObject response = handler.createErrorResponse("boom", 3, 42);
    QCOMPARE(response.value("jsonrpc").toString(), QStringLiteral("2.0"));
    QCOMPARE(response.value("error").toObject().value("code").toInt(), 42);
    QCOMPARE(response.value("error").toObject().value("message").toString(), QStringLiteral("boom"));
    QCOMPARE(response.value("id").toInt(), 3);
}

void TestQuickBaseHandler::testCreateErrorResponseOmitsIdWhenNull()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    const QJsonObject response = handler.createErrorResponse("boom");
    QVERIFY(!response.contains("id"));
}

void TestQuickBaseHandler::testValidateParametersAcceptsWhenAllPresent()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    const QJsonObject params{{"a", 1}, {"b", 2}};
    QVERIFY(handler.validateParameters(params, {"a", "b"}));
}

void TestQuickBaseHandler::testValidateParametersRejectsWhenMissing()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    const QJsonObject params{{"a", 1}};
    QVERIFY(!handler.validateParameters(params, {"a", "b"}));
}

void TestQuickBaseHandler::testCurrentCanvasNullWithNoAppController()
{
    StubHandler handler(nullptr, nullptr);
    QVERIFY(!handler.currentCanvas());
    QVERIFY(!std::as_const(handler).currentCanvas());
}

void TestQuickBaseHandler::testCurrentCanvasNullWithNoCurrentTab()
{
    QuickAppController controller;
    QVERIFY(!controller.currentTab());

    StubHandler handler(&controller, nullptr);
    QVERIFY(!handler.currentCanvas());
}

void TestQuickBaseHandler::testCurrentCanvasReturnsRealCanvas()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QCOMPARE(handler.currentCanvas(), controller.currentTab()->canvas());
}

void TestQuickBaseHandler::testValidatePositiveIntegerRejectsNonNumber()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validatePositiveInteger(QJsonValue("not a number"), "x", errorMsg));
    QVERIFY2(errorMsg.contains("must be an integer"), qPrintable(errorMsg));
}

void TestQuickBaseHandler::testValidatePositiveIntegerRejectsZeroAndNegative()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validatePositiveInteger(QJsonValue(0), "x", errorMsg));
    QVERIFY2(errorMsg.contains("positive integer"), qPrintable(errorMsg));

    QVERIFY(!handler.validatePositiveInteger(QJsonValue(-5), "x", errorMsg));
}

void TestQuickBaseHandler::testValidatePositiveIntegerAcceptsPositive()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QString errorMsg;
    QVERIFY(handler.validatePositiveInteger(QJsonValue(3), "x", errorMsg));
}

void TestQuickBaseHandler::testValidateNonNegativeIntegerRejectsNonNumber()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateNonNegativeInteger(QJsonValue("nope"), "x", errorMsg));
    QVERIFY2(errorMsg.contains("must be an integer"), qPrintable(errorMsg));
}

void TestQuickBaseHandler::testValidateNonNegativeIntegerRejectsNegative()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateNonNegativeInteger(QJsonValue(-1), "x", errorMsg));
    QVERIFY2(errorMsg.contains("non-negative"), qPrintable(errorMsg));
}

void TestQuickBaseHandler::testValidateNonNegativeIntegerAcceptsZeroAndPositive()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QString errorMsg;
    QVERIFY(handler.validateNonNegativeInteger(QJsonValue(0), "x", errorMsg));
    QVERIFY(handler.validateNonNegativeInteger(QJsonValue(5), "x", errorMsg));
}

void TestQuickBaseHandler::testValidateNonEmptyStringRejectsNonString()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateNonEmptyString(QJsonValue(5), "x", errorMsg));
    QVERIFY2(errorMsg.contains("must be a string"), qPrintable(errorMsg));
}

void TestQuickBaseHandler::testValidateNonEmptyStringRejectsEmpty()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateNonEmptyString(QJsonValue(""), "x", errorMsg));
    QVERIFY2(errorMsg.contains("cannot be empty"), qPrintable(errorMsg));
}

void TestQuickBaseHandler::testValidateNonEmptyStringAcceptsNonEmpty()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QString errorMsg;
    QVERIFY(handler.validateNonEmptyString(QJsonValue("hi"), "x", errorMsg));
}

void TestQuickBaseHandler::testValidateElementIdRejectsNonPositive()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateElementId(0, "element_id", errorMsg));
    QVERIFY2(errorMsg.contains("positive integer"), qPrintable(errorMsg));
}

void TestQuickBaseHandler::testValidateElementIdRejectsNoScene()
{
    QuickAppController controller;
    QVERIFY(!controller.currentTab());

    StubHandler handler(&controller, nullptr);
    QString errorMsg;
    QVERIFY(!handler.validateElementId(1, "element_id", errorMsg));
    QCOMPARE(errorMsg, QStringLiteral("No active circuit scene available"));
}

void TestQuickBaseHandler::testValidateElementIdRejectsNotFound()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateElementId(999999, "element_id", errorMsg));
    QVERIFY2(errorMsg.contains("Element not found"), qPrintable(errorMsg));
}

void TestQuickBaseHandler::testValidateElementIdAcceptsReal()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    auto *canvas = controller.currentTab()->canvas();
    auto *andGate = new And();
    canvas->addItem(andGate);

    QString errorMsg;
    QVERIFY(handler.validateElementId(andGate->id(), "element_id", errorMsg));
}

void TestQuickBaseHandler::testValidateNumericRejectsNonNumber()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateNumeric(QJsonValue("x"), "x", errorMsg));
    QVERIFY2(errorMsg.contains("numeric value"), qPrintable(errorMsg));
}

void TestQuickBaseHandler::testValidateNumericRejectsNanAndInf()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateNumeric(QJsonValue(std::numeric_limits<double>::quiet_NaN()), "x", errorMsg));
    QVERIFY2(errorMsg.contains("finite"), qPrintable(errorMsg));

    QVERIFY(!handler.validateNumeric(QJsonValue(std::numeric_limits<double>::infinity()), "x", errorMsg));
}

void TestQuickBaseHandler::testValidateNumericAcceptsFinite()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QString errorMsg;
    QVERIFY(handler.validateNumeric(QJsonValue(3.5), "x", errorMsg));
}

void TestQuickBaseHandler::testValidatePortRangeRejectsNullElement()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validatePortRange(nullptr, 0, false, "port", errorMsg));
    QVERIFY2(errorMsg.contains("Invalid element"), qPrintable(errorMsg));
}

void TestQuickBaseHandler::testValidatePortRangeRejectsOutOfRange()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    And andGate; // 2 inputs, 1 output by default
    QString errorMsg;
    QVERIFY(!handler.validatePortRange(&andGate, static_cast<int>(andGate.inputs().size()), false, "port", errorMsg));
    QVERIFY2(errorMsg.contains("out of range"), qPrintable(errorMsg));

    QVERIFY(!handler.validatePortRange(&andGate, static_cast<int>(andGate.outputs().size()), true, "port", errorMsg));
}

void TestQuickBaseHandler::testValidatePortRangeAcceptsInRange()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    And andGate;
    QString errorMsg;
    QVERIFY(handler.validatePortRange(&andGate, 0, false, "port", errorMsg));
    QVERIFY(handler.validatePortRange(&andGate, 0, true, "port", errorMsg));
}

void TestQuickBaseHandler::testValidatedElementRejectsNonPositiveParam()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    const QJsonObject params{{"element_id", 0}};
    QString errorMsg;
    QVERIFY(!handler.validatedElement(params, "element_id", errorMsg));
    QVERIFY2(!errorMsg.isEmpty(), qPrintable(errorMsg));
}

void TestQuickBaseHandler::testValidatedElementRejectsMissingElement()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    const QJsonObject params{{"element_id", 999999}};
    QString errorMsg;
    QVERIFY(!handler.validatedElement(params, "element_id", errorMsg));
    QVERIFY2(errorMsg.contains("Element not found"), qPrintable(errorMsg));
}

void TestQuickBaseHandler::testValidatedElementRejectsNonGraphicElementItem()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    auto *canvas = controller.currentTab()->canvas();
    // Real (heap-allocated), canvas-owned elements, not unique_ptr-managed ones: CanvasItem's
    // destructor unconditionally qDeleteAll()s everything still registered in it, so a
    // unique_ptr declared after the canvas (as `controller` here) would double-free on scope
    // exit -- same class of bug already found and fixed in TestConnectionSerialization/
    // TestICInline.
    auto *sw1 = new And();
    auto *sw2 = new And();
    canvas->addItem(sw1);
    canvas->addItem(sw2);

    auto *connection = new Connection();
    connection->setStartPort(sw1->outputPort());
    connection->setEndPort(sw2->inputPort(0));
    canvas->addItem(connection);

    const QJsonObject params{{"element_id", connection->id()}};
    QString errorMsg;
    QVERIFY(!handler.validatedElement(params, "element_id", errorMsg));
    QVERIFY2(errorMsg.contains("is not a graphic element"), qPrintable(errorMsg));
}

void TestQuickBaseHandler::testValidatedElementAcceptsRealElement()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    auto *canvas = controller.currentTab()->canvas();
    auto *andGate = new And();
    canvas->addItem(andGate);

    const QJsonObject params{{"element_id", andGate->id()}};
    QString errorMsg;
    GraphicElement *found = handler.validatedElement(params, "element_id", errorMsg);
    QCOMPARE(found, andGate);
}

void TestQuickBaseHandler::testInputPortByLabelRejectsNullElement()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    int portIndex = -1;
    QString errorMsg;
    QVERIFY(!handler.inputPortByLabel(nullptr, "A", portIndex, errorMsg));
    QCOMPARE(errorMsg, QStringLiteral("Element is null"));
}

void TestQuickBaseHandler::testInputPortByLabelRejectsNotFound()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    And andGate;
    int portIndex = -1;
    QString errorMsg;
    QVERIFY(!handler.inputPortByLabel(&andGate, "NoSuchLabel", portIndex, errorMsg));
    QVERIFY2(errorMsg.contains("not found"), qPrintable(errorMsg));
}

void TestQuickBaseHandler::testInputPortByLabelFindsRealPort()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    And andGate;
    andGate.inputPort(1)->setName("CLK");
    int portIndex = -1;
    QString errorMsg;
    QVERIFY(handler.inputPortByLabel(&andGate, "CLK", portIndex, errorMsg));
    QCOMPARE(portIndex, 1);
}

void TestQuickBaseHandler::testOutputPortByLabelRejectsNullElement()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    int portIndex = -1;
    QString errorMsg;
    QVERIFY(!handler.outputPortByLabel(nullptr, "Q", portIndex, errorMsg));
    QCOMPARE(errorMsg, QStringLiteral("Element is null"));
}

void TestQuickBaseHandler::testOutputPortByLabelRejectsNotFound()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    And andGate;
    int portIndex = -1;
    QString errorMsg;
    QVERIFY(!handler.outputPortByLabel(&andGate, "NoSuchLabel", portIndex, errorMsg));
    QVERIFY2(errorMsg.contains("not found"), qPrintable(errorMsg));
}

void TestQuickBaseHandler::testOutputPortByLabelFindsRealPort()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    And andGate;
    andGate.outputPort(0)->setName("Q");
    int portIndex = -1;
    QString errorMsg;
    QVERIFY(handler.outputPortByLabel(&andGate, "Q", portIndex, errorMsg));
    QCOMPARE(portIndex, 0);
}

void TestQuickBaseHandler::testAvailableInputPortsHandlesNullZeroAndNamed()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QCOMPARE(handler.availableInputPorts(nullptr), QStringLiteral("(element is null)"));

    InputSwitch inputSwitch; // 0 input ports (source element)
    QCOMPARE(handler.availableInputPorts(&inputSwitch), QStringLiteral("(no input ports)"));

    And andGate; // 2 unnamed input ports by default
    QVERIFY2(handler.availableInputPorts(&andGate).contains("(unnamed)"), qPrintable(handler.availableInputPorts(&andGate)));

    andGate.inputPort(0)->setName("A");
    QVERIFY2(handler.availableInputPorts(&andGate).contains("] A"), qPrintable(handler.availableInputPorts(&andGate)));
}

void TestQuickBaseHandler::testAvailableOutputPortsHandlesNullZeroAndNamed()
{
    QuickAppController controller;
    controller.newTab();
    StubHandler handler(&controller, nullptr);

    QCOMPARE(handler.availableOutputPorts(nullptr), QStringLiteral("(element is null)"));

    Led led; // 0 output ports (display-only element)
    QCOMPARE(handler.availableOutputPorts(&led), QStringLiteral("(no output ports)"));

    And andGate;
    QVERIFY2(handler.availableOutputPorts(&andGate).contains("(unnamed)"), qPrintable(handler.availableOutputPorts(&andGate)));

    andGate.outputPort(0)->setName("Q");
    QVERIFY2(handler.availableOutputPorts(&andGate).contains("] Q"), qPrintable(handler.availableOutputPorts(&andGate)));
}
