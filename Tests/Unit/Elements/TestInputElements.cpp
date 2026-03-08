// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestInputElements.h"

#include <memory>

#include <QDataStream>
#include <QTest>

#include "App/Element/GraphicElements/InputButton.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "Tests/Common/TestUtils.h"

// ============================================================================
// InputSwitch Tests
// ============================================================================

void TestInputElements::testInputSwitchConstructor()
{
    InputSwitch inputSwitch;

    // Verify element type
    QCOMPARE(inputSwitch.elementType(), ElementType::InputSwitch);

    // Verify element group is Input
    QCOMPARE(inputSwitch.elementGroup(), ElementGroup::Input);

    // Verify port configuration (0 inputs, 1 output)
    QCOMPARE(inputSwitch.inputSize(), 0);
    QCOMPARE(inputSwitch.outputSize(), 1);

    // Verify can change skin
    QVERIFY(inputSwitch.canChangeSkin());

    // Verify has label
    QVERIFY(inputSwitch.hasLabel());

    // Verify has trigger
    QVERIFY(inputSwitch.hasTrigger());

    // Verify cannot rotate
    QVERIFY(!inputSwitch.isRotatable());
}

void TestInputElements::testInputSwitchInitialState()
{
    InputSwitch inputSwitch;

    // Initial state should be OFF
    QVERIFY(!inputSwitch.isOn());

    // Output port should be Inactive initially
    QCOMPARE(inputSwitch.outputPort(0)->status(), Status::Inactive);
}

void TestInputElements::testInputSwitchSetOnToggle()
{
    InputSwitch inputSwitch;

    // Start: OFF
    QVERIFY(!inputSwitch.isOn());

    // setOn() toggles from OFF to ON
    inputSwitch.setOn();
    QVERIFY(inputSwitch.isOn());

    // setOn() toggles from ON to OFF
    inputSwitch.setOn();
    QVERIFY(!inputSwitch.isOn());

    // setOn() toggles back to ON
    inputSwitch.setOn();
    QVERIFY(inputSwitch.isOn());
}

void TestInputElements::testInputSwitchSetOffToggle()
{
    InputSwitch inputSwitch;

    // Start: OFF
    QVERIFY(!inputSwitch.isOn());

    // setOff() toggles from OFF to ON
    inputSwitch.setOff();
    QVERIFY(inputSwitch.isOn());

    // setOff() toggles from ON to OFF
    inputSwitch.setOff();
    QVERIFY(!inputSwitch.isOn());

    // setOff() toggles back to ON
    inputSwitch.setOff();
    QVERIFY(inputSwitch.isOn());
}

void TestInputElements::testInputSwitchIsOn()
{
    InputSwitch inputSwitch;

    // Port parameter is ignored
    QVERIFY(!inputSwitch.isOn(0));
    QVERIFY(!inputSwitch.isOn(1));
    QVERIFY(!inputSwitch.isOn(-1));

    // After setOn
    inputSwitch.setOn(true);
    QVERIFY(inputSwitch.isOn());
    QVERIFY(inputSwitch.isOn(0));
    QVERIFY(inputSwitch.isOn(99));  // Port parameter ignored
}

void TestInputElements::testInputSwitchPortStatus()
{
    InputSwitch inputSwitch;

    // Initial: Inactive
    QCOMPARE(inputSwitch.outputPort(0)->status(), Status::Inactive);

    // After setOn(true)
    inputSwitch.setOn(true);
    QCOMPARE(inputSwitch.outputPort(0)->status(), Status::Active);

    // After setOn(false)
    inputSwitch.setOn(false);
    QCOMPARE(inputSwitch.outputPort(0)->status(), Status::Inactive);
}

void TestInputElements::testInputSwitchSaveState()
{
    InputSwitch inputSwitch;
    inputSwitch.setOn(true);

    // Save to stream
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    inputSwitch.save(stream);

    // Verify data was written
    QVERIFY(data.size() > 0);
}

void TestInputElements::testInputSwitchLoadOldVersion()
{
    // Create and save InputSwitch with version 4.1+ format
    auto inputSwitch1 = std::unique_ptr<InputSwitch>(new InputSwitch());
    inputSwitch1->setOn(true);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    inputSwitch1->save(saveStream);
    inputSwitch1.reset();

    // Load with old version (< 4.1) - will try to read old format
    auto inputSwitch2 = std::unique_ptr<InputSwitch>(new InputSwitch());

    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> portMap;
    // For versions < 4.1, load reads isOn and potentially locked from stream
    // The test verifies the load mechanism doesn't crash and loads a valid element
    inputSwitch2->load(loadStream, portMap, QVersionNumber(3, 0));

    // Verify the element is in a valid state after loading
    QVERIFY2(inputSwitch2->outputSize() >= 1, "InputSwitch must have at least 1 output after load");
    QVERIFY2(inputSwitch2->elementType() == ElementType::InputSwitch, "Element type should be preserved");
}

