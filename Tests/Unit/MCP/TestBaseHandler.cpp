// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/MCP/TestBaseHandler.h"

#include <limits>
#include <memory>

#include <QJsonObject>
#include <QTabWidget>
#include <QTest>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MainWindow.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "MCP/Server/Handlers/BaseHandler.h"

namespace {

class StubHandler : public BaseHandler
{
public:
    using BaseHandler::BaseHandler;
    QJsonObject handleCommand(const QString &, const QJsonObject &, const QJsonValue &) override { return {}; }
};

} // namespace

void TestBaseHandler::testCreateSuccessResponseIncludesResultAndId()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    const QJsonObject result{{"value", 42}};
    const QJsonObject response = handler.createSuccessResponse(result, 7);

    QCOMPARE(response.value("jsonrpc").toString(), QStringLiteral("2.0"));
    QCOMPARE(response.value("result").toObject().value("value").toInt(), 42);
    QCOMPARE(response.value("id").toInt(), 7);
}

void TestBaseHandler::testCreateSuccessResponseOmitsIdWhenNull()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    const QJsonObject response = handler.createSuccessResponse();
    QVERIFY(!response.contains("id"));
}

void TestBaseHandler::testCreateErrorResponseIncludesCodeAndMessage()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    const QJsonObject response = handler.createErrorResponse("boom", 3, 42);
    QCOMPARE(response.value("jsonrpc").toString(), QStringLiteral("2.0"));
    QCOMPARE(response.value("error").toObject().value("code").toInt(), 42);
    QCOMPARE(response.value("error").toObject().value("message").toString(), QStringLiteral("boom"));
    QCOMPARE(response.value("id").toInt(), 3);
}

void TestBaseHandler::testCreateErrorResponseOmitsIdWhenNull()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    const QJsonObject response = handler.createErrorResponse("boom");
    QVERIFY(!response.contains("id"));
}

void TestBaseHandler::testValidateParametersAcceptsWhenAllPresent()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    const QJsonObject params{{"a", 1}, {"b", 2}};
    QVERIFY(handler.validateParameters(params, {"a", "b"}));
}

void TestBaseHandler::testValidateParametersRejectsWhenMissing()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    const QJsonObject params{{"a", 1}};
    QVERIFY(!handler.validateParameters(params, {"a", "b"}));
}

void TestBaseHandler::testCurrentSceneNullWithNoMainWindow()
{
    StubHandler handler(nullptr, nullptr);
    QVERIFY(!handler.currentScene());
    QVERIFY(!std::as_const(handler).currentScene());
}

void TestBaseHandler::testCurrentSceneNullWithNoCurrentTab()
{
    MainWindow window;
    auto *tabs = window.findChild<QTabWidget *>();
    QVERIFY(tabs);
    emit tabs->tabCloseRequested(0);
    QVERIFY(!window.currentTab());

    StubHandler handler(&window, nullptr);
    QVERIFY(!handler.currentScene());
}

void TestBaseHandler::testCurrentSceneReturnsRealScene()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QCOMPARE(handler.currentScene(), window.currentTab()->scene());
}

void TestBaseHandler::testValidatePositiveIntegerRejectsNonNumber()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validatePositiveInteger(QJsonValue("not a number"), "x", errorMsg));
    QVERIFY2(errorMsg.contains("must be an integer"), qPrintable(errorMsg));
}

void TestBaseHandler::testValidatePositiveIntegerRejectsZeroAndNegative()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validatePositiveInteger(QJsonValue(0), "x", errorMsg));
    QVERIFY2(errorMsg.contains("positive integer"), qPrintable(errorMsg));

    QVERIFY(!handler.validatePositiveInteger(QJsonValue(-5), "x", errorMsg));
}

void TestBaseHandler::testValidatePositiveIntegerAcceptsPositive()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QString errorMsg;
    QVERIFY(handler.validatePositiveInteger(QJsonValue(3), "x", errorMsg));
}

void TestBaseHandler::testValidateNonNegativeIntegerRejectsNonNumber()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateNonNegativeInteger(QJsonValue("nope"), "x", errorMsg));
    QVERIFY2(errorMsg.contains("must be an integer"), qPrintable(errorMsg));
}

void TestBaseHandler::testValidateNonNegativeIntegerRejectsNegative()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateNonNegativeInteger(QJsonValue(-1), "x", errorMsg));
    QVERIFY2(errorMsg.contains("non-negative"), qPrintable(errorMsg));
}

void TestBaseHandler::testValidateNonNegativeIntegerAcceptsZeroAndPositive()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QString errorMsg;
    QVERIFY(handler.validateNonNegativeInteger(QJsonValue(0), "x", errorMsg));
    QVERIFY(handler.validateNonNegativeInteger(QJsonValue(5), "x", errorMsg));
}

