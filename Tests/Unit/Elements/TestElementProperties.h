// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestElementProperties : public QObject
{
    Q_OBJECT

private slots:
    // Input Size Tests (4 tests)
    void testInputSizeVariableElement();
    void testInputSizeConstraints();
    void testInputSizeFixed();
    void testInputSizePortCreation();

    // Output Size Tests (2 tests)
    void testOutputSizeVariableElement();
    void testOutputSizeFixed();

    // Label Tests (2 tests)
    void testLabelProperty();
    void testLabelPersistence();

    // Color Tests (2 tests)
    void testColorProperty();
    void testColorCycling();

    // Rotation Tests (2 tests)
    void testRotationProperty();
    void testRotationNormalization();

    // Clock Properties (2 tests)
    void testFrequencyProperty();
    void testFrequencyValidation();

    // Serialization (1 test)
    void testPropertiesSaveLoad();

    // Port Access Tests (8 tests)
    void testInputPortByIndexValid();
    void testInputPortByIndexInvalid();
    void testOutputPortByIndexValid();
    void testOutputPortByIndexInvalid();
    void testInputsVectorReference();
    void testOutputsVectorReference();
    void testPortCountAfterSizeChange();
    void testPortListIntegrity();
};

