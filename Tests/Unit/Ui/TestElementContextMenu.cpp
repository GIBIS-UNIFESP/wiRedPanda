// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestElementContextMenu.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Scene/Workspace.h"
#include "App/UI/SelectionCapabilities.h"
#include "Tests/Common/TestUtils.h"

void TestElementContextMenu::testRotateRightAction()
{
    // ElementContextMenu is a namespace function — test related capabilities
    WorkSpace workspace;
    And andGate;
    workspace.scene()->addItem(&andGate);
    andGate.setSelected(true);

    SelectionCapabilities caps = computeCapabilities({&andGate});
    QVERIFY(caps.hasElements);
}

void TestElementContextMenu::testMorphMenuAction()
{
    WorkSpace workspace;
    And andGate;
    workspace.scene()->addItem(&andGate);

    SelectionCapabilities caps = computeCapabilities({&andGate});
    QVERIFY(caps.canMorph);
}

void TestElementContextMenu::testICSubcircuitAction()
{
    SelectionCapabilities caps = computeCapabilities({});
    QVERIFY(!caps.hasElements);
}