void TestBaseHandler::testValidateNonEmptyStringRejectsNonString()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateNonEmptyString(QJsonValue(5), "x", errorMsg));
    QVERIFY2(errorMsg.contains("must be a string"), qPrintable(errorMsg));
}

void TestBaseHandler::testValidateNonEmptyStringRejectsEmpty()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateNonEmptyString(QJsonValue(""), "x", errorMsg));
    QVERIFY2(errorMsg.contains("cannot be empty"), qPrintable(errorMsg));
}

void TestBaseHandler::testValidateNonEmptyStringAcceptsNonEmpty()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QString errorMsg;
    QVERIFY(handler.validateNonEmptyString(QJsonValue("hi"), "x", errorMsg));
}

void TestBaseHandler::testValidateElementIdRejectsNonPositive()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateElementId(0, "element_id", errorMsg));
    QVERIFY2(errorMsg.contains("positive integer"), qPrintable(errorMsg));
}

void TestBaseHandler::testValidateElementIdRejectsNoScene()
{
    MainWindow window;
    auto *tabs = window.findChild<QTabWidget *>();
    QVERIFY(tabs);
    emit tabs->tabCloseRequested(0);
    QVERIFY(!window.currentTab());

    StubHandler handler(&window, nullptr);
    QString errorMsg;
    QVERIFY(!handler.validateElementId(1, "element_id", errorMsg));
    QCOMPARE(errorMsg, QStringLiteral("No active circuit scene available"));
}

void TestBaseHandler::testValidateElementIdRejectsNotFound()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateElementId(999999, "element_id", errorMsg));
    QVERIFY2(errorMsg.contains("Element not found"), qPrintable(errorMsg));
}

void TestBaseHandler::testValidateElementIdAcceptsReal()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    auto *scene = window.currentTab()->scene();
    auto *andGate = new And();
    scene->addItem(andGate);

    QString errorMsg;
    QVERIFY(handler.validateElementId(andGate->id(), "element_id", errorMsg));
}

void TestBaseHandler::testValidateNumericRejectsNonNumber()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateNumeric(QJsonValue("x"), "x", errorMsg));
    QVERIFY2(errorMsg.contains("numeric value"), qPrintable(errorMsg));
}

void TestBaseHandler::testValidateNumericRejectsNanAndInf()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validateNumeric(QJsonValue(std::numeric_limits<double>::quiet_NaN()), "x", errorMsg));
    QVERIFY2(errorMsg.contains("finite"), qPrintable(errorMsg));

    QVERIFY(!handler.validateNumeric(QJsonValue(std::numeric_limits<double>::infinity()), "x", errorMsg));
}

void TestBaseHandler::testValidateNumericAcceptsFinite()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QString errorMsg;
    QVERIFY(handler.validateNumeric(QJsonValue(3.5), "x", errorMsg));
}

void TestBaseHandler::testValidatePortRangeRejectsNullElement()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QString errorMsg;
    QVERIFY(!handler.validatePortRange(nullptr, 0, false, "port", errorMsg));
    QVERIFY2(errorMsg.contains("Invalid element"), qPrintable(errorMsg));
}

void TestBaseHandler::testValidatePortRangeRejectsOutOfRange()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    And andGate; // 2 inputs, 1 output by default
    QString errorMsg;
    QVERIFY(!handler.validatePortRange(&andGate, static_cast<int>(andGate.inputs().size()), false, "port", errorMsg));
    QVERIFY2(errorMsg.contains("out of range"), qPrintable(errorMsg));

    QVERIFY(!handler.validatePortRange(&andGate, static_cast<int>(andGate.outputs().size()), true, "port", errorMsg));
}

void TestBaseHandler::testValidatePortRangeAcceptsInRange()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    And andGate;
    QString errorMsg;
    QVERIFY(handler.validatePortRange(&andGate, 0, false, "port", errorMsg));
    QVERIFY(handler.validatePortRange(&andGate, 0, true, "port", errorMsg));
}

void TestBaseHandler::testValidatedElementRejectsNonPositiveParam()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    const QJsonObject params{{"element_id", 0}};
    QString errorMsg;
    QVERIFY(!handler.validatedElement(params, "element_id", errorMsg));
    QVERIFY2(!errorMsg.isEmpty(), qPrintable(errorMsg));
}

void TestBaseHandler::testValidatedElementRejectsMissingElement()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    const QJsonObject params{{"element_id", 999999}};
    QString errorMsg;
    QVERIFY(!handler.validatedElement(params, "element_id", errorMsg));
    QVERIFY2(errorMsg.contains("Element not found"), qPrintable(errorMsg));
}

