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

void TestWorkspaceUnit::testMinimapDefaultPositionWithoutPersistedGeometry()
{
    Settings::setMinimapGeometry(QRect()); // ensure a clean slate regardless of test order

    WorkSpace workspace;
    workspace.resize(400, 300);
    workspace.layout()->activate();
    // resizeEvent() isn't reliably delivered to a never-shown top-level widget in a headless
    // test run (layout activation still sizes m_view correctly via the layout engine, just
    // not through the event path) -- call the geometry logic directly, same as the app's
    // own resizeEvent() does.
    workspace.applyMinimapGeometry();

    const QRect viewGeom = workspace.m_view.geometry();
    const int margin = 12;
    const int minimapW = workspace.m_minimap->width();
    const int minimapH = workspace.m_minimap->height();

    // No persisted geometry: falls back to the widget's own default size, bottom-right.
    QCOMPARE(workspace.m_minimap->pos(), QPoint(viewGeom.right() - minimapW - margin, viewGeom.bottom() - minimapH - margin));
}

void TestWorkspaceUnit::testMinimapRestoresPersistedGeometry()
{
    const QRect persisted(30, 40, 200, 150);
    Settings::setMinimapGeometry(persisted);

    WorkSpace workspace;
    workspace.resize(400, 300);
    workspace.layout()->activate();
    workspace.applyMinimapGeometry();

    QCOMPARE(workspace.m_minimap->geometry(), persisted);
}

void TestWorkspaceUnit::testMinimapReclampsOnSubsequentResize()
{
    Settings::setMinimapGeometry(QRect(30, 40, 200, 150));

    WorkSpace workspace;
    workspace.resize(400, 300);
    workspace.layout()->activate();
    workspace.applyMinimapGeometry();
    QCOMPARE(workspace.m_minimap->geometry(), QRect(30, 40, 200, 150));

    // Change the persisted value mid-session: a later resize must re-clamp the minimap's
    // own current geometry, not re-read (and jump to) this new Settings value -- Settings
    // is only ever written *from* a user-driven move/resize, never read again after the
    // first layout.
    Settings::setMinimapGeometry(QRect(0, 0, 50, 50));

    // Shrunk drastically -- comfortably above the minimap's own 160x120 minimum (so this
    // exercises re-clamping the position, not the Qt-enforced size floor), but well below
    // where (30,40,200,150) still fits.
    workspace.resize(200, 180);
    workspace.layout()->activate();
    workspace.applyMinimapGeometry();

    const QRect viewGeom = workspace.m_view.geometry();
    const int margin = 12;
    const QRect result = workspace.m_minimap->geometry();

    QVERIFY2(result != QRect(0, 0, 50, 50), "must not have jumped to the changed Settings value");
    QVERIFY2(result.x() >= margin && result.x() + result.width() <= viewGeom.width() - margin,
             "x must be re-clamped within the shrunk view");
    QVERIFY2(result.y() >= margin && result.y() + result.height() <= viewGeom.height() - margin,
             "y must be re-clamped within the shrunk view");
}

void TestWorkspaceUnit::testMinimapRestoreClampsOversizedGeometry()
{
    // Persisted from a much larger window/monitor than the one being restored into.
    Settings::setMinimapGeometry(QRect(10, 10, 900, 700));

    WorkSpace workspace;
    workspace.resize(300, 200);
    workspace.layout()->activate();
    workspace.applyMinimapGeometry();

    const QRect viewGeom = workspace.m_view.geometry();
    const int margin = 12;
    QVERIFY2(workspace.m_minimap->width() <= viewGeom.width() - 2 * margin,
             "restored size must be clamped to fit the current view, not just repositioned");
    QVERIFY2(workspace.m_minimap->height() <= viewGeom.height() - 2 * margin,
             "restored size must be clamped to fit the current view, not just repositioned");
    QVERIFY(workspace.m_minimap->x() >= margin);
    QVERIFY(workspace.m_minimap->y() >= margin);
}
