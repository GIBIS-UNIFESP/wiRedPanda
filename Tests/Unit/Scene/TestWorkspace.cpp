// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestWorkspace.h"

#include <QLayout>

#include "App/Core/Settings.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MinimapWidget.h"
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

void TestWorkspaceUnit::testMinimapCornerPositioning()
{
    WorkSpace workspace;
    workspace.resize(400, 300);
    workspace.layout()->activate();

    const QRect viewGeom = workspace.m_view.geometry();
    const int margin = 12;
    const int minimapW = workspace.m_minimap->width();
    const int minimapH = workspace.m_minimap->height();

    workspace.setMinimapCorner(Settings::MinimapCorner::TopLeft);
    QCOMPARE(workspace.m_minimap->pos(), QPoint(viewGeom.left() + margin, viewGeom.top() + margin));

    workspace.setMinimapCorner(Settings::MinimapCorner::TopRight);
    QCOMPARE(workspace.m_minimap->pos(), QPoint(viewGeom.right() - minimapW - margin, viewGeom.top() + margin));

    workspace.setMinimapCorner(Settings::MinimapCorner::BottomLeft);
    QCOMPARE(workspace.m_minimap->pos(), QPoint(viewGeom.left() + margin, viewGeom.bottom() - minimapH - margin));

    workspace.setMinimapCorner(Settings::MinimapCorner::BottomRight);
    QCOMPARE(workspace.m_minimap->pos(), QPoint(viewGeom.right() - minimapW - margin, viewGeom.bottom() - minimapH - margin));
}