void TestInputElements::testInputSwitchLoadNewVersion()
{
    // Create and save InputSwitch
    auto inputSwitch1 = std::unique_ptr<InputSwitch>(new InputSwitch());
    inputSwitch1->setOn(true);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    inputSwitch1->save(saveStream);
    inputSwitch1.reset();

    // Load with new version (>= 4.1)
    auto inputSwitch2 = std::unique_ptr<InputSwitch>(new InputSwitch());

    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> portMap;
    inputSwitch2->load(loadStream, portMap, QVersionNumber(4, 1));

    // State should be preserved
    QVERIFY(inputSwitch2->isOn());
}

void TestInputElements::testInputSwitchLockingMechanism()
{
    InputSwitch inputSwitch;

    // Locking is handled by parent class, we just verify state can be changed
    inputSwitch.setOn(false);
    QVERIFY(!inputSwitch.isOn());

    inputSwitch.setOn(true);
    QVERIFY(inputSwitch.isOn());

    // Verify locking doesn't affect setOn(bool) directly
    inputSwitch.setOn(false);
    QVERIFY(!inputSwitch.isOn());
}

void TestInputElements::testInputSwitchSkinChange()
{
    InputSwitch inputSwitch;

    // Switch is OFF initially
    inputSwitch.setOn(false);

    // Reset to default skin (should not crash)
    inputSwitch.setSkin(true, "");

    // Verify state is preserved
    QVERIFY(!inputSwitch.isOn());

    // Switch to ON
    inputSwitch.setOn(true);
    QVERIFY(inputSwitch.isOn());

    // Reset to default skin again
    inputSwitch.setSkin(true, "");

    // Verify state still ON
    QVERIFY(inputSwitch.isOn());
}

// ============================================================================
// InputButton Tests
// ============================================================================

void TestInputElements::testInputButtonConstructor()
{
    InputButton inputButton;

    // Verify element type
    QCOMPARE(inputButton.elementType(), ElementType::InputButton);

    // Verify element group is Input
    QCOMPARE(inputButton.elementGroup(), ElementGroup::Input);

    // Verify port configuration (0 inputs, 1 output)
    QCOMPARE(inputButton.inputSize(), 0);
    QCOMPARE(inputButton.outputSize(), 1);

    // Verify can change skin
    QVERIFY(inputButton.canChangeSkin());

    // Verify has label
    QVERIFY(inputButton.hasLabel());

    // Verify has trigger
    QVERIFY(inputButton.hasTrigger());

    // Verify cannot rotate
    QVERIFY(!inputButton.isRotatable());
}

void TestInputElements::testInputButtonInitialState()
{
    InputButton inputButton;

    // Initial state should be OFF (set in constructor)
    QVERIFY(!inputButton.isOn());

    // Output port should be Inactive initially
    QCOMPARE(inputButton.outputPort(0)->status(), Status::Inactive);
}

void TestInputElements::testInputButtonSetOn()
{
    InputButton inputButton;

    // Start: OFF
    QVERIFY(!inputButton.isOn());

    // setOn() sets to ON (no toggle)
    inputButton.setOn();
    QVERIFY(inputButton.isOn());

    // setOn() again - still ON (no toggle)
    inputButton.setOn();
    QVERIFY(inputButton.isOn());
}

void TestInputElements::testInputButtonSetOff()
{
    InputButton inputButton;

    // Set to ON first
    inputButton.setOn();
    QVERIFY(inputButton.isOn());

    // setOff() sets to OFF (no toggle)
    inputButton.setOff();
    QVERIFY(!inputButton.isOn());

    // setOff() again - still OFF (no toggle)
    inputButton.setOff();
    QVERIFY(!inputButton.isOn());
}

void TestInputElements::testInputButtonIsOn()
{
    InputButton inputButton;

    // Port parameter is ignored
    QVERIFY(!inputButton.isOn(0));
    QVERIFY(!inputButton.isOn(1));
    QVERIFY(!inputButton.isOn(-1));

    // After setOn
    inputButton.setOn();
    QVERIFY(inputButton.isOn());
    QVERIFY(inputButton.isOn(0));
    QVERIFY(inputButton.isOn(99));  // Port parameter ignored
}

