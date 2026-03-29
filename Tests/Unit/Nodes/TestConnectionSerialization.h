// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestConnectionSerialization : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // Port Reference Restoration Tests (5 tests)
    void testLoadWithEmptyPortMapDirectRestore();
    void testLoadWithPortMapIndirectRestore();
    void testLoadInvalidPortReferencesHandled();
    void testLoadPortTypeResolution();
    void testLoadMultipleConnectionsOnSamePorts();

    // Serialization Round-Trip Tests (3 tests)
    void testSaveLoadRoundTripPreservesPorts();
    void testSaveLoadPreservesConnectionStatus();
    void testSaveLoadWithStatusPropagation();

    // Orthogonal Wire Tests
    void testOrthogonalWireModeDefaultIsBezier();
    void testOrthogonalUpdatePathLineSegments();
    void testOrthogonalSaveLoadRoundTrip();
    void testOrthogonalSaveLoadEmptyWaypoints();
    void testBezierPathProducesCurves();
    void testOldFormatLoadsAsBezier();
    void testMixedModesSerializeCorrectly();
    void testWaypointsPreservedOnPortUpdate();
    void testClearWaypoints();
    void testBezierModeIgnoresWaypoints();
    void testInvalidWireModeDefaultsToBezier();
};

