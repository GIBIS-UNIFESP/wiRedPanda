// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestPort : public QObject
{
    Q_OBJECT

private slots:

    void testPortTooltip();
    void testPortHoverState();

    // Regression: F18 — setInputs must keep index() == vector position
    void testSetInputsReindexesPorts();

    void testPortCurrentPenTracksStatusColor();

    void testNameGetterSetter();
    void testDefaultValueGetterSetter();
    void testIsRequiredGetterSetter();
    void testGlobalIndexForInputVsOutputPort();
    void testMakeSerialIdPacksElementBaseAndGlobalIndex();
    void testSetGraphicElementBindsAccessor();
    void testAttachConnectionIsIdempotent();
    void testDetachConnectionRemovesFromListAndClearsPortReference();
    void testIsConnectedTrueOnlyForPortsSharingAConnection();
    void testInputPortIsValidRequiredVsOptionalUnconnected();
    void testOutputPortIsValidAlwaysTrueRegardlessOfConnections();
    void testHoverEnterAndLeaveChangeBrush();

    // Push-based render-dirty tracking (replaces CanvasItem::updatePaintNode()'s
    // per-repaint portStatuses fingerprint rebuild)
    void testOutputPortSetStatusMarksElementRenderDirty();
    void testOutputPortSetStatusNoOpDoesNotMarkElementRenderDirty();
    void testInputPortSetStatusMarksElementRenderDirty();
    void testInputPortInvalidOverrideNoOpDoesNotMarkElementRenderDirty();
};
