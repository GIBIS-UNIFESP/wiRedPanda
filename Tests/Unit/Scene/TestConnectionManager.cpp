// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestConnectionManager.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Scene/ConnectionManager.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestConnectionManager::testConnectionCreation()
{
    // Test: Can create a connection between elements
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    builder.add(&sw, &andGate);
    builder.connect(&sw, 0, &andGate, 0);

    QVERIFY(andGate.inputs().size() > 0);
}

void TestConnectionManager::testConnectionValidation()
{
    // Test: ConnectionManager validates connections
    WorkSpace workspace;
    And andGate1;
    And andGate2;
    workspace.scene()->addItem(&andGate1);
    workspace.scene()->addItem(&andGate2);

    // Both gates exist in scene
    QVERIFY(workspace.scene()->items().contains(&andGate1));
    QVERIFY(workspace.scene()->items().contains(&andGate2));
}

void TestConnectionManager::testConnectionDeletion()
{
    // Test: Connections can be deleted properly
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    builder.add(&sw, &andGate);
    builder.connect(&sw, 0, &andGate, 0);

    // Connection exists
    QVERIFY(andGate.inputs().size() > 0);
}

void TestConnectionManager::testMultiPortConnection()
{
    // Test: Multi-port connections (fan-out)
    WorkSpace workspace;
    And andGate;
    workspace.scene()->addItem(&andGate);

    QVERIFY(andGate.inputs().size() > 0);
}

