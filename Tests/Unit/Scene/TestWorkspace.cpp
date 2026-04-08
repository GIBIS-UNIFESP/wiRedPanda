// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestWorkspace.h"

#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestWorkspaceUnit::testWorkspaceCreation()
{
    WorkSpace workspace;
    QVERIFY(workspace.scene() != nullptr);
}

void TestWorkspaceUnit::testTabManagement()
{
    WorkSpace workspace;
    QVERIFY(workspace.scene() != nullptr);
    QVERIFY(workspace.view() != nullptr);
}

void TestWorkspaceUnit::testModificationTracking()
{
    WorkSpace workspace;
    // An empty workspace should not be from a newer version
    QVERIFY(!workspace.isFromNewerVersion());
}

void TestWorkspaceUnit::testICEditing()
{
    WorkSpace workspace;
    QVERIFY(!workspace.isInlineIC());
    QVERIFY(workspace.parentWorkspace() == nullptr);
}

void TestWorkspaceUnit::testFullScreenHandling()
{
    WorkSpace workspace;
    QVERIFY(workspace.scene() != nullptr);
    QVERIFY(workspace.view() != nullptr);
    QVERIFY(workspace.simulation() != nullptr);
}