void TestInputElements::testInputButtonPortStatus()
{
    InputButton inputButton;

    // Initial: Inactive
    QCOMPARE(inputButton.outputPort(0)->status(), Status::Inactive);

    // After setOn
    inputButton.setOn();
    QCOMPARE(inputButton.outputPort(0)->status(), Status::Active);

    // After setOff
    inputButton.setOff();
    QCOMPARE(inputButton.outputPort(0)->status(), Status::Inactive);
}

void TestInputElements::testInputButtonSaveState()
{
    InputButton inputButton;

    // Save to stream
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    inputButton.save(stream);

    // Verify data was written
    QVERIFY(data.size() > 0);
}

void TestInputElements::testInputButtonLoadOldVersion()
{
    // Create and save InputButton
    auto inputButton1 = std::unique_ptr<InputButton>(new InputButton());
    inputButton1->setOn();

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    inputButton1->save(saveStream);
    inputButton1.reset();

    // Load with old version (3.1 - 4.0)
    auto inputButton2 = std::unique_ptr<InputButton>(new InputButton());

    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> portMap;
    inputButton2->load(loadStream, portMap, QVersionNumber(3, 5));

    // Verify the element is in a valid state after loading
    QVERIFY2(inputButton2->outputSize() >= 1, "InputButton must have at least 1 output after load");
    QVERIFY2(inputButton2->elementType() == ElementType::InputButton, "Element type should be preserved");
}

void TestInputElements::testInputButtonLoadNewVersion()
{
    // Create and save InputButton
    auto inputButton1 = std::unique_ptr<InputButton>(new InputButton());
    inputButton1->setOn();

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    inputButton1->save(saveStream);
    inputButton1.reset();

    // Load with new version (>= 4.1)
    auto inputButton2 = std::unique_ptr<InputButton>(new InputButton());

    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> portMap;
    inputButton2->load(loadStream, portMap, QVersionNumber(4, 1));

    // Verify the element is in a valid state after loading
    QVERIFY2(inputButton2->outputSize() >= 1, "InputButton must have at least 1 output after load");
    QVERIFY2(inputButton2->elementType() == ElementType::InputButton, "Element type should be preserved");
}

void TestInputElements::testInputButtonLockingMechanism()
{
    InputButton inputButton;

    // Locking is handled by parent class, we just verify state can be changed
    inputButton.setOff();
    QVERIFY(!inputButton.isOn());

    inputButton.setOn();
    QVERIFY(inputButton.isOn());

    // Verify locking doesn't affect setOn/setOff directly
    inputButton.setOff();
    QVERIFY(!inputButton.isOn());

    inputButton.setOn();
    QVERIFY(inputButton.isOn());
}

void TestInputElements::testInputButtonSkinChange()
{
    InputButton inputButton;

    // Button is OFF initially
    inputButton.setOff();

    // Reset to default skin (should not crash)
    inputButton.setSkin(true, "");

    // Verify state is preserved
    QVERIFY(!inputButton.isOn());

    // Turn button ON
    inputButton.setOn();
    QVERIFY(inputButton.isOn());

    // Reset to default skin again
    inputButton.setSkin(true, "");

    // Verify state still ON
    QVERIFY(inputButton.isOn());
}

// ============================================================================
// Basic Input Element Tests (migrated from testelements.cpp)
// ============================================================================

void TestInputElements::testInputButton()
{
    InputButton elm;
    QCOMPARE(elm.inputSize(), 0);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.elementType(), ElementType::InputButton);

    // Test initial state (button starts off)
    QCOMPARE(elm.isOn(), false);

    // Test state changes with explicit value
    elm.setOn(true);
    QCOMPARE(elm.isOn(), true);
    QCOMPARE(elm.outputPort()->status(), Status::Active);

    elm.setOn(false);
    QCOMPARE(elm.isOn(), false);
    QCOMPARE(elm.outputPort()->status(), Status::Inactive);
}

void TestInputElements::testInputSwitch()
{
    InputSwitch elm;
    QCOMPARE(elm.inputSize(), 0);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.elementType(), ElementType::InputSwitch);

    // Test initial state
    QCOMPARE(elm.isOn(), false);

    // Test state changes with explicit value
    elm.setOn(true);
    QCOMPARE(elm.isOn(), true);
    QCOMPARE(elm.outputPort()->status(), Status::Active);

    elm.setOn(false);
    QCOMPARE(elm.isOn(), false);
    QCOMPARE(elm.outputPort()->status(), Status::Inactive);
}

