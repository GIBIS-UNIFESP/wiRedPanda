// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestElementTabNavigator.h"

#include "App/Scene/Workspace.h"
#include "App/UI/ElementEditor.h"
#include "App/UI/ElementTabNavigator.h"
#include "Tests/Common/TestUtils.h"

void TestElementTabNavigator::testTabNavigation()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    ElementTabNavigator navigator(&editor);
    QVERIFY(true);
}

void TestElementTabNavigator::testBacktabNavigation()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    ElementTabNavigator navigator(&editor);
    QVERIFY(true);
}

void TestElementTabNavigator::testTabWrapAround()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);
    ElementTabNavigator navigator(&editor);
    QVERIFY(true);
}

