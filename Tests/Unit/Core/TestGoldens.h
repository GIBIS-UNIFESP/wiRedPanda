// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/// Checks the helpers the characterization goldens share (Tests/Common/Goldens.h).
class TestGoldens : public QObject
{
    Q_OBJECT

private slots:
    void testTextGoldenMatchesOrNamesTheLine();
    void testByteGoldenMatchesOrNamesTheOffset();
    void testMissingGoldenFails();
    void testRegenerationWritesOverAnExistingGolden();
    void testRegenerationSwitchCountsWhenEmpty();
    void testFileRows_data();
    void testFileRows();
    void testLoadCircuit();
    void testPrinters();
    void testOrientations();
    void testElementsById();
    void testScopedRendering();
    void testPortCountsAndOrientationPasses();
    void testLeftOutTypes();
    void testPixmapIdentity();
};
