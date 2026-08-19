// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestElementContextMenu : public QObject
{
    Q_OBJECT

private slots:

    // Context menu action tests (3 tests)
    void testRotateRightAction();
    void testMorphMenuAction();
    void testICSubcircuitAction();

    // "Connect ports" bulk-connect helper tests -- by port order
    void testPairPortsByOrderPairsPortsInOrder();
    void testPairPortsByOrderSkipsOccupiedInputs();
    void testConnectPortsByOrderUndoRemovesAllAsOneStep();

    // "Connect ports" bulk-connect helper tests -- by matching name
    void testPairPortsByNameMatchesExactCaseInsensitiveIgnoringOrder();
    void testPairPortsByNameFirstMatchingOutputWinsOnDuplicateNames();
    void testPairPortsByNameNoMatchesReturnsEmpty();
};
