// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestArduino : public QObject
{
    Q_OBJECT

private slots:
    void testMappedPin();
    void testCodeGeneratorConstruction();
    void testGenerateEmptyCircuit();
    void testFileHandling();
    void testGenerateBasicElements();
    void testGenerateLogicElements();
    void testGenerateVccGndElements();
    void testGenerateClockElements();
    void testGenerateSequentialElements();
    void testGenerateElementsWithLabels();
    void testGenerateComplexCircuit();
    void testPortConnectionsAndMapping();
    void testMultiOutputElements();
    void testEdgeCasesAndErrorHandling();
    void testICElementSupport();
    void testForbiddenCharacterRemoval();
};