// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Phase 7a: proves the test_wiredpanda_quick harness itself works end to end (whole-archived
/// wiredpanda_lib, offscreen QPA, real SVG icon resources) before any behavioral parity tests
/// are ported onto it. See the qtquick-rewrite plan's Phase 7 section.
class TestCanvasItemSmoke : public QObject
{
    Q_OBJECT

private slots:
    void testConstructWithoutDemoStartsEmpty();
    void testAddElementViaCommandUpdatesElements();
    void testAddElementUndoRemovesIt();

    // --- renderMinimapImage() cache-skip behavior (see project memory
    // project_quick_clocked_8000_profile_finding.md for the real-world cost this caches away) ---
    void testMinimapZoomInWithinBoundsSkipsRebuild();
    void testMinimapZoomOutPastBoundsForcesRebuild();
    void testMinimapStructuralEditForcesRebuild();
};
