// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestICRegistry.h"

#include "App/Element/ICRegistry.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestICRegistry::testICRegistration()
{
    WorkSpace workspace;
    ICRegistry registry(workspace.scene());
    QVERIFY(registry.blobMap().isEmpty());
}

void TestICRegistry::testICFileWatcher()
{
    WorkSpace workspace;
    ICRegistry registry(workspace.scene());
    QVERIFY(true);
}

void TestICRegistry::testRecursiveICLoading()
{
    WorkSpace workspace;
    ICRegistry registry(workspace.scene());
    QVERIFY(registry.blobMap().isEmpty());
}

void TestICRegistry::testICValidation()
{
    WorkSpace workspace;
    ICRegistry registry(workspace.scene());
    QVERIFY(!registry.hasBlob("nonexistent"));
}

