// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// GraphicElement has no QGraphicsItem inheritance, so QGraphicsScene-dependent behavior
/// (mapToScene(), transformOriginPoint(), setSelected(), paint(QPainter*,
/// QStyleOptionGraphicsItem*, QWidget*), mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)) is
/// out of scope here; testRotationKeepsSizingPixmap and testDisconnectedInputsAreUnknown don't
/// depend on any of that.
class TestTruthTable : public QObject
{
    Q_OBJECT

private slots:
    // Constructor and configuration tests
    void testConstructorInitialization();
    void testInputPortNames();
    void testOutputPortNames();
    void testPortConfiguration();

    // Key management tests
    void testKeyInitialization();
    void testSetKey();
    void testSetKeyEnforcesSizeInvariant();
    void testKeyAccess();
    void testKeyBitArray();

    // Port properties tests
    void testUpdatePortsProperties();
    void testInputPortPositioning();
    void testOutputPortPositioning();

    // Serialization tests
    void testSaveKey();
    void testLoadKeyVersion42();
    void testLoadKeyOldVersion();

    // Rendering under rotation
    void testRotationKeepsSizingPixmap();

    // Interaction / simulation edge cases
    void testDisconnectedInputsAreUnknown();
};
