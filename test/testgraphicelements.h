// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QVector>

#include "enums.h"

class IC;
class InputSwitch;
class QNEConnection;

class TestGraphicElements : public QObject
{
    Q_OBJECT

private slots:
    void cleanup();
    void init();

    // Basic gate structure tests
    void testBasicGateStructure();
    void testInputOutputElements();
    void testSequentialElementStructure();
    void testComplexElementStructure();

    // Element-specific structure tests
    void testAndGateStructure();
    void testOrGateStructure();
    void testMuxDemuxStructure();
    void testFlipFlopStructure();
    void testLatchStructure();
    void testNodeStructure();
    void testVccGndStructure();

    // IC and complex element tests
    void testICStructure();
    void testICData();
    void testMultipleICs();

    // Display and audio element tests
    void testDisplayElements();
    void testAudioElements();

    // Annotation element tests
    void testAnnotationElements();

    // Boundary condition tests
    void testElementBoundaryConditions();

    // Negative test cases
    void testErrorConditions();

private:
    void testICData(IC *ic);
    void testElementBasicStructure(class GraphicElement *element,
                                 int expectedInputs, int expectedOutputs,
                                 int expectedMinInputs, ElementType expectedType);

    QVector<InputSwitch *> switches{5};
    QVector<QNEConnection *> connections{5};
};
