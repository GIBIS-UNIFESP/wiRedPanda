// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestGraphicelementAdvanced : public QObject
{
    Q_OBJECT

private slots:
    // Port management tests
    void testSetAndGetInputs();
    void testSetAndGetOutputs();
    void testInputPortByIndex();
    void testOutputPortByIndex();
    void testInputPortNegativeIndex();
    void testOutputPortNegativeIndex();

    // Port size constraints tests
    void testMinInputSizeConstraint();
    void testMaxInputSizeConstraint();
    void testMinOutputSizeConstraint();
    void testMaxOutputSizeConstraint();

    // Pixmap and visual properties tests
    void testPixmapCenterCalculation();
    void testBoundingRectCalculation();
    void testPortsBoundingRectCalculation();

    // Element type and group tests
    void testElementTypeRetrieval();
    void testElementGroupRetrieval();
    void testElementCreationWithDifferentTypes();

    // Port naming and properties tests
    void testPortNaming();
    void testAddInputPort();
    void testAddOutputPort();
    void testAddMultiplePorts();

    // Label and tooltip tests
    void testLabelProperty();
    void testTooltipProperty();
    void testLabelVisibility();

    // Rotation and position tests
    void testElementRotation();
    void testElementPosition();
    void testElementPositionPersistence();
};