void TestBaseHandler::testValidatedElementRejectsNonGraphicElementItem()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    auto *scene = window.currentTab()->scene();
    auto sw1 = std::make_unique<And>();
    auto sw2 = std::make_unique<And>();
    scene->addItem(sw1.get());
    scene->addItem(sw2.get());

    auto connection = std::make_unique<Connection>();
    connection->setStartPort(sw1->outputPort());
    connection->setEndPort(sw2->inputPort(0));
    scene->addItem(connection.get());

    const QJsonObject params{{"element_id", connection->id()}};
    QString errorMsg;
    QVERIFY(!handler.validatedElement(params, "element_id", errorMsg));
    QVERIFY2(errorMsg.contains("is not a graphic element"), qPrintable(errorMsg));
}

void TestBaseHandler::testValidatedElementAcceptsRealElement()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    auto *scene = window.currentTab()->scene();
    auto *andGate = new And();
    scene->addItem(andGate);

    const QJsonObject params{{"element_id", andGate->id()}};
    QString errorMsg;
    GraphicElement *found = handler.validatedElement(params, "element_id", errorMsg);
    QCOMPARE(found, andGate);
}

void TestBaseHandler::testInputPortByLabelRejectsNullElement()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    int portIndex = -1;
    QString errorMsg;
    QVERIFY(!handler.inputPortByLabel(nullptr, "A", portIndex, errorMsg));
    QCOMPARE(errorMsg, QStringLiteral("Element is null"));
}

void TestBaseHandler::testInputPortByLabelRejectsNotFound()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    And andGate;
    int portIndex = -1;
    QString errorMsg;
    QVERIFY(!handler.inputPortByLabel(&andGate, "NoSuchLabel", portIndex, errorMsg));
    QVERIFY2(errorMsg.contains("not found"), qPrintable(errorMsg));
}

void TestBaseHandler::testInputPortByLabelFindsRealPort()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    And andGate;
    andGate.inputPort(1)->setName("CLK");
    int portIndex = -1;
    QString errorMsg;
    QVERIFY(handler.inputPortByLabel(&andGate, "CLK", portIndex, errorMsg));
    QCOMPARE(portIndex, 1);
}

void TestBaseHandler::testOutputPortByLabelRejectsNullElement()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    int portIndex = -1;
    QString errorMsg;
    QVERIFY(!handler.outputPortByLabel(nullptr, "Q", portIndex, errorMsg));
    QCOMPARE(errorMsg, QStringLiteral("Element is null"));
}

void TestBaseHandler::testOutputPortByLabelRejectsNotFound()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    And andGate;
    int portIndex = -1;
    QString errorMsg;
    QVERIFY(!handler.outputPortByLabel(&andGate, "NoSuchLabel", portIndex, errorMsg));
    QVERIFY2(errorMsg.contains("not found"), qPrintable(errorMsg));
}

void TestBaseHandler::testOutputPortByLabelFindsRealPort()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    And andGate;
    andGate.outputPort(0)->setName("Q");
    int portIndex = -1;
    QString errorMsg;
    QVERIFY(handler.outputPortByLabel(&andGate, "Q", portIndex, errorMsg));
    QCOMPARE(portIndex, 0);
}

void TestBaseHandler::testAvailableInputPortsHandlesNullZeroAndNamed()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QCOMPARE(handler.availableInputPorts(nullptr), QStringLiteral("(element is null)"));

    InputSwitch inputSwitch; // 0 input ports (source element)
    QCOMPARE(handler.availableInputPorts(&inputSwitch), QStringLiteral("(no input ports)"));

    And andGate; // 2 unnamed input ports by default
    QVERIFY2(handler.availableInputPorts(&andGate).contains("(unnamed)"), qPrintable(handler.availableInputPorts(&andGate)));

    andGate.inputPort(0)->setName("A");
    QVERIFY2(handler.availableInputPorts(&andGate).contains("] A"), qPrintable(handler.availableInputPorts(&andGate)));
}

void TestBaseHandler::testAvailableOutputPortsHandlesNullZeroAndNamed()
{
    MainWindow window;
    StubHandler handler(&window, nullptr);

    QCOMPARE(handler.availableOutputPorts(nullptr), QStringLiteral("(element is null)"));

    Led led; // 0 output ports (display-only element)
    QCOMPARE(handler.availableOutputPorts(&led), QStringLiteral("(no output ports)"));

    And andGate;
    QVERIFY2(handler.availableOutputPorts(&andGate).contains("(unnamed)"), qPrintable(handler.availableOutputPorts(&andGate)));

    andGate.outputPort(0)->setName("Q");
    QVERIFY2(handler.availableOutputPorts(&andGate).contains("] Q"), qPrintable(handler.availableOutputPorts(&andGate)));
}
